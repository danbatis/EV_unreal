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

	//collisionCapsule->OnComponentHit.AddDynamic(this, &AMyPlayerCharacter::OnCompHit);
	//collisionCapsule->OnComponentBeginOverlap.AddDynamic(this, &AMyPlayerCharacter::OnBeginOverlap);

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

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
	
}

// Called when the game starts or when spawned
void AMyPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();	

	myState = idle;
	//getting my basic animation blueprint communication class
	myAnimBP = Cast<UAnimComm>(GetMesh()->GetAnimInstance());
	Cast<UPrimitiveComponent>(GetMesh()->GetChildComponent(0))->SetGenerateOverlapEvents(false);
	player = UGameplayStatics::GetPlayerController(GetWorld(), 0);//crashes if in the constructor	

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
		if (player->PlayerInput->ActionMappings[i].ActionName.GetPlainNameString() == "Shield") {
			shieldKey = player->PlayerInput->ActionMappings[i].Key;
			shieldKey_i = i;
		}
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
		
	//use walker to show links work
	//GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::White, "leftAttack chain:");
	UE_LOG(LogTemp, Warning, TEXT("walking in atk node always to the left"));
	atkWalker = &attackList[0];	
	while(atkWalker->left != NULL){
		atkWalker = atkWalker->left;
		//GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::White, "animName: " + atkWalker->myAnim->GetFName().GetPlainNameString());
		UE_LOG(LogTemp, Warning, TEXT("leftAttack chain : %s"), *atkWalker->myAnim->GetFName().GetPlainNameString());
	}
	//restarting attack walker
	atkWalker = &attackList[0];
}

// Called every frame
void AMyPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	mytime += DeltaTime;

	//if (player->WasInputKeyJustPressed(player->PlayerInput->ActionMappings[7].Key))
	//if(player->WasInputKeyJustPressed(EKeys::Y))

	//GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Yellow, FString::Printf(TEXT("vertical: %f"), player->GetInputAxisValue("MoveForward")));
	GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Yellow, FString::Printf(TEXT("horizontal: %f"), player->GetInputAnalogKeyState(horizontalIn)));	
	
	if(player->WasInputKeyJustPressed(atk1Key))
	{
		GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Yellow, TEXT("pressed atk1 key..."));
		Attack1Press();
	}
	if (player->WasInputKeyJustReleased(atk1Key))
	{
		GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Yellow, TEXT("released atk1 key..."));
		Attack1Release();
	}	
	if (player->WasInputKeyJustPressed(atk2Key))
	{
		GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Yellow, TEXT("pressed atk2 key..."));
		Attack2Press();
	}
	if (player->WasInputKeyJustReleased(atk2Key))
	{
		GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Yellow, TEXT("released atk2 key..."));
		Attack2Release();
	}
	
	if (!myAnimBP) return;
	inAir = GetMovementComponent()->IsFalling();
	//check knockDown
	if (attackChain.Num() == 0) {
		if (atk1Hold && mytime - startedHold1 >= holdTimeMin) {
			//try to start a knockdown prepare
			if (!inAir) {
				if ((myState == attacking || myState == idle) && knockDownIndex == 0) {

					GetWorld()->GetTimerManager().ClearTimer(timerHandle);
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
				if ((myState == attacking || myState == idle) && knockDownIndex == 0) {

					GetWorld()->GetTimerManager().ClearTimer(timerHandle);
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
	else{
		if(atkChainIndex==0)
			NextComboHit();
	}

	myAnimBP->speed = GetVelocity().Size();
	myAnimBP->inAir = inAir;
}

// Called to bind functionality to input
void AMyPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	//PlayerInputComponent->BindAction("Attack1", IE_Pressed,this, &AMyPlayerCharacter::Attack1Press);
	//PlayerInputComponent->BindAction("Attack1", IE_Released, this, &AMyPlayerCharacter::Attack1Release);
	//PlayerInputComponent->BindAction("Attack2", IE_Pressed, this, &AMyPlayerCharacter::Attack2Press);
	//PlayerInputComponent->BindAction("Attack2", IE_Released, this, &AMyPlayerCharacter::Attack2Release);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMyPlayerCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMyPlayerCharacter::MoveRight);
	
	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick	
	PlayerInputComponent->BindAxis("TurnRate", this, &AMyPlayerCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMyPlayerCharacter::LookUpAtRate);
	
	PlayerInputComponent->BindAxis("Turn", this, &AMyPlayerCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AMyPlayerCharacter::LookUp);
	//PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	//PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	
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
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
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
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AMyPlayerCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AMyPlayerCharacter::MoveRight(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
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
			CancelKnockDownPrepare(true);
			AttackWalk(true);
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
			CancelKnockDownPrepare(false);
			AttackWalk(false);
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
	GetWorld()->GetTimerManager().ClearTimer(timerHandle);
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
				attackChain.Add(*atkWalker);
			}
			else {
				attackLocked = true;
			}
		}
		else {
			if (atkWalker->right != NULL) {
				atkWalker = atkWalker->right;
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
		//clear current relax timer
		GetWorld()->GetTimerManager().ClearTimer(timerHandle);

		//reset previous
		attackChain[atkChainIndex].myAnim->RateScale = 1.0f;

		//play attack animation
		atkChainIndex = atkIndex;
		attackChain[atkIndex].myAnim->RateScale = attackChain[atkIndex].speed;
		GetMesh()->PlayAnimation(attackChain[atkIndex].myAnim, false);
		atkChainIndex++;
		GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::White, TEXT("play attack"));

		//call timer to start being lethal
		float time2lethal = attackChain[atkIndex].time2lethal*(attackChain[atkIndex].myAnim->SequenceLength/ attackChain[atkIndex].speed);
		GetWorldTimerManager().SetTimer(timerHandle, this, &AMyPlayerCharacter::StartLethal, time2lethal, false);
	}
}
void AMyPlayerCharacter::StartLethal(){
	Cast<UPrimitiveComponent>(GetMesh()->GetChildComponent(0))->SetGenerateOverlapEvents(true);
	float time2NotLethal = attackChain[atkIndex].lethalTime*(attackChain[atkIndex].myAnim->SequenceLength / attackChain[atkIndex].speed);
	GetWorldTimerManager().SetTimer(timerHandle, this, &AMyPlayerCharacter::StopLethal, time2NotLethal, false);
}
void AMyPlayerCharacter::StopLethal(){
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
	myState = idle;			
	
	GetWorld()->GetTimerManager().ClearTimer(timerHandle);
	GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Blue, TEXT("relaxed!"));
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
		if(attackChain.Num()==0)
			Relax();
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
	myState = idle;
	GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Blue, TEXT("entered ResetAttacks"));

	// Ensure the fuze timer is cleared by using the timer handle
	GetWorld()->GetTimerManager().ClearTimer(timerHandle);	
}


void AMyPlayerCharacter::OnCompHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if(OtherActor!=NULL && OtherActor!=this && OtherComp != NULL){
		if(GEngine){
			//GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Red, FString::Printf(TEXT("I %s just hit: %s"), GetName(), *OtherActor->GetName()));
			GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Orange, "[ComponentHit] my name: " + GetName()+"hit obj: "+ *OtherActor->GetName());
		}
	}
}

void AMyPlayerCharacter::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	GEngine->AddOnScreenDebugMessage(-1, msgTime, FColor::Green, "[OverlapBegin] my name: " + GetName() + "hit obj: " + *OtherActor->GetName());
}
