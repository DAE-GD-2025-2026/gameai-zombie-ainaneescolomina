#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "PickUpItemTaskNeesAina.generated.h"

UCLASS()
class NEESAINAZOMBIERUNTIME_API UPickUpItemTaskNeesAina : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UPickUpItemTaskNeesAina();
	
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};