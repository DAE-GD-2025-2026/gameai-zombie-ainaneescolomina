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

	// Helper functions
	void ResetWeights();
	void SetWeight(class ISteeringBehavior* Behavior, float Weight);

	void ResetStuckState();
	
	TUniquePtr<BlendedSteeringNeesAina> BlendedEngine;
	Seek* SeekBehavior = nullptr;
	Flee* FleeBehavior = nullptr;
	Evade* EvadeBehavior = nullptr;
	Wander* WanderBehavior = nullptr;
	
	UPROPERTY(EditAnywhere, Category = "Steering Base | Anti-Stuck")
	float StuckThresholdTime = 1.0f; // Time before activating detour

	FVector CurrentTaskTarget = FVector::ZeroVector;
	float DetourDuration = 0.0f;

private:
	void InitializeSteering();
	
	float StuckAngleModifier = 0.0f;
	float StuckDuration = 0.0f;
	FVector DetourVector = FVector::ZeroVector;
	FVector LastTrackedPos = FVector::ZeroVector;
};
