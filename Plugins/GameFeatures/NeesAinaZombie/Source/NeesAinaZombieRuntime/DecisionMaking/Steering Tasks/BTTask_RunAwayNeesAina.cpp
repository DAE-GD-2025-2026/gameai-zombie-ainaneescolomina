#include "BTTask_RunAwayNeesAina.h"
#include "BehaviorTree/BlackboardComponent.h"

void UBTTask_RunAwayNeesAina::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp) return;

	ResetWeights();
	FTargetData Target;
	bool bHasThreat = false;

	AActor* Enemy = Cast<AActor>(BlackboardComp->GetValueAsObject(FName("TargetEnemy")));
	bool bInPurgeZone = BlackboardComp->GetValueAsBool(FName("InPurgeZone"));

	// Prioritize fleeing from Purge Zone
	if (bInPurgeZone && BlackboardComp->IsVectorValueSet(FName("PurgeZoneLocation")))
	{
		FVector ZoneLoc = BlackboardComp->GetValueAsVector(FName("PurgeZoneLocation"));
		Target.Position = FVector2D(ZoneLoc.X, ZoneLoc.Y);
            
		FleeBehavior->SetTarget(Target);
		SetWeight(FleeBehavior, 1.0f);
		bHasThreat = true;
	}
	else if (Enemy) 
	{
		Target.Position = FVector2D(Enemy->GetActorLocation().X, Enemy->GetActorLocation().Y);
		Target.LinearVelocity = FVector2D(Enemy->GetVelocity().X, Enemy->GetVelocity().Y);
            
		FleeBehavior->SetTarget(Target);
		EvadeBehavior->SetTarget(Target);
		SetWeight(FleeBehavior, 0.80f);
		SetWeight(WanderBehavior, 0.20f);
		bHasThreat = true;
	}

	if (!bHasThreat)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);
}