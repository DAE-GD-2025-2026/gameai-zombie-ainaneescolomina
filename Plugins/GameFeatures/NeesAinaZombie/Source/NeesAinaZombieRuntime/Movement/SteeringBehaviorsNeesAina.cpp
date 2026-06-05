#include "SteeringBehaviorsNeesAina.h"

// SEEK
SteeringOutput Seek::CalculateSteering(float DeltaT, APawn* Agent)
{
    SteeringOutput Steering{};
    if (!Agent) return Steering;

    FVector Loc = Agent->GetActorLocation();
    Steering.LinearVelocity = Target.Position - FVector2D(Loc.X, Loc.Y);
    Steering.IsValid = true;
    return Steering;
}

// FLEE
SteeringOutput Flee::CalculateSteering(float DeltaT, APawn* Agent)
{
    SteeringOutput Steering{};
    if (!Agent) return Steering;

    FVector Loc = Agent->GetActorLocation();
    Steering.LinearVelocity = FVector2D(Loc.X, Loc.Y) - Target.Position;
    Steering.IsValid = true;
    return Steering;
}

// EVADE
SteeringOutput Evade::CalculateSteering(float DeltaT, APawn* Agent)
{
    SteeringOutput Steering{};
    if (!Agent) return Steering;

    FVector Loc = Agent->GetActorLocation();
    FVector2D AgentPos = FVector2D(Loc.X, Loc.Y);
    FVector2D TargetPos = Target.Position;
    
    if (EvadeRadius > 0.f)
    {
       if (FVector2D::DistSquared(AgentPos, TargetPos) > (EvadeRadius * EvadeRadius))
       {
          Steering.IsValid = false;
          return Steering;
       }
    }

    float TimeToTarget = FVector2D::Distance(AgentPos, TargetPos) / 400.0f;
    TimeToTarget = FMath::Clamp(TimeToTarget, 0.f, 1.5f);

    FVector2D PredictedTargetPos = Target.Position + Target.LinearVelocity * TimeToTarget;

    Steering.LinearVelocity = AgentPos - PredictedTargetPos;
    Steering.IsValid = true;
    return Steering;
}

// WANDER
SteeringOutput Wander::CalculateSteering(float DeltaT, APawn* Agent)
{
    SteeringOutput Steering{};
    if (!Agent) return Steering;

    FVector Loc = Agent->GetActorLocation();
    FVector2D AgentPos = FVector2D(Loc.X, Loc.Y);
    
    FVector Forward3D = Agent->GetActorForwardVector();
    FVector2D Forward = FVector2D(Forward3D.X, Forward3D.Y); 
    if (Forward.IsNearlyZero())
    {
        Forward = FVector2D(1.f, 0.f);
    }
    Forward.Normalize();

    FVector2D CircleCenter = AgentPos + Forward * WanderDistanceFromCenter;
    
    WanderAngle += FMath::FRandRange(-0.4f, 0.4f);

    FVector2D TargetOffset = FVector2D(FMath::Cos(WanderAngle), FMath::Sin(WanderAngle)) * WanderRadius;
    FVector2D WanderTarget = CircleCenter + TargetOffset;

    Steering.LinearVelocity = WanderTarget - AgentPos;
    Steering.IsValid = true;
    return Steering;
}