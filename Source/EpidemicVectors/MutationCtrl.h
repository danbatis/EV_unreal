// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"

//#include "Perception/AIPerceptionComponent.h"
//#include "Perception/AISenseConfig_Sight.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "MutationChar.h"
#include "MutationCtrl.generated.h"

/**
 *
 */

class AMutationChar;

UCLASS()
class EPIDEMICVECTORS_API AMutationCtrl : public AAIController
{
	GENERATED_BODY()

protected:
	/*A Behavior tree component in order to be able to call specific functions like starting our BT*/
	UBehaviorTreeComponent* BehaviorTreeComp;

	/*A Blackboard component which will be used to initialize our Blackboard Values*/
	UBlackboardComponent* BlackboardComp;
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite) AMutationChar* Char;

	/*Default Constructor*/
	AMutationCtrl();

	/*Called when the AI Controller possesses a Pawn*/
	virtual void Possess(APawn* InPawn) override;

	/*Sets the new sensed target value inside our Blackboard values*/
	void SetSensedTarget(APawn* NewTarget);
	void SetDonePath(bool DonePath);
	void SetAirborne(bool Airborne);
	void SetTargetVisible(bool Visible);
	void SetTargetLocated(bool Located);
	void SetGoalInAir(bool InAir);
	void SetGoal(FVector GoalLocation);
	void SetCanFly(bool CanFly);
	void SetInFightRange(bool InFightRange);
	void SetDesperate(bool Desperate);
	void SetReachedGoal(bool Reached);
	void SetBlindSearch(bool Active);
	
	FVector GetGoal();

	void StopBT();
	void RestartBT();

	/** Returns the seeing pawn. Returns null, if our AI has no target */
	AActor* GetSeeingPawn();
};
