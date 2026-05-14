// Fill out your copyright notice in the Description page of Project Settings.


#include "StudentPerceptor.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UStudentPerceptor::UStudentPerceptor()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UStudentPerceptor::BeginPlay()
{
	Super::BeginPlay();
	
	if (auto PerceptionComp = GetOwner()->GetComponentByClass<UAIPerceptionComponent>())
	{
		PerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &UStudentPerceptor::OnPerceptionUpdated);
	}
}

void UStudentPerceptor::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	bool bIsSensed = Stimulus.WasSuccessfullySensed();
	
	GEngine->AddOnScreenDebugMessage(5, 1.f, FColor::Green, 
	FString::Printf(TEXT("Saw Something!")));
	
	// get AI Controller
	AAIController* AIController = Cast<AAIController>(GetOwner());
	if (!AIController)
	{
		if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
		{
			AIController = Cast<AAIController>(OwnerPawn->GetController());
		}
	}

	// DEBUG - have we found the AI Controller
	if (!AIController)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("ERROR: Cannot find AI Controller!"));
		return;
	}
	
	// DEBUG - have we found the Blackboard Component
	UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent();
	if (!BlackboardComp)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("ERROR: Cannot find Blackboard Component!"));
		return;
	}
	
	if (Actor->ActorHasTag("Zombie")) {
		BlackboardComp->SetValueAsObject("SeenEnemy", bIsSensed ? Actor : nullptr);
	} 
	else if (Actor->ActorHasTag("Item")) {
		BlackboardComp->SetValueAsObject("SeenItem", bIsSensed ? Actor : nullptr);
	}
	else if (Actor->ActorHasTag("House")) {
		BlackboardComp->SetValueAsObject("SeenHouse", bIsSensed ? Actor : nullptr);
	}
	else if (Actor->ActorHasTag("PurgeZone")) {
		BlackboardComp->SetValueAsBool("InPurgeZone", bIsSensed);
		BlackboardComp->SetValueAsObject("SeenPurgeZone", bIsSensed ? Actor : nullptr);
	}
}
