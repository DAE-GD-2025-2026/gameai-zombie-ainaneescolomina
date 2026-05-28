#include "SteeringTask.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Survivor/SurvivorPawn.h"
#include "Common/StaminaComponent.h"

USteeringTask::USteeringTask()
{
    NodeName = "Execute Blended Steering";
    bNotifyTick = true;

    InitializeSteering();
}

void USteeringTask::InitializeSteering()
{
    SeekBehavior = new Seek();
    FleeBehavior = new Flee();
    EvadeBehavior = new Evade();
    WanderBehavior = new Wander();
    EvadeBehavior->SetEvadeRadius(750.f);

    std::vector<BlendedSteering::WeightedBehavior> BehaviorsList = {
        BlendedSteering::WeightedBehavior(SeekBehavior, 0.0f),
        BlendedSteering::WeightedBehavior(FleeBehavior, 0.0f),
        BlendedSteering::WeightedBehavior(EvadeBehavior, 0.0f),
        BlendedSteering::WeightedBehavior(WanderBehavior, 0.0f)
    };
    BlendedEngine = MakeUnique<BlendedSteering>(BehaviorsList);
}

EBTNodeResult::Type USteeringTask::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    return EBTNodeResult::InProgress;
}

void USteeringTask::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!AIController || !BlackboardComp) return;

    APawn* Survivor = AIController->GetPawn();
    if (!Survivor) return;

    // Clear out all weights
    *BlendedEngine->GetWeight(SeekBehavior) = 0.0f;
    *BlendedEngine->GetWeight(FleeBehavior) = 0.0f;
    *BlendedEngine->GetWeight(EvadeBehavior) = 0.0f;
    *BlendedEngine->GetWeight(WanderBehavior) = 0.0f;

    FTargetData Target;
    bool UnderAttack = false;
    bool IsSeekingTarget = false;
    bool InPurgeZone = BlackboardComp->GetValueAsBool(FName("InPurgeZone"));
    
    switch (CurrentState)
    {
        case ESurvivorSteeringState::FleeEnemy:
        {
            AActor* Enemy = Cast<AActor>(BlackboardComp->GetValueAsObject(FName("TargetEnemy")));
            if (Enemy)
            {
                Target.Position = FVector2D(Enemy->GetActorLocation().X, Enemy->GetActorLocation().Y);
                Target.LinearVelocity = FVector2D(Enemy->GetVelocity().X, Enemy->GetVelocity().Y);
                UnderAttack = true;
            }
            
            FleeBehavior->SetTarget(Target);
            EvadeBehavior->SetTarget(Target);
            *BlendedEngine->GetWeight(FleeBehavior) = 0.80f;
            *BlendedEngine->GetWeight(WanderBehavior) = 0.20f;
            break;
        }
        
        case ESurvivorSteeringState::FleePurgeZone:
        {
            if (BlackboardComp->IsVectorValueSet(FName("PurgeZoneLocation")))
            {
                FVector ZoneLoc = BlackboardComp->GetValueAsVector(FName("PurgeZoneLocation"));
                Target.Position = FVector2D(ZoneLoc.X, ZoneLoc.Y);
            }
        
            FleeBehavior->SetTarget(Target);
            *BlendedEngine->GetWeight(FleeBehavior) = 1.0f; 
            break;
        }

        case ESurvivorSteeringState::SeekItem:
        case ESurvivorSteeringState::LootingHouse:
        {
            IsSeekingTarget = true;
            AActor* Item = Cast<AActor>(BlackboardComp->GetValueAsObject(FName("TargetItem")));
            if (Item)
            {
                Target.Position = FVector2D(Item->GetActorLocation().X, Item->GetActorLocation().Y);
            }
            else if (BlackboardComp->IsVectorValueSet(FName("TargetLocation")))
            {
                FVector Loc = BlackboardComp->GetValueAsVector(FName("TargetLocation"));
                Target.Position = FVector2D(Loc.X, Loc.Y);
            }

            SeekBehavior->SetTarget(Target);
            *BlendedEngine->GetWeight(SeekBehavior) = 1.0f;
            break;
        }

        case ESurvivorSteeringState::Wander:
        default:
        {
            *BlendedEngine->GetWeight(WanderBehavior) = 1.0f;
            break;
        }
    }

    SteeringOutput Output = BlendedEngine->CalculateSteering(DeltaSeconds, Survivor);

    if (ASurvivorPawn* SurvivorPawn = Cast<ASurvivorPawn>(Survivor))
    {
        bool NeedSprint = false;
        if (UnderAttack || InPurgeZone)
        {
            NeedSprint = BlackboardComp->GetValueAsBool(FName("IsHeavyZombie")) || 
                          BlackboardComp->GetValueAsBool(FName("IsRunnerZombie")) || 
                          InPurgeZone;
        }
        
        if (UStaminaComponent* Stamina = SurvivorPawn->FindComponentByClass<UStaminaComponent>())
        {
            if (Stamina->GetCurrentStamina() <= 0.f) NeedSprint = false;
        }
        
        NeedSprint ? SurvivorPawn->StartRunning() : SurvivorPawn->StopRunning();
    }

    if (Output.IsValid && !Output.LinearVelocity.IsNearlyZero())
    {
        FVector Direction3D = FVector(Output.LinearVelocity.X, Output.LinearVelocity.Y, 0.f);
        Direction3D.Normalize();

        Survivor->AddMovementInput(Direction3D, 1.0f);

        FRotator TargetRot = Direction3D.Rotation();
        FRotator SmoothRot = FMath::RInterpTo(Survivor->GetActorRotation(), TargetRot, DeltaSeconds, 8.5f);
        Survivor->SetActorRotation(SmoothRot);
    }
}