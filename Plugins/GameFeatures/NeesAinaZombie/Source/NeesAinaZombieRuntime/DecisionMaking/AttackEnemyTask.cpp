#include "AttackEnemyTask.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Survivor/SurvivorPawn.h"
#include "Items/Weapon.h"
#include "Common/InventoryComponent.h"

UAttackEnemyTask::UAttackEnemyTask()
{
	NodeName = "Attack Enemy";
}

EBTNodeResult::Type UAttackEnemyTask::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!AIController || !BlackboardComp) return EBTNodeResult::Failed;

	ASurvivorPawn* Survivor = Cast<ASurvivorPawn>(AIController->GetPawn());
	AActor* Enemy = Cast<AActor>(BlackboardComp->GetValueAsObject(EnemyKey.SelectedKeyName));
	if (!Survivor || !Enemy) return EBTNodeResult::Failed;

	UInventoryComponent* Inventory = Survivor->FindComponentByClass<UInventoryComponent>();
	if (!Inventory) return EBTNodeResult::Failed;
	
	AWeapon* EquippedWeapon = nullptr;
	const TArray<ABaseItem*>& MyItems = Inventory->GetInventory();
    
	for (ABaseItem* Item : MyItems)
	{
		EquippedWeapon = Cast<AWeapon>(Item);
		if (EquippedWeapon) break;
	}

	if (!EquippedWeapon) return EBTNodeResult::Failed;
	EquippedWeapon->UseItem(*Survivor);

	return EBTNodeResult::Succeeded;
}