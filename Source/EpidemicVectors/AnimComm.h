// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Engine/Classes/Animation/AnimInstance.h"

#include "AnimComm.generated.h"


UCLASS(transient, Blueprintable, hideCategories = AnimInstance, BlueprintType)
class EPIDEMICVECTORS_API UAnimComm : public UAnimInstance
{
	GENERATED_BODY()
public:
	UAnimComm();
	~UAnimComm();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)	float speedv;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	float speedh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	bool inAir;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	int attackIndex;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	int knockDown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)	int damageIndex;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	int knockDownIndex;
	
};
