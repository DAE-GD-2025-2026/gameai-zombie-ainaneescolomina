#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "RefreshStatsServiceNeesAina.generated.h"

UCLASS()
class NEESAINAZOMBIERUNTIME_API URefreshStatsServiceNeesAina : public UBTService
{
	GENERATED_BODY()

public:
	URefreshStatsServiceNeesAina();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};