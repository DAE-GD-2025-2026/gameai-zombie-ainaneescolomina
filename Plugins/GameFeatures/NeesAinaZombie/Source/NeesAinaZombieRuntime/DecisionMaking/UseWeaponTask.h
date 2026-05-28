#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Items/ItemType.h"
#include "UseWeaponTask.generated.h"

UCLASS()
class NEESAINAZOMBIERUNTIME_API UUseWeaponTask : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UUseWeaponTask();
	
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	UPROPERTY(EditAnywhere, Category = "Weapon Settings")
	EItemType PreferredWeaponType;
};