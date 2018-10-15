// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Engine/Classes/Components/StaticMeshComponent.h"
#include "Grappable.generated.h"

UCLASS()
class EPIDEMICVECTORS_API AGrappable: public AActor
{
	GENERATED_BODY()
public:
	AGrappable();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite) UStaticMeshComponent* myMesh;
};
