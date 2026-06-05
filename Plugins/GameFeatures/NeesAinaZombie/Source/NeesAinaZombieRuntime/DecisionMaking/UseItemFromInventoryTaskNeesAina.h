#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Items/ItemType.h"
#include "UseItemFromInventoryTaskNeesAina.generated.h"

UCLASS()
class NEESAINAZOMBIERUNTIME_API UUseItemFromInventoryTaskNeesAina : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UUseItemFromInventoryTaskNeesAina();
	
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	UPROPERTY(EditAnywhere, Category = "Item Settings")
	EItemType TargetItemType;
};