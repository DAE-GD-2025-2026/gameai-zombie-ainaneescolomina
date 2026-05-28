#include "PickUpItemTask.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Items/BaseItem.h"
#include "StudentPerceptor.h"
#include "Common/InventoryComponent.h" 

UPickUpItemTask::UPickUpItemTask()
{
	NodeName = "Pick Up Target Item";
}

EBTNodeResult::Type UPickUpItemTask::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!AIController || !BlackboardComp) return EBTNodeResult::Failed;

	APawn* Survivor = AIController->GetPawn();
	ABaseItem* TargetItem = Cast<ABaseItem>(BlackboardComp->GetValueAsObject(FName("TargetItem")));
	if (!IsValid(TargetItem) || !Survivor)
	{
		BlackboardComp->ClearValue(FName("TargetItem"));
		return EBTNodeResult::Failed;
	}
	
	UInventoryComponent* InventoryComp = Survivor->FindComponentByClass<UInventoryComponent>();
	if (!InventoryComp) return EBTNodeResult::Failed;

	if (TargetItem->GetItemType() == EItemType::Garbage)
	{
		TargetItem->Destroy();
		return EBTNodeResult::Succeeded;
	}
	
	const TArray<ABaseItem*>& CurrentInventory = InventoryComp->GetInventory();
	int TargetSlotIdx = -1;

	for (int i = 0; i < CurrentInventory.Num(); i++)
	{
		if (CurrentInventory[i] == nullptr)
		{
			TargetSlotIdx = i;
			break; 
		}
	}
	
	if (TargetSlotIdx != -1)
	{
		bool bGrabSuccess = InventoryComp->GrabItem(TargetSlotIdx, TargetItem);
		if (!bGrabSuccess)return EBTNodeResult::Failed;
		TargetItem->SetActorHiddenInGame(true);
		TargetItem->SetActorEnableCollision(false);
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Orange, TEXT("ITEM PICKED UP"));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Orange, TEXT("INVENTORY FULL: CANNOT PICK UP"));
		return EBTNodeResult::Failed;
	}

	if (UStudentPerceptor* Perceptor = Survivor->FindComponentByClass<UStudentPerceptor>())
	{
		Perceptor->KnownItems.Remove(TargetItem);
	}
	
	BlackboardComp->ClearValue(FName("TargetItem"));
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan, TEXT("SUCCESSFULLY LOOTED ITEM!"));
	return EBTNodeResult::Succeeded;
}