// Fill out your copyright notice in the Description page of Project Settings.

#include "MutationCtrl.h"
#include "BehaviorTree/Blackboard/BlackboardKeyAllTypes.h"

//#include "EpidemicVectorsCharacter.h"
//#include "EpidemicVectors.h"
//#include "MyPlayerCharacter.h"


AMutationCtrl::AMutationCtrl()
{
	//Initialize our components
	BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));
	BehaviorTreeComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorComp"));
}

void AMutationCtrl::Possess(APawn* InPawn)
{
	Super::Possess(InPawn);

	//If our character is valid and has a valid Behavior Tree,
	//Initialize the values of the Blackboard and start the tree
	Char = Cast<AMutationChar>(InPawn);
	if (Char)
	{		
		UE_LOG(LogTemp, Warning, TEXT("[mutation] possessed character %s, and got char"), *GetName());
		if (Char->BehaviorTree->BlackboardAsset) {

			//Initialize the blackboard values
			BlackboardComp->InitializeBlackboard(*Char->BehaviorTree->BlackboardAsset);
			
			//Start the tree
			BehaviorTreeComp->StartTree(*Char->BehaviorTree);
			UE_LOG(LogTemp, Warning, TEXT("Mutation Behaviour Tree:%s initialized"), *Char->BehaviorTree->GetName());
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("[mutation] Could not get blackboard from tree: %s"), *Char->BehaviorTree->GetFullName());
		}		
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("[mutation] possessed character %s, but could not get mutation char"), *GetName());
	}
}

void AMutationCtrl::StopBT()
{
	//Stop current navigation
	StopMovement();
	//Stop the tree
	BehaviorTreeComp->StopTree();	
}

void AMutationCtrl::RestartBT() {
	BehaviorTreeComp->RestartLogic();
	BehaviorTreeComp->RestartTree();
	
	//UE_LOG(LogTemp, Warning, TEXT("[restart]mutation AI state: %s"), *StateName.ToString());
	//DrawDebugLine(GetWorld(), startSpot, startSpot + 1000*FVector::UpVector, FColor(255, 0, 255), true, 0.2f, 0, 5.0);
}

void AMutationCtrl::SetSensedTarget(APawn* NewTarget)
{
	//Set a new target to follow
	if (BlackboardComp) BlackboardComp->SetValueAsObject(FName("targetActor"), NewTarget);
}

void AMutationCtrl::SetDonePath(bool DonePath)
{
	//Set a new target to follow
	if (BlackboardComp) BlackboardComp->SetValueAsBool(FName("donePath"), DonePath);
}
void AMutationCtrl::SetAirborne(bool Airborne)
{
	//Set in air
	if (BlackboardComp) BlackboardComp->SetValueAsBool(FName("airborne"), Airborne);
}
void AMutationCtrl::SetTargetVisible(bool Visible) 
{
	//Set targetVisible
	if (BlackboardComp) BlackboardComp->SetValueAsBool(FName("targetVisible"), Visible);
}
void AMutationCtrl::SetTargetLocated(bool Located)
{
	//Set targetVisible
	if (BlackboardComp) BlackboardComp->SetValueAsBool(FName("targetLocated"), Located);
}
void AMutationCtrl::SetGoalInAir(bool InAir)
{
	//Set targetVisible
	if (BlackboardComp) BlackboardComp->SetValueAsBool(FName("airGoal"), InAir);
}
void AMutationCtrl::SetGoal(FVector GoalLocation)
{
	//Set life
	if (BlackboardComp) BlackboardComp->SetValueAsVector(FName("goal"), GoalLocation);
}
void AMutationCtrl::SetCanFly(bool CanFly)
{
	//Set in air
	if (BlackboardComp) BlackboardComp->SetValueAsBool(FName("canFly"), CanFly);
}
void AMutationCtrl::SetInFightRange(bool InFightRange)
{
	//Set targetVisible
	if (BlackboardComp) BlackboardComp->SetValueAsBool(FName("inFightRange"), InFightRange);
}
void AMutationCtrl::SetDesperate(bool Desperate)
{
	//Set life
	if (BlackboardComp) BlackboardComp->SetValueAsBool(FName("desperate"), Desperate);
}
void AMutationCtrl::SetReachedGoal(bool Reached)
{
	//Set life
	if (BlackboardComp) BlackboardComp->SetValueAsBool(FName("reachedGoal"), Reached);
}
void AMutationCtrl::SetBlindSearch(bool Active)
{
	//Set life
	if (BlackboardComp) BlackboardComp->SetValueAsBool(FName("blindSearch"), Active);
}
FVector AMutationCtrl::GetGoal() 
{
	if (BlackboardComp)
		return BlackboardComp->GetValue<UBlackboardKeyType_Vector>(FName("goal"));
	else
		return FVector::ZeroVector;
}
AActor* AMutationCtrl::GetSeeingPawn()
{
	//Return the seeing pawn
	UObject* object = BlackboardComp->GetValueAsObject(FName("targetActor"));

	return object ? Cast<AActor>(object) : nullptr;
}