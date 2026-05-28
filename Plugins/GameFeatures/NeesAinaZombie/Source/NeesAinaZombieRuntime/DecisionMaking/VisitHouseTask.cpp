#include "VisitHouseTask.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Village/House/House.h"
#include "StudentPerceptor.h"

UVisitHouseTask::UVisitHouseTask()
{
	NodeName = "Visit House";
}

EBTNodeResult::Type UVisitHouseTask::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!AIController || !BlackboardComp) return EBTNodeResult::Failed;

	APawn* Survivor = AIController->GetPawn();
	AHouse* TargetHouse = Cast<AHouse>(BlackboardComp->GetValueAsObject(FName("TargetHouse")));

	if (!IsValid(TargetHouse) || !Survivor)
	{
		BlackboardComp->ClearValue(FName("TargetHouse"));
		return EBTNodeResult::Failed;
	}

	if (UStudentPerceptor* Perceptor = Survivor->FindComponentByClass<UStudentPerceptor>())
	{
		Perceptor->VisitedHouses.AddUnique(TargetHouse);

		// If our visited memory matches our discovered memory, wipe the slate clean!
		if (Perceptor->VisitedHouses.Num() >= Perceptor->DiscoveredHouses.Num())
		{
			Perceptor->VisitedHouses.Empty();
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, TEXT("ALL HOUSES VISITED! Wiping memory to loop patrol paths."));
		}
	}

	BlackboardComp->ClearValue(FName("TargetHouse"));
	return EBTNodeResult::Succeeded;
}