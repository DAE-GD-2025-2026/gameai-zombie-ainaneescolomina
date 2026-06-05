#include "BTTask_VisitHouseNeesAina.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Village/House/House.h"
#include "StudentPerceptorNeesAina.h"

UBTTask_VisitHouseNeesAina::UBTTask_VisitHouseNeesAina()
{
	NodeName = "Visit House";
}

EBTNodeResult::Type UBTTask_VisitHouseNeesAina::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

	if (UStudentPerceptorNeesAina* Perceptor = Survivor->FindComponentByClass<UStudentPerceptorNeesAina>())
	{
		Perceptor->VisitedHouses.AddUnique(TargetHouse);

		/*
		if (Perceptor->VisitedHouses.Num() >= Perceptor->DiscoveredHouses.Num())
		{
			Perceptor->VisitedHouses.Empty();
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, TEXT("ALL HOUSES VISITED! Wiping memory to loop patrol paths."));
		}
		*/
	}

	BlackboardComp->ClearValue(FName("TargetHouse"));
	return EBTNodeResult::Succeeded;
}