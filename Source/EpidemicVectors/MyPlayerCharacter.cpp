// Fill out your copyright notice in the Description page of Project Settings.

#include "MyPlayerCharacter.h"

// Sets default values
AMyPlayerCharacter::AMyPlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	/*
	GetCapsuleComponent()->SetSimulatePhysics(true);
	GetCapsuleComponent()->SetNotifyRigidBodyCollision(true);
	GetCapsuleComponent()->BodyInstance.SetCollisionProfileName("BlockAllDynamic");
	*/
	collisionCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("collisionCapsule"));
	//collisionCapsule->AttachToComponent(RootComponent);
	collisionCapsule->SetupAttachment(GetCapsuleComponent());

	collisionCapsule->OnComponentHit.AddDynamic(this, &AMyPlayerCharacter::OnCompHit);
	collisionCapsule->OnComponentBeginOverlap.AddDynamic(this, &AMyPlayerCharacter::OnBeginOverlap);

	// Set as root component
	//RootComponent = GetCapsuleComponent();

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;
	//for camera sensitivity
	TurnRate = 1.0f;
	LookUpRate = 1.0f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	
	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	//the noise emitter for our AI
	PawnNoiseEmitterComp = CreateDefaultSubobject<UPawnNoiseEmitterComponent>(TEXT("PawnNoiseEmitterComp"));
}

// Called when the game starts or when spawned
void AMyPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();	

	mystate = idle;
	//getting my basic animation blueprint communication class
	myMesh = GetMesh();
	myAnimBP = Cast<UAnimComm>(myMesh->GetAnimInstance());
	if (myMesh->GetChildComponent(0))
		Cast<UPrimitiveComponent>(myMesh->GetChildComponent(0))->SetGenerateOverlapEvents(false);

	world = GetWorld();
	player = UGameplayStatics::GetPlayerController(world, 0);//crashes if in the constructor	
	
	/*
	//getting keys according to what is configured in the input module
	for(int i=0; i< player->PlayerInput->ActionMappings.Num();++i){
		if (player->PlayerInput->ActionMappings[i].ActionName.GetPlainNameString() == "Attack1")
			atk1Key = player->PlayerInput->ActionMappings[i].Key;
		if (player->PlayerInput->ActionMappings[i].ActionName.GetPlainNameString() == "Attack2")
			atk2Key = player->PlayerInput->ActionMappings[i].Key;
		if (player->PlayerInput->ActionMappings[i].ActionName.GetPlainNameString() == "Jump")
			jumpKey = player->PlayerInput->ActionMappings[i].Key;
		if (player->PlayerInput->ActionMappings[i].ActionName.GetPlainNameString() == "Hook")
			hookKey = player->PlayerInput->ActionMappings[i].Key;
		if (player->PlayerInput->ActionMappings[i].ActionName.GetPlainNameString() == "Shield")
			shieldKey = player->PlayerInput->ActionMappings[i].Key;			
		if (player->PlayerInput->ActionMappings[i].ActionName.GetPlainNameString() == "Dash")
			dashKey = player->PlayerInput->ActionMappings[i].Key;		
		//GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::White, "keyName: "+ player->PlayerInput->ActionMappings[i].ActionName.GetPlainNameString());
	}	
	for (int i = 0; i < player->PlayerInput->AxisMappings.Num(); ++i) {
		if (player->PlayerInput->AxisMappings[i].AxisName.GetPlainNameString() == "MoveForward") {
			verticalIn = player->PlayerInput->AxisMappings[i].Key;
			UE_LOG(LogTemp, Warning, TEXT("Set MoveForward key"));
		}
		if (player->PlayerInput->AxisMappings[i].AxisName.GetPlainNameString() == "MoveRight")
			horizontalIn = player->PlayerInput->AxisMappings[i].Key;
		if (player->PlayerInput->AxisMappings[i].AxisName.GetPlainNameString() == "Turn")
			horizontalCamIn = player->PlayerInput->AxisMappings[i].Key;
		if (player->PlayerInput->AxisMappings[i].AxisName.GetPlainNameString() == "LookUp")
			verticalCamIn = player->PlayerInput->AxisMappings[i].Key;
		UE_LOG(LogTemp, Warning, TEXT("axisName: %s"), *player->PlayerInput->AxisMappings[i].AxisName.GetPlainNameString());		
	}
	*/

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
	for (int i = 0; i < attackAirList.Num(); ++i) {
		//GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::White, "animName: " + attackTreeL[i].myAnim->GetFName().GetPlainNameString());
		if (attackAirList[i].leftNode != 0) {
			attackAirList[i].left = &attackAirList[attackAirList[i].leftNode];
			//GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::White, "updated left leg");
		}
		if (attackAirList[i].rightNode != 0) {
			attackAirList[i].right = &attackAirList[attackAirList[i].rightNode];
			//GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::White, "updated right leg");
		}
	}
	
	/*	
	//use walker to show links work
	//GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::White, "leftAttack chain:");
	UE_LOG(LogTemp, Warning, TEXT("walking in atk node always to the left"));
	atkWalker = &attackList[0];	
	while(atkWalker->left != NULL){
		atkWalker = atkWalker->left;
		//GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::White, "animName: " + atkWalker->myAnim->GetFName().GetPlainNameString());
		UE_LOG(LogTemp, Warning, TEXT("leftAttack chain : %s"), *atkWalker->myAnim->GetFName().GetPlainNameString());
	}
	*/
	
	player->GetViewportSize(width, height);
	myCharMove = GetCharacterMovement();

	myCharMove->MaxWalkSpeed = normalSpeed;
	myCharMove->MaxAcceleration = normalAcel;
	myCharMove->AirControl = normalAirCtrl;
	
	//to signal there is no target selected or locked
	target_i = -1;
	myGameState = Cast<AVectorsGameStateBase>(world->GetGameState());
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, "Finished beginPlay on player.");
}

