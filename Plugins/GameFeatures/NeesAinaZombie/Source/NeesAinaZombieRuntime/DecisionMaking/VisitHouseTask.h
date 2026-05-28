#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "VisitHouseTask.generated.h"

UCLASS()
class NEESAINAZOMBIERUNTIME_API UVisitHouseTask : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UVisitHouseTask();
	
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};