// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Perception/PawnSensingComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "MyAIController.h"
#include "GameFramework/Controller.h"

#include "MyAICharacter.generated.h"

//forward declaration to tell compiler the class exists
class AMyAIController;

UCLASS()
class EPIDEMICVECTORS_API AMyAICharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMyAICharacter();

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

	/*A Behavior Tree reference*/
	UPROPERTY(EditDefaultsOnly)
		UBehaviorTree* BehaviorTree;	

	AMyAIController* Con;

	UPROPERTY(EditAnywhere, BlueprintReadWrite) float silentTime;
	float startedFollowing;
	float mytime;
	bool targetLost;
	float lostTargetTime;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float stunLostTime;
};
