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
	//Damage detector
	damageDetector = CreateDefaultSubobject<UBoxComponent>(TEXT("DamageDetector"));
	//Don't collide with camera to keep 3rd person camera at position when obstructed by this char
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	damageDetector->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	damageDetector->SetupAttachment(GetCapsuleComponent());
	damageDetector->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));

	damageDetector->OnComponentBeginOverlap.AddDynamic(this, &AMutationChar::OnOverlapBegin);
	damageDetector->OnComponentEndOverlap.AddDynamic(this, &AMutationChar::OnOverlapEnd);
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
	myAnimBP = Cast<UMutationAnimComm>(GetMesh()->GetAnimInstance());

	myController = Cast<AMutationCtrl>(GetController());
	world = GetWorld();
	mystate = MutationStates::idle;

	//to enable tests on the editor
	myController->SetDesperate(life < desperateLifeLevel);
	myController->SetCanFly(canFly);
	myController->SetAirborne(startAirborne);
	if (startAirborne) {
		GetCharacterMovement()->MovementMode = MOVE_Flying;
	}
	else {
		GetCharacterMovement()->MovementMode = MOVE_Walking;
	}
	myController->SetGoalInAir(false);
	myController->SetDonePath(true);
	currentScanParams = investigateParams;

	/*
	if (!patrolPoints.IsValidIndex(0)) {
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("[Mutation %s] have no patrol points!!!!"), *GetName()));
		UGameplayStatics::SetGlobalTimeDilation(world, .2f);
	}
	if (&attackList[0] == nullptr) {
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("[Mutation %s] have no attack animation list!!!!"), *GetName()));
		UGameplayStatics::SetGlobalTimeDilation(world, .2f);
	}
	*/
	
}

// Called every frame
void AMutationChar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
		
	mytime += DeltaTime;

	inAir = GetMovementComponent()->IsFalling();
	myController->SetAirborne(inAir||flying);
	/*
	if (inAir) {
		UE_LOG(LogTemp, Warning, TEXT("mutation falling"));
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("mutation not falling"));
	}
	*/
	
	GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Blue, FString::Printf(TEXT("[Mutation] state: %d"),(uint8)mystate));
		
	switch(mystate){
	case MutationStates::idle:
		
		break;
	case MutationStates::patrol:		
			
		Navigating();
		break;
	case MutationStates::investigate:
		
		Navigating();
		break;
	case MutationStates::fight:
		GetCharacterMovement()->bOrientRotationToMovement = false;
		DrawDebugSphere(world, GetActorLocation(), fightRange, 12, FColor::Green, true, 0.1f, 0, 1.0);

		CheckRange();

		targetPos = myTarget->GetActorLocation();
		LookTo(targetPos);
		MutationFight();		
				
		break;
	case MutationStates::escape:
		Navigating();
		break;
	case MutationStates::pursuit:		
		Navigating();
		break;
		
	case MutationStates::attacking:
		if ((Controller != NULL) && (advanceAtk != 0.0f))
		{
			// get forward vector
			const FVector Direction = GetActorForwardVector();
			AddMovementInput(Direction, advanceAtk);
		}
			
	break;
	case MutationStates::suffering:			
		const FVector Direction = GetActorForwardVector();
		AddMovementInput(-Direction, recoilPortion);
	break;
	}

	//persistent elements
	if (timeHeard > 0 && mytime - timeHeard > searchNdestroyTime) {
		if (mystate != MutationStates::fight) {
			myController->SetTargetLocated(false);
			timeHeard = 0.0f;
		}		
	}
	if (timeSeen > 0 && mytime - timeSeen > lookTime) {
		if (mystate != MutationStates::fight) {
			myController->SetTargetVisible(false);
			timeSeen = 0.0f;
		}		
	}	
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
				
		myController->SetTargetLocated(true);
		timeHeard = mytime;
		
		myTarget = Cast<AMyPlayerCharacter>(PawnInstigator);
		targetPos = PawnInstigator->GetActorLocation();
		myController->SetGoal(targetPos);
		//goal is in the air?
		myController->SetGoalInAir(myTarget->inAir);
		moveMode = MoveModes::traversing;		
	}
}

