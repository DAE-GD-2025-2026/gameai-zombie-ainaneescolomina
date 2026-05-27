#include "BlendedSteering.h"
#include <algorithm>
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"

BlendedSteering::BlendedSteering(const std::vector<WeightedBehavior>& WeightedBehaviors)
    : WeightedBehaviors(WeightedBehaviors)
{};

SteeringOutput BlendedSteering::CalculateSteering(float DeltaT, APawn* Agent)
{
    SteeringOutput BlendedOutput = {};
    BlendedOutput.IsValid = false;
    
    float TotalWeight = 0.f;
    
    for (const WeightedBehavior& behavior : WeightedBehaviors)
    {
       if (!behavior.pBehavior || behavior.Weight <= 0.f) continue;

       SteeringOutput steering = behavior.pBehavior->CalculateSteering(DeltaT, Agent);
       if (!steering.IsValid) continue;

       if (!steering.LinearVelocity.IsNearlyZero())
       {
          steering.LinearVelocity.Normalize();
       }
       
       BlendedOutput.LinearVelocity += steering.LinearVelocity * behavior.Weight;
       BlendedOutput.AngularVelocity += steering.AngularVelocity * behavior.Weight;

       TotalWeight += behavior.Weight;
       BlendedOutput.IsValid = true;
    }
    
    // Divide vectors by total accumulated weights to secure true blended direction
    if (BlendedOutput.IsValid && TotalWeight > 0.f)
    {
        BlendedOutput.LinearVelocity /= TotalWeight;
        BlendedOutput.AngularVelocity /= TotalWeight;
    }

    // Clamp speed limits safely to the Character's Max Walk Capabilities
    if (Agent)
    {
        if (ACharacter* Char = Cast<ACharacter>(Agent))
        {
            float MaxSpeed = Char->GetCharacterMovement()->MaxWalkSpeed;
            BlendedOutput.LinearVelocity = BlendedOutput.LinearVelocity.GetClampedToMaxSize(MaxSpeed);
        }
    }
    
    return BlendedOutput;
}

float* BlendedSteering::GetWeight(ISteeringBehavior* const SteeringBehavior)
{
    auto it = std::find_if(WeightedBehaviors.begin(), WeightedBehaviors.end(),
       [SteeringBehavior](const WeightedBehavior& Elem) { return Elem.pBehavior == SteeringBehavior; }
    );
    if (it != WeightedBehaviors.end()) return &it->Weight;
    return nullptr;
}
