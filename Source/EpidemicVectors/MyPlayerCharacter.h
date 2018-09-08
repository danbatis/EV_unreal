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
#include "Engine.h"

//#include "Runtime/Engine/Classes/Engine/World.h"
//#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
//#include "Runtime/Engine/Classes/Components/StaticMeshComponent.h"
#include "AnimComm.h"

#include "MyPlayerCharacter.generated.h"


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
	FAtkNode* left;
	FAtkNode* right;
	
	FAtkNode() { myAnim = NULL; speed = 2.0f; time2lethal = 0.1f; lethalTime = 0.8f; coolDown = 0.3f; }
};


UCLASS()
class EPIDEMICVECTORS_API AMyPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:

	enum PlayerStates {
		idle,
		suffering,
		attacking
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
	int currentAttack;
	FTimerHandle timerHandle;
	PlayerStates myState;
	bool inAir;
	bool atk1Hold;
	bool atk2Hold;
	float startedHold1;
	float startedHold2;
	float mytime;	
	UPROPERTY(EditAnywhere, Category = Combat)
		float msgTime;
	UPROPERTY(EditAnywhere, Category = Combat)
		float holdTimeMin;
	UPROPERTY(EditAnywhere, Category = Combat)
		float holdTimeMax;
	int knockDownIndex;
	int atkIndex;
	int atkChainIndex;
		
	//input buttons
	FKey atk1Key;//square
	FKey atk2Key;//triangle
	FKey hookKey;//right trigger
	FKey jumpKey;//circle
	FKey dashKey;//cross
	FKey shieldKey;//right bumper
	int shieldKey_i;
	//input axis
	FKey horizontalIn;
	FKey verticalIn;
	FKey verticalCamIn;
	FKey horizontalCamIn;

	UPROPERTY(EditAnywhere, Category = Combat) UAnimSequence *prepSuperHitL;
	UPROPERTY(EditAnywhere, Category = Combat) UAnimSequence *superHitL;
	UPROPERTY(EditAnywhere, Category = Combat) UAnimSequence *prepSuperHitR;	
	UPROPERTY(EditAnywhere, Category = Combat) UAnimSequence *superHitR;
	TArray<FAtkNode> attackChain;
	bool attackLocked;

	UPROPERTY(EditAnywhere, Category = Combat)
		TArray<FAtkNode> attackList;		
	
private:
	APlayerController* player;
	FAtkNode* atkWalker;
	
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
	void LookUp(float Rate);
	void Attack1Press();
	void Attack1Release();
	void KnockDownHit(bool left);
	void AttackWalk(bool left);
	void NextComboHit();
	void Relax();
	void CancelKnockDownPrepare(bool left);
	void Attack2Press();
	void ResetAnims();
	void Attack2Release();
	void StartLethal();
	void StopLethal();
	void ResetAttacks();
	
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
