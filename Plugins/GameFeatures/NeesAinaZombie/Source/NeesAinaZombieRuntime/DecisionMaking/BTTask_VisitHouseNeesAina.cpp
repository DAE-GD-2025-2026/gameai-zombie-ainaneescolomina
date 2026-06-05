#include "BTTask_VisitHouseNeesAina.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Village/House/House.h"

UBTTask_VisitHouseNeesAina::UBTTask_VisitHouseNeesAina()
{
	NodeName = "Visit House";
}

EBTNodeResult::Type UBTTask_VisitHouseNeesAina::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp) return EBTNodeResult::Failed;

	/*
	if (UStudentPerceptorNeesAina* Perceptor = Survivor->FindComponentByClass<UStudentPerceptorNeesAina>())
	{
		Perceptor->VisitedHouses.AddUnique(TargetHouse);
	}
	*/
	BlackboardComp->ClearValue(FName("TargetHouse"));
	BlackboardComp->ClearValue(FName("TargetHouseEntrance"));
    
	return EBTNodeResult::Succeeded;
}