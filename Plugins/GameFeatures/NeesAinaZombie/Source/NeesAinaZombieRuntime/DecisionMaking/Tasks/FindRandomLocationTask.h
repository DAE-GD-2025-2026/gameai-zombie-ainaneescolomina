// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "FindRandomLocationTask.generated.h"

UCLASS()
class NEESAINAZOMBIERUNTIME_API UFindRandomLocationTask : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UFindRandomLocationTask();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

public:
	UPROPERTY(EditAnywhere, Category = "Search")
	float SearchRadius = 500.f;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetLocationKey;
};


