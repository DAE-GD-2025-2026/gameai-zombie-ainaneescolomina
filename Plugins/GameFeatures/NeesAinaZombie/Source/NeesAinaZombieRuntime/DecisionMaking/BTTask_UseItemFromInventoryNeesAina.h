#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Items/ItemType.h"
#include "BTTask_UseItemFromInventoryNeesAina.generated.h"

UCLASS()
class NEESAINAZOMBIERUNTIME_API UBTTask_UseItemFromInventoryNeesAina : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTask_UseItemFromInventoryNeesAina();
	
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	UPROPERTY(EditAnywhere, Category = "Item Settings")
	EItemType TargetItemType;
};