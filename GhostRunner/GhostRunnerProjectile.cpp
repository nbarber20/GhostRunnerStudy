// Copyright Epic Games, Inc. All Rights Reserved.

#include "GhostRunnerProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "GhostRunnerCharacter.h"
AGhostRunnerProjectile::AGhostRunnerProjectile() 
{
	mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(mesh);
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void AGhostRunnerProjectile::BeginPlay()
{
	Super::BeginPlay();
}

void AGhostRunnerProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	FVector prevLoc = GetActorLocation();
	SetActorLocation(GetActorLocation() + (GetActorForwardVector()*bulletSpeed * DeltaTime));

	FHitResult OutHit;
	FCollisionQueryParams CollisionParams;
	if (GetWorld()->LineTraceSingleByChannel(OutHit, prevLoc, GetActorLocation(), ECC_Pawn, CollisionParams))
	{
		AGhostRunnerCharacter* character = Cast<AGhostRunnerCharacter>(OutHit.Actor);
		if (character) {
			character->OnBulletInpact(this);
		}
		else {
			Destroy();
		}
	}
}
