#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "FindSafeLocationTask.generated.h"

UCLASS()
class NEESAINAZOMBIERUNTIME_API UFindSafeLocationTask : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UFindSafeLocationTask();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

public:
	UPROPERTY(EditAnywhere, Category = "Search")
	float SearchRadius = 500.f;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetLocationKey;
};
