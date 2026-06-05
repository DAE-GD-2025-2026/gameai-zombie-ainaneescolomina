#pragma once
class ASteeringAgent;

struct SteeringOutput final
{
	FVector2D LinearVelocity{};
	float AngularVelocity{0.f};
	bool IsValid{true};

	SteeringOutput() = default;
	SteeringOutput(const FVector2D& linearVelocity, float angularVelocity = 0.f)
		: LinearVelocity(linearVelocity), AngularVelocity(angularVelocity) {}
};

struct FSteeringParams
{
	FVector2D Position{};
	FVector2D LinearVelocity{};
};
using FTargetData = FSteeringParams;

// SteeringBehavior base, all steering behaviors should derive from this.
class ISteeringBehavior
{
public:
	ISteeringBehavior() = default;
	virtual ~ISteeringBehavior() = default;

	// Override to implement your own behavior
	virtual SteeringOutput CalculateSteering(float DeltaT, APawn* Agent) = 0;

	void SetTarget(const FTargetData& NewTarget) { Target = NewTarget; }
	
	template<class T, std::enable_if_t<std::is_base_of_v<ISteeringBehavior, T>>* = nullptr>
	T* As()
	{ return static_cast<T*>(this); }

protected:
	FTargetData Target;
};


// Your own SteeringBehaviors should follow here...
class Seek : public ISteeringBehavior
{
public:
	Seek() = default;
	virtual ~Seek() override = default;

	// steering
	virtual SteeringOutput CalculateSteering(float DeltaT, APawn* Agent) override;
};

class Flee : public ISteeringBehavior
{
public:
	Flee() = default;
	virtual ~Flee() override = default;

	// steering
	virtual SteeringOutput CalculateSteering(float DeltaT, APawn* Agent) override;
};

class Evade : public ISteeringBehavior
{
public:
	Evade() = default;
	virtual ~Evade() override = default;

	// steering
	virtual SteeringOutput CalculateSteering(float DeltaT, APawn* Agent) override;
	void SetEvadeRadius(float Radius) { EvadeRadius = Radius; }
	
private:
	float EvadeRadius = -1.f;
};

class Wander : public ISteeringBehavior
{
public:
	Wander() = default;
	virtual ~Wander() override = default;

	// steering
	virtual SteeringOutput CalculateSteering(float DeltaT, APawn* Agent) override;

private:
	float WanderRadius = 200.f;
	float WanderDistanceFromCenter = 150.f;
	float WanderAngle = 0.f;
};