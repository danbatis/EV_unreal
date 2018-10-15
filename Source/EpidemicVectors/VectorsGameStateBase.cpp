// Fill out your copyright notice in the Description page of Project Settings.

#include "VectorsGameStateBase.h"

void AVectorsGameStateBase::BeginPlay()
{
	Super::BeginPlay();

	world = GetWorld();

	//find grappable elements that are not mosquitos
	for (TActorIterator<AGrappable> itr(world, AGrappable::StaticClass()); itr; ++itr)
	{
		AGrappable* grappable = *itr;
		if (grappable != NULL) {
			grappables.Add(grappable);
		}
	}

	//find enemies
	for (TActorIterator<AMutationChar> itr(world, AMutationChar::StaticClass()); itr; ++itr)
	{
		AMutationChar* mutation = *itr;
		if (mutation != NULL) {
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, "Mutation found name: " + mutation->GetName());

			mutations.Add(mutation);
			if(mutation->grabable)
				grabables.Add(mutation);
			if (mutation->grappable)
				grappables.Add(mutation);			
		}
	}
}