// Called every frame
void AMyPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (debugInfo)
		GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Blue, FString::Printf(TEXT("[playerState: %d"), mystate));

	mytime += DeltaTime;
	inAir = GetMovementComponent()->IsFalling() || flying;
	if(!inAir) {
		//if (debugInfo)
		//	GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Blue, TEXT("falling..."));
		airAttackLocked = false;
		myAnimBP->airdashed = false;
	}
	myRotation = Controller->GetControlRotation();
	YawRotation = FRotator(0, myRotation.Yaw, 0);
	
	if (dashStart > 0 && mytime - dashStart < dashTime && dashPower > 0.0f) {
		//dashing
		CancelAttack();
		dashing = true;
		startRecoverStamina = 0.0f;
		//if forward, then it is the infinite dash, no invincibility
		if (vertIn > 0 && FMath::Abs(horIn) <= 0.5f && mystate != evading) {
			dashLocked = true;
			//cancel the timer
			dashStart = mytime;
			dashPowerRate = dashForthRate;
			dashDirH = 0.0f;
			dashDirV = 1.0f;
		}
		else{
			if(!dashLocked){
				//if no move axe input, evade backwards
				if (vertIn == 0.0f && horIn == 0.0f) {
					mystate = evading;
					dashPowerRate = evadeRate;
					dashDirV = -1.0f;
					dashDirH = 0.0f;
				}
				else {
					//evade diagonal
					mystate = evading;
					dashPowerRate = evadeRate;
					dashDirV = vertIn;
					dashDirH = horIn;
				}
			}
		}
	}
	else {
		dashing = false;
		dashPowerRate = recoverStaminaRate;
		if (dashStart != 0.0f) {
			startRecoverStamina = mytime;
			if (inAir)
				myAnimBP->airdashed = true;
		}
		dashStart = 0.0f;
	}
	if(!dashDesire && mystate != evading && dashStart != 0.0f) {	
		//dash button released, stop turboForth
		dashLocked = false;
		dashing = false;
		dashPowerRate = recoverStaminaRate;
		if (dashStart != 0.0f) {
			startRecoverStamina = mytime;
			if (inAir)
				myAnimBP->airdashed = true;
		}
		dashStart = 0.0f;
	}

	if (dashing) {
		dashPower -= dashPowerRate * DeltaTime;
		if (dashPower <= 0) {
			dashing = false;
			dashPowerRate = recoverStaminaRate;
		}
		myCharMove->MaxWalkSpeed = dashSpeed;
		myCharMove->MaxAcceleration = dashAcel;
		myCharMove->bOrientRotationToMovement = false;
		myCharMove->AirControl = dashAirCtrl;
	}
	else {		
		myCharMove->MaxWalkSpeed = normalSpeed;
		myCharMove->MaxAcceleration = normalAcel;
		myCharMove->bOrientRotationToMovement = true;
		myCharMove->AirControl = normalAirCtrl;
		if (mystate == evading || (dashDirH==0.0f && dashDirV == 1.0f)) {
			mystate = idle;
			myCharMove->Velocity *= (normalSpeed / dashSpeed);
			dashDirH = 0.0f;
			dashDirV = 0.0f;
		}
		if(startRecoverStamina > 0.0f && mytime-startRecoverStamina > recoverStaminaDelay && dashPower < 1.0f){
			dashPower += dashPowerRate * DeltaTime;
		}
	}	
	/*
	//test rotation
	if (player->WasInputKeyJustPressed(EKeys::R)) {
		FRotator NewRotation = FRotator(0.0f, 90.0f, 0.0f);
		FQuat QuatRotation = FQuat(NewRotation);
		AddActorLocalRotation(QuatRotation, false, 0, ETeleportType::None);
	}
	*/

	forthVec = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	rightVec = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	vertIn = player->GetInputAnalogKeyState(vertical_Up) + (-1)*player->GetInputAnalogKeyState(vertical_Down);
	horIn = player->GetInputAnalogKeyState(horizontal_R) + (-1)*player->GetInputAnalogKeyState(horizontal_L);
	
	if (!myAnimBP) return;
	myAnimBP->inAir = inAir;
	myAnimBP->dash = dashing;

	myVel = GetVelocity();
	
	if (lookInCamDir) {
		myCharMove->bOrientRotationToMovement = false;
		//using always the normal speed to normalize, so the dash animation is max out			
		myAnimBP->speedv = 100.0f*(FVector::DotProduct(myVel, forthVec) / (dashAnimGain*normalSpeed));
		myAnimBP->speedh = 100.0f*(FVector::DotProduct(myVel, rightVec) / (dashAnimGain*normalSpeed));
	}
	else {
		myCharMove->bOrientRotationToMovement = true;
		//using always the normal speed to normalize, so the dash animation is max out
		myAnimBP->speedv = 100.0f*(FVector::VectorPlaneProject(myVel, FVector::UpVector).Size() / (dashAnimGain*normalSpeed));
		myAnimBP->speedh = 0.0f;
	}

	//UE_LOG(LogTemp, Warning, TEXT("player state: %d | attackChain size: %d"), (int)mystate, attackChain.Num());

	/*
	if (player->WasInputKeyJustPressed(EKeys::O)) {
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "[gamestate]first Mutation found name: " + myGameState->mutations[0]->GetName());
	}
	*/
	/*
	if (player->IsInputKeyDown(EKeys::I)) {
		player->ProjectWorldLocationToScreen(myGameState->mutations[target_i]->GetActorLocation(), targetScreenPos, false);
		targetScreenPos.X /= width;
		targetScreenPos.Y /= height;
		FVector targetdir = myGameState->mutations[target_i]->GetActorLocation() - GetActorLocation();
		targetdir.Normalize();
		const FRotator Rotation = Controller->GetControlRotation();
		FVector forthVec = FRotationMatrix(Rotation).GetUnitAxis(EAxis::X);
		float angle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(targetdir, forthVec)));
		float angle2 = FMath::RadiansToDegrees(FMath::Acos(forthVec.CosineAngle2D(targetdir)));
		UE_LOG(LogTemp, Warning, TEXT("target at: X: %f Y: %f|angle: %f| angle: %f"), targetScreenPos.X, targetScreenPos.Y, angle, angle2);
	}
	*/
	
	
	/*
	//array tests:
	if (player->WasInputKeyJustPressed(EKeys::J)) {
		myGameState->mutations.RemoveAt(0);
	}
	if (player->WasInputKeyJustPressed(EKeys::K)) {
		myGameState->mutations.RemoveAt(1);
	}
	if (player->WasInputKeyJustPressed(EKeys::L)) {
		myGameState->mutations.RemoveAt(2);
	}
	*/

	/*
	//if (player->WasInputKeyJustPressed(player->PlayerInput->ActionMappings[7].Key))
	//if(player->WasInputKeyJustPressed(EKeys::Y))
	//GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Yellow, FString::Printf(TEXT("horizontal: %f"), player->GetInputAnalogKeyState(horizontal_R)+ (-1)*player->GetInputAnalogKeyState(horizontal_L)));
	//GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Yellow, FString::Printf(TEXT("horizontal cam: %f"), player->GetInputAnalogKeyState(horizontalCam)));
	
	if (player->WasInputKeyJustPressed(shieldKey))
	{
		GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Yellow, TEXT("pressed shield key..."));
	}
	if (player->WasInputKeyJustPressed(dashKey))
	{
		GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Yellow, TEXT("pressed dash key..."));		
	}
	if (player->WasInputKeyJustPressed(hookKey))
	{
		GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Yellow, TEXT("pressed hook key..."));
	}
	*/	

	FVector targetDir;
	float distToTarget;

	switch (mystate) {
	case idle:
		myCharMove->GravityScale = 1.0f;

		if (player->WasInputKeyJustPressed(EKeys::T)) {
			const FVector SpawnPosition = GetActorLocation() + GetActorForwardVector()*50.0f;
			FActorSpawnParameters SpawnInfo;

			AMosquitoCharacter* newMosquito;
			if (player->IsInputKeyDown(EKeys::B)) {
				// spawn new mosquito using blueprint
				newMosquito = GWorld->SpawnActor<AMosquitoCharacter>(MosquitoClass, SpawnPosition, FRotator::ZeroRotator, SpawnInfo);

			}
			else {
				//newMosquito = GWorld->SpawnActor<AMosquitoCharacter>(myGameState->mutations[0]->GetClass(), SpawnPosition, FRotator::ZeroRotator, SpawnInfo);
				// spawn new mosquito without blueprint
				newMosquito = GWorld->SpawnActor<AMosquitoCharacter>(AMosquitoCharacter::StaticClass(), SpawnPosition, FRotator::ZeroRotator, SpawnInfo);
			}

			if (newMosquito != NULL)
			{
				DrawDebugLine(world, GetActorLocation(), SpawnPosition, FColor(0, 255, 0), true, -1, 0, 5.0);
			}
			else
			{
				// Failed to spawn actor!
				if (debugInfo)
					GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Red, TEXT("failed to place new mosquito!"));
			}
		}


		if (player->WasInputKeyJustPressed(grabKey) && target_i >= 0) {
			//grab last mosquito
			FVector mosquitoTarget = myGameState->mutations[target_i]->GetActorLocation() - GetActorLocation();
			FVector upVec = FRotationMatrix(myRotation).GetUnitAxis(EAxis::Z);

			grabForth = mosquitoTarget.ProjectOnTo(forthVec).Size();
			grabRight = mosquitoTarget.ProjectOnTo(rightVec).Size();
			grabHeight = mosquitoTarget.ProjectOnTo(upVec).Size();
		}
		if (player->IsInputKeyDown(grabKey) && target_i >= 0) {
			FVector upVec = FRotationMatrix(myRotation).GetUnitAxis(EAxis::Z);

			//hold last mosquito
			FVector mosquitoPos = grabForth * forthVec + grabRight * rightVec + grabHeight * upVec + GetActorLocation();
			myGameState->mutations[target_i]->SetActorLocation(mosquitoPos);
			DrawDebugLine(world, GetActorLocation(), GetActorLocation() + forthVec * throwPower, FColor(255, 255, 0), true, 0.1f, 0, 5.0);
		}
		if (player->WasInputKeyJustReleased(grabKey) && target_i >= 0) {
			//throw him
			myGameState->mutations[target_i]->GetCharacterMovement()->Velocity = throwPower * forthVec;
		}

		if (player->WasInputKeyJustPressed(hookKey)) {
			//freeze time
			UGameplayStatics::SetGlobalTimeDilation(world, aimTimeDilation);
			aiming = true;
			targetScreenPos.X = 0.5f;
			targetScreenPos.Y = 0.5f;
		}
		//grapple aim
		if (player->IsInputKeyDown(hookKey)) {
			myLoc = GetActorLocation();
			forthVec = GetActorForwardVector();
			GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Green, FString::Printf(TEXT("grappleTarget: %d screenPos X: %f Y: %f"), grapleTarget_i, targetScreenPos.X, targetScreenPos.Y));
			DrawDebugLine(world, myLoc, myLoc + forthVec * (hookRange + disengageHookDist), FColor::Green, true, -1, 0, 5.0);

			targetLocked = false;
			target_i = -1;
			//do the aiming with the grappables
			//turn on UI hints for all visible grappling points
			//check which is the best option, the most centered one, if inside grappleTol
			grappleValue = 10.0f;
			grapleTarget_i = -1;
			crossHairColor = FColor::Silver;
			FVector2D grapScreenPos;
			for (int i = 0; i < myGameState->grappables.Num(); ++i) {
				player->ProjectWorldLocationToScreen(myGameState->grappables[i]->GetActorLocation(), grapScreenPos, false);
				grapScreenPos.X /= width;
				grapScreenPos.Y /= height;
				float currGrappleValue = FVector2D::Distance(grapScreenPos, targetScreenPos);
				if (currGrappleValue < grappleTol && currGrappleValue < grappleValue) {
					grapleTarget_i = i;
					grappleValue = currGrappleValue;
					//and make the correspondent UI hint green
					crossHairColor = FColor::Emerald;
				}
			}

		}
		if (player->WasInputKeyJustReleased(hookKey)) {
			aiming = false;
			if (grapleTarget_i >= 0) {
				myLoc = GetActorLocation();
				targetLoc = myGameState->grappables[grapleTarget_i]->GetActorLocation();
				//play animation to pull
				targetDir = targetLoc - myLoc;
				distToTarget = targetDir.Size();
				//look in target direction
				myCharMove->bOrientRotationToMovement = false;
				lookToTarget = UKismetMathLibrary::FindLookAtRotation(myLoc, targetLoc);
				lookToTarget.Pitch = 0;
				lookToTarget.Roll = 0;
				SetActorRotation(lookToTarget);
				if (grapleTarget_i >= 0 && distToTarget < hookRange + disengageHookDist) {
					myAnimBP->attackIndex = 1;
					this->CustomTimeDilation = 1.0f;
					GetWorldTimerManager().SetTimer(timerHandle, this, &AMyPlayerCharacter::DelayedHookPull, throwHookTime, false);
				}
			}
			else {
				UGameplayStatics::SetGlobalTimeDilation(world, 1.0f);
			}
		}

		Listen4Attack();
		Listen4Move(DeltaTime);
		Listen4Look();
		Listen4Dash();
		Reorient();
		break;
	case attacking:
		Advance();
		Listen4Attack();
		Listen4Look();
		Listen4Dash();
		Reorient();
		if (updateAtkDir) {
			if (lookInCamDir) {
				Look2Dir(forthVec);
			}
			else {
				if (horIn != 0.0f || vertIn != 0.0f)
					Look2Dir(vertIn*forthVec + horIn * rightVec);
			}
		}
		break;
	case hookFlying:
		if (grapleTarget_i >= 0) {
			myLoc = GetActorLocation();
			targetLoc = myGameState->grappables[grapleTarget_i]->GetActorLocation();
			targetDir = targetLoc - myLoc;
			distToTarget = targetDir.Size();
			targetDir.Normalize();

			//set player orientation
			myCharMove->bOrientRotationToMovement = false;
			lookToTarget = UKismetMathLibrary::FindLookAtRotation(myLoc, targetLoc);
			lookToTarget.Pitch = 0;
			lookToTarget.Roll = 0;
			SetActorRotation(lookToTarget);

			if (distToTarget < disengageHookDist)
			{
				flying = false;
				myCharMove->MovementMode = MOVE_Walking;
				myCharMove->MaxWalkSpeed = normalSpeed;
				myCharMove->MaxFlySpeed = normalSpeed;
				myCharMove->MaxAcceleration = normalAcel;
				myCharMove->AirControl = normalAirCtrl;
				myCharMove->GravityScale = 1.0f;
				mystate = idle;
				myAnimBP->attackIndex = 0;
			}
			else
			{
				flying = true;
				myCharMove->MovementMode = MOVE_Flying;
				myCharMove->GravityScale = 0.0f;
				myCharMove->MaxWalkSpeed = dashSpeed * (distToTarget) / hookRange;
				myCharMove->MaxFlySpeed = dashSpeed * (distToTarget) / hookRange;
				myCharMove->MaxAcceleration = dashAcel;
				//AddMovementInput(targetDir, 1.0f);
				myCharMove->Velocity = targetDir * dashSpeed * (distToTarget) / hookRange;;
				//myCharMove->Velocity = forthVec * throwPower;
				myCharMove->AirControl = 0.0f;
			}
		}
		Listen4Look();
		Listen4Dash();
		Reorient();
		break;
	case evading:
		Listen4Look();
		Listen4Dash();
		MoveForward(dashDirV);
		MoveRight(dashDirH);
		Reorient();
		break;
	case suffering:
		if (recoilValue >= 0.0f) {
			AddMovementInput(damageDir, recoilValue);
		}
		Listen4Look();
		break;
	case kdTakeOff:
		if (recoilValue >= 0.0f) {
			AddMovementInput(damageDir + 0.7*FVector::UpVector, recoilValue);
		}
		Listen4Look();
		break;
	case kdFlight:
		//check for fast recover, if right before start falling down
		if (FMath::Abs(myVel.Z) < fastRecoverSpeedTarget){
			if (player->WasInputKeyJustPressed(dashKey)) {
				myAnimBP->damageIndex = 25;
				mystate = kdGround;
				world->GetTimerManager().ClearTimer(timerHandle);
				GetWorldTimerManager().SetTimer(timerHandle, this, &AMyPlayerCharacter::Relax, recoverTime, false);
			}
		}
		//otherwise, only recover when hitting some ground
		if (!inAir) {
			mystate = kdGround;
			myAnimBP->damageIndex = 30;
			GetWorldTimerManager().SetTimer(timerHandle, this, &AMyPlayerCharacter::DelayedKDground, recoverTime, false);
		}
		Listen4Look();
		break;
	case kdGround:
		Listen4Look();
		break;
	case kdRise:
		if (!arising) {
			//simple rise, seesaw attack or spin attack
			if (player->IsInputKeyDown(atk1Key) && player->WasInputKeyJustPressed(atk2Key)){
				//spin rise attack
				attackPower = attackKDPower;
				knockingDown = true;
				myAnimBP->damageIndex = 33;
				mystate = kdGround;
				if (myMesh->GetChildComponent(0))
					Cast<UPrimitiveComponent>(myMesh->GetChildComponent(0))->SetGenerateOverlapEvents(true);
				advanceAtk = 0.0f;
				GetWorldTimerManager().SetTimer(timerHandle, this, &AMyPlayerCharacter::DelayedAtkRise, spinRiseTime, false);
			}
			if (player->WasInputKeyJustReleased(atk1Key)) {
				//seesaw attack rise
				attackPower = attackNormalPower;
				myAnimBP->damageIndex = 32;
				mystate = attacking;
				if (myMesh->GetChildComponent(0))
					Cast<UPrimitiveComponent>(myMesh->GetChildComponent(0))->SetGenerateOverlapEvents(true);
				advanceAtk = 1.0f;
				GetWorldTimerManager().SetTimer(timerHandle, this, &AMyPlayerCharacter::DelayedAtkRise, seesawRiseTime, false);
			}
			if (player->WasInputKeyJustReleased(atk2Key) || player->WasInputKeyJustPressed(vertical_Up)) {
				//simple rise
				arising = true;
				myAnimBP->damageIndex = 31;
				GetWorldTimerManager().SetTimer(timerHandle, this, &AMyPlayerCharacter::Relax, riseTime, false);
			}
		}
		Listen4Look();
		break;
	}	
}

