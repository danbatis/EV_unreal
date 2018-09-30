// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "MutationAnimComm.generated.h"

/**
 * 
 */
UCLASS(transient, Blueprintable, hideCategories = AnimInstance, BlueprintType)
class EPIDEMICVECTORS_API UMutationAnimComm : public UAnimInstance
{
	GENERATED_BODY()
public:
	UMutationAnimComm();
	~UMutationAnimComm();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)	float speed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	bool inAir;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	int damage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	int knockDown;
};
