#include "FindSafeLocationTask.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Actor.h"

UFindSafeLocationTask::UFindSafeLocationTask()
{
    NodeName = "Find Safe Location";
}

EBTNodeResult::Type UFindSafeLocationTask::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!AIController || !BlackboardComp) return EBTNodeResult::Failed;

    APawn* ControlledPawn = AIController->GetPawn();
    if (!ControlledPawn) return EBTNodeResult::Failed;
    
    UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!NavSystem) return EBTNodeResult::Failed;

    FVector MyLocation = ControlledPawn->GetActorLocation();
    FVector DangerLocation = FVector::ZeroVector;
    bool bHasDanger = false;

    // Move away from Purge Zone
    if (BlackboardComp->GetValueAsBool("InPurgeZone"))
    {
        AActor* PurgeZone = Cast<AActor>(BlackboardComp->GetValueAsObject("SeenPurgeZone"));
        if (PurgeZone)
        {
            DangerLocation = PurgeZone->GetActorLocation();
            bHasDanger = true;
        }
    }
    // Move away from Enemy
    else
    {
        AActor* Enemy = Cast<AActor>(BlackboardComp->GetValueAsObject("SeenEnemy"));
        if (Enemy)
        {
            DangerLocation = Enemy->GetActorLocation();
            bHasDanger = true;
        }
    }

    if (!bHasDanger) return EBTNodeResult::Failed;

    FVector FleeDirection = MyLocation - DangerLocation;
    FleeDirection.Normalize();

    FVector SafeLocation = MyLocation + (FleeDirection * SearchRadius);

    FNavLocation SafePoint;
    if (NavSystem->ProjectPointToNavigation(SafeLocation, SafePoint, FVector(500.f, 500.f, 500.f)))
    {
        BlackboardComp->SetValueAsVector(TargetLocationKey.SelectedKeyName, SafePoint.Location);
        return EBTNodeResult::Succeeded;
    }

    return EBTNodeResult::Failed;
}