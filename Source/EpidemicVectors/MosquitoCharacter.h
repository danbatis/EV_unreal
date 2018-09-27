// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
#include "Components/InputComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "GameChar.h"
#include "GameCharacter.h"
#include "SimpleMosquitoAComm.h"
#include "MyPlayerCharacter.h"

#include "MosquitoCharacter.generated.h"

class AMyPlayerCharacter;

UCLASS()
class EPIDEMICVECTORS_API AMosquitoCharacter : public ACharacter, public UGameChar
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMosquitoCharacter(const class FObjectInitializer& ObjectInitializer);
		
	enum mosquitoStates {
		idle,
		suffering,
		attacking
	};

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	/*
	virtual void ReceiveHit(class UPrimitiveComponent* MyComp, AActor* OtherActor, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;
	*/

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	//from GameChar.h
	//virtual void Recover(float RecoverAmount) override;
	//from GameCharacter.h
	//virtual void Recover_Implementation(float RecoverAmount) override;
	//virtual void Damage_Implementation(float DamagePower) override;

	//collision component to handle the overlaps for the combat
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UBoxComponent* collisionCapsule;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float damageTime;
	FTimerHandle timerHandle;
	USimpleMosquitoAComm* myAnimBP;
	AMyPlayerCharacter* algoz;
	mosquitoStates myState;
	float recoilPortion;

	void MyDamage(float DamagePower, FVector AlgozPos);
	void Stabilize();
	void MoveForward(float Value);
	void MoveRight(float Value);
	void MoveDir(float Value, EAxis::Type Dir);
	void OnCompHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	
	UFUNCTION()
		void OnOverlap(AActor* MyOverlappedActor, AActor* OtherActor);

	UFUNCTION()
		void OnActorBeginOverlap(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** called when something enters the component */
	UFUNCTION()
		void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** called when something leaves the component */
	UFUNCTION()
		void OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

};