void AMutationChar::OnSeenTarget(APawn* PawnInstigator)
{
	//We don't want to hear ourselves
	if (myController && PawnInstigator != this)
	{		
		UE_LOG(LogTemp, Warning, TEXT("[Mutation] target seen!."));
		//Updates our target based on what we've heard.
		myController->SetTargetLocated(true);
		//here the player was not heard, but the timeHeard is used to reset the location...
		timeHeard = mytime;
		myController->SetTargetVisible(true);
		timeSeen = mytime;
		
		myTarget = Cast<AMyPlayerCharacter>(PawnInstigator);
		targetPos = PawnInstigator->GetActorLocation();
		//DrawDebugSphere(world, targetPos, moveTolerance, 12, FColor::Green, true, 5.0f, 0, 5.0);
		myController->SetGoal(targetPos);
		myController->SetGoalInAir(myTarget->inAir);
		CheckRange();
	}
}
void AMutationChar::CheckRange()
{
	targetPos = myTarget->GetActorLocation();
	distToTarget = FVector::Distance(GetActorLocation(), targetPos);
	inFightRange = distToTarget < fightRange;
	
	myController->SetInFightRange(inFightRange);
	
	if (inFightRange) {
		//UE_LOG(LogTemp, Warning, TEXT("[mutation] entered the fight"));
		//PawnSensingComp->SetActive(false);
		PawnSensingComp->SetSensingUpdatesEnabled(false);
		PawnSensingComp->bSeePawns = false;
		myController->StopBT();
		if (mystate != MutationStates::fight && mystate != MutationStates::attacking && mystate != MutationStates::suffering) {
			UE_LOG(LogTemp, Warning, TEXT("[mutation] changed to state fight"));
			newgoalset = true;
			mystate = MutationStates::fight;
			myController->SetReachedGoal(false);
			ArrivedAtGoal();
		}
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("[mutation] sensing enabled"));		
		//PawnSensingComp->SetActive(true);
		PawnSensingComp->SetSensingUpdatesEnabled(true);
		PawnSensingComp->bSeePawns = true;
		ArrivedAtGoal();
		myController->RestartBT();		
	}
}
void AMutationChar::LookTo(FVector LookPosition) {
	FRotator lookToRot = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), LookPosition);
	const FRotator lookToYaw(0, lookToRot.Yaw, 0);
	SetActorRotation(lookToYaw);
}

