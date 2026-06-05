#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "VisitHouseTaskNeesAina.generated.h"

UCLASS()
class NEESAINAZOMBIERUNTIME_API UVisitHouseTaskNeesAina : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UVisitHouseTaskNeesAina();
	
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};