 // Fill out your copyright notice in the Description page of Project Settings.


#include "GrappleCursorWidget.h"
#include "Components/Image.h"
#include "Components/CanvasPanelSlot.h"
#include "Styling/SlateBrush.h"

UGrappleCursorWidget::UGrappleCursorWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{

}

void UGrappleCursorWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UGrappleCursorWidget::UpdateCursorPosition(FVector2D screenPos, FVector2D screenSize)
{
	UCanvasPanelSlot* CursorAsPanelSlot = Cast<UCanvasPanelSlot>(grappleCursor->Slot);
	FAnchors anchor;
	FVector2D viewportPos = screenPos / screenSize;
	anchor.Minimum = viewportPos;
	anchor.Maximum = viewportPos;
	CursorAsPanelSlot->SetAnchors(anchor);
	CursorAsPanelSlot->SetPosition(FVector2D(0, 0));
}

void UGrappleCursorWidget::SetGrappleIconEnabled(bool enabled)
{
	if (enabled) {
		grappleCursor->SetBrushResourceObject(ActiveColor);
	}
	else {
		grappleCursor->SetBrushResourceObject(DeactiveColor);
	}
}


void UGrappleCursorWidget::SetGrappleVisible(bool enabled)
{
	if (enabled) {
		grappleCursor->SetVisibility(ESlateVisibility::Visible);
	}
	else {
		grappleCursor->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UGrappleCursorWidget::SetEdgeVisibility(bool enabled)
{
	if (enabled) {
		edgeBlur->SetVisibility(ESlateVisibility::Visible);
	}
	else {
		edgeBlur->SetVisibility(ESlateVisibility::Hidden);
	}
}