void AMutationChar::NextPatrolPoint()
{
	patrol_i = nextPatrol_i;
	if (patrolPoints.Num() >= 2) {
		if (patrol_in_order || patrolBackNforth) {
			nextPatrol_i += patrolDir;

			if (nextPatrol_i >= patrolPoints.Num()) {
				if (patrolBackNforth) {
					patrolDir *= -1;
					nextPatrol_i = patrolPoints.Num() - 2;
				}
				else {
					nextPatrol_i = 0;
				}
			}
			if (nextPatrol_i < 0) {
				if (patrolBackNforth) {
					patrolDir *= -1;
					nextPatrol_i = 1;
				}
			}
		}
		else {
			nextPatrol_i = FMath::RandRange(0, patrolPoints.Num() - 1);
			if (nextPatrol_i == patrol_i) {
				nextPatrol_i++;
			}
			if (nextPatrol_i >= patrolPoints.Num())
				nextPatrol_i = 0;
		}
	}
	//extract information from the TargetPoint MutationWaypoint
	myController->SetGoalInAir(false);
	UE_LOG(LogTemp, Warning, TEXT("Updated next patrol point to: %d"), nextPatrol_i);
}
void AMutationChar::Navigating() {
	switch (moveMode) {
	case MoveModes::waitOldHead:
		GEngine->AddOnScreenDebugMessage(-1, 0.2f, FColor::Yellow, TEXT("[Mutation] waiting in old heading"));
		if (mytime - startMoveTimer < currentScanParams.timeInOldHead && startMoveTimer != 0.0f) {}
		else {
			startMoveTimer = mytime;
			moveMode = MoveModes::scanning;
		}
		break;
	case MoveModes::scanning:
		GEngine->AddOnScreenDebugMessage(-1, 0.2f, FColor::Yellow, TEXT("[Mutation] scanning environment"));
		if (mytime - startMoveTimer < currentScanParams.timeToScan && startMoveTimer != 0.0f) {
			//UE_LOG(LogTemp, Warning, TEXT("mutation scanning patrolpoint: %d, going to %d"),patrol_i, nextPatrol_i);
			//if scanning location
			float scanGain = (mytime - startMoveTimer) / currentScanParams.timeToScan;

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
			FVector immediateTarget = forthLoc + (targetLoc - forthLoc)*pow(scanGain, 3);

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
			startMoveTimer = mytime;
			moveMode = MoveModes::waitAfterScan;
		}
		break;
	case MoveModes::waitAfterScan:
		GEngine->AddOnScreenDebugMessage(-1, 0.2f, FColor::Yellow, TEXT("[Mutation] waiting after scan"));
		if (mytime - startMoveTimer < currentScanParams.timeInMidHead && startMoveTimer != 0.0f) {}
		else {
			startMoveTimer = mytime;
			moveMode = MoveModes::turn2NewHead;
		}
		break;
	case MoveModes::turn2NewHead:
		GEngine->AddOnScreenDebugMessage(-1, 0.2f, FColor::Yellow, TEXT("[Mutation] turning 2 new heading"));
		if (mytime - startMoveTimer < currentScanParams.timeToLookNewHead && startMoveTimer != 0.0f) {}
		else {
			startMoveTimer = mytime;
			moveMode = MoveModes::waitInNewHead;
		}
		break;
	case MoveModes::waitInNewHead:
		GEngine->AddOnScreenDebugMessage(-1, 0.2f, FColor::Yellow, TEXT("[Mutation] waiting in new heading"));
		if (mytime - startMoveTimer < currentScanParams.timeBeforeTraverse && startMoveTimer != 0.0f) {}
		else {
			DrawDebugSphere(world, targetPos, moveTolerance, 12, FColor::Green, true, 5.0f, 0, 5.0);
			startMoveTimer = 0.0f;
			
			StartTraverse();
		}
		break;
	case MoveModes::traversing:
		GetCharacterMovement()->bOrientRotationToMovement = true;
		GEngine->AddOnScreenDebugMessage(-1, 0.2f, FColor::Yellow, TEXT("[Mutation] traversing"));

		distToTarget = FVector::Distance(GetActorLocation(), targetPos);
		if (distToTarget <= moveTolerance) { 
			myController->SetReachedGoal(true);
			ArrivedAtGoal(); 
		}
		else {
			if (flying) {
				AddMovementInput(targetPos - GetActorLocation(), 1.0f);
				//slow down
				if (distToTarget <= flyStopFactor * moveTolerance) {
					
					float deacelRate = pow((distToTarget - moveTolerance) / (flyStopFactor*moveTolerance - moveTolerance), 0.5f);
					GetCharacterMovement()->Velocity *= deacelRate;
					UE_LOG(LogTemp, Warning, TEXT("[mutation] deacel %f"), deacelRate);
				}				
			}
		}
		break;
	}
}
void AMutationChar::StartTraverse() {
	moveMode = MoveModes::traversing;

	UE_LOG(LogTemp, Warning, TEXT("mutation going to new goal"));

	if (!flying) {
		UE_LOG(LogTemp, Warning, TEXT("walking"));
		EPathFollowingRequestResult::Type movingRes;
		movingRes = myController->MoveToLocation(targetPos, 5.0f, true, true, true, true, 0, true);
		if (movingRes != EPathFollowingRequestResult::RequestSuccessful)
			ArrivedAtGoal();
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("flying"));
	}
}
void AMutationChar::ArrivedAtGoal()
{
	if (newgoalset) {
		newgoalset = false;
		if (mystate == MutationStates::patrol) {
			UE_LOG(LogTemp, Warning, TEXT("arrived at point: %d"), nextPatrol_i);
			NextPatrolPoint();
		}

		if (GetCharacterMovement()->IsMovingOnGround()) {
			flying = false;
			myController->SetAirborne(false);
			UE_LOG(LogTemp, Warning, TEXT("[navigation] it is grounded by char movement test"), nextPatrol_i);
		}

		//check if on the navigation mesh to find out if it is grounded
		if (flying) {
			EPathFollowingRequestResult::Type movingRes;
			movingRes = myController->MoveToLocation(targetPos, 5.0f, true, true, true, true, 0, true);
			if (movingRes == EPathFollowingRequestResult::RequestSuccessful || movingRes == EPathFollowingRequestResult::AlreadyAtGoal) {
				flying = false;
				myController->SetAirborne(false);
				UE_LOG(LogTemp, Warning, TEXT("[navigation] it is grounded by AI test"), nextPatrol_i);
			}
			myController->StopMovement();
		}		
		//free behaviour tree
		myController->SetDonePath(true);
	}
}
void AMutationChar::NewGoal(bool Flying) {
	newgoalset = true;
	targetPos = myController->GetGoal();

	if (mystate == MutationStates::patrol) {
		currentScanParams = investigateParams;
	}
	else {
		currentScanParams = investigateParams;
	}	
	
	moveMode = MoveModes::waitOldHead;
	startMoveTimer = mytime;

	flying = Flying;	
	if (flying) {
		GetCharacterMovement()->MovementMode = MOVE_Flying;
	}
	else {
		GetCharacterMovement()->MovementMode = MOVE_Walking;
	}
	GetCharacterMovement()->bOrientRotationToMovement = true;
}

