// Fill out your copyright notice in the Description page of Project Settings.

#include "StudentPerceptorNeesAina.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

// these work bc we changed the dependencies in the .Build.cs file
#include "Zombies/BaseZombie.h"
#include "Items/BaseItem.h"
#include "Items/Medkit.h"
#include "Items/Weapon.h"
#include "Items/Food.h"
#include "Village/House/House.h"
#include "Common/InventoryComponent.h"
#include "PurgeZones/PurgeZone.h"

UStudentPerceptorNeesAina::UStudentPerceptorNeesAina()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UStudentPerceptorNeesAina::BeginPlay()
{
	Super::BeginPlay();
	
	if (auto PerceptionComp = GetOwner()->GetComponentByClass<UAIPerceptionComponent>())
	{
		PerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &UStudentPerceptorNeesAina::OnPerceptionUpdated);
	}
}

void UStudentPerceptorNeesAina::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!Actor) return;

	bool isSensed = Stimulus.WasSuccessfullySensed();
	
	// AI Controller
	AAIController* AIController = Cast<AAIController>(GetOwner());
	if (!AIController)
	{
		if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
		{
			AIController = Cast<AAIController>(OwnerPawn->GetController());
		}
	}
	if (!AIController) return;
	
	// Blackboard Component
	UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent();
	if (!BlackboardComp) return;
	
	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Damage>())
	{
		if (Actor->IsA(ABaseZombie::StaticClass()))
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Orange, TEXT("SOS! TAKING DAMAGE FROM BEHIND!"));
            
			// Force zombie to become the tracked target
			CheckZombie(Actor, true, BlackboardComp);
			return;
		}
	}
	
	if (Actor->IsA(ABaseZombie::StaticClass()))
	{
		CheckZombie(Actor, isSensed, BlackboardComp);
	}
	else 
	{
		if (BlackboardComp->GetValueAsObject(FName("TargetEnemy")) != nullptr) return;
		
		if (Actor->IsA(ABaseItem::StaticClass()))
		{
			RecordItem(Actor, isSensed, BlackboardComp);
		}
		else if (Actor->IsA(AHouse::StaticClass()))
		{
			RecordHouse(Actor, isSensed, BlackboardComp);
		}
		else if (Actor->IsA(APurgeZone::StaticClass()))
		{
			//auto PurgeZone = Cast<APurgeZone>(Actor);
			BlackboardComp->SetValueAsBool(FName("InPurgeZone"), isSensed);
			BlackboardComp->SetValueAsVector(FName("PurgeZoneLocation"), Actor->GetActorLocation());
		}
	}
}

void UStudentPerceptorNeesAina::CheckZombie(AActor* Zombie, bool IsSensed, class UBlackboardComponent* BlackboardComp)
{
	float CurrentTime = GetWorld()->GetTimeSeconds();

	if (IsSensed)
	{
		TrackedZombies.FindOrAdd(Zombie) = CurrentTime;

		if (BlackboardComp->GetValueAsObject(FName("TargetEnemy")) == nullptr)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("NEW THREAT LOCKED!"));
			BlackboardComp->SetValueAsObject(FName("TargetEnemy"), Zombie);
            
			FString const ZombieName = Zombie->GetName();
			BlackboardComp->SetValueAsBool(FName("IsHeavyZombie"), ZombieName.Contains(TEXT("Heavy")));
			BlackboardComp->SetValueAsBool(FName("IsRunnerZombie"), ZombieName.Contains(TEXT("Runner")));
		}
	}
    
	ForgetExpiredZombies(BlackboardComp);
}

