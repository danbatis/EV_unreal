// Fill out your copyright notice in the Description page of Project Settings.

#include "BasicCharacter.h"

// Sets default values
ABasicCharacter::ABasicCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CubeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CubeMesh"));
}

// Called when the game starts or when spawned
void ABasicCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	player = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	//UE_LOG(LogTemp, Warning, TEXT("Default movement mode: %s"), *GetCharacterMovement()->MovementMode.GetValue().GetPlainNameString());	
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange, "[ComponentName]: " + GetCharacterMovement()->MovementMode);
}

// Called every frame
void ABasicCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (player->IsInputKeyDown(EKeys::W))
	{
		if(Controller != NULL)
		{
			// find out which way is forward
			const FRotator Rotation = Controller->GetControlRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);

			// get forward vector
			const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
			AddMovementInput(Direction, 1.0f);
			//GetCharacterMovement()->Velocity = Direction * Value * myspeed;
		}
	}
	if (player->IsInputKeyDown(EKeys::S))
	{
		if(Controller != NULL)
		{
			// find out which way is forward
			const FRotator Rotation = Controller->GetControlRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);

			// get forward vector
			const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
			AddMovementInput(Direction, -1.0f);
			//GetCharacterMovement()->Velocity = Direction * Value * myspeed;
		}
	}
	if (player->IsInputKeyDown(EKeys::A))
	{
		AddControllerYawInput((-1)*TurnRate);
	}
	if (player->IsInputKeyDown(EKeys::D))
	{
		AddControllerYawInput(TurnRate);
	}
}

// Called to bind functionality to input
void ABasicCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

