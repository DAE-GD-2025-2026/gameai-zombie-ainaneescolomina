#pragma once
#include <vector>
#include "SteeringBehaviors.h"

class BlendedSteering final : public ISteeringBehavior
{
public:
	struct WeightedBehavior
	{
		ISteeringBehavior* pBehavior = nullptr;
		float Weight = 0.f;

		WeightedBehavior(ISteeringBehavior* const pBehavior, float Weight) 
		   : pBehavior(pBehavior), Weight(Weight) {};
	};

	BlendedSteering(const std::vector<WeightedBehavior>& WeightedBehaviors);
	virtual ~BlendedSteering() override = default;

	virtual SteeringOutput CalculateSteering(float DeltaT, APawn* Agent) override;
	float* GetWeight(ISteeringBehavior* const SteeringBehavior);

private:
	std::vector<WeightedBehavior> WeightedBehaviors = {};
};