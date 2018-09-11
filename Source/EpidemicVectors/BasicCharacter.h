// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "Runtime/Engine/Classes/Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "BasicCharacter.generated.h"

UCLASS()
class EPIDEMICVECTORS_API ABasicCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABasicCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	APlayerController* player;
	FVector myPosition;

	UPROPERTY(EditAnywhere)	UStaticMeshComponent*CubeMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat") float baseSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat") float TurnRate;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	
	
};
