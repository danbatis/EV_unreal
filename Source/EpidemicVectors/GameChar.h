// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

//#include "GameChar.generated.h"

/**
 * 
 */

class EPIDEMICVECTORS_API UGameChar
{	
public:
	UGameChar();
	~UGameChar();

	float life;
	float maxLife;

	void Damage(float DamagePower);
	void Death();
	void Recover(float RecoverAmount);
};

