#include "BTTask_BaseSteeringNeesAina.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Survivor/SurvivorPawn.h"
#include "Common/StaminaComponent.h"

UBTTask_BaseSteeringNeesAina::UBTTask_BaseSteeringNeesAina()
{
    bNotifyTick = true;
    InitializeSteering();
}

void UBTTask_BaseSteeringNeesAina::InitializeSteering()
{
    SeekBehavior = new Seek();
    FleeBehavior = new Flee();
    EvadeBehavior = new Evade();
    WanderBehavior = new Wander();
    EvadeBehavior->SetEvadeRadius(750.f);

    std::vector<BlendedSteeringNeesAina::WeightedBehavior> BehaviorsList = {
        BlendedSteeringNeesAina::WeightedBehavior(SeekBehavior, 0.0f),
        BlendedSteeringNeesAina::WeightedBehavior(FleeBehavior, 0.0f),
        BlendedSteeringNeesAina::WeightedBehavior(EvadeBehavior, 0.0f),
        BlendedSteeringNeesAina::WeightedBehavior(WanderBehavior, 0.0f)
    };
    BlendedEngine = MakeUnique<BlendedSteeringNeesAina>(BehaviorsList);
}

void UBTTask_BaseSteeringNeesAina::ResetWeights()
{
    *BlendedEngine->GetWeight(SeekBehavior) = 0.0f;
    *BlendedEngine->GetWeight(FleeBehavior) = 0.0f;
    *BlendedEngine->GetWeight(EvadeBehavior) = 0.0f;
    *BlendedEngine->GetWeight(WanderBehavior) = 0.0f;
}

void UBTTask_BaseSteeringNeesAina::SetWeight(ISteeringBehavior* Behavior, float Weight)
{
    if (Behavior)
    {
        *BlendedEngine->GetWeight(Behavior) = Weight;
    }
}

EBTNodeResult::Type UBTTask_BaseSteeringNeesAina::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    return EBTNodeResult::InProgress;
}

void UBTTask_BaseSteeringNeesAina::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!AIController || !BlackboardComp) return;

    APawn* Survivor = AIController->GetPawn();
    if (!Survivor) return;

    SteeringOutput Output = BlendedEngine->CalculateSteering(DeltaSeconds, Survivor);

    // Dynamic Sprint Controls
    if (ASurvivorPawn* SurvivorPawn = Cast<ASurvivorPawn>(Survivor))
    {
        bool UnderAttack = BlackboardComp->GetValueAsObject(FName("TargetEnemy")) != nullptr;
        bool InPurgeZone = BlackboardComp->GetValueAsBool(FName("InPurgeZone"));
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

    // Process Movement
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