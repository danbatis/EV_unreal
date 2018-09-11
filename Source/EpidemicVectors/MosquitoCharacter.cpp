// Fill out your copyright notice in the Description page of Project Settings.

#include "MosquitoCharacter.h"


// Sets default values
AMosquitoCharacter::AMosquitoCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	
	collisionCapsule = CreateDefaultSubobject<UBoxComponent>(TEXT("collisionCapsule"));
	//collisionCapsule->SetNotifyRigidBodyCollision(true);
	/*
	collisionCapsule->OnComponentHit.AddDynamic(this, &AMosquitoCharacter::OnCompHit);
	collisionCapsule->OnComponentBeginOverlap.AddDynamic(this, &AMosquitoCharacter::OnBeginOverlap);
	collisionCapsule->OnComponentEndOverlap.AddDynamic(this, &AMosquitoCharacter::OnOverlapEnd);
	
	GetMesh()->OnComponentHit.AddDynamic(this, &AMosquitoCharacter::OnCompHit);
	GetMesh()->OnComponentBeginOverlap.AddDynamic(this, &AMosquitoCharacter::OnBeginOverlap);
	GetMesh()->OnComponentEndOverlap.AddDynamic(this, &AMosquitoCharacter::OnOverlapEnd);

	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &AMosquitoCharacter::OnCompHit);
	*/	
	//OnActorBeginOverlap.AddDynamic(this, &AMosquitoCharacter::OnOverlap);	
}
/*
void AMosquitoCharacter::Recover_Implementation(float RecoverAmount)
{
	if (life + RecoverAmount <= maxLife) {
		life += RecoverAmount;
	}
}
void AMosquitoCharacter::Damage_Implementation(float DamagePower)
{
	life -= DamagePower;
	if (life <= 0.0f) {
		if (GEngine) {
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "I am dead: " + GetName());
		}
	}
}
*/

// Called when the game starts or when spawned
void AMosquitoCharacter::BeginPlay()
{
	Super::BeginPlay();

	life = 90.0f;
	maxLife = 100.0f;
	if (GEngine) {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("old life: %f"), life));
	}
	
	//Recover_Implementation(10.0f);
	Recover(10.0f);
	
	if (GEngine) {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("new life: %f"), life));
	}

	myAnimBP = Cast<USimpleMosquitoAComm>(GetMesh()->GetAnimInstance());
	myState = idle;
}

// Called every frame
void AMosquitoCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);	
}

// Called to bind functionality to input
void AMosquitoCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	//Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	check(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveForward", this, &AMosquitoCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMosquitoCharacter::MoveRight);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	
}

void AMosquitoCharacter::MoveForward(float Value)
{
	MoveDir(Value, EAxis::X);
}

void AMosquitoCharacter::MoveRight(float Value){
	MoveDir(Value, EAxis::Y);
}

void AMosquitoCharacter::MoveDir(float Value, EAxis::Type Dir)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(Dir);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void AMosquitoCharacter::OnCompHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor != NULL && OtherActor != this && OtherComp != NULL) {
		if (GEngine) {
			//GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, FString::Printf(TEXT("I %s just hit: %s"), GetName(), *OtherActor->GetName()));
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Orange, "[ComponentHit] my name: " + GetName() + "hit obj: " + *OtherActor->GetName());
		}
	}
}


void AMosquitoCharacter::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{	
	if (OtherActor && (OtherActor != this) && OtherComp)
	{
		if (GEngine) {
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Black, "[OverlapEnd] my name: " + GetName() + "hit obj: " + *OtherActor->GetName());
		}
	}
}


void AMosquitoCharacter::OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && (OtherActor != this) && OtherComp)
	{
		if (GEngine) {
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, "[OverlapBegin] my name: " + GetName() + "hit obj: " + *OtherActor->GetName());
		}
	}
}

void AMosquitoCharacter::MyDamage(float DamagePower) {
	if (GEngine) {
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, "[entered MyDamage] ");
	}
	myState = suffering;
	//play damage sound
	Damage(DamagePower);
	//play damage animation
	if(FMath::RandRange(0,10) < 5)
	myAnimBP->damage = 1;
	else
	myAnimBP->damage = 2;
	//start stabilize timer
	GetWorldTimerManager().SetTimer(timerHandle, this, &AMosquitoCharacter::Stabilize, damageTime, false);
}

void AMosquitoCharacter::OnOverlap(AActor* MyOverlappedActor, AActor* OtherActor)
{
	if (OtherActor && (OtherActor != this))
	{
		algoz = Cast<AMyPlayerCharacter>(OtherActor);
		if (GEngine) {
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, "[actorOverlap] my name: " + GetName() + " myComponent: " + *MyOverlappedActor->GetName() + " hit obj: " + *OtherActor->GetName());
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("[algoz] %d"), algoz->myState));
		}
		
		
		if (myState != suffering) {
			MyDamage(10.0f);
		}
		
	}
}

void AMosquitoCharacter::Stabilize(){
	myState = idle;
	myAnimBP->damage = 0;	
}

void AMosquitoCharacter::OnActorBeginOverlap(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult){
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, "[actorBeginOverlap] my name: " + GetName() +" myComponent: "+ *OverlappedComp->GetName() + " hit obj: " + *OtherActor->GetName());
}

/*
void AMosquitoCharacter::ReceiveHit(UPrimitiveComponent * MyComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult & Hit)
{
	//Super::ReceiveHit(MyComp, Other, OtherComp, bSelfMoved,HitLocation,HitNormal,NormalImpulse,Hit);
	if (OtherActor != NULL && OtherActor != this && OtherComp != NULL) {
		if (GEngine) {
			//GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, FString::Printf(TEXT("I %s just hit: %s"), GetName(), *OtherActor->GetName()));
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Orange, "[ReceiveHit] my name: " + GetName() + "hit obj: " + *OtherActor->GetName());
		}
	}
}
*/