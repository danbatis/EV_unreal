// Fill out your copyright notice in the Description page of Project Settings.

#include "Grappable.h"
#include "EpidemicVectors.h"

AGrappable::AGrappable() {
	myMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("myMesh"));
}
void AGrappable::BeginPlay() {}
void AGrappable::Tick(float DeltaTime) {}