// Called to bind functionality to input
void AMyPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	/*
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("Attack1", IE_Pressed,this, &AMyPlayerCharacter::Attack1Press);
	PlayerInputComponent->BindAction("Attack1", IE_Released, this, &AMyPlayerCharacter::Attack1Release);
	PlayerInputComponent->BindAction("Attack2", IE_Pressed, this, &AMyPlayerCharacter::Attack2Press);
	PlayerInputComponent->BindAction("Attack2", IE_Released, this, &AMyPlayerCharacter::Attack2Release);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMyPlayerCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMyPlayerCharacter::MoveRight);
	
	 We have 2 versions of the rotation bindings to handle different kinds of devices differently
	 "turn" handles devices that provide an absolute delta, such as a mouse.
	 "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick	
	PlayerInputComponent->BindAxis("TurnRate", this, &AMyPlayerCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMyPlayerCharacter::LookUpAtRate);
	
	PlayerInputComponent->BindAxis("Turn", this, &AMyPlayerCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AMyPlayerCharacter::LookUp);
	*/
}

void AMyPlayerCharacter::Turn(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * TurnRate);
	//GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Blue, TEXT("turning!"));
}

void AMyPlayerCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * world->GetDeltaSeconds());
	//GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Blue, TEXT("rate turning!"));
}

