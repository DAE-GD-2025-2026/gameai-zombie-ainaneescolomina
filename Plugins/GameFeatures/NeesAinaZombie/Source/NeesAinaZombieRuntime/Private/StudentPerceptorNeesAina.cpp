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
    if (!CastHouse) return;

    if (IsSensed)
    {
        VisitedHouses.AddUnique(CastHouse);

        // Analyze and map the entrance openings if this building is new to our memory
        if (!HouseEntranceMemory.Contains(CastHouse))
        {
            FHouseBounds Bounds = CastHouse->GetBounds();
            
            FVector WorldMin = Bounds.Origin - Bounds.Extent;
            FVector WorldMax = Bounds.Origin + Bounds.Extent;

            float MidX = Bounds.Origin.X;
            float MidY = Bounds.Origin.Y;

            // Establish 4 scan origins positioned 50 units outside the bounds facing inward
            TArray<FVector> ScanOrigins;
            ScanOrigins.Add(FVector(WorldMax.X + 50.f, MidY, 15.f));  // North Face Approach
            ScanOrigins.Add(FVector(WorldMin.X - 50.f, MidY, 15.f));  // South Face Approach
            ScanOrigins.Add(FVector(MidX, WorldMax.Y + 50.f, 15.f));  // East Face Approach
            ScanOrigins.Add(FVector(MidX, WorldMin.Y - 50.f, 15.f));  // West Face Approach

            FHouseMemoryLayout LayoutData;
            LayoutData.HouseActor = CastHouse;

            FCollisionQueryParams TraceParams(FName(TEXT("EntranceScan")), true, CastHouse);

            for (const FVector& TraceStart : ScanOrigins)
            {
                FVector TraceEnd = FVector(MidX, MidY, 15.f); // Target house center
                FHitResult HitResult;

                // Raycast looking for walls
                bool bHitWall = GetWorld()->LineTraceSingleByChannel(
                    HitResult, 
                    TraceStart, 
                    TraceEnd, 
                    ECC_WorldStatic, 
                    TraceParams
                );
            	
                if (!bHitWall || (HitResult.Distance > 100.f))
                {
                    FVector EntryDirection = (TraceEnd - TraceStart).GetSafeNormal();
                    FVector PaddedEntrance = TraceStart + (EntryDirection * 125.f); 
                    
                    LayoutData.CalculatedEntrances.Add(PaddedEntrance);
                }
            }

            // Fallback
            if (LayoutData.CalculatedEntrances.Num() == 0)
            {
                LayoutData.CalculatedEntrances.Add(Bounds.Origin);
            }

            HouseEntranceMemory.Add(CastHouse, LayoutData);
        }
    }
	
    // Choose the closest entrance
    if (VisitedHouses.Num() > 0)
    {
       AHouse* BestHouse = nullptr;
       FVector ChosenEntranceVector = FVector::ZeroVector;
       float MinDistSqUnvisited = MAX_FLT;
       float MinDistSqVisited = MAX_FLT;
       AHouse* FallbackVisitedHouse = nullptr;
       FVector FallbackEntranceVector = FVector::ZeroVector;
       
       FVector MyLoc = GetOwner()->GetActorLocation();

       for (AHouse* CurrentHouse : VisitedHouses)
       {
          if (!CurrentHouse) continue;

          // Pull up our structural memory array for this building
          if (FHouseMemoryLayout* Layout = HouseEntranceMemory.Find(CurrentHouse))
          {
              for (const FVector& EntrancePos : Layout->CalculatedEntrances)
              {
                  float DistSq = FVector::DistSquared(MyLoc, EntrancePos);

                  if (!VisitedHouses.Contains(CurrentHouse))
                  {
                     if (DistSq < MinDistSqUnvisited)
                     {
                        MinDistSqUnvisited = DistSq;
                        BestHouse = CurrentHouse;
                        ChosenEntranceVector = EntrancePos; // Track closest entryway
                     }
                  }
                  else
                  {
                     if (DistSq < MinDistSqVisited)
                     {
                        MinDistSqVisited = DistSq;
                        FallbackVisitedHouse = CurrentHouse;
                        FallbackEntranceVector = EntrancePos; // Track backup entryway
                     }
                  }
              }
          }
       }

       AHouse* FinalHouseTarget = (BestHouse != nullptr) ? BestHouse : FallbackVisitedHouse;
       FVector FinalEntranceTarget = (BestHouse != nullptr) ? ChosenEntranceVector : FallbackEntranceVector;

       if (FinalHouseTarget)
       {
          BlackboardComp->SetValueAsObject(FName("TargetHouse"), FinalHouseTarget);
          BlackboardComp->SetValueAsVector(FName("TargetHouseEntrance"), FinalEntranceTarget);
       }
    }
}
