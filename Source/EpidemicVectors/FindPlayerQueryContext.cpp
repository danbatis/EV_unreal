// Fill out your copyright notice in the Description page of Project Settings.

#include "FindPlayerQueryContext.h"

#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"

#include "MutationCtrl.h"

void UFindPlayerQueryContext::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	Super::ProvideContext(QueryInstance, ContextData);

	//Get the Owner of this Query and cast it to an actor
	//Then, get the actor's controller and cast to it our AIController
	//This code works for our case but avoid that many casts and one-liners in cpp.
	AMutationCtrl* AICon = Cast<AMutationCtrl>((Cast<AActor>((QueryInstance.Owner).Get())->GetInstigatorController()));

	if (AICon && AICon->GetSeeingPawn())
	{
		//Set the context SeeingPawn to the provided context data
		UEnvQueryItemType_Actor::SetContextHelper(ContextData, AICon->GetSeeingPawn());		
	}
}