// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_MutationFight.h"
#include "MutationCtrl.h"


EBTNodeResult::Type UBTTask_MutationFight::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AMutationCtrl * myCtrl = Cast<AMutationCtrl>(OwnerComp.GetAIOwner());
	
	//AMutationChar* myChar = Cast<AMutationChar>(OwnerComp.GetBlackboardComponent()->GetValue<UBlackboardKeyType_Object>(FName("SelfActor")));

	if (myCtrl->Char) {
		UE_LOG(LogTemp, Warning, TEXT("[task] Mutation %s fighting"), *OwnerComp.GetName());
		return EBTNodeResult::Succeeded;
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("not able to get MutationCharacter"));
		return EBTNodeResult::Failed;
	}

}

