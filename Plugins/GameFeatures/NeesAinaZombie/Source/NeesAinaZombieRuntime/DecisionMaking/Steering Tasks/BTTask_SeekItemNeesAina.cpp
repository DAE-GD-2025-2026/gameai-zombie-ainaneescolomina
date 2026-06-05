#include "BTTask_SeekItemNeesAina.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

EBTNodeResult::Type UBTTask_SeekItemNeesAina::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	AActor* Item = BlackboardComp ? Cast<AActor>(BlackboardComp->GetValueAsObject(FName("TargetItem"))) : nullptr;

	if (!Item) return EBTNodeResult::Failed;

	CurrentTaskTarget = Item->GetActorLocation();
    
	return EBTNodeResult::InProgress;
}

void UBTTask_SeekItemNeesAina::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	AActor* Item = BlackboardComp ? Cast<AActor>(BlackboardComp->GetValueAsObject(FName("TargetItem"))) : nullptr;

	if (!Item)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	CurrentTaskTarget = Item->GetActorLocation();
    
	ResetWeights();
	SetWeight(SeekBehavior, 1.0f);

	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	APawn* Survivor = OwnerComp.GetAIOwner()->GetPawn();
	if (Survivor)
	{
		FVector2D CurrentPos = FVector2D(Survivor->GetActorLocation());
		FVector2D GoalTarget = FVector2D(CurrentTaskTarget.X, CurrentTaskTarget.Y);
       
		if (FVector2D::DistSquared(CurrentPos, GoalTarget) <= 10000.f)
		{
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		}
	}
}