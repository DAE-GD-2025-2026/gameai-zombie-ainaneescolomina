#include "UseItemFromInventoryTaskNeesAina.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Common/InventoryComponent.h"
#include "Items/BaseItem.h"

UUseItemFromInventoryTaskNeesAina::UUseItemFromInventoryTaskNeesAina()
{
    NodeName = "Use Item From Inventory";
}

EBTNodeResult::Type UUseItemFromInventoryTaskNeesAina::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController) return EBTNodeResult::Failed;

    APawn* Survivor = AIController->GetPawn();
    if (!Survivor) return EBTNodeResult::Failed;

    UInventoryComponent* InventoryComp = Survivor->FindComponentByClass<UInventoryComponent>();
    if (!InventoryComp) return EBTNodeResult::Failed;

    const TArray<ABaseItem*>& CurrentInventory = InventoryComp->GetInventory();
    int FoundSlotIdx = -1;

    for (int32 i = 0; i < CurrentInventory.Num(); i++)
    {
        ABaseItem* CurrentItem = CurrentInventory[i];
        
        if (CurrentItem && CurrentItem->GetItemType() == TargetItemType && CurrentItem->GetValue() > 0)
        {
            FoundSlotIdx = i;
            break;
        }
    }

    if (FoundSlotIdx != -1)
    {
        bool bUseSuccess = InventoryComp->UseItem(FoundSlotIdx);
        
        if (bUseSuccess)
        {
            InventoryComp->RemoveItem(FoundSlotIdx);

            GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("SUCCESSFULLY CONSUMED ITEM!"));
            return EBTNodeResult::Succeeded;
        }
    }

    GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("COULD NOT FIND MATCHING ITEM TO USE"));
    return EBTNodeResult::Failed;
}