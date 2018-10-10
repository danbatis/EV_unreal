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
	myMesh = GetMesh();
	myAnimBP = Cast<UMutationAnimComm>(myMesh->GetAnimInstance());

	myController = Cast<AMutationCtrl>(GetController());
	world = GetWorld();
	mystate = MutationStates::patrol;

	//to enable tests on the editor
	myController->SetDesperate(life < desperateLifeLevel);
	myController->SetCanFly(canFly);
	myController->SetAirborne(startAirborne);
	flying = startAirborne;
	
	myCharMove = GetCharacterMovement();
	if (startAirborne) {
		myCharMove->MovementMode = MOVE_Flying;
	}
	else {
		myCharMove->MovementMode = MOVE_Walking;
	}
	myCharMove->MaxWalkSpeed = normalSpeed;
	myCharMove->MaxAcceleration = normalAcel;
	myController->SetGoalInAir(false);
	myController->SetDonePath(true);
	targetPos = patrolPoints[0]->GetActorLocation();
	myController->SetGoal(targetPos);
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
	myAnimBP->inAir = inAir || flying;
	/*
	if (inAir) {
		UE_LOG(LogTemp, Warning, TEXT("mutation falling"));
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("mutation not falling"));
	}
	*/
	
	if (debugInfo) {
		GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Blue, FString::Printf(TEXT("[Mutation %s] state: %d"), *GetName(),(uint8)mystate));
		DrawDebugSphere(world, GetActorLocation(), strikeDistance, 12, FColor::Yellow, true, 0.1f, 0, 1.0);
		DrawDebugSphere(world, GetActorLocation(), 2*strikeDistance, 12, FColor::Yellow, true, 0.1f, 0, 1.0);
		DrawDebugSphere(world, GetActorLocation(), fightRange, 12, FColor::Green, true, 0.1f, 0, 1.0);
		DrawDebugLine(world, GetActorLocation(), targetPos, FColor(0, 255, 0), true, 0.1f, 0, 5.0);
	}

	switch(mystate){
		case MutationStates::idle:
			targetPos = myTarget->GetActorLocation();
			LookTo(targetPos);
			//CheckRange();
			break;
		case MutationStates::patrol:
			Navigating(DeltaTime);
			break;
		case MutationStates::investigate:
			Navigating(DeltaTime);
			break;
		case MutationStates::fight:
			timeSeen = mytime;
			timeHeard = 0.0f;
			myCharMove->bOrientRotationToMovement = false;

			MutationFight(DeltaTime);							
			break;
		case MutationStates::escape:
			Navigating(DeltaTime);
			break;
		case MutationStates::pursuit:	
			Navigating(DeltaTime);
			break;
		
		case MutationStates::attacking:
			if ((Controller != NULL) && (advanceAtk != 0.0f))
			{
				// get forward vector
				moveDir = GetActorForwardVector();
				AddMovementInput(moveDir, advanceAtk);
			}
			
		break;
		case MutationStates::suffering:			
			moveDir = GetActorForwardVector();
			AddMovementInput(-moveDir, recoilPortion);
		break;
		case MutationStates::evading:
			if ((Controller != NULL) && (evadeValue != 0.0f))
			{
				moveDir = GetActorRightVector();
				AddMovementInput(moveDir, evadeValue);
			}
			targetPos = myTarget->GetActorLocation();
			LookTo(targetPos);
			break;
		case MutationStates::approach:
			if(Controller != NULL)
			{
				moveDir = GetActorForwardVector();
				AddMovementInput(moveDir, 1.0f);
			}
			targetPos = myTarget->GetActorLocation();
			LookTo(targetPos);
			//CheckRange();
			break;	
	}
	

	if(targetSensedTime > 0 && mytime - targetSensedTime > blindPursuitTime){
		
		myController->SetTargetLocated(false);
		myController->SetBlindSearch(false);
		targetSensedTime = 0.0f;
		donePath = true;
		//myController->SetDonePath(true);
		ArrivedAtGoal();		
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
		myTarget = Cast<AMyPlayerCharacter>(PawnInstigator);
		if (myTarget) {
			//Updates our target based on what we've heard.
			UE_LOG(LogTemp, Warning, TEXT("[Mutation] target heard."));

			myController->SetTargetLocated(true);
			timeHeard = mytime;
			
			targetPos = PawnInstigator->GetActorLocation();
			myController->SetGoal(targetPos);
			//goal is in the air?
			myController->SetGoalInAir(myTarget->inAir);
			//CheckRange();
			flying = myTarget->inAir || inAir;
			
			distToTarget = FVector::Distance(GetActorLocation(), targetPos);
			inFightRange = distToTarget < fightRange;
			if (inFightRange && mystate != MutationStates::fight) {
				mystate = MutationStates::fight;
				myController->SetInFightRange(inFightRange);
				PawnSensingComp->SetActive(!inFightRange);
				PawnSensingComp->SetSensingUpdatesEnabled(!inFightRange);
				PawnSensingComp->bSeePawns = !inFightRange;
				PawnSensingComp->bHearNoises = !inFightRange;
			}
			else { 
				StartTraverse(); 
			}
		}
	}
}

