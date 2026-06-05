#include "BTTask_WanderNeesAina.h"

void UBTTask_WanderNeesAina::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	ResetWeights();
	SetWeight(WanderBehavior, 1.0f);
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);
}