#include "RefreshStatsService.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Common/HealthComponent.h"
#include "Common/InventoryComponent.h"
#include "Common/StaminaComponent.h" 
#include "Items/BaseItem.h"

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
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!AIController || !BlackboardComp) return;

	APawn* Survivor = AIController->GetPawn();
	if (!Survivor) return;

	if (UHealthComponent* HealthComp = Survivor->FindComponentByClass<UHealthComponent>())
	{
		float CurrentHealth = HealthComp->GetHealth(); 
		BlackboardComp->SetValueAsFloat(FName("HealthStat"), CurrentHealth);
	}

	if (UStaminaComponent* StaminaComp = Survivor->FindComponentByClass<UStaminaComponent>())
	{
		float CurrentEnergy = StaminaComp->GetCurrentStamina(); 
		BlackboardComp->SetValueAsFloat(FName("EnergyStat"), CurrentEnergy);
	}
	
	bool HasFood = false;
	bool HasMedkit = false;
	bool HasWeapon = false;
	if (UInventoryComponent* InvComp = Survivor->FindComponentByClass<UInventoryComponent>())
	{
		for (ABaseItem* Item : InvComp->GetInventory())
		{
			if (Item && Item->GetValue() > 0)
			{
				switch (Item->GetItemType())
				{
				case EItemType::Food:
					HasFood = true;
					break;

				case EItemType::Medkit:
					HasMedkit = true;
					break;

				case EItemType::Pistol:
				case EItemType::Shotgun:
					HasWeapon = true;
					break;

				default:
					break;
				}
            
				if (HasFood && HasMedkit && HasWeapon)
				{
					break;
				}
			}
		}

		BlackboardComp->SetValueAsBool(FName("HasFood"), HasFood);
		BlackboardComp->SetValueAsBool(FName("HasMedikit"), HasMedkit);
		BlackboardComp->SetValueAsBool(FName("HasWeapon"), HasWeapon);
	}
}