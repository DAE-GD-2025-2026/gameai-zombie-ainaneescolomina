#include "UseWeaponTaskNeesAina.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Common/InventoryComponent.h"
#include "Items/Weapon.h"
#include "Survivor/SurvivorPawn.h"

UUseWeaponTaskNeesAina::UUseWeaponTaskNeesAina()
{
    NodeName = "Attack With Weapon";
    PreferredWeaponType = EItemType::Pistol; // Default
}

EBTNodeResult::Type UUseWeaponTaskNeesAina::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController) return EBTNodeResult::Failed;

    ASurvivorPawn* Survivor = Cast<ASurvivorPawn>(AIController->GetPawn());
    if (!Survivor) return EBTNodeResult::Failed;

    UInventoryComponent* InventoryComp = Survivor->FindComponentByClass<UInventoryComponent>();
    if (!InventoryComp) return EBTNodeResult::Failed;

    const TArray<ABaseItem*>& CurrentInventory = InventoryComp->GetInventory();
    int FoundSlotIdx = -1;
    
    for (int32 i = 0; i < CurrentInventory.Num(); i++)
    {
        ABaseItem* Item = CurrentInventory[i];
        if (Item && Item->GetItemType() == PreferredWeaponType && Item->GetValue() > 0)
        {
            FoundSlotIdx = i;
            break; // Preference found! Stop searching.
        }
    }
    
    if (FoundSlotIdx == -1)
    {
        for (int32 i = 0; i < CurrentInventory.Num(); i++)
        {
            ABaseItem* Item = CurrentInventory[i];
            if (Item && Item->GetValue() > 0)
            {
                if (Item->GetItemType() == EItemType::Pistol || Item->GetItemType() == EItemType::Shotgun)
                {
                    FoundSlotIdx = i;
                    break; 
                }
            }
        }
    }

    if (FoundSlotIdx != -1)
    {
        ABaseItem* WeaponToUse = CurrentInventory[FoundSlotIdx];
        
        bool WeaponFired = InventoryComp->UseItem(FoundSlotIdx);
        
        if (WeaponFired)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, 
                FString::Printf(TEXT("SHOT FIRED WITH: %s! Ammo Left: %d"), *WeaponToUse->GetName(), WeaponToUse->GetValue()));

            // CRITICAL: Only drop/delete the weapon asset if it has completely run out of ammo
            if (WeaponToUse->GetValue() <= 0)
            {
                InventoryComp->RemoveItem(FoundSlotIdx);
                GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Orange, TEXT("Weapon completely empty! Discarded."));
            }

            return EBTNodeResult::Succeeded;
        }
    }

    GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("COMBAT FAIL: No weapons with ammo available in inventory!"));
    return EBTNodeResult::Failed;
}