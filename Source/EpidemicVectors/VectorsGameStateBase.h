// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"

#include "MutationChar.h"
#include "Grappable.h"

#include "VectorsGameStateBase.generated.h"

class AMutationChar;
/**
 * 
 */
UCLASS()
class EPIDEMICVECTORS_API AVectorsGameStateBase : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;

	UWorld* world;
	//to store all mutations in scene
	TArray<AMutationChar*> mutations;
	//all grappable elements
	TArray<AActor*> grappables;
	//all grabable elements
	TArray<AActor*> grabables;
};
