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
	myAnimBP = Cast<UAnimComm>(GetMesh()->GetAnimInstance());
	Cast<UPrimitiveComponent>(GetMesh()->GetChildComponent(0))->SetGenerateOverlapEvents(false);

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
	//restarting attack walker
	atkWalker = &attackList[0];

	//now find enemies	
	for (TActorIterator<AMosquitoCharacter> it(world, AMosquitoCharacter::StaticClass()); it; ++it)
	{
		AMosquitoCharacter* actor = *it;
		if (actor != NULL) {
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, "Mosquito found name: " + actor->GetName());
			mosquitos.Add(actor);
		}
	}
	
	player->GetViewportSize(width, height);
	myCharMove = GetCharacterMovement();

	myCharMove->MaxWalkSpeed = normalSpeed;
	myCharMove->MaxAcceleration = normalAcel;
	myCharMove->AirControl = normalAirCtrl;
}

// Called every frame
void AMyPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	mytime += DeltaTime;
	inAir = GetMovementComponent()->IsFalling() || flying;
	//if (inAir) {
	//	GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Blue, TEXT("falling..."));
	//}
	myRotation = Controller->GetControlRotation();
	YawRotation = FRotator(0, myRotation.Yaw, 0);
	
	forthVec = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	rightVec = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	vertIn = player->GetInputAnalogKeyState(vertical_Up) + (-1)*player->GetInputAnalogKeyState(vertical_Down);
	horIn = player->GetInputAnalogKeyState(horizontal_R) + (-1)*player->GetInputAnalogKeyState(horizontal_L);
	
	UE_LOG(LogTemp, Warning, TEXT("player state: %d | attackChain size: %d"), (int)mystate, attackChain.Num());
	
	if (player->IsInputKeyDown(EKeys::I)) {
		player->ProjectWorldLocationToScreen(mosquitos[0]->GetActorLocation(), targetScreenPos, false);
		targetScreenPos.X /= width;
		targetScreenPos.Y /= height;
		UE_LOG(LogTemp, Warning, TEXT("target at: X: %f Y: %f"), targetScreenPos.X, targetScreenPos.Y);
	}
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

	switch (mystate){
		case idle:
			myCharMove->GravityScale = 1.0f;
			
			if(player->WasInputKeyJustPressed(EKeys::T)){
				const FVector SpawnPosition = GetActorLocation() + GetActorForwardVector()*50.0f;
				FActorSpawnParameters SpawnInfo;
								
				AMosquitoCharacter* newMosquito;
				if (player->IsInputKeyDown(EKeys::B)) {
					// spawn new mosquito using blueprint
					newMosquito = GWorld->SpawnActor<AMosquitoCharacter>(MosquitoClass, SpawnPosition, FRotator::ZeroRotator, SpawnInfo);
					
				}
				else {
					//newMosquito = GWorld->SpawnActor<AMosquitoCharacter>(mosquitos[0]->GetClass(), SpawnPosition, FRotator::ZeroRotator, SpawnInfo);
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
					GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Red, TEXT("failed to place new mosquito!"));
				}
			}
			

			if (player->WasInputKeyJustPressed(grabKey)){
				//grab last mosquito
				FVector mosquitoTarget = mosquitos[mosquitos.Num() - 1]->GetActorLocation() - GetActorLocation();				
				
				FVector upVec = FRotationMatrix(myRotation).GetUnitAxis(EAxis::Z);

				grabForth = mosquitoTarget.ProjectOnTo(forthVec).Size();
				grabRight = mosquitoTarget.ProjectOnTo(rightVec).Size();
				grabHeight = mosquitoTarget.ProjectOnTo(upVec).Size();
			}
			if (player->IsInputKeyDown(grabKey)){				
				FVector upVec = FRotationMatrix(myRotation).GetUnitAxis(EAxis::Z);

				//hold last mosquito
				FVector mosquitoPos = grabForth* forthVec + grabRight* rightVec + grabHeight* upVec+GetActorLocation();
				mosquitos[mosquitos.Num() - 1]->SetActorLocation(mosquitoPos);
				DrawDebugLine(world, GetActorLocation(), GetActorLocation() + forthVec*throwPower, FColor(255, 255, 0), true, 0.1f, 0, 5.0);
			}
			if (player->WasInputKeyJustReleased(grabKey)){
				//throw him
				mosquitos[mosquitos.Num() - 1]->GetCharacterMovement()->Velocity = throwPower * forthVec;
			}

			if(player->WasInputKeyJustPressed(hookKey)){
				//freeze time
				UGameplayStatics::SetGlobalTimeDilation(world, aimTimeDilation);
			}
			if(player->WasInputKeyJustReleased(hookKey)){
				//release time and change state
				UGameplayStatics::SetGlobalTimeDilation(world, 1.0f);
				
				//play animation to pull


				mystate = hookFlying;
			}

			Listen4Attack();
			Listen4Move();
			Listen4Look();
			Listen4Dash();
			Reorient();
			break;
		case attacking:			
			Advance();
			Listen4Attack();
			Listen4Look();
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
			FVector targetDir = mosquitos[mosquitos.Num() - 1]->GetActorLocation() - GetActorLocation();
			float distToTarget = targetDir.Size();
			if( distToTarget < disengageHookDist)
			{
				flying = false;
				myCharMove->MovementMode = MOVE_Walking;
				myCharMove->MaxWalkSpeed = normalSpeed;
				myCharMove->AirControl = normalAirCtrl;
				myCharMove->GravityScale = 1.0f;
				mystate = idle;
			}
			else
			{
				flying = true;
				myCharMove->MovementMode = MOVE_Flying;
				myCharMove->GravityScale = 0.0f;
				myCharMove->MaxWalkSpeed = dashSpeed;
				//AddMovementInput(targetDir, 1.0f);
				myCharMove->Velocity = targetDir;
				//myCharMove->Velocity = forthVec * throwPower;
				myCharMove->AirControl = 0.0f;
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
	//PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	//PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	//PlayerInputComponent->BindAction("Attack1", IE_Pressed,this, &AMyPlayerCharacter::Attack1Press);
	//PlayerInputComponent->BindAction("Attack1", IE_Released, this, &AMyPlayerCharacter::Attack1Release);
	//PlayerInputComponent->BindAction("Attack2", IE_Pressed, this, &AMyPlayerCharacter::Attack2Press);
	//PlayerInputComponent->BindAction("Attack2", IE_Released, this, &AMyPlayerCharacter::Attack2Release);

	//PlayerInputComponent->BindAxis("MoveForward", this, &AMyPlayerCharacter::MoveForward);
	//PlayerInputComponent->BindAxis("MoveRight", this, &AMyPlayerCharacter::MoveRight);
	
	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick	
	//PlayerInputComponent->BindAxis("TurnRate", this, &AMyPlayerCharacter::TurnAtRate);
	//PlayerInputComponent->BindAxis("LookUpRate", this, &AMyPlayerCharacter::LookUpAtRate);
	
	//PlayerInputComponent->BindAxis("Turn", this, &AMyPlayerCharacter::Turn);
	//PlayerInputComponent->BindAxis("LookUp", this, &AMyPlayerCharacter::LookUp);	
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
	if (player->WasInputKeyJustPressed(targetLockKey)) {
		startReorient = mytime;
	}
	if (player->IsInputKeyDown(targetLockKey)) {
		
		FVector myLoc = GetActorLocation();
		FVector targetLoc = mosquitos[0]->GetActorLocation();
							
		FVector forthLoc = myLoc + (targetLoc - myLoc).Size()*forthVec;
		
		//GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Blue, FString::Printf(TEXT("size vector: %f"), (targetLoc - myLoc).Size()));

		myCharMove->bOrientRotationToMovement = false;
		FRotator lookToTarget = UKismetMathLibrary::FindLookAtRotation(myLoc, targetLoc);
		const FRotator lookToTargetYaw(0, lookToTarget.Yaw, 0);
		SetActorRotation(lookToTargetYaw);
		
		//DrawDebugSphere(world, myLoc, 10.0f, 20, FColor(255, 0, 0), true, -1, 0, 2);
		//DrawDebugSphere(world, myLoc + forthVec * 50.0f + FVector(0.0f, 0.0f, 50.0f), 10.0f, 20, FColor(255, 0, 0), true, -1, 0, 2);

		if (startReorient == 0.0f) {
			Controller->SetControlRotation(lookToTarget);
			//GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, "[oriented] ");
		}
		else {
			//GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, "[orienting] ");
			float reorientGain = (mytime - startReorient) / reorientTime;
			if (reorientGain >= 1.0f)
				startReorient = 0.0f;
			
			FVector immediateTarget = forthLoc+(targetLoc - forthLoc)*reorientGain;

			DrawDebugLine(world, myLoc, myLoc + forthVec * 100.0f *reorientGain, FColor(255, 0, 0), true, -1, 0, 5.0);
			DrawDebugLine(world, myLoc, immediateTarget, FColor(0, 255, 0), true, -1, 0, 5.0);

			//calculate intermediate position for the smooth lock on
			FRotator almostLookToTarget = UKismetMathLibrary::FindLookAtRotation(myLoc, immediateTarget);
			Controller->SetControlRotation(almostLookToTarget);
		}
		
	}
	else {
		myCharMove->bOrientRotationToMovement = true;
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
void AMyPlayerCharacter::Look2Dir(FVector LookDir) {
	FVector myLoc = GetActorLocation();
	FVector targetLoc = myLoc + LookDir;
	FRotator lookToTarget = UKismetMathLibrary::FindLookAtRotation(myLoc, targetLoc);
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
	GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Orange, "[ComponentName]: " + GetMesh()->GetChildComponent(0)->GetName());
	GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Blue, TEXT("attack2!"));
	GetMesh()->PlayAnimation(attackList[1].myAnim, false);
	GetWorldTimerManager().SetTimer(timerHandle, this, &AMyPlayerCharacter::ResetAnims, 1.5f, false);
	*/
}
void AMyPlayerCharacter::Attack1Release() {
	if(atk1Hold){
		if (mytime - startedHold1 < holdTimeMin) {
			GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Blue, TEXT("quick attack1 press"));
			
			AttackWalk(true);
			CancelKnockDownPrepare(true);
		}
		else {
			GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Blue, TEXT("long attack1 press"));
			atk1Hold = false;
			KnockDownHit(true);
		}
	}
}
void AMyPlayerCharacter::Attack2Release() {
	if (atk2Hold) {
		if (mytime - startedHold2 < holdTimeMin) {
			GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Blue, TEXT("quick attack2 press"));
			
			AttackWalk(false);
			CancelKnockDownPrepare(false);
		}
		else {
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

	if(left){
		//start the attack
		GetMesh()->PlayAnimation(superHitL, false);
	}
	else{
		GetMesh()->PlayAnimation(superHitR, false);
	}
	
	//start next relax
	float relaxTime = 1.0f;// superHitL->GetPlayLength();
	GetWorldTimerManager().SetTimer(timerHandle, this, &AMyPlayerCharacter::Relax, relaxTime, false);
}
void AMyPlayerCharacter::AttackWalk(bool left){
	if(!attackLocked){
		if (left) {
			if (atkWalker->left != NULL) {
				atkWalker = atkWalker->left;
				myCharMove->bOrientRotationToMovement = false;
				attackChain.Add(*atkWalker);
			}
			else {
				attackLocked = true;
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
			}
		}
	}
}
void AMyPlayerCharacter::NextComboHit(){
	if (atkIndex >= attackChain.Num()){
		//reset attackChain
		attackChain.Empty();
		atkIndex = 0;
		atkChainIndex = 0;
		//restarting attack walker
		atkWalker = &attackList[0];
		attackLocked = false;
		GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::White, TEXT("reset attack chain"));
		Relax();
	}
	else {		
		mystate = attacking;
		updateAtkDir = true;
		
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
		GetMesh()->PlayAnimation(attackChain[atkIndex].myAnim, false);
		atkChainIndex++;
		//GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::White, TEXT("play attack"));

		//call timer to start being lethal
		float time2lethal = attackChain[atkIndex].time2lethal*(attackChain[atkIndex].myAnim->SequenceLength/ attackChain[atkIndex].speed);
		GetWorldTimerManager().SetTimer(timerHandle, this, &AMyPlayerCharacter::StartLethal, time2lethal, false);
	}
}
void AMyPlayerCharacter::StartLethal(){
	updateAtkDir = false;
	Cast<UPrimitiveComponent>(GetMesh()->GetChildComponent(0))->SetGenerateOverlapEvents(true);
	float time2NotLethal = attackChain[atkIndex].lethalTime*(attackChain[atkIndex].myAnim->SequenceLength / attackChain[atkIndex].speed);
	GetWorldTimerManager().SetTimer(timerHandle, this, &AMyPlayerCharacter::StopLethal, time2NotLethal, false);
}
void AMyPlayerCharacter::StopLethal(){
	advanceAtk = 0.0f;
	Cast<UPrimitiveComponent>(GetMesh()->GetChildComponent(0))->SetGenerateOverlapEvents(false);
	float time4NextHit = (1-(attackChain[atkIndex].time2lethal+attackChain[atkIndex].lethalTime))*(attackChain[atkIndex].myAnim->SequenceLength / attackChain[atkIndex].speed)+ attackChain[atkIndex].coolDown;
	atkIndex++;
	GetWorldTimerManager().SetTimer(timerHandle, this, &AMyPlayerCharacter::NextComboHit, time4NextHit, false);
}
void AMyPlayerCharacter::Relax(){
	ResetAnims();
	//atk1Index = 0;
	//myAnimBP->attackIndex = atk1Index;
	knockDownIndex = 0;
	//myAnimBP->knockDown = knockDownIndex;
	mystate = idle;			
	
	world->GetTimerManager().ClearTimer(timerHandle);
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
		GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Blue, TEXT("knockDownAtk cancelled"));
	}
}

