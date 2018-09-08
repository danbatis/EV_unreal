// Fill out your copyright notice in the Description page of Project Settings.

#include "CubeCollision.h"


// Sets default values
ACubeCollision::ACubeCollision()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Use a sphere as a simple collision representation
	MyComp = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComp"));
	/*
	MyComp->SetSimulatePhysics(true);
	MyComp->SetNotifyRigidBodyCollision(true);
	MyComp->BodyInstance.SetCollisionProfileName("BlockAllDynamic");
	*/
	MyComp->OnComponentHit.AddDynamic(this, &ACubeCollision::OnCompHit);
	/*
	MyComp->OnComponentBeginOverlap.AddDynamic(this, &ACubeCollision::OnBeginOverlap);
	MyComp->OnComponentEndOverlap.AddDynamic(this, &ACubeCollision::OnOverlapEnd);

	OnActorBeginOverlap.AddDynamic(this, &ACubeCollision::OnOverlap);
	*/
	
	// Set as root component
	RootComponent = MyComp;
}

// Called when the game starts or when spawned
void ACubeCollision::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACubeCollision::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ACubeCollision::OnCompHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("colliding cube!"));
	//UE_LOG(LogTemp, Warning, TEXT("I'm hit, I'm hit"));
	if ((OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL))
	{
		if (GEngine) {
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("I Hit: %s"), *OtherActor->GetName()));
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, "[CubeColHit] I: " + GetName() + " hit: " + *OtherActor->GetName());
		}
	}
}
void ACubeCollision::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (GEngine) {
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, "[CubeColOver] my name: " + GetName() + "beginOve: " + *OtherActor->GetName());
	}
}

void ACubeCollision::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && (OtherActor != this) && OtherComp)
	{
		if (GEngine) {
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Black, "[CubeColEndOver] my name: " + GetName() + "endOver: " + *OtherActor->GetName());
		}
	}
}

void ACubeCollision::OnOverlap(AActor* MyOverlappedActor, AActor* OtherActor)
{
	if (OtherActor && (OtherActor != this))
	{
		if (GEngine) {
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, "[actorOver] my name: " + GetName() + "hit obj: " + *OtherActor->GetName());
		}
	}
}