void AMyPlayerCharacter::LookUp(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * LookUpRate);
}
void AMyPlayerCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * world->GetDeltaSeconds());
}
void AMyPlayerCharacter::Reorient(){
	if (player->WasInputKeyJustReleased(targetLockKey)) {
		targetLocking = false;
		if (lockTargetPersistent) {
			if (!targetLockUpdated) {
				targetLocked = false;
			}			
		}
		else { targetLocked = false; }
	}
	if (player->WasInputKeyJustPressed(targetLockKey)) {
		targetLocking = true;
		if(!lockTargetPersistent || !targetLocked) {
			startReorient = mytime;
			FindEnemy(0);
			targetLocked = true;
			targetLockUpdated = true;
		}
		else{
			targetLockUpdated = false;
		}
	}
	if (targetLocked){
		crossHairColor = FColor::Emerald;
		if (lockTargetPersistent && targetLocking) {
			if (player->WasInputKeyJustPressed(horizontal_L)) {
				startReorient = mytime;
				FindEnemy(3);
				targetLockUpdated = true;
			}
			if (player->WasInputKeyJustPressed(horizontal_R)) {
				startReorient = mytime;
				FindEnemy(1);
				targetLockUpdated = true;
			}
			if (player->WasInputKeyJustPressed(vertical_Up)) {
				startReorient = mytime;
				FindEnemy(2);
				targetLockUpdated = true;
			}
			if (player->WasInputKeyJustPressed(vertical_Down)) {
				startReorient = mytime;
				FindEnemy(4);
				targetLockUpdated = true;
			}
		}

		FVector targetpos = myGameState->mutations[target_i]->GetActorLocation();
		myLoc = GetActorLocation();
		//check if target position is valid, if it is too high disengage targetLock
		if(targetpos.Z < myLoc.Z + targetHeightTol){		
			targetLoc = targetpos - heightOffset_tgtLock * FVector::UpVector;
			FVector forthLoc = myLoc + (targetLoc - myLoc).Size()*forthVec;

			//turn on crosshairs
			player->ProjectWorldLocationToScreen(myGameState->mutations[target_i]->GetActorLocation(), targetScreenPos, false);
			//targetScreenPosAbs = targetScreenPos;
			targetScreenPos.X /= width;
			targetScreenPos.Y /= height;

			//GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Blue, FString::Printf(TEXT("size vector: %f"), (targetLoc - myLoc).Size()));

			myCharMove->bOrientRotationToMovement = false;
			lookToTarget = UKismetMathLibrary::FindLookAtRotation(myLoc, targetLoc);
			const FRotator lookToTargetYaw(0, lookToTarget.Yaw, 0);
			SetActorRotation(lookToTargetYaw);

			//DrawDebugSphere(world, myLoc, 10.0f, 20, FColor(255, 0, 0), true, -1, 0, 2);
			//DrawDebugSphere(world, myLoc + forthVec * 50.0f + FVector(0.0f, 0.0f, 50.0f), 10.0f, 20, FColor(255, 0, 0), true, -1, 0, 2);

			if (startReorient == 0.0f) {
				targetScreenPosAbs = targetScreenPos;
				Controller->SetControlRotation(lookToTarget);
				//GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, "[oriented] ");
			}
			else {
				targetScreenPosAbs.X = -1;
				targetScreenPosAbs.Y = -1;
				//GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, "[orienting] ");
				float reorientGain = (mytime - startReorient) / reorientTime;
				if (reorientGain >= 1.0f)
					startReorient = 0.0f;

				FVector immediateTarget = forthLoc + (targetLoc - forthLoc)*reorientGain;

				DrawDebugLine(world, myLoc, myLoc + forthVec * 100.0f *reorientGain, FColor(255, 0, 0), true, -1, 0, 5.0);
				DrawDebugLine(world, myLoc, immediateTarget, FColor(0, 255, 0), true, -1, 0, 5.0);

				//calculate intermediate position for the smooth lock on
				FRotator almostLookToTarget = UKismetMathLibrary::FindLookAtRotation(myLoc, immediateTarget);
				Controller->SetControlRotation(almostLookToTarget);
			}
		}
		else {
			targetLocked = false;
			//turn off the crosshairs
			targetScreenPos.X = -1.0f;
			targetScreenPos.Y = -1.0f;
			targetScreenPosAbs = targetScreenPos;
			myCharMove->bOrientRotationToMovement = true;
		}		
	}
	else {
		//turn off the crosshairs if not aiming
		if (!aiming) {
			targetScreenPos.X = -1.0f;
			targetScreenPos.Y = -1.0f;
			targetScreenPosAbs = targetScreenPos;
			myCharMove->bOrientRotationToMovement = true;
		}
	}	
}
void AMyPlayerCharacter::ReportNoise(USoundBase* SoundToPlay, float Volume)
{
	//If we have a valid sound to play, play the sound and
	//report it to our game
	if (SoundToPlay)
	{
		//Play the actual sound
		UGameplayStatics::PlaySoundAtLocation(world, SoundToPlay, GetActorLocation(), Volume);

		//Report that we've played a sound with a certain volume in a specific location
		MakeNoise(Volume, this, GetActorLocation());
	}

}

