// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Engine/Classes/Animation/AnimInstance.h"
#include "GhostRunnerAnimator.generated.h"

UCLASS(Transient,Blueprintable, hideCategories = AnimInstance, BlueprintType)
class GHOSTRUNNER_API UGhostRunnerAnimator : public UAnimInstance
{
	GENERATED_BODY()
public:
	UGhostRunnerAnimator() {};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	bool attacking = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	bool crouching = true;

	UFUNCTION(BlueprintImplementableEvent)
	void PreformAttack();

	UFUNCTION(BlueprintImplementableEvent)
	void PreformGrapplePull();

	UFUNCTION(BlueprintImplementableEvent)
	void PreformLedgePull();
};
