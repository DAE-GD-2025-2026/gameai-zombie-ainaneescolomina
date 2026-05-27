#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"

#include "../Movement/SurvivorStates.h"
#include "../Movement/BlendedSteering.h"

#include "SteeringTask.generated.h"

UCLASS()
class NEESAINAZOMBIERUNTIME_API USteeringTask : public UBTTaskNode
{
	GENERATED_BODY()
public:
	USteeringTask();

	UPROPERTY(EditAnywhere, Category = "Search")
	ESurvivorSteeringState CurrentState;
	
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
private:
	TUniquePtr<BlendedSteering> BlendedEngine;
	Seek* SeekBehavior = nullptr;
	Flee* FleeBehavior = nullptr;
	Evade* EvadeBehavior = nullptr;
	Wander* WanderBehavior = nullptr;

	void InitializeSteering();
};