void AMyPlayerCharacter::FindEnemy(int locDir) {
	//now find enemies	
	float bestValue = 0.0f;	
	float currValue;
	
	if (target_i >= 0) {
		player->ProjectWorldLocationToScreen(myGameState->mutations[target_i]->GetActorLocation(), targetScreenPos, false);
		//targetScreenPosAbs = targetScreenPos;
		targetScreenPos.X /= width;
		targetScreenPos.Y /= height;
		switch (locDir) {
		case 0://center
			bestValue = FVector2D::Distance(targetScreenPos, FVector2D(0.5f, 0.5f));
			break;
		case 1://right
			bestValue = targetScreenPos.X;
			break;
		case 2://up
			bestValue = targetScreenPos.Y;
			break;
		case 3://left
			bestValue = targetScreenPos.X;
			break;
		case 4://down
			bestValue = targetScreenPos.Y;
			break;
		}
	}
	for (int i=0; i<myGameState->mutations.Num(); ++i)
	{
			//check if inside the viewport
			player->ProjectWorldLocationToScreen(myGameState->mutations[i]->GetActorLocation(), targetScreenPos, false);
			targetScreenPos.X /= width;
			targetScreenPos.Y /= height;
			
			if ((targetScreenPos.X > 0.0f && targetScreenPos.X < 1.0f) && (targetScreenPos.Y > 0.0f && targetScreenPos.Y < 1.0f)) {
				if (target_i == -1) {
					target_i = i;					
					switch (locDir) {
					case 0://center
						bestValue = FVector2D::Distance(targetScreenPos, FVector2D(0.5f, 0.5f));
						break;
					case 1://right
						bestValue = targetScreenPos.X;
						break;
					case 2://up
						bestValue = targetScreenPos.Y;
						break;
					case 3://left
						bestValue = targetScreenPos.X;
						break;
					case 4://down
						bestValue = targetScreenPos.Y;
						break;
					}
				}
				else {					
					switch (locDir) {
					case 0://center						
						currValue = FVector2D::Distance(targetScreenPos, FVector2D(0.5f, 0.5f));
						if (currValue < bestValue) {
							bestValue = currValue;
							target_i = i;
						}
						break;
					case 1://right
						if (targetScreenPos.X > bestValue) {
							bestValue = targetScreenPos.X;
							target_i = i;
						}
						break;
					case 2://up
						if (targetScreenPos.Y < bestValue) {
							bestValue = targetScreenPos.Y;
							target_i = i;
						}
						break;
					case 3://left
						if (targetScreenPos.X < bestValue) {
							bestValue = targetScreenPos.X;
							target_i = i;
						}
						break;
					case 4://down
						if (targetScreenPos.Y > bestValue) {
							bestValue = targetScreenPos.Y;
							target_i = i;
						}
						break;
					}
				}
			}
	}
	
}
void AMyPlayerCharacter::Look2Dir(FVector LookDir) {
	myLoc = GetActorLocation();
	targetLoc = myLoc + LookDir;
	lookToTarget = UKismetMathLibrary::FindLookAtRotation(myLoc, targetLoc);
	const FRotator lookToTargetYaw(0, lookToTarget.Yaw, 0);
	SetActorRotation(lookToTargetYaw);
}
void AMyPlayerCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		if(lookInCamDir)
			Look2Dir(forthVec);
		AddMovementInput(forthVec, Value);
		//myCharMove->Velocity = forthVec * Value * myspeed;
	}
}

