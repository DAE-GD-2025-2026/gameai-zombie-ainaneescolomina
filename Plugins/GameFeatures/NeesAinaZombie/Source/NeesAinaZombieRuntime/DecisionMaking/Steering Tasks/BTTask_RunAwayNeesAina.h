#pragma once

#include "CoreMinimal.h"
#include "BTTask_BaseSteeringNeesAina.h"
#include "BTTask_RunAwayNeesAina.generated.h"

UCLASS()
class NEESAINAZOMBIERUNTIME_API UBTTask_RunAwayNeesAina : public UBTTask_BaseSteeringNeesAina
{
	GENERATED_BODY()
public:
	UBTTask_RunAwayNeesAina() { NodeName = "Run Away Steering"; }
	
protected:
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};