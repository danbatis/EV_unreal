// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_MoveToDestination.h"
#include "MutationCtrl.h"
#include "MutationChar.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyAllTypes.h"

EBTNodeResult::Type UBTTask_MoveToDestination::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AMutationCtrl * myCtrl = Cast<AMutationCtrl>(OwnerComp.GetAIOwner());
	FVector goal = OwnerComp.GetBlackboardComponent()->GetValue<UBlackboardKeyType_Vector>(BlackboardKey.SelectedKeyName);

	//AMutationChar* myChar = Cast<AMutationChar>(OwnerComp.GetBlackboardComponent()->GetValue<UBlackboardKeyType_Object>(FName("SelfActor")));
	
	if (myCtrl->Char) {
		//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("[task]moving to destination"));
				
		//UE_LOG(LogTemp, Warning, TEXT("mutation AI state: %s"), *myCtrl->StateName.ToString());
		EPathFollowingRequestResult::Type movingRes;
		if (flying) {
			myCtrl->Char->moveTolerance = tolerance;
			//stop previous move
			//myCtrl->StopMovement();

			myCtrl->Char->GetCharacterMovement()->MovementMode = MOVE_Flying;
			myCtrl->Char->flying = true;

			//fly straight without pathfinding
			//myCtrl->Char->mystate = MutationStates::flight;
			myCtrl->Char->GetCharacterMovement()->bOrientRotationToMovement = true;
			myCtrl->StopBT();
			movingRes = EPathFollowingRequestResult::Failed;
			//UE_LOG(LogTemp, Warning, TEXT("flying to %s"), *BlackboardKey.SelectedKeyName.ToString());
		}
		else {
			//move on the floor with pathfinding
			myCtrl->Char->GetCharacterMovement()->MovementMode = MOVE_Walking;
			myCtrl->Char->flying = false;
			movingRes = myCtrl->MoveToLocation(goal, tolerance, true, true, true, true, 0, true);
			//UE_LOG(LogTemp, Warning, TEXT("moving to %s"), *BlackboardKey.SelectedKeyName.ToString());
		}
			
		DrawDebugLine(GetWorld(), goal, goal+100.0f*FVector::UpVector, FColor(0, 0, 255), true, 0.5f, 0, 5.0);

		float distToTarget = FVector::Distance(myCtrl->Char->GetActorLocation(), goal);

		
		/*
		if (movingRes == EPathFollowingRequestResult::Failed) {
			UE_LOG(LogTemp, Warning, TEXT("[move] fail"));		
		}
		if (movingRes == EPathFollowingRequestResult::RequestSuccessful) {
			UE_LOG(LogTemp, Warning, TEXT("[move] req success"));
		}
		*/

		//UE_LOG(LogTemp, Warning, TEXT("moving to destination: %s"), *myCtrl->Char->patrolPoints[myCtrl->Char->patrol_i]->GetName());
		return EBTNodeResult::Succeeded;
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("not able to get MutationCharacter"));
		return EBTNodeResult::Failed;
	}

}


