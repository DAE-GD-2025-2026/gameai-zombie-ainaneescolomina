// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Damage.h"
#include "Perception/AISense_Damage.h"
#include "StudentPerceptor.generated.h"

class ABaseItem;
class AHouse;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NEESAINAZOMBIERUNTIME_API UStudentPerceptor : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UStudentPerceptor();
	
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);
	
	// MEMORY
	UPROPERTY(Transient)
	TArray<ABaseItem*> KnownItems;
	UPROPERTY(Transient)
	TArray<AHouse*> DiscoveredHouses;
	UPROPERTY(Transient)
	TArray<AHouse*> VisitedHouses;

private:
	void CheckZombie(AActor* Zombie, bool IsSensed, class UBlackboardComponent* BlackboardComp);
	void RecordItem(AActor* Item, bool IsSensed, class UBlackboardComponent* BlackboardComp);
	void RecordHouse(AActor* House, bool IsSensed, class UBlackboardComponent* BlackboardComp);
};
