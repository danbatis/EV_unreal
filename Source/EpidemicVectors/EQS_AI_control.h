// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "EQS_AI_control.generated.h"

/**
 * 
 */
UCLASS()
class EPIDEMICVECTORS_API AEQS_AI_control : public AAIController
{
	GENERATED_BODY()
	
private:
	/** BlackboardComponent - used to initialize blackboard values and set/get values from a blackboard asset */
	UBlackboardComponent* BlackboardComp;

	/** BehaviorTreeComponent - used to start a behavior tree */
	UBehaviorTreeComponent* BehaviorTreeComp;

	/** Blackboard Key Value Name */
	const FName BlackboardEnemyKey = FName("EnemyActor");

	/** The function that fires when the perception of our AI gets updated */
	UFUNCTION()
		void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);
	//FPerceptionUpdatedDelegate(const TArray<AActor*>&, UpdatedActors);
	
	/** A Sight Sense config for our AI */
	UPROPERTY(VisibleAnywhere) UAISenseConfig_Sight* Sight;

protected:

	/** The Behavior Tree that contains the logic of our AI */
	UPROPERTY(EditAnywhere)
		UBehaviorTree* BehaviorTree;

	/** The Perception Component of our AI */
	UPROPERTY(VisibleAnywhere)
		UAIPerceptionComponent* AIPerceptionComponent;
public:
	AEQS_AI_control();

	virtual void Possess(APawn* InPawn) override;

	/** Returns the seeing pawn. Returns null, if our AI has no target */
	AActor* GetSeeingPawn();	
};
