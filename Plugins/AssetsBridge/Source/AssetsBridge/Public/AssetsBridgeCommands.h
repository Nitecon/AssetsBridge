// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "AssetsBridgeStyle.h"

class FAssetsBridgeCommands : public TCommands<FAssetsBridgeCommands>
{
public:

	FAssetsBridgeCommands()
		: TCommands<FAssetsBridgeCommands>(TEXT("AssetsBridge"), NSLOCTEXT("Contexts", "AssetsBridge", "AssetsBridge Plugin"), NAME_None, FAssetsBridgeStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenSettingsWindow;
	TSharedPtr< FUICommandInfo > ContentSwapAction;
	TSharedPtr< FUICommandInfo > MakeAssetAction;
	TSharedPtr< FUICommandInfo > ContentExportAction;
	TSharedPtr< FUICommandInfo > ContentImportAction;
};