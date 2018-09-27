// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_MoveToPlayer.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyAllTypes.h"
#include "PureCppAIChar.h"
#include "PureCppAIControl.h"
#include "MyPlayerCharacter.h"

EBTNodeResult::Type UBTTask_MoveToPlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	APureCppAIControl * charPC = Cast<APureCppAIControl>(OwnerComp.GetAIOwner());

	AMyPlayerCharacter* Enemy = Cast<AMyPlayerCharacter>(OwnerComp.GetBlackboardComponent()->GetValue<UBlackboardKeyType_Object>(charPC->EnemyKeyID));

	if(Enemy){
		//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("[task]moving to player"));
		charPC->MoveToActor(Enemy, 5.0f, true, true, true, 0, true);
		return EBTNodeResult::Succeeded;
	}
	else {
		return EBTNodeResult::Failed;
	}

}

