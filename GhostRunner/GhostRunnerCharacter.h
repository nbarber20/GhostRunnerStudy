// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Particles/ParticleSystemComponent.h"
#include "GhostRunnerGrapplePoint.h"
#include "Blueprint/UserWidget.h"
#include "GhostRunnerProjectile.h"
#include "GhostRunnerCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class USceneComponent;
class UCameraComponent;
class UMotionControllerComponent;
class UAnimMontage;
class USoundBase;

UCLASS(config=Game)
class AGhostRunnerCharacter : public ACharacter
{
	enum MovementState {
		Normal,
		Sliding,
		WallRunning,
		TemporalDash,
		LedgePull,
	};

	GENERATED_BODY()


	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	USkeletalMeshComponent* mesh1P;


	UPROPERTY(VisibleDefaultsOnly, Category = Collision)
	UParticleSystemComponent* GrappleLineEmitter;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* firstPersonCameraComponent;

	UPROPERTY(VisibleDefaultsOnly, Category = Collision)
	UCapsuleComponent* OverlapCapsule;

	UPROPERTY(VisibleDefaultsOnly, Category = Collision)
	USceneComponent* LedgeChecker;

public:
	AGhostRunnerCharacter();

public:
	UPROPERTY(EditAnywhere, Category = Attack)
	float AttackDist;
	UPROPERTY(EditAnywhere, Category = Movement)
	float runningSpeed;

	UPROPERTY(EditAnywhere, Category = Movement)
	float crouchSpeed;

	UPROPERTY(EditAnywhere, Category = Movement)
	float walRunSpeed;

	UPROPERTY(EditAnywhere, Category = Movement)
	float temporalDashAdjustSpeed;

	UPROPERTY(EditAnywhere, Category = Movement)
	float normalAirControl;

	UPROPERTY(EditAnywhere, Category = Movement)
	int availableDashes;

	UPROPERTY(EditAnywhere, Category = Movement)
	float dashSpeed;	

	UPROPERTY(EditAnywhere, Category = Movement)
	float airDashSpeed;

	UPROPERTY(EditAnywhere, Category = Movement)
	float airDashTimeDilation;

	UPROPERTY(EditAnywhere, Category = Movement)
	FVector walrunJumpVelocity;

	UPROPERTY(EditAnywhere, Category = Movement)
	float slideJumpVelocity;

	UPROPERTY(EditAnywhere, Category = Movement)
	float walrunLeanSpeed;

	UPROPERTY(EditAnywhere, Category = Movement)
	float walrunLeanAngle;

	UPROPERTY(EditAnywhere, Category = Movement)
	float maxYVelocityForWallRun;

	UPROPERTY(EditAnywhere, Category = Movement)
	float minVelocityForWallRun;

	UPROPERTY(EditAnywhere, Category = Movement)
	float crouchHeight;

	UPROPERTY(EditAnywhere, Category = Movement)
	float slideImpulse;

	UPROPERTY(EditAnywhere, Category = Movement)
	float slideDecay;

	UPROPERTY(EditAnywhere, Category = Movement)
	float slideLowThreshold;


	UPROPERTY(EditAnywhere, Category = Movement)
	float ledgePullVerticalThrust;
	UPROPERTY(EditAnywhere, Category = Movement)
	float ledgePullHorizontalThrust;
	UPROPERTY(EditAnywhere, Category = Movement)
	float ledgePullTime;

	UPROPERTY(EditAnywhere, Category = GrappleMovement)
	AGhostRunnerGrapplePoint* currentGrapelPoint;

	UPROPERTY(EditAnywhere, Category = GrappleMovement)
	UCurveFloat* grappleForce;

	UPROPERTY(EditAnywhere, Category = GrappleMovement)
	float maxGrappleDist;
	UPROPERTY(EditAnywhere, Category = GrappleMovement)
	float maxGrappleMarkerDist;


	UFUNCTION(BlueprintCallable)
	USkeletalMeshComponent* GetMesh1P() const { return mesh1P; }
	UFUNCTION(BlueprintCallable)
	UCameraComponent* GetFirstPersonCameraComponent() const { return firstPersonCameraComponent; }

	void OnBulletInpact(AGhostRunnerProjectile* projectile);

protected:
	MovementState currentMovementState = Normal;
	FVector inputDir;
	FVector wallrunDir;
	FVector wallrunNormalDir;
	FVector slideDir;
	float leanDir;
	bool crouching;
	float currentSlideVelocity;
	bool didSlide;
	float currentLedgePullTime;
	float ledgePullCooldown;
	float grappleVisualCooldown;

	AGhostRunnerGrapplePoint* currentGrappleCandidate;
	TArray<AActor*> allGrapplePoints;

	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	virtual void BeginPlay() override;
	virtual void Tick(float delta);
	FVector2D GetWorldToScreenPos(FVector WorldPos, bool percent = true);
	FVector2D GetScreenSize();
	void FindGrappleCandidate();
	void CheckDistanceOnCurrentGrapple();


	void MoveForward(float Val);
	void MoveRight(float Val);
	void LookUp(float Val);
	void ActivateGrapple();
	void ActivateDash();
	void ReleaseDash();
	void PreformDash(FVector dir); 
	void Jump() override;
	void StopJumping() override;
	void ResetLean();
	void SetMovementState(MovementState newState);
	void StartCrouch();
	void StopCrouch();
	void PullLedge();
	void BeginSlide();
	void Attack();
	

	UFUNCTION()
	void OnCollisionEnter(class UPrimitiveComponent* HitComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

};