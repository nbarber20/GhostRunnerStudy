#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GhostRunnerProjectile.h"
#include "GhostRunnerCharacter.h"
#include "GhostRunnerEnemy.generated.h"

UCLASS()
class GHOSTRUNNER_API AGhostRunnerEnemy : public AActor
{
	GENERATED_BODY()
	
public:	
	AGhostRunnerEnemy();

	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* mesh;

	UPROPERTY(VisibleDefaultsOnly, Category = Collision)
	USceneComponent* BulletSpawner;

	UPROPERTY(EditAnywhere, Category = Weapon)
	float fireCooldown = 1;

	UPROPERTY(EditAnywhere, Category = Weapon)
		float bulletSpeed;

	UPROPERTY(EditAnywhere, Category = Weapon)
	TSubclassOf<AGhostRunnerProjectile> bulletActor;

protected:
	AGhostRunnerCharacter* player;
	bool agro = true;
	float currentCooldown;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

};
