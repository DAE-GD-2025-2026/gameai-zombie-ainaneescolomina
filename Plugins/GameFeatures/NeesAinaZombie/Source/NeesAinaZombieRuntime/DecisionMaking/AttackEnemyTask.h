#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "AttackEnemyTask.generated.h"

UCLASS()
class NEESAINAZOMBIERUNTIME_API UAttackEnemyTask : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UAttackEnemyTask();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

public:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector EnemyKey;
};

