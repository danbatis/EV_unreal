// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

//my includes
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
#include "Engine.h"
#include "MosquitoCharacter.h"
#include "MutationChar.h"
//#include "Runtime/Engine/Classes/Engine/World.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
//#include "Runtime/Engine/Classes/Components/StaticMeshComponent.h"
#include "AnimComm.h"
#include "Perception/PawnSensingComponent.h"
#include "VectorsGameStateBase.h"

#include "MyPlayerCharacter.generated.h"

//forward declaration
class AMosquitoCharacter;
class AMutationChar;
class AVectorsGameStateBase;

//basic node class for the attack tree
USTRUCT(BlueprintType, Category = "Combat")
struct FAtkNode {
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat") UAnimSequence *myAnim;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat") float speed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat") float time2lethal;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat") float lethalTime;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat") int leftNode;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat") int rightNode;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat") float coolDown;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat") float advanceAtkValue;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat") bool knockDown;

	FAtkNode* left;
	FAtkNode* right;
	
	FAtkNode() { 
		myAnim = NULL; 
		speed = 2.0f; 
		time2lethal = 0.1f; 
		lethalTime = 0.8f; 
		coolDown = 0.3f; 
		advanceAtkValue = 1.0f;
		knockDown = false;
	}
};


UCLASS()
class EPIDEMICVECTORS_API AMyPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:

	enum PlayerStates {
		idle,
		attacking,		
		hookFlying,
		evading,
		suffering,
		kdTakeOff,
		kdFlight,
		kdGround,
		kdRise,
	};	

	// Sets default values for this character's properties
	AMyPlayerCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;	
	//-----------------------------------------------------------mycode starts here
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat)
	UCapsuleComponent* collisionCapsule;

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* FollowCamera;
	
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(EditAnywhere, Category = Camera)
		float BaseTurnRate;
	UPROPERTY(EditAnywhere, Category = Camera)
		float TurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(EditAnywhere, Category = Camera)
		float BaseLookUpRate;
	UPROPERTY(EditAnywhere, Category = Camera)
		float LookUpRate;

	//combat related variables
	UAnimComm * myAnimBP;
	bool lethal;
	FTimerHandle timerHandle;
	PlayerStates mystate;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite) FVector2D targetScreenPos;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite) FColor crossHairColor;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite) FVector2D targetScreenPosAbs;
	bool dashLocked;
	bool evadeLock;
	bool updateAtkDir;
	bool inAir;
	bool flying;
	bool atk1Hold;
	bool atk2Hold;
	float startedHold1;
	float startedHold2;
	float mytime;
	float startReorient;
	UPROPERTY(EditAnywhere, Category = Combat)bool debugInfo;
	UPROPERTY(EditAnywhere, Category = Combat)float reorientTime;
	UPROPERTY(EditAnywhere, Category = Combat)float msgTime;
	UPROPERTY(EditAnywhere, Category = Combat)float holdTimeMin;
	UPROPERTY(EditAnywhere, Category = Combat)float holdTimeMax;
	float advanceAtk;
	float recoilValue;
	int knockDownIndex;
	int atkIndex;
	int atkChainIndex;
	bool knockingDown;
	
	UPROPERTY(EditAnywhere, Category = Movement) float normalSpeed;
	UPROPERTY(EditAnywhere, Category = Movement) float dashSpeed;
	UPROPERTY(EditAnywhere, Category = Movement) float normalAcel;
	UPROPERTY(EditAnywhere, Category = Movement) float dashAcel;

	UPROPERTY(EditAnywhere, Category = Combat) float throwPower;
	UPROPERTY(EditAnywhere, Category = Combat) float aimTimeDilation;
	UPROPERTY(EditAnywhere, Category = Combat) float disengageHookDist;
	UPROPERTY(EditAnywhere, Category = Combat) float normalAirCtrl;
	UPROPERTY(EditAnywhere, Category = Combat) float dashAirCtrl;
	//to instantiate new mosquitos
	UPROPERTY(EditAnywhere, Category = Combat) TSubclassOf<class AMosquitoCharacter> MosquitoClass;
	UPROPERTY(EditAnywhere, Category = Combat) bool lookInCamDir;
	UPROPERTY(EditAnywhere, Category = Combat) bool lockTargetPersistent = false;
	UPROPERTY(EditAnywhere, Category = Combat) float heightOffset_tgtLock = 100.0f;
	UPROPERTY(EditAnywhere, Category = Combat) float targetHeightTol = 1000.0f;
	UPROPERTY(EditAnywhere, Category = Combat) float grappleTol = 0.1f;
	UPROPERTY(EditAnywhere, Category = Combat) float dashAnimGain = 0.5f;
	UPROPERTY(EditAnywhere, Category = Combat) float dashTime = 0.5f;
	UPROPERTY(EditAnywhere, Category = Combat) float hookRange = 1500.0f;
	UPROPERTY(EditAnywhere, Category = Combat) float recoverStaminaDelay = 1.0f;
	UPROPERTY(EditAnywhere, Category = Combat) float recoverStaminaRate = 0.5f;
	UPROPERTY(EditAnywhere, Category = Combat) float evadeRate = 2.5f;
	UPROPERTY(EditAnywhere, Category = Combat) float dashForthRate = 0.5f; 
	UPROPERTY(EditAnywhere, Category = Combat) float attackNormalPower = 10.0f;
	UPROPERTY(EditAnywhere, Category = Combat) float attackKDPower = 20.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat) float dashPower = 1.0f;	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat) float life = 100.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat) float damageTime = 0.3f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat) float takeOffTime = 0.3f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat) float recoverTime = 0.3f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat) float fastRecoverSpeedTarget = 100.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat) float riseTime = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat) float seesawRiseTime = 0.6f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat) float spinRiseTime = 0.8f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat) float riseAtkCoolDown = 0.3f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat) float throwHookTime = 0.3f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat) float hitPause = 0.01f;


	//input buttons
	UPROPERTY(EditAnywhere, Category = Combat) FKey atk1Key;//square
	UPROPERTY(EditAnywhere, Category = Combat) FKey atk2Key;//triangle
	UPROPERTY(EditAnywhere, Category = Combat) FKey hookKey;//right trigger
	UPROPERTY(EditAnywhere, Category = Combat) FKey jumpKey;//circle
	UPROPERTY(EditAnywhere, Category = Combat) FKey dashKey;//cross
	UPROPERTY(EditAnywhere, Category = Combat) FKey shieldKey;//right bumper
	UPROPERTY(EditAnywhere, Category = Combat) FKey grabKey;//left trigger
	UPROPERTY(EditAnywhere, Category = Combat) FKey targetLockKey;//left bumper
	//input axis
	UPROPERTY(EditAnywhere, Category = Combat) FKey horizontal_L;
	UPROPERTY(EditAnywhere, Category = Combat) FKey horizontal_R;
	UPROPERTY(EditAnywhere, Category = Combat) FKey vertical_Up;
	UPROPERTY(EditAnywhere, Category = Combat) FKey vertical_Down;
	UPROPERTY(EditAnywhere, Category = Combat) FKey horizontalCam;
	UPROPERTY(EditAnywhere, Category = Combat) FKey verticalCam;	
	//knockdown animations
	UPROPERTY(EditAnywhere, Category = Combat) FAtkNode prepSuperHitL;
	UPROPERTY(EditAnywhere, Category = Combat) FAtkNode superHitL;
	UPROPERTY(EditAnywhere, Category = Combat) FAtkNode prepSuperHitR;
	UPROPERTY(EditAnywhere, Category = Combat) FAtkNode superHitR;
	//combo variables
	TArray<FAtkNode> attackChain;
	bool attackLocked;
	bool airAttackLocked;
	float attackPower;
	UPROPERTY(EditAnywhere, Category = Combat) TArray<FAtkNode> attackList;
	UPROPERTY(EditAnywhere, Category = Combat) TArray<FAtkNode> attackAirList;
	
	AVectorsGameStateBase *myGameState;
		
	UWorld* world;
	UCharacterMovementComponent *myCharMove;
	USkeletalMeshComponent * myMesh;
	FRotator myRotation;
	FRotator YawRotation;
	float vertIn;
	float horIn;
	FVector forthVec;
	FVector rightVec;
	FVector myVel;
	FVector damageDir;

	float grabForth;
	float grabRight;
	float grabHeight;	

	/*A Pawn Noise Emitter component which is used in order to emit the sounds to nearby AIs*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		UPawnNoiseEmitterComponent* PawnNoiseEmitterComp;

	/*The function that is going to play the sound and report it to our game*/
	UFUNCTION(BlueprintCallable, Category = AI)
		void ReportNoise(USoundBase* SoundToPlay, float Volume);
	
