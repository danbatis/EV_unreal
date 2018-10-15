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
#include "MutationAnimComm.h"

#include "Engine.h"

#include "MutationChar.generated.h"

//forward declaration to avoid circular reference
class AMutationCtrl;
class AMyPlayerCharacter;

USTRUCT(BlueprintType, Category = "MutationAI")
struct FMutAtkNode {
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") UAnimSequence *myAnim;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") float speed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") float time2lethal;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") float lethalTime;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") int leftNode;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") int rightNode;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") float coolDown;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") float advanceAtkValue;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") float telegraphPortion;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") float telegraphAdvance;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") bool knockDown;
	FMutAtkNode* left;
	FMutAtkNode* right;

	FMutAtkNode() {
		myAnim = NULL;
		speed = 2.0f;
		time2lethal = 0.1f;
		lethalTime = 0.8f;
		coolDown = 0.3f;
		advanceAtkValue = 1.0f;
		telegraphPortion = 0.0f;
		telegraphAdvance = 0.0f;
		knockDown = false;
	}
};

USTRUCT(BlueprintType, Category = "MutationAI")
struct FScanParams {
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") float timeInOldHead;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") float timeToScan;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") float angleToScan;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") float timeInMidHead;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") float timeToLookNewHead;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") float timeBeforeTraverse;
		
	FScanParams() { 
		timeInOldHead = 0.0f;
		timeToScan = 0.0f;
		angleToScan = 0.0f;
		timeInMidHead = 0.0f;
		timeToLookNewHead = 0.0f;
		timeBeforeTraverse = 0.0f;
	}
};

UENUM(BlueprintType) enum class MutationStates:uint8 {
	idle		UMETA(DisplayName = "idle"),
	patrol		UMETA(DisplayName = "patrol"),
	investigate UMETA(DisplayName = "investigate"),
	fight		UMETA(DisplayName = "fight"),
	escape		UMETA(DisplayName = "escape"),
	pursuit		UMETA(DisplayName = "pursuit"),	
	attacking	UMETA(DisplayName = "attacking"),
	suffering	UMETA(DisplayName = "suffering"),
	evading		UMETA(DisplayName = "evading"),
	approach	UMETA(DisplayName = "approach"),
	kdFlight	UMETA(DisplayName = "kdFlight"),
	kdGround	UMETA(DisplayName = "kdGround"),
	kdRise		UMETA(DisplayName = "kdRise"),
	dizzy		UMETA(DisplayName = "dizzy"),
	grabbed		UMETA(DisplayName = "grabbed"),
	grappled	UMETA(DisplayName = "grappled"),
	heightRoll	UMETA(DisplayName = "heightRoll"),
};

UCLASS()
class EPIDEMICVECTORS_API AMutationChar : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMutationChar();

	enum MoveModes{
		waitOldHead,
		scanning,
		waitAfterScan,
		turn2NewHead,
		waitInNewHead,
		traversing,		
	};
	MoveModes moveMode;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") FScanParams investigateParams;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/*A Pawn Sensing Component, responsible for sensing other Pawns*/
	UPROPERTY(VisibleAnywhere) UPawnSensingComponent* PawnSensingComp;
	//collision component to handle the overlaps for the combat
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	UBoxComponent* damageDetector;

	//sensing
	UFUNCTION()	void OnHearNoise(APawn* PawnInstigator, const FVector& Location, float Volume);
	UFUNCTION() void OnSeenTarget(APawn * PawnInstigator);

	//Damage Detection
	UFUNCTION() void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION() void OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	UFUNCTION(BlueprintCallable) void NextPatrolPoint();
	UFUNCTION(BlueprintCallable) void ArrivedAtGoal();
	UFUNCTION(BlueprintCallable) void MutationFight(float DeltaTime);

	/*A Behavior Tree reference*/
	UPROPERTY(EditDefaultsOnly)
		UBehaviorTree* BehaviorTree;

	AMutationCtrl* myController;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") bool debugInfo;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") bool grappable;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") bool grabable;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") float silentTime;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") float stunLostTime;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") TArray<ATargetPoint*> patrolPoints;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") float desperateLifeLevel = 10.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") float fightRange = 500.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") float rollTolHeight = 200.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") float heightRollSpeed = 500.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") float heightRollTol = 5.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") bool patrol_in_order;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") bool patrolBackNforth = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") int nextPatrol_i;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") float searchNdestroyTime = 10.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") float lookTime = 3.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") bool canFly;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") MutationStates mystate;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") float strikeDistance = 250.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") float damagePower = 5.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") float moveTolerance = 100.0f;
	UPROPERTY(BlueprintReadWrite) FScanParams currentScanParams;
	float mytime;
	bool flying;
	bool inAir;
	bool inFightRange;
	bool donePath;
	bool targetVisible;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") float life = 100.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") float damageTime = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") float normalSpeed = 600.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") float normalAcel = 2048.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") float spiralSpeed = 1000.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") float spiralAcel = 4000.0f;
	float timeHeard;
	float timeSeen;
	float targetSensedTime;
	int patrol_i;
	int patrolDir = 1;
	float distToTarget;
	AMyPlayerCharacter* myTarget;
	FVector targetPos;
	bool becomeDesperate = false;
	bool reachedGoal = false;
	bool facingTarget = false;
	int scanDir = 1;
	float scanTurnSpeed;
	float angleToTurn2Target = 0.0f;
	FRotator ScanRotation;
	FQuat QuatScanRotation;
	int rotateToFaceTargetDir=1;
	bool knockingDown;
	bool attackConnected;
	bool spiralAtk;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") float faceTolerance = 2.0f; 	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") float faceTargetRotSpeed = 350.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") float blindPursuitTime = 5.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") float aggressivity = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") float maxIdleTime = 2.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") float minIdleTime = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") float dashTime = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") float hitPauseDuration = 0.01f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") float KDspeed = 600.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") float KDriseTime = 0.8f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") float knockDownTakeOffTime = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") float flyStopFactor = 2.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") bool startAirborne = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MutationAI") FMutAtkNode spiralAtkNode;
	UPROPERTY(EditAnywhere, Category = "MutationAI") TArray<FMutAtkNode> attackList;

	void NewGoal(bool Flying);	

private:
	bool kdrecovering = false;
	bool kdTakingOff = false;
	float evadeValue;
	float oldTargetDist;	
	FVector moveDir;
	// You can use this to customize various properties about the trace
	FCollisionQueryParams RayParams;
	// The hit result gets populated by the line trace
	FHitResult hitres;
	UWorld* world;
	UMutationAnimComm * myAnimBP;
	UCharacterMovementComponent *myCharMove;
	USkeletalMeshComponent * myMesh;
	float startMoveTimer;
	float advanceAtk;
	float recoilPortion;
	void CheckDesperation();
	bool CheckRange();
	void LookTo(FVector LookPosition);
	void ResetFightAnims();
	void StartLethal();
	void StopLethal();
	void Investigate();

	void IdleWait(float WaitTime);
	void MeleeAttack();	
	void SpiralAttack();
	void Approach();
	void DashSideways();
	void CombatAction(int near_i);

	void NextComboHit();
	void CancelAttack();
	void MyDamage(float DamagePower, FVector AlgozPos, bool KD);
	void Death();
	void DelayedStartKDtakeOff();
	void DelayedStartKDFlight();
	void DelayedStartKDFall();
	void DelayedKDRise();
	void DelayedStabilize();
	void Stabilize();
	void Navigating(float DeltaTime);
	void StartTraverse();
	
	FTimerHandle timerHandle;	
	FMutAtkNode* atkWalker;
};
