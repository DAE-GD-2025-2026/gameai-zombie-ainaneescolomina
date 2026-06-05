#include "BTTask_WanderNeesAina.h"
#include "StudentPerceptorNeesAina.h"
#include "Village/House/House.h"
#include "AIController.h"

void UBTTask_WanderNeesAina::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	ResetWeights();
	
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (AIController)
	{
		if (APawn* Survivor = AIController->GetPawn())
		{
			if (auto Perceptor = Survivor->FindComponentByClass<UStudentPerceptorNeesAina>())
			{
				FVector MyLoc = Survivor->GetActorLocation();
				AHouse* CuriosityHouse = Perceptor->GetClosestUnlootedHouse(MyLoc, 3500.f);

				if (CuriosityHouse)
				{
					FTargetData HouseTarget;
					HouseTarget.Position = FVector2D(CuriosityHouse->GetActorLocation().X, CuriosityHouse->GetActorLocation().Y);
					SeekBehavior->SetTarget(HouseTarget);

					SetWeight(WanderBehavior, 0.75f);
					SetWeight(SeekBehavior, 0.25f); 
                    
					Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);
					return;
				}
			}
		}
	}
	
	SetWeight(WanderBehavior, 1.0f);
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);
		
}
