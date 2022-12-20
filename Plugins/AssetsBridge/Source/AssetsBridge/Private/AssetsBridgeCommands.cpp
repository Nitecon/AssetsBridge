// Copyright Epic Games, Inc. All Rights Reserved.

#include "AssetsBridgeCommands.h"

#define LOCTEXT_NAMESPACE "FAssetsBridgeModule"

void FAssetsBridgeCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "AssetsBridge", "Bring up AssetsBridge Settings Window", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(ContentSwapAction, "AssetsBridge", "Replace the current item in the level with the selected item in your content library.", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(ContentExportAction, "AssetsBridge", "Bridge (Export) the current item to your 3D application", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(ContentImportAction, "AssetsBridge", "Bridge In (Import) the current item from your selected 3D application", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
