#pragma once

#include "CoreMinimal.h"
#include "BTTask_BaseSteeringNeesAina.h"
#include "BTTask_SeekItemNeesAina.generated.h"

UCLASS()
class NEESAINAZOMBIERUNTIME_API UBTTask_SeekItemNeesAina : public UBTTask_BaseSteeringNeesAina
{
	GENERATED_BODY()
public:
	UBTTask_SeekItemNeesAina() { NodeName = "Seek Item Steering"; }
	
protected:
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};