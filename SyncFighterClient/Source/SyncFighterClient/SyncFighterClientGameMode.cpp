// Copyright Epic Games, Inc. All Rights Reserved.

#include "SyncFighterClientGameMode.h"
#include "SyncFighterClientCharacter.h"
#include "UObject/ConstructorHelpers.h"

ASyncFighterClientGameMode::ASyncFighterClientGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