void AMyPlayerCharacter::MoveRight(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{	
		if (lookInCamDir)
			Look2Dir(forthVec);
		AddMovementInput(rightVec, Value);
		//myCharMove->Velocity = rightVec * Value * myspeed;
	}
}

void AMyPlayerCharacter::Attack1Press(){
	startedHold1 = mytime;//GetWorld->GetTimeSeconds();
	atk1Hold = true;
	//GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Green, FString::Printf(TEXT("Time press 1: %f"), startedHold1));
}
void AMyPlayerCharacter::Attack2Press() {
	startedHold2 = mytime;
	atk2Hold = true;
	/*
	//test animation without blueprints
	GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Orange, "[ComponentName]: " + myMesh->GetChildComponent(0)->GetName());
	GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Blue, TEXT("attack2!"));
	myMesh->PlayAnimation(attackList[1].myAnim, false);
	GetWorldTimerManager().SetTimer(timerHandle, this, &AMyPlayerCharacter::ResetAnims, 1.5f, false);
	*/
}
void AMyPlayerCharacter::Attack1Release() {
	if(atk1Hold){
		if (mytime - startedHold1 < holdTimeMin) {
			if (debugInfo)
				GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Blue, TEXT("quick attack1 press"));
			
			AttackWalk(true);
			CancelKnockDownPrepare(true);
		}
		else {
			if (debugInfo)
				GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Blue, TEXT("long attack1 press"));
			atk1Hold = false;
			KnockDownHit(true);
		}
	}
}
void AMyPlayerCharacter::Attack2Release() {
	if (atk2Hold) {
		if (mytime - startedHold2 < holdTimeMin) {
			if (debugInfo)
				GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Blue, TEXT("quick attack2 press"));
			
			AttackWalk(false);
			CancelKnockDownPrepare(false);
		}
		else {
			if (debugInfo)
				GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Blue, TEXT("long attack2 press"));
			atk2Hold = false;
			KnockDownHit(false);
		}
	}
}
void AMyPlayerCharacter::KnockDownHit(bool left) {
	//clear current relax timer
	world->GetTimerManager().ClearTimer(timerHandle);
	//old way, with the animation blueprint
	knockDownIndex = 2;
	//myAnimBP->knockDown = knockDownIndex;

	knockingDown = true;
	attackPower = attackKDPower;

	//become lethal
	if (myMesh->GetChildComponent(0))
		Cast<UPrimitiveComponent>(myMesh->GetChildComponent(0))->SetGenerateOverlapEvents(true);
	
	float relaxTime;
	if(left){
		//start the attack
		superHitL.myAnim->RateScale = superHitL.speed;
		myMesh->PlayAnimation(superHitL.myAnim, false);
		advanceAtk = superHitL.advanceAtkValue;
		relaxTime = superHitL.myAnim->SequenceLength/superHitL.speed;
	}
	else{		
		superHitR.myAnim->RateScale = superHitR.speed;
		myMesh->PlayAnimation(superHitR.myAnim, false);
		advanceAtk = superHitR.advanceAtkValue;
		relaxTime = superHitR.myAnim->SequenceLength/superHitR.speed;
	}
	mystate = attacking;
	
	if (inAir) {
		myCharMove->GravityScale = 0.0f;
		//myCharMove->StopMovementImmediately();
		//myCharMove->StopActiveMovement();		
		FVector oldMove = myCharMove->Velocity;
		oldMove = oldMove.ProjectOnTo(FVector(1.0f, 0.0f, 0.0f)) + oldMove.ProjectOnTo(FVector(0.0f, 1.0f, 0.0f));
		myCharMove->Velocity = oldMove / 2;
	}
	
	//start next relax
	GetWorldTimerManager().SetTimer(timerHandle, this, &AMyPlayerCharacter::Relax, relaxTime, false);
}
void AMyPlayerCharacter::AttackWalk(bool left){
	if (inAir && airAttackLocked)
		return;
	if (!inAir && attackLocked)
		return;
		
		if(attackChain.Num() == 0){
			if (inAir) {
				atkWalker = &attackAirList[0];
			}
			else {
				atkWalker = &attackList[0];
			}
		}
		
		//GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Orange, FString::Printf(TEXT("atkIndex: %d atkChainIndex: %d"), atkIndex, atkChainIndex));
		if (left) {
			if (atkWalker->left != NULL) {
				atkWalker = atkWalker->left;
				myCharMove->bOrientRotationToMovement = false;
				attackChain.Add(*atkWalker);
			}
			else {
				attackLocked = true;
				airAttackLocked = true;
			}
		}
		else {
			if (atkWalker->right != NULL) {
				atkWalker = atkWalker->right;
				myCharMove->bOrientRotationToMovement = false;
				attackChain.Add(*atkWalker);
			}
			else {
				attackLocked = true;
				airAttackLocked = true;
			}
		}
	
}
void AMyPlayerCharacter::NextComboHit(){
	if (atkIndex >= attackChain.Num()){
		//reset attackChain
		//atkWalker = &attackList[0];
		attackChain.Empty();
		atkIndex = 0;
		atkChainIndex = 0;
		airAttackLocked = true;
		attackLocked = false;
		if(debugInfo)
			GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::White, TEXT("reset attack chain"));
		Relax();
	}
	else {
		mystate = attacking;
		updateAtkDir = true;
		knockingDown = attackChain[atkIndex].knockDown;
		if (knockingDown)
			attackPower = attackKDPower;
		else
			attackPower = attackNormalPower;
		
		//clear current relax timer
		world->GetTimerManager().ClearTimer(timerHandle);

		//reset previous
		attackChain[atkChainIndex].myAnim->RateScale = 1.0f;

		//play attack animation
		atkChainIndex = atkIndex;
		attackChain[atkIndex].myAnim->RateScale = attackChain[atkIndex].speed;
		if(inAir){
			myCharMove->GravityScale = 0.0f;
			//myCharMove->StopMovementImmediately();
			//myCharMove->StopActiveMovement();		
			FVector oldMove = myCharMove->Velocity;
			oldMove = oldMove.ProjectOnTo(FVector(1.0f,0.0f,0.0f)) + oldMove.ProjectOnTo(FVector(0.0f, 1.0f, 0.0f));
			myCharMove->Velocity = oldMove/2;
		}
		else{
			advanceAtk = attackChain[atkIndex].advanceAtkValue;
		}
		myMesh->PlayAnimation(attackChain[atkIndex].myAnim, false);
		atkChainIndex++;		

		//call timer to start being lethal
		float time2lethal = attackChain[atkIndex].time2lethal*(attackChain[atkIndex].myAnim->SequenceLength/ attackChain[atkIndex].speed);
		GetWorldTimerManager().SetTimer(timerHandle, this, &AMyPlayerCharacter::StartLethal, time2lethal, false);
	}
}
void AMyPlayerCharacter::StartLethal(){
	updateAtkDir = false;
	if (myMesh->GetChildComponent(0))
		Cast<UPrimitiveComponent>(myMesh->GetChildComponent(0))->SetGenerateOverlapEvents(true);
	float time2NotLethal = attackChain[atkIndex].lethalTime*(attackChain[atkIndex].myAnim->SequenceLength / attackChain[atkIndex].speed);
	GetWorldTimerManager().SetTimer(timerHandle, this, &AMyPlayerCharacter::StopLethal, time2NotLethal, false);
}
void AMyPlayerCharacter::StopLethal(){
	advanceAtk = 0.0f;
	knockingDown = false;
	attackPower = attackNormalPower;
	if (myMesh->GetChildComponent(0))
		Cast<UPrimitiveComponent>(myMesh->GetChildComponent(0))->SetGenerateOverlapEvents(false);
	float time4NextHit = (1-(attackChain[atkIndex].time2lethal+attackChain[atkIndex].lethalTime))*(attackChain[atkIndex].myAnim->SequenceLength / attackChain[atkIndex].speed)+ attackChain[atkIndex].coolDown;
	atkIndex++;
	GetWorldTimerManager().SetTimer(timerHandle, this, &AMyPlayerCharacter::NextComboHit, time4NextHit, false);
}
void AMyPlayerCharacter::Relax(){
	ResetAnims();
	arising = false;
	advanceAtk = 0.0f;
	//atk1Index = 0;
	//myAnimBP->attackIndex = atk1Index;
	knockDownIndex = 0;
	//myAnimBP->knockDown = knockDownIndex;
	mystate = idle;

	if (myMesh->GetChildComponent(0))
		Cast<UPrimitiveComponent>(myMesh->GetChildComponent(0))->SetGenerateOverlapEvents(false);
	
	world->GetTimerManager().ClearTimer(timerHandle);
	if (debugInfo)
		GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Yellow, TEXT("relaxed!"));
}
void AMyPlayerCharacter::CancelKnockDownPrepare(bool left){
	if(left){
		atk1Hold = false;
	}
	else{
		atk2Hold = false;
	}
	if (!atk1Hold && !atk2Hold) {
		knockDownIndex = 0;
		//myAnimBP->knockDown = knockDownIndex;
		if (attackChain.Num() == 0) {
			Relax();
		}
		if (debugInfo)
			GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Blue, TEXT("knockDownAtk cancelled"));
	}
}
void AMyPlayerCharacter::CancelAttack() {
	world->GetTimerManager().ClearTimer(timerHandle);
	CancelKnockDownPrepare(true);
	CancelKnockDownPrepare(false);
	attackChain.Empty();
	atkIndex = 0;
	atkChainIndex = 0;
	airAttackLocked = false;
	attackLocked = false;
	
	//stop being lethal
	if (myMesh->GetChildComponent(0))
		Cast<UPrimitiveComponent>(myMesh->GetChildComponent(0))->SetGenerateOverlapEvents(false);

	myMesh->Stop();
	ResetAnims();
}
void AMyPlayerCharacter::ResetAnims(){
	if (myMesh->GetAnimationMode() != EAnimationMode::AnimationBlueprint) {
		myMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		myAnimBP = Cast<UAnimComm>(myMesh->GetAnimInstance());
		if (debugInfo)
			GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Blue, TEXT("following the anim blueprint again!"));
	}
	myAnimBP->damageIndex = 0;
}

