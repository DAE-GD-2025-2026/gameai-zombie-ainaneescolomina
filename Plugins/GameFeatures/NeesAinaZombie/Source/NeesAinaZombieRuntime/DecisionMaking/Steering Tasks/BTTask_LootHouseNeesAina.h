#pragma once

#include "CoreMinimal.h"
#include "BTTask_BaseSteeringNeesAina.h"
#include "BTTask_LootHouseNeesAina.generated.h"

UCLASS()
class NEESAINAZOMBIERUNTIME_API UBTTask_LootHouseNeesAina : public UBTTask_BaseSteeringNeesAina
{
	GENERATED_BODY()
public:
	UBTTask_LootHouseNeesAina() { NodeName = "Loot House Steering"; }
	
protected:
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};