void UStudentPerceptorNeesAina::ForgetExpiredZombies(class UBlackboardComponent* BlackboardComp)
{
	float CurrentTime = GetWorld()->GetTimeSeconds();
    
    TArray<AActor*> Keys;
    TrackedZombies.GetKeys(Keys);
    for (AActor* ZombieKey : Keys)
    {
        if (!IsValid(ZombieKey))
        {
            TrackedZombies.Remove(ZombieKey);
        }
    }

    AActor* CurrentTarget = Cast<AActor>(BlackboardComp->GetValueAsObject(FName("TargetEnemy")));

    if (CurrentTarget && TrackedZombies.Contains(CurrentTarget))
    {
        float LastSeenTime = TrackedZombies[CurrentTarget];
        
        if (CurrentTime - LastSeenTime > ZombieMemoryDuration)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("THREAT LOST: Zombie cleared from memory."));
            
            TrackedZombies.Remove(CurrentTarget);
            BlackboardComp->ClearValue(FName("TargetEnemy"));
            BlackboardComp->SetValueAsBool(FName("IsHeavyZombie"), false);
            BlackboardComp->SetValueAsBool(FName("IsRunnerZombie"), false);
            CurrentTarget = nullptr;
        }
    }

    // Fallback
    if (CurrentTarget == nullptr && TrackedZombies.Num() > 0)
    {
        AActor* BestNextThreat = nullptr;
        float MostRecentTime = 0.0f;

        for (auto& Elem : TrackedZombies)
        {
            if (Elem.Value > MostRecentTime)
            {
                MostRecentTime = Elem.Value;
                BestNextThreat = Elem.Key;
            }
        }

        if (BestNextThreat)
        {
            BlackboardComp->SetValueAsObject(FName("TargetEnemy"), BestNextThreat);
            FString const ZombieName = BestNextThreat->GetName();
            BlackboardComp->SetValueAsBool(FName("IsHeavyZombie"), ZombieName.Contains(TEXT("Heavy")));
            BlackboardComp->SetValueAsBool(FName("IsRunnerZombie"), ZombieName.Contains(TEXT("Runner")));
        }
    }
}

void UStudentPerceptorNeesAina::RecordItem(AActor* Item, bool IsSensed, class UBlackboardComponent* BlackboardComp)
{
	ABaseItem* CastItem = Cast<ABaseItem>(Item);
	if (!CastItem) return;
	if (IsSensed) KnownItems.AddUnique(CastItem);
    
    // Clean out memory pointers to items that have been looted/destroyed
    KnownItems.RemoveAll([](ABaseItem* I) { return !IsValid(I); });

	AAIController* AIController = Cast<AAIController>(GetOwner());
	if (!AIController)
	{
		if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
		{
			AIController = Cast<AAIController>(OwnerPawn->GetController());
		}
	}
	APawn* Survivor = AIController ? AIController->GetPawn<APawn>() : Cast<APawn>(GetOwner());
	UInventoryComponent* InventoryComp = Survivor ? Survivor->FindComponentByClass<UInventoryComponent>() : nullptr;
    
	int EmptySlots = 0;

	if (InventoryComp)
	{
		for (ABaseItem* SlotItem : InventoryComp->GetInventory())
		{
			if (SlotItem == nullptr) EmptySlots++;
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("ERROR: Inventory Component Not Found!"));
		return;
	}

	if (InventoryComp)
	{
		for (ABaseItem* SlotItem : InventoryComp->GetInventory())
		{
			if (SlotItem == nullptr) EmptySlots++;
		}
	}
	
    if (KnownItems.Num() > 0 && EmptySlots > 0)
    {
    	ABaseItem* BestItem = nullptr;
    	float MinDistSq = MAX_FLT;
    	FVector MyLoc = GetOwner()->GetActorLocation();
    	
        for (ABaseItem* CurrentItem : KnownItems)
        {
            if (!IsValid(CurrentItem)) continue;

        	if (CurrentItem->GetItemType() == EItemType::Garbage)
        	{
        		float DistSq = FVector::DistSquared(MyLoc, CurrentItem->GetActorLocation());
        		if (DistSq < MinDistSq)
        		{
        			MinDistSq = DistSq;
        			BestItem = CurrentItem;
        		}
        	}
        	else
            {
                float DistSq = FVector::DistSquared(MyLoc, CurrentItem->GetActorLocation());
                if (DistSq < MinDistSq)
                {
                    MinDistSq = DistSq;
                    BestItem = CurrentItem;
                }
            }
        }

        if (BestItem)
        {
            BlackboardComp->SetValueAsObject(FName("TargetItem"), BestItem);
            return;
        }
    }
	
    BlackboardComp->ClearValue(FName("TargetItem"));
}