void AMutationChar::OnSeenTarget(APawn* PawnInstigator)
{
	//We don't want to hear ourselves
	if (myController && PawnInstigator != this)
	{
		myTarget = Cast<AMyPlayerCharacter>(PawnInstigator);
		if (myTarget) {
			UE_LOG(LogTemp, Warning, TEXT("[Mutation] target seen!."));
			//Updates our target based on what we've heard.
			myController->SetTargetLocated(true);
			//here the player was not heard, but the timeHeard is used to reset the location...
			timeHeard = mytime;
			targetVisible = true;
			myController->SetTargetVisible(targetVisible);
			timeSeen = mytime;

			targetPos = PawnInstigator->GetActorLocation();
			//DrawDebugSphere(world, targetPos, moveTolerance, 12, FColor::Green, true, 5.0f, 0, 5.0);
			myController->SetGoal(targetPos);
			myController->SetGoalInAir(myTarget->inAir);
			//CheckRange();
			flying = myTarget->inAir || inAir;
			
			distToTarget = FVector::Distance(GetActorLocation(), targetPos);
			inFightRange = distToTarget < fightRange;
			if (inFightRange && mystate != MutationStates::fight) {
				mystate = MutationStates::fight;
				myController->SetInFightRange(inFightRange);
				PawnSensingComp->SetActive(!inFightRange);
				PawnSensingComp->SetSensingUpdatesEnabled(!inFightRange);
				PawnSensingComp->bSeePawns = !inFightRange;
				PawnSensingComp->bHearNoises = !inFightRange;
			}
			else{
				StartTraverse();
			}
		}
	}
}
void AMutationChar::CheckDesperation(){
	if (life < desperateLifeLevel) {
		myController->SetDesperate(true);
		//myController->SetDonePath(true);
		ArrivedAtGoal();
	}
}
bool AMutationChar::CheckRange()
{
	distToTarget = FVector::Distance(GetActorLocation(), myTarget->GetActorLocation());
	inFightRange = distToTarget < fightRange;
	
	myController->SetInFightRange(inFightRange);
	PawnSensingComp->SetActive(!inFightRange);
	PawnSensingComp->SetSensingUpdatesEnabled(!inFightRange);
	PawnSensingComp->bSeePawns = !inFightRange;
	PawnSensingComp->bHearNoises = !inFightRange;
	
	myCharMove->StopActiveMovement();
	myController->StopMovement();
	myCharMove->Velocity *= 0;

	if (inFightRange) {
		if (mystate != MutationStates::fight) {
			
			//calculating what direction to rotate to face player
			FVector targetDir = targetPos - GetActorLocation();
			targetDir.Normalize();
			FVector pseudoUp = FVector::CrossProduct(GetActorForwardVector(), targetDir);
			if(FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(pseudoUp, FVector::UpVector))) < 90)
				rotateToFaceTargetDir = 1;
			else
				rotateToFaceTargetDir = -1;

			mystate = MutationStates::fight;
			//PawnSensingComp->SetActive(true);
			//PawnSensingComp->SetSensingUpdatesEnabled(true);
			//PawnSensingComp->bSeePawns = true;
			//PawnSensingComp->bHearNoises = true;
			return false;
		}
		else {
			return true;
		}
	}
	else {
		reachedGoal = false;
		donePath = true;
		//myController->SetDonePath(true);
		NextPatrolPoint();
		ArrivedAtGoal();
		return false;
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
void AMutationChar::Navigating(float DeltaTime){
	
	FVector myLoc = GetActorLocation();
	FVector forthVec = GetActorForwardVector();
	FVector targetDir = targetPos - myLoc;
	targetDir.Normalize();	

	switch (moveMode) {
	case MoveModes::waitOldHead:
		myCharMove->bOrientRotationToMovement = false;
		if (debugInfo)
			GEngine->AddOnScreenDebugMessage(-1, 0.2f, FColor::Yellow, TEXT("[Mutation] waiting in old heading"));
		if (mytime - startMoveTimer < currentScanParams.timeInOldHead && startMoveTimer != 0.0f) {}
		else {
			startMoveTimer = mytime;
			moveMode = MoveModes::scanning;
			angleToTurn2Target = FMath::RadiansToDegrees(FMath::Acos(targetDir.CosineAngle2D(forthVec)));
			UE_LOG(LogTemp, Warning, TEXT("Angle to face target %f"), angleToTurn2Target);
			
			//calculate what direction to turn to scan, it is the opposite from the direction he will need to go			
			FVector pseudoUp = FVector::CrossProduct(forthVec, targetDir);
			if (FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(pseudoUp, FVector::UpVector))) < 90)
				scanDir = -1;
			else
				scanDir = 1;

			//calculate speed to scan
			if(currentScanParams.timeToScan>0.0f)
				scanTurnSpeed = currentScanParams.angleToScan / currentScanParams.timeToScan;
		}
		break;
	case MoveModes::scanning:
		if (debugInfo) 
			GEngine->AddOnScreenDebugMessage(-1, 0.2f, FColor::Yellow, TEXT("[Mutation] scanning environment"));
		if (mytime - startMoveTimer < currentScanParams.timeToScan && startMoveTimer != 0.0f){
			FVector myLoc = GetActorLocation();
			FVector forthVec = GetActorForwardVector();
			DrawDebugLine(world, myLoc, myLoc + forthVec * 100.0f, FColor::Black, true, 0.1f, 0, 5.0);
			DrawDebugLine(world, myLoc, myLoc + targetDir * 100.0f, FColor::Green, true, 0.1f, 0, 5.0);
			//rotate scanning area
			ScanRotation = FRotator(0.0f, scanDir*scanTurnSpeed*DeltaTime, 0.0f);
			QuatScanRotation = FQuat(ScanRotation);
			AddActorLocalRotation(QuatScanRotation, false, 0, ETeleportType::None);
		}
		else{			
			startMoveTimer = mytime;
			moveMode = MoveModes::waitAfterScan;
		}
		break;
	case MoveModes::waitAfterScan:
		if (debugInfo)
			GEngine->AddOnScreenDebugMessage(-1, 0.2f, FColor::Yellow, TEXT("[Mutation] waiting after scan"));
		if (mytime - startMoveTimer < currentScanParams.timeInMidHead && startMoveTimer != 0.0f) {}
		else {
			startMoveTimer = mytime;
			moveMode = MoveModes::turn2NewHead;

			//calculate turn direction
			scanDir *= -1;
			//calculate turn speed
			if(currentScanParams.timeToLookNewHead > 0.0f)
				scanTurnSpeed = (angleToTurn2Target + currentScanParams.angleToScan) / currentScanParams.timeToLookNewHead;
		}
		break;
	case MoveModes::turn2NewHead:
		if (debugInfo)
			GEngine->AddOnScreenDebugMessage(-1, 0.2f, FColor::Yellow, TEXT("[Mutation] turning 2 new heading"));
		if (mytime - startMoveTimer < currentScanParams.timeToLookNewHead && startMoveTimer != 0.0f) {
			//rotate to new head
			ScanRotation = FRotator(0.0f, scanDir*scanTurnSpeed*DeltaTime, 0.0f);
			QuatScanRotation = FQuat(ScanRotation);
			AddActorLocalRotation(QuatScanRotation, false, 0, ETeleportType::None);
		}
		else {
			startMoveTimer = mytime;
			moveMode = MoveModes::waitInNewHead;
		}
		break;
	case MoveModes::waitInNewHead:
		if (debugInfo)
			GEngine->AddOnScreenDebugMessage(-1, 0.2f, FColor::Yellow, TEXT("[Mutation] waiting in new heading"));
		UE_LOG(LogTemp, Warning, TEXT("[Mutation] waiting in new heading"));
		if (mytime - startMoveTimer < currentScanParams.timeBeforeTraverse && startMoveTimer != 0.0f) {}
		else {
			//if (debugInfo)				
				//DrawDebugSphere(world, targetPos, moveTolerance, 12, FColor::Green, true, 5.0f, 0, 5.0);
			
			startMoveTimer = 0.0f;
			StartTraverse();
		}
		break;
	case MoveModes::traversing:
		if (!donePath) {
			myCharMove->bOrientRotationToMovement = true;
			if (debugInfo) {
				GEngine->AddOnScreenDebugMessage(-1, 0.2f, FColor::Yellow, TEXT("[Mutation] traversing"));
				//UE_LOG(LogTemp, Warning, TEXT("[Mutation] traversing"));
			}

			distToTarget = FVector::Distance(GetActorLocation(), targetPos);
			if (distToTarget <= moveTolerance){
				if (mystate == MutationStates::patrol) {
					UE_LOG(LogTemp, Warning, TEXT("arrived at point: %d"), nextPatrol_i);
					NextPatrolPoint();
				}
			
				if (!reachedGoal) {
					myController->SetBlindSearch(true);
					targetSensedTime = mytime;
					myController->SetTargetLocated(false);
				}
				reachedGoal = true;

				//to stop drifting when fighting
				myCharMove->StopActiveMovement();
				myController->StopMovement();
				myCharMove->Velocity *= 0;
				donePath = true;
				ArrivedAtGoal();
			}
			else {
				if (flying) {
					AddMovementInput(targetPos - GetActorLocation(), 1.0f);
					//slow down
					if (distToTarget <= flyStopFactor * moveTolerance) {

						float deacelRate = pow((distToTarget - moveTolerance) / (flyStopFactor*moveTolerance - moveTolerance), 0.5f);
						myCharMove->Velocity *= deacelRate;
						UE_LOG(LogTemp, Warning, TEXT("[mutation] deacel %f"), deacelRate);
					}
					float currDist = FVector::Distance(GetActorLocation(), targetPos);
					if (oldTargetDist >= currDist) {
						//signal something might be wrong
						if (debugInfo) {
							DrawDebugSphere(world, targetPos, 2 * strikeDistance, 12, FColor::Red, true, 5.0f, 0, 1.0);
						}
						//Investigate();
					}
					oldTargetDist = currDist;
				}
				else {
					float currDist = FVector::Distance(GetActorLocation(), targetPos);
					if (oldTargetDist >= currDist) {
						//signal something might be wrong
						if (debugInfo) {
							DrawDebugSphere(world, targetPos, 2 * strikeDistance, 12, FColor::Red, true, 5.0f, 0, 1.0);
						}
						//Investigate();
						//newgoalset = true;
						//StartTraverse();
					}
					oldTargetDist = currDist;
				}
			}
		}
		break;
	}
}
void AMutationChar::StartTraverse() {
	moveMode = MoveModes::traversing;
	//oldTargetDist = -1.0f;
	startMoveTimer = mytime;
	UE_LOG(LogTemp, Warning, TEXT("[mutation %s] Starting traverse"),*GetName());

	if (!flying) {
		UE_LOG(LogTemp, Warning, TEXT("walking"));
		EPathFollowingRequestResult::Type movingRes;
		movingRes = myController->MoveToLocation(targetPos, 5.0f, true, true, true, true, 0, true);
		if (movingRes != EPathFollowingRequestResult::RequestSuccessful) {
			//myController->SetDonePath(true);
			ArrivedAtGoal();
		}
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("flying"));
		//check if path is obstructed, otherwise find the projection
		RayParams.AddIgnoredActor(this);
		targetVisible = !world->LineTraceSingleByChannel(hitres, GetActorLocation(), targetPos, ECC_Pawn, RayParams);
		DrawDebugLine(world, GetActorLocation(), patrolPoints[nextPatrol_i]->GetActorLocation(), FColor(0, 0, 255), true, -1, 0, 5.0);

		myController->SetTargetVisible(targetVisible);
		if (!targetVisible) {
			if(debugInfo)
				UE_LOG(LogTemp, Warning, TEXT("[mutation] straight flight obstructed, finding another path"));
			//myController->SetGoalInAir(false);
			//myController->SetGoal(targetPos);
			
			//myController->SetDonePath(true);
			ArrivedAtGoal();
		}
	}
}
void AMutationChar::ArrivedAtGoal()
{
	if (myCharMove->IsMovingOnGround()) {
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
			myCharMove->MovementMode = MOVE_Walking;
			UE_LOG(LogTemp, Warning, TEXT("[navigation] it is grounded by AI test"), nextPatrol_i);
		}
		myController->StopMovement();
	}		
	//free behaviour tree
	myController->SetDonePath(true);
}
void AMutationChar::NewGoal(bool Flying) {
	targetPos = myController->GetGoal();
	donePath = false;
	startMoveTimer = mytime;
	switch (mystate) {
		case MutationStates::patrol:
			moveMode = MoveModes::waitOldHead;
		break;
		case MutationStates::investigate:
			moveMode = MoveModes::waitOldHead;
			currentScanParams = investigateParams;
			break;
		case MutationStates::pursuit:
			currentScanParams = investigateParams;
			moveMode = MoveModes::waitOldHead;
			break;
		case MutationStates::escape:
			StartTraverse();
			break;
	}	

	flying = Flying;	
	if (flying) {
		myCharMove->MovementMode = MOVE_Flying;
	}
	else {
		myCharMove->MovementMode = MOVE_Walking;
	}	
}

