// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_MoveToDestination.generated.h"

/**
 * 
 */
UCLASS()
class EPIDEMICVECTORS_API UBTTask_MoveToDestination : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool flying;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float tolerance = 20.0f;
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;	
};
