#include "GhostRunnerCharacter.h"
#include "GhostRunnerProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/InputSettings.h"
#include "DrawDebugHelpers.h"
#include "GhostRunnerAnimator.h"
#include "GhostRunnerHUD.h"
#include "GhostRunnerEnemy.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

AGhostRunnerCharacter::AGhostRunnerCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &AGhostRunnerCharacter::OnCollisionEnter);
	
	// Create a CameraComponent	
	firstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	firstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	firstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	firstPersonCameraComponent->bUsePawnControlRotation = true;

	GrappleLineEmitter = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("GrappleLineEmitter"));
	GrappleLineEmitter->SetActorParameter("Start", this);

	OverlapCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("OverlapCapsule"));
	OverlapCapsule->InitCapsuleSize(58.0f, 99.0f);

	LedgeChecker = CreateDefaultSubobject<USceneComponent>(TEXT("LedgeChecker"));
	LedgeChecker->SetRelativeLocation(FVector(0.f, 0.f, 10.0f));

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	mesh1P->SetOnlyOwnerSee(true);
	mesh1P->SetupAttachment(firstPersonCameraComponent);
	mesh1P->bCastDynamicShadow = false;
	mesh1P->CastShadow = false;
	mesh1P->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	mesh1P->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));

}

void AGhostRunnerCharacter::OnBulletInpact(AGhostRunnerProjectile* projectile)
{
	UGhostRunnerAnimator* anim = Cast<UGhostRunnerAnimator>(mesh1P->GetAnimInstance());
	if (anim) {
		if (anim->attacking == true) {
			projectile->SetActorRotation(GetActorRotation());
		}
		else {
			UGameplayStatics::OpenLevel(this, GetWorld()->GetFName(),false);
		}
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("Anim not found"));
	}
}

void AGhostRunnerCharacter::SetupPlayerInputComponent(class UInputComponent* playerInputComponent)
{
	// set up gameplay key bindings
	check(playerInputComponent);

	// Bind Actions
	playerInputComponent->BindAction("Space", IE_Pressed, this, &AGhostRunnerCharacter::Jump);
	playerInputComponent->BindAction("Space", IE_Released, this, &AGhostRunnerCharacter::StopJumping);
	playerInputComponent->BindAction("Shift", IE_Pressed, this, &AGhostRunnerCharacter::ActivateDash);
	playerInputComponent->BindAction("Shift", IE_Released, this, &AGhostRunnerCharacter::ReleaseDash);
	playerInputComponent->BindAction("Click", IE_Pressed, this, &AGhostRunnerCharacter::Attack);
	playerInputComponent->BindAction("Ctrl", IE_Pressed, this, &AGhostRunnerCharacter::StartCrouch);
	playerInputComponent->BindAction("Ctrl", IE_Released, this, &AGhostRunnerCharacter::StopCrouch);
	playerInputComponent->BindAction("RightClick", IE_Pressed, this, &AGhostRunnerCharacter::ActivateGrapple);


	// Bind Axis
	playerInputComponent->BindAxis("MoveForward", this, &AGhostRunnerCharacter::MoveForward);
	playerInputComponent->BindAxis("MoveRight", this, &AGhostRunnerCharacter::MoveRight);
	playerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	playerInputComponent->BindAxis("LookUp", this, &AGhostRunnerCharacter::LookUp);

	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	MovementComponent->GravityScale = 1.5f;
	MovementComponent->AirControl = normalAirControl;
	MovementComponent->MaxWalkSpeed = runningSpeed;
}

void AGhostRunnerCharacter::BeginPlay()
{
	Super::BeginPlay(); 
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGhostRunnerGrapplePoint::StaticClass(), allGrapplePoints);
}

