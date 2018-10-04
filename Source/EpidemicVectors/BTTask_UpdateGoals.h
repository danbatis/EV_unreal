// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"

#include "MutationCtrl.h"
#include "MutationChar.h"

#include "BTTask_UpdateGoals.generated.h"

/**
 * 
 */
UCLASS()
class EPIDEMICVECTORS_API UBTTask_UpdateGoals : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool flying;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) MutationStates targetstate;
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;	
};
