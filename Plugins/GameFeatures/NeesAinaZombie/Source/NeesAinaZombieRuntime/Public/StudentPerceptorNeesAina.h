#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Damage.h"
#include "Perception/AISense_Damage.h"
#include "StudentPerceptorNeesAina.generated.h"

class ABaseItem;
class AHouse;

USTRUCT(BlueprintType)
struct FHouseMemoryLayout
{
	GENERATED_BODY()

	UPROPERTY()
	AHouse* HouseActor = nullptr;

	UPROPERTY()
	TArray<FVector> CalculatedEntrances;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class NEESAINAZOMBIERUNTIME_API UStudentPerceptorNeesAina : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UStudentPerceptorNeesAina();
	
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);
	
	// MEMORY
	UPROPERTY(Transient)
	TArray<ABaseItem*> KnownItems;
	UPROPERTY(Transient)
	TArray<AHouse*> VisitedHouses;

	UPROPERTY()
	TMap<AHouse*, FHouseMemoryLayout> HouseEntranceMemory;
	
	UPROPERTY()
	TMap<AActor*, float> TrackedZombies;
	const float ZombieMemoryDuration = 0.8f; // How long do you remember a Zombie for

private:
	void CheckZombie(AActor* Zombie, bool IsSensed, class UBlackboardComponent* BlackboardComp);
	void ForgetExpiredZombies(class UBlackboardComponent* BlackboardComp);
	
	void RecordItem(AActor* Item, bool IsSensed, class UBlackboardComponent* BlackboardComp);
	void RecordHouse(AActor* House, bool IsSensed, class UBlackboardComponent* BlackboardComp);
};