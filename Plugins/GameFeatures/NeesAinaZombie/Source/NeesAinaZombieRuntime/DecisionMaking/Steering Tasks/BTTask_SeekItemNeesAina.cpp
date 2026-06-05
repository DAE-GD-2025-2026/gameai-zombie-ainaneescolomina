#include "BTTask_SeekItemNeesAina.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

void UBTTask_SeekItemNeesAina::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	AActor* Item = BlackboardComp ? Cast<AActor>(BlackboardComp->GetValueAsObject(FName("TargetItem"))) : nullptr;

	if (!Item)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	ResetWeights();
	FTargetData Target;
	Target.Position = FVector2D(Item->GetActorLocation().X, Item->GetActorLocation().Y);
        
	SeekBehavior->SetTarget(Target);
	SetWeight(SeekBehavior, 1.0f);

	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	APawn* Survivor = OwnerComp.GetAIOwner()->GetPawn();
	if (Survivor && FVector2D::DistSquared(FVector2D(Survivor->GetActorLocation()), Target.Position) <= 10000.f)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}