void AMyPlayerCharacter::ResetAnims(){
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	myAnimBP = Cast<UAnimComm>(GetMesh()->GetAnimInstance());
	GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Blue, TEXT("following the anim blueprint again!"));
}

void AMyPlayerCharacter::ResetAttacks(){
	
	if (!myAnimBP) {
		GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Blue, TEXT("could not get anim comm instance"));
		return;
	}

	myAnimBP->attackIndex = 0;
	mystate = idle;
	GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Blue, TEXT("entered ResetAttacks"));

	// Ensure the fuze timer is cleared by using the timer handle
	world->GetTimerManager().ClearTimer(timerHandle);	
}

void AMyPlayerCharacter::Listen4Dash() {
	if (player->IsInputKeyDown(dashKey)) {
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
	}
}
void AMyPlayerCharacter::Listen4Move(){
	if (player->WasInputKeyJustPressed(jumpKey))
	{
		GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Yellow, TEXT("pressed jump key..."));
		Jump();
	}
	if (player->WasInputKeyJustReleased(jumpKey))
	{
		StopJumping();
	}				
	MoveForward(vertIn);
	MoveRight(horIn);

	if (!myAnimBP) return;	
	myAnimBP->inAir = inAir;
	myVel = GetVelocity();

	if (lookInCamDir) {
		myCharMove->bOrientRotationToMovement = false;
			
		myAnimBP->speedv = 100.0f*(FVector::DotProduct(myVel, forthVec) / myCharMove->MaxWalkSpeed);
		myAnimBP->speedh = 100.0f*(FVector::DotProduct(myVel, rightVec) / myCharMove->MaxWalkSpeed);
	}
	else {
		myCharMove->bOrientRotationToMovement = true;
		myAnimBP->speedv = 100.0f*(FVector::VectorPlaneProject(myVel, FVector::UpVector).Size() / myCharMove->MaxWalkSpeed);
		myAnimBP->speedh = 0.0f;
	}	
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
					GetMesh()->PlayAnimation(prepSuperHitL, false);

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
					GetMesh()->PlayAnimation(prepSuperHitR, false);

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
		if(GEngine){
			//GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Red, FString::Printf(TEXT("I %s just hit: %s"), GetName(), *OtherActor->GetName()));
			GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Orange, "[player ComponentHit] my name: " + GetName()+"hit obj: "+ *OtherActor->GetName());
		}
	}
}

void AMyPlayerCharacter::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Green, "[player OverlapBegin] my name: " + GetName() + "hit obj: " + *OtherActor->GetName());
}
