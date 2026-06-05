#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Items/ItemType.h"
#include "BTTask_UseWeaponNeesAina.generated.h"

UCLASS()
class NEESAINAZOMBIERUNTIME_API UBTTask_UseWeaponNeesAina : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTask_UseWeaponNeesAina();
	
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	UPROPERTY(EditAnywhere, Category = "Weapon Settings")
	EItemType PreferredWeaponType;
};