void AGhostRunnerCharacter::Tick(float delta)
{
	Super::Tick(delta);

	UGhostRunnerAnimator* animator = Cast<UGhostRunnerAnimator>(mesh1P->GetAnimInstance());
	if (animator) {
		if (animator->attacking == true) {

			TArray <TEnumAsByte<EObjectTypeQuery>> objectTypes;
			TArray <AActor*> ActorsToIgnore;
			TArray <AActor*> OutActors;
			if(UKismetSystemLibrary::SphereOverlapActors(GetWorld(), GetActorLocation()+(GetActorForwardVector() * AttackDist), AttackDist, objectTypes,AGhostRunnerEnemy::StaticClass(), ActorsToIgnore, OutActors))
			{
				for (int i = 0; i < OutActors.Num(); i++) {
					AGhostRunnerEnemy* enemy = Cast<AGhostRunnerEnemy>(OutActors[i]);
					if (enemy) {
						enemy->Destroy();
					}
				}				
			}
		}
	}


	if (currentMovementState == Normal) {
		if (currentGrapelPoint == NULL) {
			FindGrappleCandidate();

			if (currentGrappleCandidate != NULL) {

				AGhostRunnerHUD* hud = Cast<AGhostRunnerHUD>(GetWorld()->GetFirstPlayerController()->GetHUD());
				if (hud) {
					FVector2D screenPos = GetWorldToScreenPos(currentGrappleCandidate->GetActorLocation(), false);
					hud->UpdateGrappleCursor(screenPos, GetScreenSize(), true, false);
				}


				//DrawDebugLine(GetWorld(), GetActorLocation(), currentGrappleCandidate->GetActorLocation(), FColor::Blue);
				FVector dir = currentGrappleCandidate->GetActorLocation() - GetActorLocation();
				float dist = dir.Size();
				if (dist < maxGrappleDist) {
					FVector2D screenPos = GetWorldToScreenPos(currentGrappleCandidate->GetActorLocation());
					if (screenPos.X >= .35f && screenPos.X <= .65f && screenPos.Y >= .35f && screenPos.Y <= .65f) {
						currentGrapelPoint = currentGrappleCandidate;
					}
				}
			}
			else {
				AGhostRunnerHUD* hud = Cast<AGhostRunnerHUD>(GetWorld()->GetFirstPlayerController()->GetHUD());
				if (hud) {
					hud->UpdateGrappleCursor(FVector2D(0, 0), GetScreenSize(), false, false);
				}
			}
		}
		else {
			AGhostRunnerHUD* hud = Cast<AGhostRunnerHUD>(GetWorld()->GetFirstPlayerController()->GetHUD());
			if (hud) {
				FVector2D screenPos = GetWorldToScreenPos(currentGrapelPoint->GetActorLocation(), false);
				hud->UpdateGrappleCursor(screenPos, GetScreenSize(), true, true);
			}
			CheckDistanceOnCurrentGrapple();
		}
	}
	else {
		currentGrapelPoint = NULL;
		currentGrappleCandidate = NULL; 
		AGhostRunnerHUD* hud = Cast<AGhostRunnerHUD>(GetWorld()->GetFirstPlayerController()->GetHUD());
		if (hud) {
			hud->UpdateGrappleCursor(FVector2D(0,0), GetScreenSize(), false, false);
		}
	}




	//Lean
	float roll = firstPersonCameraComponent->GetComponentRotation().Roll;
	firstPersonCameraComponent->SetRelativeRotation(FRotator(firstPersonCameraComponent->GetRelativeRotation().Pitch, 0, FMath::Lerp(roll, leanDir, walrunLeanSpeed*delta)));

	//Crouch
	if (crouching&&currentMovementState == Normal) {
		if (GetMovementComponent()->IsMovingOnGround()) {
			if (GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() != crouchHeight) {
				GetCapsuleComponent()->SetCapsuleHalfHeight(crouchHeight);
				GetCharacterMovement()->MaxWalkSpeed = crouchSpeed;
				if (animator)animator->crouching = true;
			}
			if (!didSlide) {
				if (inputDir.Size() != 0) {
					BeginSlide();
				}
			}
		}
	}
	else if (crouching == false) {
		if (currentMovementState == Normal) {
			if (GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() == crouchHeight) {
				GetCapsuleComponent()->SetCapsuleHalfHeight(96.0f);
				GetCharacterMovement()->MaxWalkSpeed = runningSpeed;
				if(animator)animator->crouching = false;
			}
			didSlide = false;
		}
	}

	//Wallrun
	if (currentMovementState == WallRunning) {

		
		GetMovementComponent()->Velocity = wallrunDir * walRunSpeed;

		//Left the wall so jump off it
		TSet<AActor*> actors;
		OverlapCapsule->GetOverlappingActors(actors);
		if (actors.Num() == 0) {
			FVector dir = (GetActorForwardVector()*walrunJumpVelocity.X) + (wallrunNormalDir * walrunJumpVelocity.Y) + (GetActorUpVector()* walrunJumpVelocity.Z);
			LaunchCharacter(dir, false, false);
			SetMovementState(Normal);
		}
	}
	else if (currentMovementState == Sliding) { //Slide
		FHitResult OutHit;
		bool isSlope = false;
		FCollisionQueryParams CollisionParams;
		if (GetWorld()->LineTraceSingleByChannel(OutHit, GetActorLocation(), -GetActorUpVector()*1, ECC_Visibility, CollisionParams))
		{
			FVector relDir = (GetActorForwardVector()*-inputDir.Y) + (GetActorRightVector()*inputDir.X);
			FVector cross = FVector::CrossProduct(relDir, OutHit.ImpactNormal);
			cross.Normalize();
			GetMovementComponent()->Velocity = (cross * currentSlideVelocity) + (GetActorUpVector()*-1500.8f);
			if (cross.Z <= -0.3f) {
				isSlope = true;
			}
		}		
		if(isSlope==false)currentSlideVelocity -= slideDecay * delta;
		if (currentSlideVelocity <= slideLowThreshold || inputDir.Size() == 0) {
			SetMovementState(Normal);
			if (crouching == false) {
				GetCapsuleComponent()->SetCapsuleHalfHeight(96.0f);
				UGhostRunnerAnimator* anim = Cast<UGhostRunnerAnimator>(mesh1P->GetAnimInstance());
				if (anim)anim->crouching = false;
				didSlide = false;
			}
			else {
				GetCharacterMovement()->MaxWalkSpeed = crouchSpeed;
			}
		}
	}
	else if (currentMovementState == LedgePull) {
		currentLedgePullTime -= delta;
		if (currentLedgePullTime <= 0) {
			LaunchCharacter(GetActorForwardVector() * ledgePullHorizontalThrust, false, false);
			SetMovementState(Normal);
		}
	}
	if (ledgePullCooldown > 0) {
		ledgePullCooldown -= delta;
	}
	if (grappleVisualCooldown > 0) {
		grappleVisualCooldown -= delta;
	}
	else if(GrappleLineEmitter->IsActive()) {
		GrappleLineEmitter->SecondsBeforeInactive = 0;
		GrappleLineEmitter->Deactivate();
	}
}

