#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Items/ItemType.h"
#include "UseWeaponTaskNeesAina.generated.h"

UCLASS()
class NEESAINAZOMBIERUNTIME_API UUseWeaponTaskNeesAina : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UUseWeaponTaskNeesAina();
	
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	UPROPERTY(EditAnywhere, Category = "Weapon Settings")
	EItemType PreferredWeaponType;
};