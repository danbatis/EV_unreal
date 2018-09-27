// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyAllTypes.h"

#include "PureCppAIControl.generated.h"

/**
 * 
 */
UCLASS()
class EPIDEMICVECTORS_API APureCppAIControl : public AAIController
{
	GENERATED_BODY()
	
	UPROPERTY(transient) UBlackboardComponent* BlackboardComp;
	UPROPERTY(transient) UBehaviorTreeComponent* BehaviorComp;

public:

	APureCppAIControl();

	/*Called when the AI Controller possesses a Pawn*/
	virtual void Possess(APawn* InPawn) override;

	uint8 EnemyKeyID;
	
};
