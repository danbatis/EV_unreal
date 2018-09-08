// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Object.h"
#include "UObject/Interface.h"
#include "Lookable.generated.h"

/**
 * 
 */
UINTERFACE()
class EPIDEMICVECTORS_API ULookable: public UInterface {

	GENERATED_UINTERFACE_BODY()

};
//Create a new class ILookable and use GENERATED_IINTERFACE_BODY() Makro in class body 
class ILookable { 
	GENERATED_IINTERFACE_BODY()

public: 
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Gameplay) 
		void ProcessEvent(FName TestParamName, float TestParamFloat); 
};
