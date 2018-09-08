// Fill out your copyright notice in the Description page of Project Settings.

#include "GameChar.h"
#include "EpidemicVectors.h"

UGameChar::UGameChar()
{
}

UGameChar::~UGameChar()
{
}

void UGameChar::Damage(float DamagePower)
{
	life -= DamagePower;
	if (life <= 0)
		Death();
}

void UGameChar::Death()
{
	/*
	if(GEngine) {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, "my name: " + GetName()+" and I am dead");
	}
	*/
}

void UGameChar::Recover(float RecoverAmount)
{
	if(life + RecoverAmount <= maxLife) {
		life += RecoverAmount;
	}
}
