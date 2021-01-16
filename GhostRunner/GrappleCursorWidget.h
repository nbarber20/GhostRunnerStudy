// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Texture2D.h"
#include "GrappleCursorWidget.generated.h"

/**
 * 
 */
UCLASS()
class GHOSTRUNNER_API UGrappleCursorWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UGrappleCursorWidget(const FObjectInitializer& ObjectInitializer);
	virtual void NativeConstruct() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UImage* edgeBlur;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BindWidget))
	class UImage* grappleCursor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* ActiveColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* DeactiveColor;

	void UpdateCursorPosition(FVector2D screenPos, FVector2D screenSize);
	void SetGrappleIconEnabled(bool enabled);
	void SetGrappleVisible(bool enabled);
	void SetEdgeVisibility(bool enabled);

};
