// Copyright Epic Games, Inc. All Rights Reserved.

#include "AssetBridgeGameMode.h"
#include "AssetBridgeCharacter.h"
#include "UObject/ConstructorHelpers.h"

AAssetBridgeGameMode::AAssetBridgeGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