void AMyPlayerCharacter::ResetAttacks(){
	
	if (!myAnimBP) {
		GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Blue, TEXT("could not get anim comm instance"));
		return;
	}

	myAnimBP->attackIndex = 0;
	mystate = idle;
	if (debugInfo)
		GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Blue, TEXT("entered ResetAttacks"));

	// Ensure the fuze timer is cleared by using the timer handle
	world->GetTimerManager().ClearTimer(timerHandle);
}

void AMyPlayerCharacter::Listen4Dash() {
	if (!dashing && player->WasInputKeyJustPressed(dashKey)) {
		dashDesire = true;
		dashStart = mytime;
	}
	if (player->WasInputKeyJustReleased(dashKey)) {
		dashDesire = false;
	}
}
void AMyPlayerCharacter::Listen4Move(float DeltaTime){
	if (player->WasInputKeyJustPressed(jumpKey))
	{
		myAnimBP->jumped = true;
		if (debugInfo)
			GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Yellow, TEXT("pressed jump key..."));
		Jump();
	}
	if (player->WasInputKeyJustReleased(jumpKey))
	{
		myAnimBP->jumped = false;
		StopJumping();
	}				
	MoveForward(vertIn);
	MoveRight(horIn);	

	/*
	//print speeds for animation
	if (debugInfo) {
		GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Orange, FString::Printf(TEXT("[player anim] speedv: %f speedh: %f"), myAnimBP->speedv, myAnimBP->speedh));
	}
	*/
}
void AMyPlayerCharacter::Listen4Look(){
	
	Turn(player->GetInputAnalogKeyState(horizontalCam));
	LookUp((-1)*player->GetInputAnalogKeyState(verticalCam));
}
void AMyPlayerCharacter::Listen4Attack(){
	if (player->WasInputKeyJustPressed(atk1Key))
	{
		//GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Yellow, TEXT("pressed atk1 key..."));
		Attack1Press();
	}
	if (player->WasInputKeyJustReleased(atk1Key))
	{
		//GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Yellow, TEXT("released atk1 key..."));
		Attack1Release();
	}
	if (player->WasInputKeyJustPressed(atk2Key))
	{
		//GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Yellow, TEXT("pressed atk2 key..."));
		Attack2Press();
	}
	if (player->WasInputKeyJustReleased(atk2Key))
	{
		//GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Yellow, TEXT("released atk2 key..."));
		Attack2Release();
	}
	//check knockDown
	if (attackChain.Num() == 0) {
		if (atk1Hold && mytime - startedHold1 >= holdTimeMin) {
			//try to start a knockdown prepare
			if (!inAir) {
				if ((mystate == attacking || mystate == idle) && knockDownIndex == 0) {

					world->GetTimerManager().ClearTimer(timerHandle);
					//old way, with the animation blueprint
					knockDownIndex = 1;
					//myAnimBP->knockDown = knockDownIndex;

					//start the attack
					prepSuperHitL.myAnim->RateScale = prepSuperHitL.speed;
					myMesh->PlayAnimation(prepSuperHitL.myAnim, false);
					if (debugInfo)
						GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Blue, TEXT("knockDownAtk started"));
				}
			}
			if (mytime - startedHold1 > holdTimeMax) {
				CancelKnockDownPrepare(true);
			}
		}
		if (atk2Hold && mytime - startedHold2 >= holdTimeMin) {
			//try to start a knockdown prepare
			if (!inAir) {
				if ((mystate == attacking || mystate == idle) && knockDownIndex == 0) {

					world->GetTimerManager().ClearTimer(timerHandle);
					//old way, with the animation blueprint
					knockDownIndex = 1;
					//myAnimBP->knockDown = knockDownIndex;

					//start the attack
					prepSuperHitR.myAnim->RateScale = prepSuperHitR.speed;
					myMesh->PlayAnimation(prepSuperHitR.myAnim, false);
					if (debugInfo)
						GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Blue, TEXT("knockDownAtk started"));
				}
			}
			if (mytime - startedHold2 > holdTimeMax) {
				CancelKnockDownPrepare(false);
			}
		}
	}
	else {
		if (atkChainIndex == 0)
			NextComboHit();
	}
}
void AMyPlayerCharacter::Advance(){	
	if ((Controller != NULL) && (advanceAtk != 0.0f))
	{
		// get forward vector
		const FVector Direction = GetActorForwardVector();
		AddMovementInput(Direction, advanceAtk);
	}
}
void AMyPlayerCharacter::OnCompHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if(OtherActor!=NULL && OtherActor!=this && OtherComp != NULL){
		if (debugInfo){
			//GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Red, FString::Printf(TEXT("I %s just hit: %s"), GetName(), *OtherActor->GetName()));
			GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Orange, "[player ComponentHit] my name: " + GetName()+"hit obj: "+ *OtherActor->GetName());
		}
	}
}

