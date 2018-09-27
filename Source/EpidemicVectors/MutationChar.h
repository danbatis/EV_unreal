// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "Perception/PawnSensingComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "MutationCtrl.h"
#include "GameFramework/Controller.h"
#include "Runtime/Engine/Classes/Engine/TargetPoint.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"

#include "MyPlayerCharacter.h"

#include "Engine.h"

#include "MutationChar.generated.h"

//forward declaration to tell compiler the class exists
class AMutationCtrl;

UENUM(BlueprintType) enum class MutationStates:uint8 {
	idle		UMETA(DisplayName = "idle"),
	patrol		UMETA(DisplayName = "patrol"),
	investigate UMETA(DisplayName = "investigate"),
	fight		UMETA(DisplayName = "fight"),
	escape		UMETA(DisplayName = "escape"),
	pursuit		UMETA(DisplayName = "pursuit"),	
	flight		UMETA(DisplayName = "flight"),
	attacking	UMETA(DisplayName = "attacking"),
};

UCLASS()
class EPIDEMICVECTORS_API AMutationChar : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMutationChar();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/*A Pawn Sensing Component, responsible for sensing other Pawns*/
	UPROPERTY(VisibleAnywhere)
		UPawnSensingComponent* PawnSensingComp;

	/*Hearing function - will be executed when we hear a Pawn*/
	UFUNCTION()	void OnHearNoise(APawn* PawnInstigator, const FVector& Location, float Volume);

	UFUNCTION() void OnSeenTarget(APawn * PawnInstigator);

	UFUNCTION(BlueprintCallable) void NextPatrolPoint();
	UFUNCTION(BlueprintCallable) void GoNextPatrolPoint();
	UFUNCTION(BlueprintCallable) void ArrivedAtPatrolPoint();
	UFUNCTION(BlueprintCallable) void MutationFight();

	/*A Behavior Tree reference*/
	UPROPERTY(EditDefaultsOnly)
		UBehaviorTree* BehaviorTree;

	AMutationCtrl* myController;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") float silentTime;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") float stunLostTime;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") TArray<ATargetPoint*> patrolPoints;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") float desperateLifeLevel = 10.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") float fightRange = 500.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") float scanTime = 3.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") float scanPatrol_a = 2.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") float scanPatrol_b = -1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") bool patrol_in_order;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") int nextPatrol_i;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") float searchNdestroyTime = 10.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") float lookTime = 3.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") bool canFly;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") MutationStates mystate;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") float strikeDistance = 250.0f;
	
	float moveTolerance = 100.0f;
		
	float mytime;
	bool flying;
	bool inAir;
	bool targetVisible;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") float life = 100.0f;
	float timeHeard;
	float timeSeen;
	float arrivedTime;		
	int patrol_i;
	float distToTarget;
	AMyPlayerCharacter* myTarget;
	FVector targetPos;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") float aggressivity = 0.5f;
	UPROPERTY(EditAnywhere, Category = "MutationAI") TArray<FAtkNode> attackList;

private:
	// You can use this to customize various properties about the trace
	FCollisionQueryParams RayParams;
	// The hit result gets populated by the line trace
	FHitResult hitres;
	UWorld* world;
	UAnimComm * myAnimBP;
	float advanceAtk;
	void ResetAnims();
	void StartLethal();
	void StopLethal();
	void MeleeAttack();
	void NextComboHit();
	FTimerHandle timerHandle;
	FAtkNode* atkWalker;
};
