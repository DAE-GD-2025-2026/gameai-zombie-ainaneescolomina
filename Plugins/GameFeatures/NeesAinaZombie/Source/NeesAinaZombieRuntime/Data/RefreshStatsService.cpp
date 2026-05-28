#include "RefreshStatsService.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Common/HealthComponent.h"
#include "Common/InventoryComponent.h"
#include "Common/StaminaComponent.h" 

URefreshStatsService::URefreshStatsService()
{
	NodeName = "Refresh Survivor Stats";
    
	Interval = 0.2f; 
	RandomDeviation = 0.05f; // Adds tiny variance to optimize CPU overhead
}

void URefreshStatsService::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AIController = OwnerComp.GetAIOwner();
	UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent();
	if (!AIController || !BBComp) return;

	APawn* Pawn = AIController->GetPawn();
	if (!Pawn) return;

	if (UHealthComponent* HealthComp = Pawn->FindComponentByClass<UHealthComponent>())
	{
		float CurrentHealth = HealthComp->GetHealth(); 
		BBComp->SetValueAsFloat(FName("HealthStat"), CurrentHealth);
	}

	if (UStaminaComponent* StaminaComp = Pawn->FindComponentByClass<UStaminaComponent>())
	{
		float CurrentEnergy = StaminaComp->GetCurrentStamina(); 
		BBComp->SetValueAsFloat(FName("EnergyStat"), CurrentEnergy);
	}
	
	bool HasFood = false;
	bool HasMedkit = false;
	if (UInventoryComponent* InvComp = Pawn->FindComponentByClass<UInventoryComponent>())
	{
		for (ABaseItem* Item : InvComp->GetInventory())
		{
			if (Item && Item->GetValue() > 0)
			{
				if (Item->GetItemType() == EItemType::Food)
				{
					HasFood = true;
				}
				else if (Item->GetItemType() == EItemType::Medkit)
				{
					HasMedkit = true;
				}
			}
            
			if (HasFood && HasMedkit)
			{
				break;
			}
		}
	}

	BBComp->SetValueAsBool(FName("HasFood"), HasFood);
	BBComp->SetValueAsBool(FName("HasMedikit"), HasMedkit);
}