void AMutationChar::MutationFight(float DeltaTime) {
	//UE_LOG(LogTemp, Warning, TEXT("[task] Mutation %s fighting"), *GetName());

	//find the angle between player and mutation
	targetPos = myTarget->GetActorLocation();
	FVector targetDir = targetPos - GetActorLocation();
	targetDir.Normalize();
	float facingAngle = FMath::RadiansToDegrees(FMath::Acos(targetDir.CosineAngle2D(GetActorForwardVector())));
	if (facingAngle < faceTolerance) {
		if (CheckRange()) {
			if (distToTarget < strikeDistance) {
				CombatAction(0);
			}
			else {
				if (distToTarget < 2 * strikeDistance) {
					CombatAction(1);
				}
				else {
					CombatAction(2);
				}
			}
		}		
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Turning to face player"));
		//rotate to face the player
		FRotator NewRotation = FRotator(0.0f, rotateToFaceTargetDir*faceTargetRotSpeed*DeltaTime, 0.0f);
		FQuat QuatRotation = FQuat(NewRotation);
		AddActorLocalRotation(QuatRotation, false, 0, ETeleportType::None);
	}	
}
void AMutationChar::CombatAction(int near_i){
	UE_LOG(LogTemp, Warning, TEXT("[combat action] Mutation %s is %d"), *GetName(), near_i);
	//to stop drifting when fighting
	myCharMove->StopActiveMovement();
	myController->StopMovement();
	myCharMove->Velocity *= 0;
	if (FMath::RandRange(0.0f, 1.0f) <= aggressivity) {
		//attack
		switch (near_i) {
			case 0://close combat
				attackConnected = false;
				//look in player's direction right before first attack
				targetPos = myTarget->GetActorLocation();
				LookTo(targetPos);

				MeleeAttack();
				break;
			case 1://middle range attacks
				if (FMath::RandRange(0.0f, 1.0f) <= aggressivity)
					SpiralAttack();
				else
					DashSideways();
				break;
			case 2://long distance behaviour
				if (FMath::RandRange(0.0f, 1.0f) <= aggressivity)
					Approach();
				else
					DashSideways();
				break;
			default:
				GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("[Mutation] combatAction ID not recognized!"));
				break;
		}
	}
	else {
		//wait
		float waitTime = FMath::RandRange(minIdleTime, maxIdleTime / (3-near_i));
		IdleWait(waitTime);
	}	
}

