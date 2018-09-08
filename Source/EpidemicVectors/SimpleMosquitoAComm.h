// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Engine/Classes/Animation/AnimInstance.h"

#include "SimpleMosquitoAComm.generated.h"


UCLASS(transient, Blueprintable, hideCategories = AnimInstance, BlueprintType)
class EPIDEMICVECTORS_API USimpleMosquitoAComm : public UAnimInstance
{
	GENERATED_BODY()

public:
	USimpleMosquitoAComm();
	~USimpleMosquitoAComm();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float speed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool inAir;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int damage;
};