void AMutationChar::MutationFight() {
	//UE_LOG(LogTemp, Warning, TEXT("[task] Mutation %s fighting"), *GetName());
	if (distToTarget < strikeDistance){
		MeleeAttack();
	}
	else {
		if (distToTarget < 2*strikeDistance) {
			//SpiralAttack();
		}
		else {
			//DashSideways();
		}
	}
}

void AMutationChar::StartLethal() {
	advanceAtk = 1.0f;
	if(GetMesh()->GetChildComponent(0))
		Cast<UPrimitiveComponent>(GetMesh()->GetChildComponent(0))->SetGenerateOverlapEvents(true);
	float time2NotLethal = atkWalker->lethalTime*(atkWalker->myAnim->SequenceLength / atkWalker->speed);
	GetWorldTimerManager().SetTimer(timerHandle, this, &AMutationChar::StopLethal, time2NotLethal, false);
}
void AMutationChar::StopLethal() {
	advanceAtk = 0.0f;
	if (GetMesh()->GetChildComponent(0))
		Cast<UPrimitiveComponent>(GetMesh()->GetChildComponent(0))->SetGenerateOverlapEvents(false);
	float time4NextHit = (1 - (atkWalker->time2lethal + atkWalker->lethalTime))*(atkWalker->myAnim->SequenceLength / atkWalker->speed) + atkWalker->coolDown;
	GetWorldTimerManager().SetTimer(timerHandle, this, &AMutationChar::NextComboHit, time4NextHit, false);
}
void AMutationChar::ResetFightAnims() {
	UE_LOG(LogTemp, Warning, TEXT("[mutation] finished attacking"));
	mystate = MutationStates::fight;
	//CheckRange();
	//myController->RestartBT();
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	myAnimBP = Cast<UMutationAnimComm>(GetMesh()->GetAnimInstance());
	GEngine->AddOnScreenDebugMessage(-1, 0.2f, FColor::Blue, TEXT("[Mutation] following the anim blueprint again!"));
}
void AMutationChar::MeleeAttack() {
	UE_LOG(LogTemp, Warning, TEXT("mutation %s in meleeAttack"), *GetName());
	mystate = MutationStates::attacking;
	//myController->StopBT();
	GetMesh()->Stop();

	//look in player's direction
	//LookTo(targetPos);

	GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
	GetMesh()->PlayAnimation(atkWalker->myAnim, false);
	StartLethal();
}
void AMutationChar::NextComboHit() {
	if (distToTarget < strikeDistance) {
		//Decide if following up or just restarting attacks
		if (FMath::RandRange(0.0f, 1.0f) < aggressivity) {
			ResetFightAnims();
		}
		else {
			if (FMath::RandRange(0.0f, 1.0f) < 0.5f) {
				if (atkWalker->right) {
					atkWalker = atkWalker->right;
				}
				else { ResetFightAnims(); }
			}
			else {
				if (atkWalker->left) {
					atkWalker = atkWalker->left;
				}
				else { ResetFightAnims(); }
			}
		}
	}
	else {
		ResetFightAnims();
	}
}

