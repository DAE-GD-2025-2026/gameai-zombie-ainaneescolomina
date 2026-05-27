#pragma once

#include "CoreMinimal.h"
#include "SurvivorStates.generated.h"

UENUM(BlueprintType)
enum class ESurvivorSteeringState : uint8
{
	Wander       UMETA(DisplayName = "Wander"),
	SeekItem     UMETA(DisplayName = "SeekItem"),
	FleeEnemy    UMETA(DisplayName = "FleeEnemy"),
	LootingHouse UMETA(DisplayName = "LootingHouse")
};