// Fill out your copyright notice in the Description page of Project Settings.

#include "VectorsGameStateBase.h"

void AVectorsGameStateBase::BeginPlay()
{
	Super::BeginPlay();

	world = GetWorld();

	mutations.Empty();
	grappables.Empty();
	grabables.Empty();

	int grabable_i = 0;
	int grappable_i = 0;
	int mutation_i = 0;

	//find grappable elements that are not mosquitos
	for (TActorIterator<AGrappable> itr(world, AGrappable::StaticClass()); itr; ++itr)
	{
		AGrappable* grappable = *itr;
		if (grappable != NULL) {
			grappables.Add(grappable);
			grappable->grappable_i = grappable_i;
			grappable_i++;
		}
	}

	//find enemies
	for (TActorIterator<AMutationChar> itr(world, AMutationChar::StaticClass()); itr; ++itr)
	{
		AMutationChar* mutation = *itr;
		if (mutation != NULL) {
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, "Mutation found name: " + mutation->GetName());

			mutations.Add(mutation);
			mutation->mutation_i = mutation_i;
			mutation_i++;

			if (mutation->grabable) {
				grabables.Add(mutation);
				mutation->grabable_i = grabable_i;
				grabable_i++;
			}
			if (mutation->grappable) {
				grappables.Add(mutation);
				mutation->grappable_i = grappable_i;
				grappable_i++;
			}
		}
	}
}

void AVectorsGameStateBase::InitLists() {}

void AVectorsGameStateBase::RemoveMutation(int Mutation_i, int Grab_i, int Grap_i){
	//if in the middle, update the others
	if(Mutation_i < mutations.Num()-1)
	{
		for (int i = Mutation_i+1; i < mutations.Num(); ++i) {
			mutations[i]->mutation_i = mutations[i]->mutation_i - 1;
		}
	}
	mutations.RemoveAt(Mutation_i);	

	if (Grab_i >= 0) {
		//if in the middle, update the others
		if (Grab_i < grabables.Num() - 1)
		{
			for (int i = Grab_i + 1; i < grabables.Num(); ++i) {
				grabables[i]->grabable_i = grabables[i]->grabable_i - 1;
			}
		}
		grabables.RemoveAt(Grab_i);
	}

	if (Grap_i >= 0) {
		//if in the middle, update the others
		if (Grap_i < grappables.Num() - 1)
		{
			AMutationChar *grappMut;
			for (int i = Grap_i + 1; i < grappables.Num(); ++i) {
				grappMut = Cast<AMutationChar>(grappables[i]);
				grappMut->grappable_i = grappMut->grappable_i - 1;
			}
		}
		grappables.RemoveAt(Grap_i);
	}
}
