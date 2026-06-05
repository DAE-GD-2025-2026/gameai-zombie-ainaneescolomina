#include "BTTask_LootHouseNeesAina.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Village/House/House.h"
#include "AIController.h"

void UBTTask_LootHouseNeesAina::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	AHouse* House = BlackboardComp ? Cast<AHouse>(BlackboardComp->GetValueAsObject(FName("TargetHouse"))) : nullptr;

	if (!House)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	ResetWeights();
	FTargetData Target;

	FHouseBounds Bounds = House->GetBounds();

	FVector WorldMin = Bounds.Origin - Bounds.Extent;
	FVector WorldMax = Bounds.Origin + Bounds.Extent;

	if (BlackboardComp->IsVectorValueSet(FName("TargetHouseEntrance")))
	{
		FVector EntranceLoc = BlackboardComp->GetValueAsVector(FName("TargetHouseEntrance"));
		Target.Position = FVector2D(EntranceLoc.X, EntranceLoc.Y);
	}
	else
	{
		Target.Position = FVector2D(Bounds.Origin.X, Bounds.Origin.Y);
	}

	SeekBehavior->SetTarget(Target);
	SetWeight(SeekBehavior, 1.0f);

	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	APawn* Survivor = OwnerComp.GetAIOwner()->GetPawn();
	if (Survivor)
	{
		FVector SurvivorLoc = Survivor->GetActorLocation();

		if (SurvivorLoc.X >= WorldMin.X && SurvivorLoc.X <= WorldMax.X &&
			SurvivorLoc.Y >= WorldMin.Y && SurvivorLoc.Y <= WorldMax.Y)
		{
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		}
	}
}