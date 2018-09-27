// Fill out your copyright notice in the Description page of Project Settings.

#include "MyAIController.h"


AMyAIController::AMyAIController()
{
	//Initialize our components
	BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));
	BehaviorTreeComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorComp"));

}

void AMyAIController::Possess(APawn* InPawn)
{
	Super::Possess(InPawn);

	//If our character is valid and has a valid Behavior Tree,
	//Initialize the values of the Blackboard and start the tree
	AMyAICharacter* Char = Cast<AMyAICharacter>(InPawn);
	if (Char && Char->BehaviorTree->BlackboardAsset)
	{
		//Initialize the blackboard values
		BlackboardComp->InitializeBlackboard(*Char->BehaviorTree->BlackboardAsset);

		//Start the tree
		BehaviorTreeComp->StartTree(*Char->BehaviorTree);
	}
}

void AMyAIController::StopBT()
{
	//Stop the tree
	BehaviorTreeComp->StopTree();
}

void AMyAIController::RestartBT() {
	BehaviorTreeComp->RestartTree();
}

void AMyAIController::SetSensedTarget(APawn* NewTarget)
{
	//Set a new target to follow
	if (BlackboardComp) BlackboardComp->SetValueAsObject(TargetKey, NewTarget);
}

