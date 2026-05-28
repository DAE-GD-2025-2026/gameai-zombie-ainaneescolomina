// Fill out your copyright notice in the Description page of Project Settings.

#include "StudentPerceptor.h"
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

UStudentPerceptor::UStudentPerceptor()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UStudentPerceptor::BeginPlay()
{
	Super::BeginPlay();
	
	if (auto PerceptionComp = GetOwner()->GetComponentByClass<UAIPerceptionComponent>())
	{
		PerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &UStudentPerceptor::OnPerceptionUpdated);
	}
}

void UStudentPerceptor::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
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
	
	//GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Green, TEXT("I Something!"));
	
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

void UStudentPerceptor::CheckZombie(AActor* Zombie, bool IsSensed, class UBlackboardComponent* BlackboardComp)
{
	if (IsSensed)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("ZOMBIE!"));
		BlackboardComp->SetValueAsObject(FName("TargetEnemy"), Zombie);
        
		FString const ZombieName = Zombie->GetName();
		bool const bIsHeavy = ZombieName.Contains(TEXT("Heavy"));
		bool const bIsRunner = ZombieName.Contains(TEXT("Runner"));
        
		BlackboardComp->SetValueAsBool(FName("IsHeavyZombie"), bIsHeavy);
		BlackboardComp->SetValueAsBool(FName("IsRunnerZombie"), bIsRunner);
	}
	else if (BlackboardComp->GetValueAsObject(FName("TargetEnemy")) == Zombie)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("FORGUET ZOMBIE!"));
		BlackboardComp->ClearValue(FName("TargetEnemy"));
		BlackboardComp->SetValueAsBool(FName("IsHeavyZombie"), false);
		BlackboardComp->SetValueAsBool(FName("IsRunnerZombie"), false);
	}
}

void UStudentPerceptor::RecordItem(AActor* Item, bool IsSensed, class UBlackboardComponent* BlackboardComp)
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

void UStudentPerceptor::RecordHouse(AActor* House, bool IsSensed, class UBlackboardComponent* BlackboardComp)
{
	AHouse* CastHouse = Cast<AHouse>(House);
	if (!CastHouse) return;
	if (IsSensed) DiscoveredHouses.AddUnique(CastHouse);

	if (DiscoveredHouses.Num() > 0)
	{
		AHouse* BestHouse = nullptr;
		float MinDistSqUnvisited = MAX_FLT;
		float MinDistSqVisited = MAX_FLT;
		AHouse* FallbackVisitedHouse = nullptr;
		FVector MyLoc = GetOwner()->GetActorLocation();

		for (AHouse* CurrentHouse : DiscoveredHouses)
		{
			if (!CurrentHouse) continue;
			float DistSq = FVector::DistSquared(MyLoc, CurrentHouse->GetActorLocation());

			if (!VisitedHouses.Contains(CurrentHouse))
			{
				if (DistSq < MinDistSqUnvisited)
				{
					MinDistSqUnvisited = DistSq;
					BestHouse = CurrentHouse;
				}
			}
			else
			{
				if (DistSq < MinDistSqVisited)
				{
					MinDistSqVisited = DistSq;
					FallbackVisitedHouse = CurrentHouse;
				}
			}
		}

		AHouse* FinalHouseTarget = (BestHouse != nullptr) ? BestHouse : FallbackVisitedHouse;

		if (FinalHouseTarget)
		{
			//GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("HOUSE!"));
			BlackboardComp->SetValueAsObject(FName("TargetHouse"), FinalHouseTarget);
		}
	}
}
