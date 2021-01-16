// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once 

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "GrappleCursorWidget.h"
#include "GhostRunnerHUD.generated.h"

UCLASS()
class AGhostRunnerHUD : public AHUD
{
	GENERATED_BODY()

public:
	AGhostRunnerHUD();

	virtual void DrawHUD() override;
	virtual void BeginPlay() override;
	virtual void Tick(float delta) override;

	UFUNCTION()
	void UpdateGrappleCursor(FVector2D screenSpace, FVector2D screenSize, bool visible, bool enableGrapple);
	UFUNCTION()
	void SetEdgeVisibility(bool visible);
	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<UUserWidget> grappleWidgetClass;


private:
	UGrappleCursorWidget* grappleWidget;
};

