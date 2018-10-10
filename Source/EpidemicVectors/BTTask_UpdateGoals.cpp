// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_UpdateGoals.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyAllTypes.h"

EBTNodeResult::Type UBTTask_UpdateGoals::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AMutationCtrl * myCtrl = Cast<AMutationCtrl>(OwnerComp.GetAIOwner());
	//FVector goal = OwnerComp.GetBlackboardComponent()->GetValue<UBlackboardKeyType_Vector>(BlackboardKey.SelectedKeyName);
		
	if (myCtrl->Char) {
		myCtrl->SetDonePath(false);
		
		//do update the state on MutationChar
		myCtrl->Char->mystate = targetstate;
		
		//UE_LOG(LogTemp, Warning, TEXT("mutation AI state: %s"), *myCtrl->StateName.ToString());
		myCtrl->Char->NewGoal(flying);
		
		//UE_LOG(LogTemp, Warning, TEXT("moving to destination: %s"), *myCtrl->Char->patrolPoints[myCtrl->Char->patrol_i]->GetName());
		return EBTNodeResult::Succeeded;
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("not able to get MutationCharacter"));
		return EBTNodeResult::Failed;
	}
}