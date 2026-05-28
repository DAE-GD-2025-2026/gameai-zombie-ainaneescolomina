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
	
	// DAMAGE SENSE - 360 DEGREE VISION (feedback)
	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Damage>())
	{
		// took damage!
		if (ABaseZombie* Attacker = Cast<ABaseZombie>(Actor))
		{
			BlackboardComp->SetValueAsObject(FName("NearestZombie"), Attacker);
			
			// check for Heavy zombie
			if (Attacker->GetName().Contains("Heavy")) {
				BlackboardComp->SetValueAsBool(FName("IsHeavyZombie"), true);
			} else {
				BlackboardComp->SetValueAsBool(FName("IsHeavyZombie"), false);
			}

			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("OUCH! BEHIND USE!"));
			
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
			BlackboardComp->SetValueAsBool(FName("InPurgeZone"), isSensed);
		}
	}
}

void UStudentPerceptor::CheckZombie(AActor* Zombie, bool IsSensed, class UBlackboardComponent* BBComp)
{
	if (IsSensed)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("ZOMBIE!"));
		BBComp->SetValueAsObject(FName("TargetEnemy"), Zombie);
        
		FString const ZombieName = Zombie->GetName();
		bool const bIsHeavy = ZombieName.Contains(TEXT("Heavy"));
		bool const bIsRunner = ZombieName.Contains(TEXT("Runner"));
        
		BBComp->SetValueAsBool(FName("IsHeavyZombie"), bIsHeavy);
		BBComp->SetValueAsBool(FName("IsRunnerZombie"), bIsRunner);
	}
	else if (BBComp->GetValueAsObject(FName("TargetEnemy")) == Zombie)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("FORGUET ZOMBIE!"));
		BBComp->ClearValue(FName("TargetEnemy"));
		BBComp->SetValueAsBool(FName("IsHeavyZombie"), false);
		BBComp->SetValueAsBool(FName("IsRunnerZombie"), false);
	}
}

void UStudentPerceptor::RecordItem(AActor* Item, bool IsSensed, class UBlackboardComponent* BBComp)
{
	ABaseItem* CastItem = Cast<ABaseItem>(Item);
	if (!CastItem) return;
	if (IsSensed) KnownItems.AddUnique(CastItem);
    
    // Clean out memory pointers to items that have been looted/destroyed
    KnownItems.RemoveAll([](ABaseItem* I) { return !IsValid(I); });

	AAIController* AIController = Cast<AAIController>(GetOwner());
	APawn* Pawn = AIController ? AIController->GetPawn() : nullptr;
	UInventoryComponent* InventoryComp = Pawn ? Pawn->FindComponentByClass<UInventoryComponent>() : nullptr;
	int EmptySlots = 0;

	if (InventoryComp)
	{
		for (ABaseItem* SlotItem : InventoryComp->GetInventory())
		{
			if (SlotItem == nullptr) EmptySlots++;
		}
	}
	
	if (KnownItems.Num() <= 0)GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("ITEM NOT KNOWN"));
	if (EmptySlots <= 0)GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("ITEM NO SLOTS"));
	
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
        	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("ITEM!"));
            BBComp->SetValueAsObject(FName("TargetItem"), BestItem);
            return;
        }
    }
	
    BBComp->ClearValue(FName("TargetItem"));
}

void UStudentPerceptor::RecordHouse(AActor* House, bool IsSensed, class UBlackboardComponent* BBComp)
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
			BBComp->SetValueAsObject(FName("TargetHouse"), FinalHouseTarget);
		}
	}
}
