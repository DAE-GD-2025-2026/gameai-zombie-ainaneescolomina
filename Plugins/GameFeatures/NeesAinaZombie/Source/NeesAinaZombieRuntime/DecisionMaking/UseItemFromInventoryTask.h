#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Items/ItemType.h"
#include "UseItemFromInventoryTask.generated.h"

UCLASS()
class NEESAINAZOMBIERUNTIME_API UUseItemFromInventoryTask : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UUseItemFromInventoryTask();
	
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	UPROPERTY(EditAnywhere, Category = "Item Settings")
	EItemType TargetItemType;
};