FVector2D AGhostRunnerCharacter::GetWorldToScreenPos(FVector WorldPos, bool percent)
{
	const APlayerController* const PlayerController = Cast<const APlayerController>(GetController());

	FVector2D ScreenLocation;
	PlayerController->ProjectWorldLocationToScreen(WorldPos, ScreenLocation);

	int32 ScreenWidth = 0;
	int32 ScreenHeight = 0;
	PlayerController->GetViewportSize(ScreenWidth, ScreenHeight);
	if (percent) {
		return FVector2D((float)ScreenLocation.X / (float)ScreenWidth, (float)ScreenLocation.Y / (float)ScreenHeight);
	}
	else {
		return FVector2D(ScreenLocation.X, ScreenLocation.Y);
	}
}

FVector2D AGhostRunnerCharacter::GetScreenSize()
{
	const APlayerController* const PlayerController = Cast<const APlayerController>(GetController());
	int32 ScreenWidth = 0;
	int32 ScreenHeight = 0;
	PlayerController->GetViewportSize(ScreenWidth, ScreenHeight);

	return FVector2D(ScreenWidth, ScreenHeight);
}

void AGhostRunnerCharacter::FindGrappleCandidate()
{
	currentGrappleCandidate = NULL;
	float closestDist = maxGrappleMarkerDist;
	//Find Grapple Candidate
	for (int i = 0; i < allGrapplePoints.Num(); i++) {
		FVector dir = allGrapplePoints[i]->GetActorLocation() - GetActorLocation();
		float dist = dir.Size();
		if (dist < maxGrappleDist&& dist < closestDist) {
			FHitResult OutHit;
			FCollisionQueryParams CollisionParams;
			CollisionParams.AddIgnoredActor(this);
			if (!GetWorld()->LineTraceSingleByChannel(OutHit, firstPersonCameraComponent->GetComponentTransform().GetLocation(), allGrapplePoints[i]->GetActorLocation(), ECC_Visibility, CollisionParams))
			{
				DrawDebugLine(GetWorld(), firstPersonCameraComponent->GetComponentTransform().GetLocation(), allGrapplePoints[i]->GetActorLocation(), FColor::Red);
				FVector2D screenPos = GetWorldToScreenPos(allGrapplePoints[i]->GetActorLocation());
				if (screenPos.X >= 0.1 && screenPos.X <= 0.9 && screenPos.Y >= 0.1 && screenPos.Y <= 0.9) {
					 
					//we can see this point and it is closer than the last candidate
					closestDist = dist;
					currentGrappleCandidate = Cast<AGhostRunnerGrapplePoint>(allGrapplePoints[i]);
				}
			}
		}
	}
}

