// Fill out your copyright notice in the Description page of Project Settings.

#include "MyAICharacter.h"


// Sets default values
AMyAICharacter::AMyAICharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Initializing our Pawn Sensing comp and our behavior tree reference
	PawnSensingComp = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComp"));
	BehaviorTree = CreateDefaultSubobject<UBehaviorTree>(TEXT("BehaviorTreeReference"));
}

// Called when the game starts or when spawned
void AMyAICharacter::BeginPlay()
{
	Super::BeginPlay();

	if (PawnSensingComp)
	{
		//Registering the delegate which will fire when we hear something
		PawnSensingComp->OnHearNoise.AddDynamic(this, &AMyAICharacter::OnHearNoise);
		PawnSensingComp->OnSeePawn.AddDynamic(this, &AMyAICharacter::OnSeenTarget);
	}

	Con = Cast<AMyAIController>(GetController());
}

// Called every frame
void AMyAICharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	mytime += DeltaTime;
	if (mytime - startedFollowing > silentTime) {
		//stop behavior tree to stop following
		Con->StopBT();
		if (!targetLost) {
			targetLost = true;
			lostTargetTime = mytime;
		}
	}
	if (targetLost && mytime - lostTargetTime < stunLostTime) {
		const FVector Direction = GetActorForwardVector();
		AddMovementInput(-Direction, 1.0f);
	}
}

// Called to bind functionality to input
void AMyAICharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AMyAICharacter::OnHearNoise(APawn* PawnInstigator, const FVector& Location, float Volume)
{		
	//We don't want to hear ourselves
	if (Con && PawnInstigator != this)
	{
		UE_LOG(LogTemp, Warning, TEXT("target heard."));
		//Updates our target based on what we've heard.
		Con->SetSensedTarget(PawnInstigator);
		Con->RestartBT();
		startedFollowing = mytime;
		targetLost = false;
	}
}

void AMyAICharacter::OnSeenTarget(APawn* PawnInstigator)
{	
	//We don't want to hear ourselves
	if (Con && PawnInstigator != this)
	{
		//UE_LOG(LogTemp, Warning, TEXT("target seen!."));
		//Updates our target based on what we've heard.
		Con->SetSensedTarget(PawnInstigator);
		Con->RestartBT();
		startedFollowing = mytime;
		targetLost = false;
	}
}