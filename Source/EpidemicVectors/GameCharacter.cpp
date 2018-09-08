// Fill out your copyright notice in the Description page of Project Settings.

#include "GameCharacter.h"
#include "EpidemicVectors.h"

UGameCharacter::UGameCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {

}
// Add default functionality here for any IGameCharacter functions that are not pure virtual.
/*
void IGameCharacter::Damage(float DamagePower)
{
	life -= DamagePower;
	if (life <= 0)
		Death();
}

void IGameCharacter::Death()
{
}

void IGameCharacter::Recover(float RecoverAmount)
{
	if (life + RecoverAmount <= maxLife) {
		life += RecoverAmount;
	}
}
*/
/*
void IGameCharacter::Recover_Implementation(float RecoverAmount)
{
	if (life + RecoverAmount <= maxLife) {
		life += RecoverAmount;
	}
}
*/

