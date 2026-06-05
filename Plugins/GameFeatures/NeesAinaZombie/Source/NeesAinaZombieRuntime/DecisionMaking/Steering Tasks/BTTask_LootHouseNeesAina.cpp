#include "BTTask_LootHouseNeesAina.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Village/House/House.h"
#include "AIController.h"
#include "StudentPerceptorNeesAina.h"

EBTNodeResult::Type UBTTask_LootHouseNeesAina::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    Super::ExecuteTask(OwnerComp, NodeMemory);

    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    APawn* Survivor = OwnerComp.GetAIOwner() ? OwnerComp.GetAIOwner()->GetPawn() : nullptr;
    AHouse* House = BlackboardComp ? Cast<AHouse>(BlackboardComp->GetValueAsObject(FName("TargetHouse"))) : nullptr;

    if (!House || !Survivor) return EBTNodeResult::Failed;

    FVector ChosenEntrance = House->GetBounds().Origin;
    if (UStudentPerceptorNeesAina* Perceptor = Survivor->FindComponentByClass<UStudentPerceptorNeesAina>())
    {
        if (FHouseData* Data = Perceptor->GetHouseData(House))
        {
            if (Data->DiscoveredEntrances.Num() > 0)
            {
                ChosenEntrance = Data->DiscoveredEntrances[0];
            }
        }
    }

    CurrentTaskTarget = ChosenEntrance;
    return EBTNodeResult::InProgress;
}

void UBTTask_LootHouseNeesAina::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    APawn* Survivor = OwnerComp.GetAIOwner() ? OwnerComp.GetAIOwner()->GetPawn() : nullptr;
    AHouse* House = BlackboardComp ? Cast<AHouse>(BlackboardComp->GetValueAsObject(FName("TargetHouse"))) : nullptr;

    if (!House || !Survivor || !BlackboardComp)
    {
       FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
       return;
    }

    FHouseBounds Bounds = House->GetBounds();
    FVector WorldMin = Bounds.Origin - Bounds.Extent;
    FVector WorldMax = Bounds.Origin + Bounds.Extent;
    FVector SurvivorLoc = Survivor->GetActorLocation();
    
    if (SurvivorLoc.X >= WorldMin.X && SurvivorLoc.X <= WorldMax.X &&
        SurvivorLoc.Y >= WorldMin.Y && SurvivorLoc.Y <= WorldMax.Y)
    {
        if (UStudentPerceptorNeesAina* Perceptor = Survivor->FindComponentByClass<UStudentPerceptorNeesAina>())
        {
            if (FHouseData* Data = Perceptor->GetHouseData(House))
            {
                Data->bHasBeenLooted = true; 
                Data->DiscoveredEntrances.AddUnique(SurvivorLoc);
            }
        }
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        return;
    }
    
    if (DetourDuration <= 0.0f)
    {
        FVector ChosenEntrance = Bounds.Origin;
        if (UStudentPerceptorNeesAina* Perceptor = Survivor->FindComponentByClass<UStudentPerceptorNeesAina>())
        {
            if (FHouseData* Data = Perceptor->GetHouseData(House))
            {
                float MinDistSq = MAX_FLT;
                for (const FVector& DoorPos : Data->DiscoveredEntrances)
                {
                    float DistSq = FVector::DistSquared(SurvivorLoc, DoorPos);
                    if (DistSq < MinDistSq)
                    {
                        MinDistSq = DistSq;
                        ChosenEntrance = DoorPos;
                    }
                }
            }
        }
        
        // closest door position
        CurrentTaskTarget = ChosenEntrance;
        
        ResetWeights();
        SetWeight(SeekBehavior, 1.0f);
    }

    Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);
}