void AMyPlayerCharacter::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && (OtherActor != this) && OtherComp)
	{
		AMutationChar *myAlgoz = Cast<AMutationChar>(OtherActor);

		if (myAlgoz) {
			if (debugInfo)
				GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Green, "[player OverlapBegin] my name: " + GetName() + "hit obj: " + *OtherActor->GetName());
			
			//only call damage if not already in some damage state
			if((int)mystate < 3 || (int)mystate > 8) {
				myAlgoz->attackConnected = true;
				damageDir = GetActorLocation() - OtherActor->GetActorLocation();
				damageDir.Normalize();
				MyDamage(myAlgoz->damagePower, myAlgoz->knockingDown, myAlgoz->spiralAtk);
			}
		}
	}
}

void AMyPlayerCharacter::MyDamage(float DamagePower, bool KD, bool Spiral) {
	CancelAttack();
	UGameplayStatics::SetGlobalTimeDilation(world, 1.0f);
		
	flying = false;

	life -= DamagePower;
	if (life <= 0)
		Death();

	//reactivate anim blueprint
	ResetAnims();

	recoilValue = 1.0f;

	if (KD){
		targetLocked = false;
		mystate = kdTakeOff;
		myCharMove->MovementMode = MOVE_Flying;
		myCharMove->MaxWalkSpeed = normalSpeed;
		myCharMove->MaxFlySpeed = normalSpeed;
		myCharMove->MaxAcceleration = normalAcel;
		myCharMove->AirControl = normalAirCtrl;
		myCharMove->GravityScale = 1.0f;

		if(Spiral)
			myAnimBP->damageIndex = 10;
		else
			myAnimBP->damageIndex = 11;

		FVector myLoc = GetActorLocation();

		//to start the knockDown you look to your algoz
		FRotator lookToRot = UKismetMathLibrary::FindLookAtRotation(myLoc, myLoc - damageDir*500.0f);
		const FRotator lookToYaw(0, lookToRot.Yaw, 0);
		SetActorRotation(lookToYaw);

		//hit pause, or, in this case, time dilation
		UGameplayStatics::SetGlobalTimeDilation(world, 0.1f);
		//start delayed takeoff
		GetWorldTimerManager().SetTimer(timerHandle, this, &AMyPlayerCharacter::DelayedKDtakeOff, hitPause, false);	
	}
	else {
		mystate = suffering;
		
		int damageAnimChoice = FMath::RandRange(0, 10);
		if (damageAnimChoice <= 3) {
			myAnimBP->damageIndex = 1;
		}
		else {
			if (damageAnimChoice <= 6)
				myAnimBP->damageIndex = 2;
			else
				myAnimBP->damageIndex = 3;
		}

		//hit pause, or, in this case, time dilation
		UGameplayStatics::SetGlobalTimeDilation(world, 0.1f);
		//start stabilize timer
		GetWorldTimerManager().SetTimer(timerHandle, this, &AMyPlayerCharacter::DelayedStabilize, hitPause, false);
	}
}
void AMyPlayerCharacter::DelayedKDtakeOff() {
	UGameplayStatics::SetGlobalTimeDilation(world, 1.0f);
	GetWorldTimerManager().SetTimer(timerHandle, this, &AMyPlayerCharacter::KnockDownFlight, takeOffTime, false);	
}
void AMyPlayerCharacter::KnockDownFlight() {
	mystate = kdFlight;
	myCharMove->MovementMode = MOVE_Walking;
	myAnimBP->damageIndex = 20;
}
void AMyPlayerCharacter::DelayedStabilize() {
	UGameplayStatics::SetGlobalTimeDilation(world, 1.0f);
	GetWorldTimerManager().SetTimer(timerHandle, this, &AMyPlayerCharacter::Stabilize, damageTime, false);
}
void AMyPlayerCharacter::DelayedHookPull() {
	myAnimBP->attackIndex = 11;
	mystate = hookFlying;
	//release time and change state
	UGameplayStatics::SetGlobalTimeDilation(world, 1.0f);
}
void AMyPlayerCharacter::DelayedKDground() {
	mystate = kdRise;
}
void AMyPlayerCharacter::DelayedAtkRise() {
	if(debugInfo)
		GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Yellow, "[player] finished rise.");
	advanceAtk = 0.0f;
	knockingDown = false;
	attackPower = attackNormalPower;
	if (myMesh->GetChildComponent(0))
		Cast<UPrimitiveComponent>(myMesh->GetChildComponent(0))->SetGenerateOverlapEvents(false);
		
	GetWorldTimerManager().SetTimer(timerHandle, this, &AMyPlayerCharacter::Relax, riseAtkCoolDown, false);
}
void AMyPlayerCharacter::Stabilize() {
	recoilValue = 0.0f;
	myAnimBP->damageIndex = 0;
	
	mystate = idle;
}
void AMyPlayerCharacter::Death() {
	GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Red, "[player] is dead.");
}
