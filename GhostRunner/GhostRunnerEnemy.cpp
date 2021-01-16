#include "GhostRunnerEnemy.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
AGhostRunnerEnemy::AGhostRunnerEnemy()
{
	PrimaryActorTick.bCanEverTick = true;
	mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	SetRootComponent(mesh);

	BulletSpawner = CreateDefaultSubobject<USceneComponent>(TEXT("BulletSpawner"));
}

void AGhostRunnerEnemy::BeginPlay()
{
	Super::BeginPlay();
	TArray<AActor*> characterActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGhostRunnerCharacter::StaticClass(), characterActors);
	if (characterActors.Num() > 0) {
		player = Cast<AGhostRunnerCharacter>(characterActors[0]);
	}
}

void AGhostRunnerEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector start = GetActorLocation();
	start.Z = player->GetActorLocation().Z;
	FRotator selflookRot = UKismetMathLibrary::FindLookAtRotation(start, player->GetActorLocation()) + FRotator(0, -90,0);
	SetActorRotation(selflookRot);


	if (currentCooldown <= 0) {
		currentCooldown = fireCooldown;

		/*
			Using equation from:
			https://www.gamasutra.com/blogs/KainShin/20090515/83954/Predictive_Aim_Mathematics_for_AI_Targeting.php
		*/
		FVector playerApprox = player->GetActorLocation();
		for (int i = 0; i < 4;i++) {
			FVector dir = BulletSpawner->GetComponentTransform().GetLocation() - playerApprox;
			float baseTime = (dir.Size() / bulletSpeed);
			playerApprox = (player->GetVelocity()* baseTime) + player->GetActorLocation();
		}


		FRotator bulletLookRot = UKismetMathLibrary::FindLookAtRotation(BulletSpawner->GetComponentTransform().GetLocation(), playerApprox);
		FActorSpawnParameters SpawnInfo;
		GetWorld()->SpawnActor<AGhostRunnerProjectile>(bulletActor, BulletSpawner->GetComponentTransform().GetLocation(), bulletLookRot, SpawnInfo);
	}
	else {
		currentCooldown -= DeltaTime;
	}
}