void AMutationChar::StartLethal() {
	advanceAtk = atkWalker->advanceAtkValue;
	if(myMesh->GetChildComponent(0))
		Cast<UPrimitiveComponent>(myMesh->GetChildComponent(0))->SetGenerateOverlapEvents(true);
	float time2NotLethal = atkWalker->lethalTime*(1 - atkWalker->telegraphPortion)*(atkWalker->myAnim->SequenceLength / atkWalker->speed);
	GetWorldTimerManager().SetTimer(timerHandle, this, &AMutationChar::StopLethal, time2NotLethal, false);
}
void AMutationChar::StopLethal() {
	//update targetPos, so the mutation faces in the direction the player evaded
	targetPos = myTarget->GetActorLocation();
	//reset speeds that were changed on the SpiralAttack
	myCharMove->MaxWalkSpeed = normalSpeed;
	myCharMove->MaxAcceleration = normalAcel;
	advanceAtk = 0.0f;
	if (myMesh->GetChildComponent(0))
		Cast<UPrimitiveComponent>(myMesh->GetChildComponent(0))->SetGenerateOverlapEvents(false);
	float time4NextHit = (1 - (atkWalker->time2lethal + atkWalker->lethalTime))*(1-atkWalker->telegraphPortion)*(atkWalker->myAnim->SequenceLength / atkWalker->speed) + atkWalker->coolDown;
	GetWorldTimerManager().SetTimer(timerHandle, this, &AMutationChar::NextComboHit, time4NextHit, false);
}

