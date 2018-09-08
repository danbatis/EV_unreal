// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameCharacter.generated.h"

// This class does not need to be modified.
UINTERFACE(Blueprintable, MinimalAPI) 
class UGameCharacter : public UInterface 
{ GENERATED_UINTERFACE_BODY() };
	
class IGameCharacter
{
	GENERATED_IINTERFACE_BODY()

public:
	float life;
	float maxLife;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = GameChar)
		void Recover(float RecoverAmount);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = GameChar)
		void Damage(float DamagePower);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = GameChar)
		void Death();
};
	