// Fill out your copyright notice in the Description page of Project Settings.

#include "MutationChar.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "Runtime/Engine/Classes/Kismet/KismetSystemLibrary.h"

// Sets default values
AMutationChar::AMutationChar()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Initializing our Pawn Sensing comp and our behavior tree reference
	PawnSensingComp = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComp"));
	BehaviorTree = CreateDefaultSubobject<UBehaviorTree>(TEXT("BehaviorTreeReference"));
}

// Called when the game starts or when spawned
void AMutationChar::BeginPlay()
{
	Super::BeginPlay();
	
	if (PawnSensingComp)
	{
		//Registering the delegate which will fire when we hear something
		PawnSensingComp->OnHearNoise.AddDynamic(this, &AMutationChar::OnHearNoise);
		PawnSensingComp->OnSeePawn.AddDynamic(this, &AMutationChar::OnSeenTarget);
	}

	//update links in attackTree
	for (int i = 0; i < attackList.Num(); ++i) {
		//GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::White, "animName: " + attackTreeL[i].myAnim->GetFName().GetPlainNameString());
		if (attackList[i].leftNode != 0) {
			attackList[i].left = &attackList[attackList[i].leftNode];
			//GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::White, "updated left leg");
		}
		if (attackList[i].rightNode != 0) {
			attackList[i].right = &attackList[attackList[i].rightNode];
			//GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::White, "updated right leg");
		}
	}
	atkWalker = &attackList[0];


	myController = Cast<AMutationCtrl>(GetController());
	myController->SetCanFly(canFly);
	world = GetWorld();
	mystate = MutationStates::patrol;
}

