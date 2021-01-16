// Copyright Epic Games, Inc. All Rights Reserved.

#include "GhostRunnerGameMode.h"
#include "GhostRunnerHUD.h"
#include "GhostRunnerCharacter.h"
#include "UObject/ConstructorHelpers.h"

AGhostRunnerGameMode::AGhostRunnerGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/Blueprints/GhostRunner"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AGhostRunnerHUD::StaticClass();
}