void AMutationChar::ResetFightAnims() {
	UE_LOG(LogTemp, Warning, TEXT("[mutation] finished attacking"));
	atkWalker = &attackList[0];
		
	//stop drifting
	myCharMove->StopActiveMovement();
	myController->StopMovement();
	myCharMove->Velocity *= 0;
	
	myMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	myAnimBP = Cast<UMutationAnimComm>(myMesh->GetAnimInstance());
	if (debugInfo)
		GEngine->AddOnScreenDebugMessage(-1, 0.2f, FColor::Blue, TEXT("[Mutation] following the anim blueprint again!"));
	
	CheckRange();
}
void AMutationChar::Investigate() {
	mystate = MutationStates::investigate;
	//myController->SetReachedGoal(true);
	myController->SetTargetVisible(false);
	ArrivedAtGoal();
}
void AMutationChar::IdleWait(float WaitTime) {
	mystate = MutationStates::idle;	
	GetWorldTimerManager().SetTimer(timerHandle, this, &AMutationChar::ResetFightAnims, WaitTime, false);
}
void AMutationChar::MeleeAttack() {
	UE_LOG(LogTemp, Warning, TEXT("mutation %s in meleeAttack"), *GetName());
	mystate = MutationStates::attacking;
	//myController->StopBT();
	myMesh->Stop();

	myCharMove->StopActiveMovement();
	myController->StopMovement();
	myCharMove->Velocity *= 0;	

	myMesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);
	myMesh->PlayAnimation(atkWalker->myAnim, false);
	StartLethal();
}
void AMutationChar::SpiralAttack() {
	UE_LOG(LogTemp, Warning, TEXT("mutation %s in spiral attack"), *GetName());
	myCharMove->MaxWalkSpeed = spiralSpeed;	
	myCharMove->MaxAcceleration = spiralAcel;
	mystate = MutationStates::attacking;
	//myController->StopBT();
	myMesh->Stop();
	targetPos = myTarget->GetActorLocation();
	LookTo(targetPos);
	myCharMove->StopActiveMovement();
	myController->StopMovement();
	myCharMove->Velocity *= 0;
	atkWalker = &spiralAtkNode;
	advanceAtk = atkWalker->telegraphAdvance;
	
	myMesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);
	myMesh->PlayAnimation(spiralAtkNode.myAnim, false);
	
	float time2Lethal = atkWalker->telegraphPortion*(atkWalker->myAnim->SequenceLength / atkWalker->speed);
	GetWorldTimerManager().SetTimer(timerHandle, this, &AMutationChar::StartLethal, time2Lethal, false);
}
void AMutationChar::Approach() {
	UE_LOG(LogTemp, Warning, TEXT("mutation %s getting closer"), *GetName());
	mystate = MutationStates::approach;
	GetWorldTimerManager().SetTimer(timerHandle, this, &AMutationChar::ResetFightAnims, dashTime, false);
}
void AMutationChar::DashSideways(){
	UE_LOG(LogTemp, Warning, TEXT("mutation %s dashing sideways"), *GetName());
	mystate = MutationStates::evading;
	if (FMath::RandRange(0.0f, 1.0f) < 0.5)
		evadeValue = 1.0f;
	else
		evadeValue = -1.0f;

	GetWorldTimerManager().SetTimer(timerHandle, this, &AMutationChar::ResetFightAnims, dashTime, false);
}
void AMutationChar::NextComboHit() {
	if (distToTarget < strikeDistance) {
		//Decide if following up or just restarting attacks
		//if (FMath::RandRange(0.0f, 1.0f) < aggressivity) {
		if(!attackConnected){
			ResetFightAnims();
		}
		else {
			if (FMath::RandRange(0.0f, 1.0f) < 0.5f) {
				if (atkWalker->right) {
					atkWalker = atkWalker->right;
					MeleeAttack();
				}
				else { ResetFightAnims(); }
			}
			else {
				if (atkWalker->left) {
					atkWalker = atkWalker->left;
					MeleeAttack();
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
	myCharMove->MaxWalkSpeed = normalSpeed;
	myCharMove->MaxAcceleration = normalAcel;
	//myController->StopBT();
	
	advanceAtk = 0.0f;
	if (myMesh->GetChildComponent(0))
		Cast<UPrimitiveComponent>(myMesh->GetChildComponent(0))->SetGenerateOverlapEvents(false);
}

void AMutationChar::MyDamage(float DamagePower, FVector AlgozPos) {
	if (debugInfo) {
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, "[Mutation] damaged: "+GetName());
	}
	UE_LOG(LogTemp, Warning, TEXT("mutation %s in damage"), *GetName());
	//stop the traversing
	donePath = true;

	mystate = MutationStates::suffering;
	CancelAttack();
	myCharMove->StopActiveMovement();
	myController->StopMovement();
	myCharMove->Velocity *= 0;
	myCharMove->bOrientRotationToMovement = false;
	
	//look to your algoz
	LookTo(AlgozPos);
	
	recoilPortion = 1.0f;
	
	//play damage sound

	if (life - DamagePower < desperateLifeLevel && life >= desperateLifeLevel) {
		myController->SetDesperate(true);
		becomeDesperate = true;
	}

	life -= DamagePower;
	if (life <= 0)
		Death();	
	
	//play damage animation
	myMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	myAnimBP = Cast<UMutationAnimComm>(myMesh->GetAnimInstance());
	if (FMath::RandRange(0, 10) < 5)
		myAnimBP->damage = 1;
	else
		myAnimBP->damage = 2;

	UGameplayStatics::SetGlobalTimeDilation(world, 0.1f);
	//start stabilize timer
	GetWorldTimerManager().SetTimer(timerHandle, this, &AMutationChar::DelayedStabilize, hitPauseDuration, false);
}
void AMutationChar::Death() {
	if (debugInfo) {
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, "[Mutation %s] is dead"+GetName());
	}
}
void AMutationChar::DelayedStabilize() {
	UGameplayStatics::SetGlobalTimeDilation(world, 1.0f);
	GetWorldTimerManager().SetTimer(timerHandle, this, &AMutationChar::Stabilize, damageTime, false);
}
void AMutationChar::Stabilize() {
	UE_LOG(LogTemp, Warning, TEXT("[mutation] recovered from damage"));

	myAnimBP->damage = 0;
	recoilPortion = 0.0f;

	if (becomeDesperate) {
		becomeDesperate = false;
		//myController->SetDonePath(true);
		ArrivedAtGoal();
	}
	else{
		targetPos = myTarget->GetActorLocation();
		if (mystate != MutationStates::escape)
			mystate = MutationStates::fight;	
	}
}

void AMutationChar::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && (OtherActor != this) && OtherComp)
	{
		if (debugInfo) {
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

		if (debugInfo) {
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Black, "[OverlapEnd] my name: " + GetName() + "hit obj: " + *OtherActor->GetName());
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("[algoz] %d"), myTarget->mystate));
		}
	}
}

