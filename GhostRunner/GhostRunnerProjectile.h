// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GhostRunnerProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;

UCLASS(config=Game)
class AGhostRunnerProjectile : public AActor
{
	GENERATED_BODY()

public:
	AGhostRunnerProjectile();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Mesh)
	UStaticMeshComponent* mesh;

	UPROPERTY(EditAnywhere, Category = Projectile)
	float bulletSpeed;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
};

