// Fill out your copyright notice in the Description page of Project Settings.

#include "BTService_CheckForPlayer.h"

#include "EpidemicVectors.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyAllTypes.h"

//#include "EpidemicVectorsCharacter.h"
#include "MyPlayerCharacter.h"

#include "PureCppAIControl.h"
#include "PureCppAIChar.h"

UBTService_CheckForPlayer::UBTService_CheckForPlayer() {
	bCreateNodeInstance = true;
}

void UBTService_CheckForPlayer::TickNode(UBehaviorTreeComponent & OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	//APureCppAIControl * pureCppAIpc = Cast<APureCppAIControl>(OwnerComp.GetAIOwner());

	//if (pureCppAIpc)
	//{
		//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("Found pureAI"));
		//AMyPlayerCharacter* Enemy = Cast<AMyPlayerCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn());

		ACharacter* Enemy = Cast<ACharacter>(GetWorld()->GetFirstPlayerController()->GetPawn());

		if (Enemy) {
			OwnerComp.GetBlackboardComponent()->SetValue<UBlackboardKeyType_Object>(OwnerComp.GetBlackboardComponent()->GetKeyID(BlackboardEnemyKey), Enemy);
			//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("[service]Enemy is here"));
		}
		else {
			OwnerComp.GetBlackboardComponent()->SetValue<UBlackboardKeyType_Object>(OwnerComp.GetBlackboardComponent()->GetKeyID(BlackboardEnemyKey), nullptr);
			
			//OwnerComp.GetBlackboardComponent()->SetValue<UBlackboardKeyType_Vector>(OwnerComp.GetBlackboardComponent()->GetKeyID(OriginKey), FVector(0.0f,0.0f,0.0f));

		}
	//}
}


