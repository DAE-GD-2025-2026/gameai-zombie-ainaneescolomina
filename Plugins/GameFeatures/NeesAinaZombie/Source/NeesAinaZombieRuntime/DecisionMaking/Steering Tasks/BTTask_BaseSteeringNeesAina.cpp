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

void UBTTask_BaseSteeringNeesAina::ResetStuckState()
{
    StuckDuration = 0.0f;
    DetourDuration = 0.0f;
    DetourVector = FVector::ZeroVector;
    LastTrackedPos = FVector::ZeroVector;
}

EBTNodeResult::Type UBTTask_BaseSteeringNeesAina::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    ResetStuckState();    
    return EBTNodeResult::InProgress;
}

void UBTTask_BaseSteeringNeesAina::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!AIController || !BlackboardComp) return;

    APawn* Survivor = AIController->GetPawn();
    if (!Survivor) return;

    FVector SurvivorLoc = Survivor->GetActorLocation();

    // Check if agent is stuck
    bool bIsPhysicallyMoving = Survivor->GetVelocity().SizeSquared() > 100.0f; 

    if (!bIsPhysicallyMoving)
    {
        StuckDuration += DeltaSeconds;
    }
    else
    {
        StuckDuration = 0.0f;
        StuckAngleModifier = 0.0f;
    }
    
    if (DetourDuration > 0.0f)
    {
        DetourDuration -= DeltaSeconds;
        FTargetData DetourTarget;
        DetourTarget.Position = FVector2D(DetourVector.X, DetourVector.Y);
        SeekBehavior->SetTarget(DetourTarget);
        ResetWeights();
        SetWeight(SeekBehavior, 1.0f);
    }
    else
    {
        if (StuckDuration >= StuckThresholdTime)
        {
            if (StuckDuration >= (StuckThresholdTime * 3.0f))
            {
                StuckAngleModifier = 45.0f; // 135-degree turn out of tight corners
                GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Purple, TEXT("CRITICAL BLOCKAGE: Applying 135-degree turn!"));
            }
            else
            {
                StuckAngleModifier = 0.0f;  // Standard 90-degree sidestep
                GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Purple, TEXT("Agent Stuck: Applying 90-degree turn."));
            }

            FVector StraightHeading = (CurrentTaskTarget - SurvivorLoc).GetSafeNormal2D();
            float FinalTurnAngle = FMath::DegreesToRadians(90.0f + StuckAngleModifier);
            float RotatedX = StraightHeading.X * FMath::Cos(FinalTurnAngle) - StraightHeading.Y * FMath::Sin(FinalTurnAngle);
            float RotatedY = StraightHeading.X * FMath::Sin(FinalTurnAngle) + StraightHeading.Y * FMath::Cos(FinalTurnAngle);
            
            FVector EscapeDirection = FVector(RotatedX, RotatedY, 0.0f);
            EscapeDirection.Normalize();
            
            DetourVector = SurvivorLoc + (EscapeDirection * 200.0f);
            DetourDuration = 0.75f;

            FTargetData DetourTarget;
            DetourTarget.Position = FVector2D(DetourVector.X, DetourVector.Y);
            SeekBehavior->SetTarget(DetourTarget);
            ResetWeights();
            SetWeight(SeekBehavior, 1.0f);
        }
        else
        {
            FTargetData NormalTarget;
            NormalTarget.Position = FVector2D(CurrentTaskTarget.X, CurrentTaskTarget.Y);
            SeekBehavior->SetTarget(NormalTarget);
        }
    }
    
    SteeringOutput Output = BlendedEngine->CalculateSteering(DeltaSeconds, Survivor);

    // Check if need Sprint
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

    // Provess movement
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