void AMutationChar::CancelAttack() {
	world->GetTimerManager().ClearTimer(timerHandle);
	myController->StopBT();
	
	advanceAtk = 0.0f;
	if (GetMesh()->GetChildComponent(0))
		Cast<UPrimitiveComponent>(GetMesh()->GetChildComponent(0))->SetGenerateOverlapEvents(false);
}

void AMutationChar::MyDamage(float DamagePower, FVector AlgozPos) {
	if (GEngine) {
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, "[Mutation] damaged: "+GetName());
	}
	UE_LOG(LogTemp, Warning, TEXT("mutation %s in damage"), *GetName());
	mystate = MutationStates::suffering;
	CancelAttack();
	GetCharacterMovement()->StopActiveMovement();
	
	//look to your algoz
	//LookTo(AlgozPos);
	
	recoilPortion = 1.0f;
	
	//play damage sound

	life -= DamagePower;
	if (life <= 0)
		Death();
	myController->SetDesperate(life < desperateLifeLevel);
	
	//play damage animation
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	myAnimBP = Cast<UMutationAnimComm>(GetMesh()->GetAnimInstance());
	if (FMath::RandRange(0, 10) < 5)
		myAnimBP->damage = 1;
	else
		myAnimBP->damage = 2;

	UGameplayStatics::SetGlobalTimeDilation(world, 0.1f);
	//start stabilize timer
	GetWorldTimerManager().SetTimer(timerHandle, this, &AMutationChar::DelayedStabilize, hitPauseDuration, false);
}
void AMutationChar::Death() {
	if (GEngine) {
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, "[Mutation %s] is dead"+GetName());
	}
}
void AMutationChar::DelayedStabilize() {
	UGameplayStatics::SetGlobalTimeDilation(world, 1.0f);
	GetWorldTimerManager().SetTimer(timerHandle, this, &AMutationChar::Stabilize, damageTime, false);
}
void AMutationChar::Stabilize() {
	UE_LOG(LogTemp, Warning, TEXT("[mutation] recovered from damage"));
	mystate = MutationStates::fight;
	//CheckRange();
	//myController->StopBT();
	myAnimBP->damage = 0;
	recoilPortion = 0.0f;	
}

void AMutationChar::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && (OtherActor != this) && OtherComp)
	{
		if (GEngine) {
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, "[OverlapBegin] my name: " + GetName() + "hit obj: " + *OtherActor->GetName());
		}
	}
}

void AMutationChar::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && (OtherActor != this) && OtherComp)
	{
		myTarget = Cast<AMyPlayerCharacter>(OtherActor);

		if (mystate != MutationStates::suffering) {
			MyDamage(10.0f, myTarget->GetActorLocation());
		}

		if (GEngine) {
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Black, "[OverlapEnd] my name: " + GetName() + "hit obj: " + *OtherActor->GetName());
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("[algoz] %d"), myTarget->mystate));
		}
	}
}

