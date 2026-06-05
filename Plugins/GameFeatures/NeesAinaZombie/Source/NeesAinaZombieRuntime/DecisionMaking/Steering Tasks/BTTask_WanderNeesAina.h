#pragma once

#include "CoreMinimal.h"
#include "BTTask_BaseSteeringNeesAina.h"
#include "BTTask_WanderNeesAina.generated.h"

UCLASS()
class NEESAINAZOMBIERUNTIME_API UBTTask_WanderNeesAina : public UBTTask_BaseSteeringNeesAina
{
	GENERATED_BODY()
public:
	UBTTask_WanderNeesAina() { NodeName = "Wander Steering"; }
	
protected:
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};