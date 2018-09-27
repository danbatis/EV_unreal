// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_CheckForPlayer.generated.h"

/**
 * 
 */
UCLASS()
class EPIDEMICVECTORS_API UBTService_CheckForPlayer : public UBTService
{
	GENERATED_BODY()
	
public:
	UBTService_CheckForPlayer();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName BlackboardEnemyKey = FName("Target");
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName OriginKey = FName("origin");

	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
};