void AGhostRunnerCharacter::CheckDistanceOnCurrentGrapple()
{
	FVector dir = currentGrapelPoint->GetActorLocation() - GetActorLocation();
	float dist = dir.Size();
	if (dist > maxGrappleDist) {
		currentGrapelPoint = NULL;
		return;
	}

	FHitResult OutHit;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);
	//DrawDebugLine(GetWorld(), startPos, startPos + GetActorForwardVector() * 100, FColor::Red, true, 100);
	if (GetWorld()->LineTraceSingleByChannel(OutHit, firstPersonCameraComponent->GetComponentTransform().GetLocation(), currentGrapelPoint->GetActorLocation(), ECC_Visibility, CollisionParams))
	{
		currentGrapelPoint = NULL;
		return;
	}

	FVector2D screenPos = GetWorldToScreenPos(currentGrapelPoint->GetActorLocation());
	if (screenPos.X < .35 || screenPos.X > .65 || screenPos.Y < .35 || screenPos.Y > .65) {
		currentGrapelPoint = NULL;
		return;
	}
}

void AGhostRunnerCharacter::MoveForward(float Value)
{
	if (Value == 0.0f) {
		inputDir.X = 0;
		return;
	}
	if (currentMovementState != Sliding) {
		inputDir.X = Value;
		if (currentMovementState == Normal) {
			AddMovementInput(GetActorForwardVector(), Value);
		}
	}
}

void AGhostRunnerCharacter::MoveRight(float Value)
{
	if (Value == 0.0f) {
		inputDir.Y = 0;
		return;
	}
	if (currentMovementState != Sliding) {
		inputDir.Y = Value;
		if (currentMovementState == Normal) {
			AddMovementInput(GetActorRightVector(), Value);
		}
		else if (currentMovementState == TemporalDash) {
			FVector delta = (GetActorRightVector() * Value * temporalDashAdjustSpeed);
			SetActorRelativeLocation(GetActorLocation() + delta * FApp::GetDeltaTime());
		}
	}
}

//Overriding as we cant use camera input with the lean system
void AGhostRunnerCharacter::LookUp(float Value)
{
	if (Value != 0) {
		FRotator rot = firstPersonCameraComponent->GetRelativeRotation();
		if ((rot.Pitch < 85 && Value<0) || (rot.Pitch > -85 && Value > 0)) {
			firstPersonCameraComponent->SetRelativeRotation(rot + FRotator(-Value, 0, 0));
		}
	}
}

