#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "RefreshStatsService.generated.h"

UCLASS()
class NEESAINAZOMBIERUNTIME_API URefreshStatsService : public UBTService
{
	GENERATED_BODY()

public:
	URefreshStatsService();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};