#pragma once
#include <vector>
#include "SteeringBehaviorsNeesAina.h"

class BlendedSteeringNeesAina final : public ISteeringBehavior
{
public:
	struct WeightedBehavior
	{
		ISteeringBehavior* pBehavior = nullptr;
		float Weight = 0.f;

		WeightedBehavior(ISteeringBehavior* const pBehavior, float Weight) 
		   : pBehavior(pBehavior), Weight(Weight) {};
	};

	BlendedSteeringNeesAina(const std::vector<WeightedBehavior>& WeightedBehaviors);
	virtual ~BlendedSteeringNeesAina() override = default;

	virtual SteeringOutput CalculateSteering(float DeltaT, APawn* Agent) override;
	float* GetWeight(ISteeringBehavior* const SteeringBehavior);

private:
	std::vector<WeightedBehavior> WeightedBehaviors = {};
};