void AGhostRunnerCharacter::ActivateGrapple()
{
	if (currentGrapelPoint) {
		FVector dir = currentGrapelPoint->GetActorLocation() - GetActorLocation();
		if (dir.Size() <= maxGrappleDist) {
			GetCharacterMovement()->FallingLateralFriction = 0;			
			LaunchCharacter(dir* grappleForce->GetFloatValue(dir.Size() /maxGrappleDist), true, true);
			if (GrappleLineEmitter) {
				GrappleLineEmitter->Activate();
				GrappleLineEmitter->SetActorParameter("Target", currentGrapelPoint);
				grappleVisualCooldown = 0.01f; 		
			}
			UGhostRunnerAnimator* animation = Cast<UGhostRunnerAnimator>(mesh1P->GetAnimInstance());
			if (animation) {
				animation->PreformGrapplePull();
			}
		}
	}
}

void AGhostRunnerCharacter::ActivateDash()
{
	if (currentMovementState == WallRunning)return;
	if (GetCharacterMovement()->IsMovingOnGround()) {
		if (inputDir.Size() != 0) {
			FVector dir = (GetActorForwardVector()*inputDir.X) + (GetActorRightVector()*inputDir.Y);
			PreformDash(dir);
		}
	}
	else {
		AGhostRunnerHUD* hud = Cast<AGhostRunnerHUD>(GetWorld()->GetFirstPlayerController()->GetHUD());
		if (hud) {
			hud->SetEdgeVisibility(true);
		}
		SetMovementState(TemporalDash);
	}
}

void AGhostRunnerCharacter::ReleaseDash()
{
	if (currentMovementState == TemporalDash) {
		AGhostRunnerHUD* hud = Cast<AGhostRunnerHUD>(GetWorld()->GetFirstPlayerController()->GetHUD());
		if (hud) {
			hud->SetEdgeVisibility(false);
		}
		PreformDash(firstPersonCameraComponent->GetForwardVector());
	}
}

void AGhostRunnerCharacter::PreformDash(FVector dir)
{
	float speed = airDashSpeed;
	UGameplayStatics::SetGlobalTimeDilation(this, 1.f);
	if (GetCharacterMovement()->IsMovingOnGround())speed = dashSpeed;
	dir.Normalize();
	SetMovementState(Normal);
	if (dir.Z > 0)dir.Z = 0;
	GetCharacterMovement()->Velocity= dir*speed; 
}

void AGhostRunnerCharacter::Jump()
{
	if (currentMovementState == WallRunning) {
		FVector dir = (GetActorForwardVector()*walrunJumpVelocity.X) + (wallrunNormalDir * walrunJumpVelocity.Y) + (GetActorUpVector()* walrunJumpVelocity.Z);
		LaunchCharacter(dir, false, false);
		return;
	}
	else if (currentMovementState == Sliding) {
		GetCharacterMovement()->FallingLateralFriction = 0;
		GetCharacterMovement()->Velocity = slideDir * slideJumpVelocity;
	}
	ACharacter::Jump();

	SetMovementState(Normal);
}

void AGhostRunnerCharacter::StopJumping()
{
	ACharacter::StopJumping();
}

void AGhostRunnerCharacter::ResetLean()
{
	leanDir = 0;
}

void AGhostRunnerCharacter::SetMovementState(MovementState newState)
{
	currentMovementState = newState;
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	switch (newState)
	{
	case AGhostRunnerCharacter::Normal:
		ResetLean();
		MovementComponent->MaxWalkSpeed = runningSpeed;
		MovementComponent->AirControl = normalAirControl;
		MovementComponent->GroundFriction = 8.0f;
		MovementComponent->GravityScale = 1.5f;
		UGameplayStatics::SetGlobalTimeDilation(this, 1.1f);
		break;
	case AGhostRunnerCharacter::WallRunning:
		MovementComponent->MaxWalkSpeed = walRunSpeed;
		MovementComponent->AirControl = normalAirControl;
		MovementComponent->Velocity = FVector::ZeroVector;
		MovementComponent->GravityScale = 0.f;
		UGameplayStatics::SetGlobalTimeDilation(this, 1.1f);
		break;
	case AGhostRunnerCharacter::TemporalDash:
		ResetLean();
		MovementComponent->MaxWalkSpeed = temporalDashAdjustSpeed;
		MovementComponent->AirControl = 1.0f;
		UGameplayStatics::SetGlobalTimeDilation(this, airDashTimeDilation);
		break;
	case AGhostRunnerCharacter::Sliding:
		MovementComponent->GroundFriction = 0.0f;
		UGameplayStatics::SetGlobalTimeDilation(this, 1.1f);
		break;
	case AGhostRunnerCharacter::LedgePull:
		ResetLean();
		MovementComponent->MaxWalkSpeed = runningSpeed;
		MovementComponent->AirControl = normalAirControl;
		MovementComponent->GroundFriction = 8.0f;
		MovementComponent->GravityScale = 1.5f;
		UGameplayStatics::SetGlobalTimeDilation(this, 1.1f);
	default:
		break;
	}
}

