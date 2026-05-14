#include "FindRandomLocationTask.h"

#include "AIController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Actor.h"

UFindRandomLocationTask::UFindRandomLocationTask()
{
	NodeName = "Find Random Location";
}

EBTNodeResult::Type UFindRandomLocationTask::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;
	
	APawn* ControlledPawn = AIController->GetPawn();
	if (!ControlledPawn) return EBTNodeResult::Failed;
	
	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSystem) return EBTNodeResult::Failed;

	FVector MyLocation = ControlledPawn->GetActorLocation();
	FNavLocation RandomPoint;

	bool bFoundPoint =
		NavSystem->GetRandomReachablePointInRadius(
			MyLocation,
			SearchRadius,
			RandomPoint
		);

	if (!bFoundPoint) return EBTNodeResult::Failed;

	OwnerComp.GetBlackboardComponent()->SetValueAsVector(
		TargetLocationKey.SelectedKeyName,
		RandomPoint.Location
	);

	return EBTNodeResult::Succeeded;
}