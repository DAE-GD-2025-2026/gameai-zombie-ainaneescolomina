#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "PickUpItemTask.generated.h"

UCLASS()
class NEESAINAZOMBIERUNTIME_API UPickUpItemTask : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UPickUpItemTask();
	
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};