void AGhostRunnerCharacter::StartCrouch()
{
	crouching = true;
	if (GetCharacterMovement()->IsMovingOnGround() && inputDir.Size() == 0) {
		didSlide = true;
	}
}

void AGhostRunnerCharacter::StopCrouch()
{
	crouching = false;
}

void AGhostRunnerCharacter::PullLedge()
{
	if (ledgePullCooldown <= 0) {
		SetMovementState(LedgePull);
		LaunchCharacter(GetActorUpVector()* ledgePullVerticalThrust, false, false);
		currentLedgePullTime = ledgePullTime;
		UGhostRunnerAnimator* animation = Cast<UGhostRunnerAnimator>(mesh1P->GetAnimInstance());
		if (animation) {
			animation->PreformLedgePull();
		}
		ledgePullCooldown = 1.5f;
	}
}

void AGhostRunnerCharacter::BeginSlide()
{
	didSlide = true;
	SetMovementState(Sliding);
	slideDir = GetActorForwardVector()*inputDir.X + GetActorRightVector()*inputDir.Y;
	currentSlideVelocity = slideImpulse;
}

void AGhostRunnerCharacter::Attack()
{
	UGhostRunnerAnimator* animation = Cast<UGhostRunnerAnimator>(mesh1P->GetAnimInstance());
	if (animation) {
		animation->PreformAttack();
	}
}

void AGhostRunnerCharacter::OnCollisionEnter(class UPrimitiveComponent* HitComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	GetCharacterMovement()->FallingLateralFriction = 8.0f;
	if (currentMovementState == Normal|| currentMovementState == WallRunning) {

		UCharacterMovementComponent* movement = GetCharacterMovement();

		if (movement->Velocity.Z < maxYVelocityForWallRun && !movement->IsMovingOnGround()) {
			if (movement->Velocity.Size() > minVelocityForWallRun) {
				if (Hit.ImpactNormal.Z < 0.1f && Hit.ImpactNormal.Z > -0.1f) {
					float angle = UKismetMathLibrary::DegAcos(FVector::DotProduct(GetActorForwardVector(), Hit.ImpactNormal));

					//WallRunCheck
					if (angle > 50 && angle < 130) {
						SetMovementState(WallRunning);
						FVector cross = FVector::CrossProduct(Hit.ImpactNormal, GetActorUpVector());
						ResetLean();
						wallrunNormalDir = Hit.ImpactNormal;
						if (FVector::DotProduct(GetActorForwardVector(), cross) < 0) {
							wallrunDir = FVector::CrossProduct(GetActorUpVector(), Hit.ImpactNormal);
							leanDir = -walrunLeanAngle;
						}
						else {
							wallrunDir = cross;
							leanDir = walrunLeanAngle;
						}
						wallrunDir.Normalize();
						return;
					}

					//Ledge Grab Check
					if (currentMovementState != WallRunning && currentMovementState!= LedgePull && inputDir.X>0) {
						FHitResult OutHit;
						FCollisionQueryParams CollisionParams;
						CollisionParams.AddIgnoredActor(this);
						FVector startPos = LedgeChecker->GetComponentTransform().GetLocation();
						//DrawDebugLine(GetWorld(), startPos, startPos + GetActorForwardVector() * 100, FColor::Red, true, 100);
						if (!GetWorld()->LineTraceSingleByChannel(OutHit, startPos, startPos + GetActorForwardVector() * 200, ECC_Visibility, CollisionParams))
						{
							PullLedge();
						}
					}
				}
			}
		}		
	}
}
