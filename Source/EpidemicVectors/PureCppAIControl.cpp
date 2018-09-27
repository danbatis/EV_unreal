// Fill out your copyright notice in the Description page of Project Settings.
#include "PureCppAIControl.h"
#include "PureCppAIChar.h"

APureCppAIControl::APureCppAIControl()
{
	//Initialize our components
	BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));
	BehaviorComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorComp"));

}

void APureCppAIControl::Possess(APawn* InPawn)
{
	Super::Possess(InPawn);

	//If our character is valid and has a valid Behavior Tree,
	//Initialize the values of the Blackboard and start the tree
	APureCppAIChar* Char = Cast<APureCppAIChar>(InPawn);
	if (Char && Char->BotBehavior->BlackboardAsset)
	{
		//Initialize the blackboard values
		BlackboardComp->InitializeBlackboard(*Char->BotBehavior->BlackboardAsset);

		EnemyKeyID = BlackboardComp->GetKeyID("Target");

		//Start the tree
		BehaviorComp->StartTree(*Char->BotBehavior);
	}
}



