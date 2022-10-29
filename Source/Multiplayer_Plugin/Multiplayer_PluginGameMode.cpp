// Copyright Epic Games, Inc. All Rights Reserved.

#include "Multiplayer_PluginGameMode.h"
#include "Multiplayer_PluginCharacter.h"
#include "UObject/ConstructorHelpers.h"

AMultiplayer_PluginGameMode::AMultiplayer_PluginGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
