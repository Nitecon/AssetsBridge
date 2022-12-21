// Copyright Epic Games, Inc. All Rights Reserved.

#include "AssetsBridgeStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Framework/Application/SlateApplication.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FAssetsBridgeStyle::StyleInstance = nullptr;

void FAssetsBridgeStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FAssetsBridgeStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FAssetsBridgeStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("AssetsBridgeStyle"));
	return StyleSetName;
}

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);

TSharedRef< FSlateStyleSet > FAssetsBridgeStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("AssetsBridgeStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("AssetsBridge")->GetBaseDir() / TEXT("Resources"));

	//Style->Set("AssetsBridge.OpenSettingsWindow", new IMAGE_BRUSH_SVG(TEXT("PlaceholderButtonIcon"), Icon20x20));
	Style->Set("AssetsBridge.ContentSwapAction", new IMAGE_BRUSH_SVG(TEXT("PlaceholderButtonIcon"), Icon20x20));
	Style->Set("AssetsBridge.ContentExportAction", new IMAGE_BRUSH_SVG(TEXT("PlaceholderButtonIcon"), Icon20x20));
	Style->Set("AssetsBridge.ContentImportAction", new IMAGE_BRUSH_SVG(TEXT("PlaceholderButtonIcon"), Icon20x20));

	return Style;
}

void FAssetsBridgeStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FAssetsBridgeStyle::Get()
{
	return *StyleInstance;
}
