#include "SteeringTask.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Survivor/SurvivorPawn.h"
#include "Common/StaminaComponent.h"

USteeringTask::USteeringTask()
{
    NodeName = "Execute Blended Steering";
    bNotifyTick = true; // Ensures TickTask triggers every single frame

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
    UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent();
    if (!AIController || !BBComp) return;

    APawn* Pawn = AIController->GetPawn();
    if (!Pawn) return;
    //ESurvivorSteeringState CurrentState = static_cast<ESurvivorSteeringState>(BBComp->GetValueAsEnum(FName("SteeringState")));

    // Clear out all weights
    *BlendedEngine->GetWeight(SeekBehavior) = 0.0f;
    *BlendedEngine->GetWeight(FleeBehavior) = 0.0f;
    *BlendedEngine->GetWeight(EvadeBehavior) = 0.0f;
    *BlendedEngine->GetWeight(WanderBehavior) = 0.0f;

    // 3. Gather target variables from the Blackboard depending on the chosen State
    FTargetData Target;
    bool bThreatActive = false;

    switch (CurrentState)
    {
        case ESurvivorSteeringState::FleeEnemy:
        {
            AActor* Enemy = Cast<AActor>(BBComp->GetValueAsObject(FName("TargetEnemy")));
            if (Enemy)
            {
                Target.Position = FVector2D(Enemy->GetActorLocation().X, Enemy->GetActorLocation().Y);
                Target.LinearVelocity = FVector2D(Enemy->GetVelocity().X, Enemy->GetVelocity().Y);
                bThreatActive = true;
            }
            
            // Set the active fleeing weights seamlessly
            FleeBehavior->SetTarget(Target);
            EvadeBehavior->SetTarget(Target);
            *BlendedEngine->GetWeight(FleeBehavior) = 0.80f;
            *BlendedEngine->GetWeight(WanderBehavior) = 0.20f;
            break;
        }

        case ESurvivorSteeringState::SeekItem:
        case ESurvivorSteeringState::LootingHouse:
        {
            AActor* Item = Cast<AActor>(BBComp->GetValueAsObject(FName("TargetItem")));
            if (Item)
            {
                Target.Position = FVector2D(Item->GetActorLocation().X, Item->GetActorLocation().Y);
            }
            else if (BBComp->IsVectorValueSet(FName("TargetLocation")))
            {
                FVector Loc = BBComp->GetValueAsVector(FName("TargetLocation"));
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

    // 4. Calculate the Blended Physics output from your modular classes
    SteeringOutput Output = BlendedEngine->CalculateSteering(DeltaSeconds, Pawn);

    // 5. Handle Context-Driven Sprinting (Only use stamina when necessary)
    if (ASurvivorPawn* Survivor = Cast<ASurvivorPawn>(Pawn))
    {
        bool bWantSprint = false;
        if (bThreatActive)
        {
            bWantSprint = BBComp->GetValueAsBool(FName("IsHeavyZombie")) || 
                          BBComp->GetValueAsBool(FName("IsRunnerZombie")) || 
                          BBComp->GetValueAsBool(FName("IsInPurgeZone"));
        }
        
        // Safety check to ensure we have stamina left to run
        if (UStaminaComponent* Stamina = Survivor->FindComponentByClass<UStaminaComponent>())
        {
            if (Stamina->GetCurrentStamina() <= 0.f) bWantSprint = false;
        }
        
        bWantSprint ? Survivor->StartRunning() : Survivor->StopRunning();
    }

    // 6. Apply Movement Input and Align Character Looking Rotation
    if (Output.IsValid && !Output.LinearVelocity.IsNearlyZero())
    {
        FVector Direction3D = FVector(Output.LinearVelocity.X, Output.LinearVelocity.Y, 0.f);
        Direction3D.Normalize();

        Pawn->AddMovementInput(Direction3D, 1.0f);

        // Turn smoothly toward our target movement vector direction
        FRotator TargetRot = Direction3D.Rotation();
        FRotator SmoothRot = FMath::RInterpTo(Pawn->GetActorRotation(), TargetRot, DeltaSeconds, 8.5f);
        Pawn->SetActorRotation(SmoothRot);
    }
}