void UStudentPerceptorNeesAina::RecordHouse(AActor* House, bool IsSensed, class UBlackboardComponent* BlackboardComp)
{
	AHouse* CastHouse = Cast<AHouse>(House);
    if (!CastHouse || !IsSensed || !BlackboardComp) return;

    if (!HouseMemoryMap.Contains(CastHouse))
    {
        FHouseData NewHouseData;
        NewHouseData.HouseActor = CastHouse;

        FHouseBounds Bounds = CastHouse->GetBounds();
        FVector WorldMin = Bounds.Origin - Bounds.Extent;
        FVector WorldMax = Bounds.Origin + Bounds.Extent;

        FCollisionQueryParams TraceParams(FName(TEXT("PerceptorPerimeterScan")), true, CastHouse);
        
        TArray<FVector> ScanPoints;
        TArray<FVector> Directions;
        float InterpSteps[3] = { 0.25f, 0.50f, 0.75f };

        for (float Alpha : InterpSteps)
        {
            float LerpX = FMath::Lerp(WorldMin.X, WorldMax.X, Alpha);
            float LerpY = FMath::Lerp(WorldMin.Y, WorldMax.Y, Alpha);

            ScanPoints.Add(FVector(WorldMax.X + 20.f, LerpY, 15.f));  Directions.Add(FVector(-1.f, 0.f, 0.f)); // North
            ScanPoints.Add(FVector(WorldMin.X - 20.f, LerpY, 15.f));  Directions.Add(FVector(1.f, 0.f, 0.f));  // South
            ScanPoints.Add(FVector(LerpX, WorldMax.Y + 20.f, 15.f));  Directions.Add(FVector(0.f, -1.f, 0.f)); // East
            ScanPoints.Add(FVector(LerpX, WorldMin.Y - 20.f, 15.f));  Directions.Add(FVector(0.f, 1.f, 0.f));  // West
        }

        for (int32 i = 0; i < ScanPoints.Num(); ++i)
        {
            FVector EndPoint = ScanPoints[i] + (Directions[i] * 80.f);
            FHitResult Hit;
            
            if (!GetWorld()->LineTraceSingleByChannel(Hit, ScanPoints[i], EndPoint, ECC_WorldStatic, TraceParams))
            {
                FVector SafeEntrancePos = ScanPoints[i] + (Directions[i] * 150.f);
                NewHouseData.DiscoveredEntrances.Add(SafeEntrancePos);
            }
        }

        // Fallback: If no door was found, use center point
        if (NewHouseData.DiscoveredEntrances.Num() == 0)
        {
            NewHouseData.DiscoveredEntrances.Add(Bounds.Origin);
        }

        HouseMemoryMap.Add(CastHouse, NewHouseData);
    }

    // TARGET SELECTION: Find the closest unlooted building
    AHouse* BestHouseTarget = nullptr;
    float MinDistSq = MAX_FLT;
    FVector MyLoc = GetOwner()->GetActorLocation();

    for (auto& Pair : HouseMemoryMap)
    {
        if (Pair.Value.bHasBeenLooted) continue;

        float DistSq = FVector::DistSquared(MyLoc, Pair.Key->GetActorLocation());
        if (DistSq < MinDistSq)
        {
            MinDistSq = DistSq;
            BestHouseTarget = Pair.Key;
        }
    }

    if (BestHouseTarget)
    {
        BlackboardComp->SetValueAsObject(FName("TargetHouse"), BestHouseTarget);
    }
}
