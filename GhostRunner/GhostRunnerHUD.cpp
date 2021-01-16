// Copyright Epic Games, Inc. All Rights Reserved.

#include "GhostRunnerHUD.h"
#include "Engine/Canvas.h"
#include "Engine/Texture2D.h"
#include "TextureResource.h"
#include "CanvasItem.h"
#include "UObject/ConstructorHelpers.h"
#include "Blueprint/UserWidget.h"

AGhostRunnerHUD::AGhostRunnerHUD()
{

}

void AGhostRunnerHUD::DrawHUD()
{

}

void AGhostRunnerHUD::BeginPlay()
{
	Super::BeginPlay();
	if (grappleWidgetClass) {
		grappleWidget = CreateWidget<UGrappleCursorWidget>(GetWorld(), grappleWidgetClass);
		if (grappleWidget) {
			grappleWidget->AddToViewport();
		}
	}
}

void AGhostRunnerHUD::Tick(float delta)
{
	Super::Tick(delta);
}

void AGhostRunnerHUD::UpdateGrappleCursor(FVector2D screenSpace, FVector2D screenSize, bool visible, bool enableGrapple)
{
	if (grappleWidget) {
		grappleWidget->UpdateCursorPosition(screenSpace, screenSize);
		grappleWidget->SetGrappleIconEnabled(enableGrapple);
		grappleWidget->SetGrappleVisible(visible);
	}
}

void AGhostRunnerHUD::SetEdgeVisibility(bool visible)
{
	if (grappleWidget) {
		grappleWidget->SetEdgeVisibility(visible);
	}
}

