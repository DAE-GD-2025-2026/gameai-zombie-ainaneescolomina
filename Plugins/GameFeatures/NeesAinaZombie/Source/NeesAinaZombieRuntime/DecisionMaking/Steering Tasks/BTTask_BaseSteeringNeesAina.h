#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "../Movement/BlendedSteeringNeesAina.h"
#include "BTTask_BaseSteeringNeesAina.generated.h"

UCLASS()
class NEESAINAZOMBIERUNTIME_API UBTTask_BaseSteeringNeesAina : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_BaseSteeringNeesAina();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	// Helper functions for children to modify weights and targets smoothly
	void ResetWeights();
	void SetWeight(class ISteeringBehavior* Behavior, float Weight);

	TUniquePtr<BlendedSteeringNeesAina> BlendedEngine;
	Seek* SeekBehavior = nullptr;
	Flee* FleeBehavior = nullptr;
	Evade* EvadeBehavior = nullptr;
	Wander* WanderBehavior = nullptr;

private:
	void InitializeSteering();
};