private:
	APlayerController* player;
	FAtkNode* atkWalker;
	
	INT32 width, height;	
	int grapleTarget_i = -1;
	int grabTarget_i = -1;
	int target_i = -1;
	bool aiming;
	float grappleValue;
	bool targetLocked = false;
	bool targetLockUpdated = false;
	bool targetLocking = false;
	FRotator lookToTarget;
	FVector myLoc;
	FVector targetLoc;
	bool dashing;
	bool dashDesire;
	bool arising;
	
	float dashStart;
	float dashPowerRate;
	float startRecoverStamina;
	float dashDirH;
	float dashDirV;
	
	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);
	void Turn(float Rate);
	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);
	void Reorient();
	void LookUp(float Rate);
	void Attack1Press();
	void Attack1Release();
	void KnockDownHit(bool left);
	void AttackWalk(bool left);
	void NextComboHit();
	void Relax();
	void CancelKnockDownPrepare(bool left);
	void CancelAttack();
	void Attack2Press();
	void ResetAnims();
	void Attack2Release();
	void StartLethal();
	void StopLethal();
	void ResetAttacks();
	void Listen4Dash();
	void Listen4Move(float DeltaTime);
	void Listen4Attack();
	void Advance();
	void Listen4Look();
	void Look2Dir(FVector LookDir);
	void FindEnemy(int locDir);
	void DelayedStabilize();
	void Stabilize();
	void MyDamage(float DamagePower, bool KD, bool Spiral);
	void DelayedKDtakeOff();
	void DelayedKDground();
	void DelayedAtkRise();
	void DelayedHookPull();
	void KnockDownFlight();
	void Death();
public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

private:
	UFUNCTION()
		void OnCompHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	UFUNCTION()
		 void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