// Called every frame
void AMutationChar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
		
	mytime += DeltaTime;

	inAir = GetMovementComponent()->IsFalling();
	myController->SetAirborne(inAir||flying);
	
	//DrawDebugSphere(world, GetActorLocation(), fightRange, 48, FColor::Cyan, true, 0.1f, 0, 5.0);
	myController->SetDesperate(life < desperateLifeLevel);	
	
	switch(mystate){
	case MutationStates::idle:
		GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Blue,"[Mutation] idle");
		break;
	case MutationStates::patrol:
		//targetPos = patrolPoints[nextPatrol_i]->GetActorLocation();
		targetPos = myController->GetGoal();
		DrawDebugSphere(world, targetPos, moveTolerance, 48, FColor::Green, true, 0.1f, 0, 5.0);
		//DrawDebugSphere(world, GetActorLocation(), moveTolerance, 48, FColor::Blue, true, 0.1f, 0, 5.0);	

		myController->SetDesperate(life < desperateLifeLevel);
		GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Blue, "[Mutation] patrol");
		//check if next patrol point is visible, do raycast here
		// Ignore the player's pawn
		RayParams.AddIgnoredActor(this);
		targetVisible = !world->LineTraceSingleByChannel(hitres, GetActorLocation(), patrolPoints[nextPatrol_i]->GetActorLocation(), ECC_Pawn, RayParams);
		DrawDebugLine(world, GetActorLocation(), patrolPoints[nextPatrol_i]->GetActorLocation(), FColor(0, 0, 255), true, -1, 0, 5.0);

		myController->SetTargetVisible(targetVisible);

		if(arrivedTime > 0.0f){//scanning
				if (mytime - arrivedTime < scanTime) {
					//UE_LOG(LogTemp, Warning, TEXT("mutation scanning patrolpoint: %d, going to %d"),patrol_i, nextPatrol_i);
					//if scanning location
					float scanGain = (mytime - arrivedTime) / scanTime;
					
					FVector myLoc = GetActorLocation();
					FVector targetLoc = patrolPoints[nextPatrol_i]->GetActorLocation();

					// get forward vector
					const FRotator Rotation = Controller->GetControlRotation();
					FVector forthVec = FRotationMatrix(Rotation).GetUnitAxis(EAxis::X);

					//FVector forthLoc = myLoc + 2.0f*forthVec;
					FVector forthLoc = myLoc + (targetLoc - myLoc).Size()*forthVec;

					//GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Blue, FString::Printf(TEXT("size vector: %f"), (targetLoc - myLoc).Size()));

					GetCharacterMovement()->bOrientRotationToMovement = false;
					//third order response
					//FVector immediateTarget = forthLoc + (targetLoc - forthLoc)*pow(scanGain,3);

					//first order response with quadratic time (a=scanPatrol_a, b=scanPatrol_b,c=0)
					FVector immediateTarget = forthLoc + (targetLoc - forthLoc)*(scanPatrol_a*pow(scanGain, 2)+scanPatrol_b*scanGain);

					DrawDebugLine(world, myLoc, myLoc + forthVec * 100.0f *scanGain, FColor(255, 0, 0), true, -1, 0, 5.0);
					DrawDebugLine(world, myLoc, immediateTarget, FColor(0, 255, 0), true, -1, 0, 5.0);

					//calculate intermediate position for the smooth lock on
					FRotator almostLookToTarget = UKismetMathLibrary::FindLookAtRotation(myLoc, immediateTarget);
					const FRotator lookToTargetYaw(0, almostLookToTarget.Yaw, 0);

					Controller->SetControlRotation(lookToTargetYaw);
					SetActorRotation(lookToTargetYaw);
					/*
					//SetActorRotation(almostLookToTarget);					
					FRotator NewRotation = FRotator(0.0f, scanGain * 10, 0.0f);
					FQuat QuatRotation = FQuat(NewRotation);
					AddActorLocalRotation(QuatRotation, false, 0, ETeleportType::None);
					//Controller->AddActorWorldRotation(QuatRotation, false,0, ETeleportType::None);
					*/

				}
				else {
					UE_LOG(LogTemp, Warning, TEXT("mutation going to next patrol"));
					//go to next location
					GoNextPatrolPoint();
				}
		}
		else{//traversing
			//distToTarget = FVector::Distance(GetActorLocation(), targetPos);
			//if (distToTarget <= moveTolerance) { ArrivedAtPatrolPoint(); }
		}
		break;
	case MutationStates::investigate:
		targetPos = myController->GetGoal();
		DrawDebugSphere(world, targetPos, moveTolerance, 48, FColor::Green, true, 0.1f, 0, 5.0);
		//distToTarget = FVector::Distance(GetActorLocation(), targetPos);
		//if (distToTarget <= moveTolerance) { myController->SetDonePath(true); }

		GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Blue, "[Mutation] investigate");
		if (timeHeard > 0 && mytime - timeHeard > searchNdestroyTime) {
			myController->SetTargetLocated(false);
			timeHeard = 0.0f;
		}
		if (timeSeen > 0 && mytime - timeSeen > lookTime) {
			myController->SetTargetVisible(false);
			timeSeen = 0.0f;
		}
		break;
	case MutationStates::fight:
		GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Blue, "[Mutation] fight");
		distToTarget = FVector::Distance(GetActorLocation(), myTarget->GetActorLocation());
		myController->SetInFightRange(distToTarget <= fightRange);		
		MutationFight();
		break;
	case MutationStates::escape:
		GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Blue, "[Mutation] escape");
		break;
	case MutationStates::pursuit:
		GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Blue, "[Mutation] pursuit");
		targetPos = myController->GetGoal();
		DrawDebugSphere(world, targetPos, moveTolerance, 48, FColor::Green, true, 0.1f, 0, 5.0);
				
		if (timeHeard > 0 && mytime - timeHeard > searchNdestroyTime) {
			myController->SetTargetLocated(false);
			timeHeard = 0.0f;
			myController->StopBT();
			myController->RestartBT();
		}
		if (timeSeen > 0 && mytime - timeSeen > lookTime) {
			myController->SetTargetVisible(false);
			timeSeen = 0.0f;
			myController->StopBT();
			myController->RestartBT();
		}
		distToTarget = FVector::Distance(GetActorLocation(), myTarget->GetActorLocation());
		//if (distToTarget <= moveTolerance) { myController->SetDonePath(true);}

		myController->SetInFightRange(distToTarget <= fightRange);

		break;
		case MutationStates::flight:
			targetPos = myController->GetGoal();
			AddMovementInput(targetPos - GetActorLocation(), 1.0f);
			distToTarget = FVector::Distance(GetActorLocation(), targetPos);
			if (distToTarget <= moveTolerance) { ArrivedAtPatrolPoint();}			

			break;
		case MutationStates::attacking:
			if ((Controller != NULL) && (advanceAtk != 0.0f))
			{
				// get forward vector
				const FVector Direction = GetActorForwardVector();
				AddMovementInput(Direction, advanceAtk);
			}
			break;
	}
	/*
	if (mytime - startedFollowing > silentTime) {
		//stop behavior tree to stop following
		myController->StopBT();
		if (!targetLost) {
			targetLost = true;
			lostTargetTime = mytime;
		}
	}
	if (targetLost && mytime - lostTargetTime < stunLostTime) {
		const FVector Direction = GetActorForwardVector();
		AddMovementInput(-Direction, 1.0f);
	}
	*/
	
}

// Called to bind functionality to input
void AMutationChar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AMutationChar::OnHearNoise(APawn* PawnInstigator, const FVector& Location, float Volume)
{
	//We don't want to hear ourselves
	if (myController && PawnInstigator != this)
	{
		//Updates our target based on what we've heard.
		UE_LOG(LogTemp, Warning, TEXT("[Mutation] target heard."));

		myController->StopBT();
		myController->SetTargetLocated(true);
		timeHeard = mytime;
		//myController->SetSensedTarget(PawnInstigator);
		myTarget = Cast<AMyPlayerCharacter>(PawnInstigator);
		targetPos = PawnInstigator->GetActorLocation();
		myController->SetGoal(targetPos);
		//goal is in the air?
		myController->SetGoalInAir(myTarget->inAir);
		myController->RestartBT();		
		/*
		startedFollowing = mytime;
		targetLost = false;
		*/
	}
}

void AMutationChar::OnSeenTarget(APawn* PawnInstigator)
{
	//We don't want to hear ourselves
	if (myController && PawnInstigator != this)
	{
		myController->StopBT();
		UE_LOG(LogTemp, Warning, TEXT("[Mutation] target seen!."));
		//Updates our target based on what we've heard.
		myController->SetTargetLocated(true);
		//here the player was not heard, but the timeHeard is used to reset the location...
		timeHeard = mytime;
		myController->SetTargetVisible(true);
		timeSeen = mytime;
		//myController->SetSensedTarget(PawnInstigator);
		myTarget = Cast<AMyPlayerCharacter>(PawnInstigator);
		targetPos = PawnInstigator->GetActorLocation();
		myController->SetGoal(targetPos);
		myController->SetGoalInAir(myTarget->inAir);
		myController->RestartBT();
		/*
		myController->SetSensedTarget(PawnInstigator);
		myController->RestartBT();
		startedFollowing = mytime;
		targetLost = false;
		*/
	}
}

void AMutationChar::GoNextPatrolPoint()
{
	arrivedTime = 0.0f;
	myController->SetDonePath(false);
	GetCharacterMovement()->bOrientRotationToMovement = true;
	myController->RestartBT();
	UE_LOG(LogTemp, Warning, TEXT("mutation going to patrolpoint: %d"), nextPatrol_i);
}

void AMutationChar::NextPatrolPoint()
{
	patrol_i = nextPatrol_i;
	if (patrol_in_order) {
		nextPatrol_i++;
	}
	else {
		nextPatrol_i = FMath::RandRange(0, patrolPoints.Num() - 1);
		if (nextPatrol_i == patrol_i) {
			nextPatrol_i++;
		}
	}	

	if (nextPatrol_i >= patrolPoints.Num())
		nextPatrol_i = 0;
		
	UE_LOG(LogTemp, Warning, TEXT("Updated next patrol point to: %d"), nextPatrol_i);
}

void AMutationChar::ArrivedAtPatrolPoint()
{
	if (arrivedTime == 0.0f) {
		myController->SetDonePath(true);
		//GetCharacterMovement()->MovementMode = MOVE_Walking;
		flying = false;
		arrivedTime = mytime;
		mystate = MutationStates::patrol;
		myController->StopBT();
		GetCharacterMovement()->bOrientRotationToMovement = false;
		UE_LOG(LogTemp, Warning, TEXT("arrived at point: %d"), nextPatrol_i);
		NextPatrolPoint();
	}
}

void AMutationChar::MutationFight() {
	UE_LOG(LogTemp, Warning, TEXT("[task] Mutation %s fighting"), *GetName());
	//DrawDebugSphere(world, GetActorLocation(), strikeDistance, 48, FColor::Orange, true, 0.1f, 0, 5.0);
	if (distToTarget < strikeDistance){
		MeleeAttack();
	}
}
void AMutationChar::StartLethal() {
	Cast<UPrimitiveComponent>(GetMesh()->GetChildComponent(0))->SetGenerateOverlapEvents(true);
	float time2NotLethal = atkWalker->lethalTime*(atkWalker->myAnim->SequenceLength / atkWalker->speed);
	GetWorldTimerManager().SetTimer(timerHandle, this, &AMutationChar::StopLethal, time2NotLethal, false);
}
void AMutationChar::StopLethal() {
	advanceAtk = 0.0f;
	Cast<UPrimitiveComponent>(GetMesh()->GetChildComponent(0))->SetGenerateOverlapEvents(false);
	float time4NextHit = (1 - (atkWalker->time2lethal + atkWalker->lethalTime))*(atkWalker->myAnim->SequenceLength / atkWalker->speed) + atkWalker->coolDown;
	GetWorldTimerManager().SetTimer(timerHandle, this, &AMutationChar::NextComboHit, time4NextHit, false);
}
void AMutationChar::ResetAnims() {
	mystate = MutationStates::fight;
	myController->RestartBT();
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	myAnimBP = Cast<UAnimComm>(GetMesh()->GetAnimInstance());
	GEngine->AddOnScreenDebugMessage(-1, 0.2f, FColor::Blue, TEXT("[Mutation] following the anim blueprint again!"));
}
void AMutationChar::MeleeAttack() {
	mystate = MutationStates::attacking;
	myController->StopBT();
	GetMesh()->Stop();
	GetMesh()->PlayAnimation(atkWalker->myAnim, false);	
}
void AMutationChar::NextComboHit() {
	//Decide if following up or just restarting attacks
	if (FMath::RandRange(0.0f, 1.0f) < aggressivity) {
		ResetAnims();
	}
	else{
		if (FMath::RandRange(0.0f, 1.0f) < 0.5f) {
			if (atkWalker->right) {
				atkWalker = atkWalker->right;
			}
			else { ResetAnims(); }
		}
		else {
			if (atkWalker->left) {
				atkWalker = atkWalker->left;
			}
			else { ResetAnims(); }			
		}
	}
}

