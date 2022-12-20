// Copyright 2023 Nitecon Studios LLC. All rights reserved.


#include "BPFunctionLib.h"

#include "ABSettings.h"
#include "ContentBrowserModule.h"
#include "EditorDirectories.h"
#include "IContentBrowserSingleton.h"
#include "Developer/DesktopPlatform/Public/IDesktopPlatform.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonSerializer.h"

void UBPFunctionLib::GetSelectedFolderPath(FString& OutContentLocation)
{
	TArray<FAssetData> OutSelectedAssets;
	TArray<FString> OutSelectedFolders;
	TArray<FString> OutViewFolders;
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	IContentBrowserSingleton& ContentBrowserSingleton = ContentBrowserModule.Get();
	
	ContentBrowserSingleton.GetSelectedFolders(OutSelectedFolders);
	ContentBrowserSingleton.GetSelectedPathViewFolders(OutViewFolders);
	// First select the last view item.
	for (auto Asset : OutViewFolders)
	{
		//UE_LOG(LogTemp, Warning, TEXT("View Folder is: %s"), *Asset)
		// We do a replace since "show all" in content browser can cause a change in the virtual path
		OutContentLocation = Asset.Replace(TEXT("/All"),TEXT(""));
		
	}
	// Now we iterate through the non view path and select here.
	for (auto Asset : OutSelectedFolders)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Asset is: %s"), *Asset)
		// We do a replace since "show all" in content browser can cause a change in the virtual path
		OutContentLocation = Asset.Replace(TEXT("/All"),TEXT(""));
	}
	
}

FString UBPFunctionLib::GetOSDirectoryLocation(const FString& DialogTitle)
{
	if( IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get() )
	{
		FString DestinationFolder;
		const void* ParentWindowHandle = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);
		const FString DefaultLocation( FEditorDirectories::Get().GetLastDirectory( ELastDirectory::GENERIC_IMPORT ) );

		const bool bFolderSelected = DesktopPlatform->OpenDirectoryDialog(
			ParentWindowHandle,
			DialogTitle,
			DefaultLocation,
			DestinationFolder
		);

		if( bFolderSelected )
		{
			FEditorDirectories::Get().SetLastDirectory(ELastDirectory::GENERIC_EXPORT, DestinationFolder);
			return FPaths::ConvertRelativePathToFull( DestinationFolder );
		}
	}
	return FString("Unknown");
}

FString UBPFunctionLib::GetOSFileLocation(const FString& DialogTitle, const FString& FileTypes)
{
	if( IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get() )
	{
		FString DestinationFolder;
		TArray<FString> OutFiles;
		const void* ParentWindowHandle = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);
		const FString DefaultLocation( FEditorDirectories::Get().GetLastDirectory( ELastDirectory::GENERIC_IMPORT ) );

		const bool bFolderSelected = DesktopPlatform->OpenFileDialog(
			ParentWindowHandle,
			DialogTitle,
			DefaultLocation,
			TEXT(""),
			FileTypes,
			EFileDialogFlags::None,
			OutFiles
		);

		if( bFolderSelected && OutFiles.Num() > 0)
		{
			FEditorDirectories::Get().SetLastDirectory(ELastDirectory::GENERIC_EXPORT, DestinationFolder);
			return FPaths::ConvertRelativePathToFull( OutFiles[0] );
		}
	}
	return FString("Unknown");
}

FString UBPFunctionLib::ReadStringFromFile(FString FilePath, bool& bOutSuccess, FString& OutInfoMessage)
{
	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*FilePath))
	{
		bOutSuccess = false;
		OutInfoMessage = FString::Printf(TEXT("failed to open file for reading: '%s'"), *FilePath);
		return "";
	}

	FString Result = "";
	 if (!FFileHelper::LoadFileToString(Result, *FilePath))
	 {
		 bOutSuccess = false;
	 	OutInfoMessage = FString::Printf(TEXT("unable to read file: '%s'"), *FilePath);
	 	return "";
	 }
	bOutSuccess = true;
	OutInfoMessage = "success";
	return Result;
}

void UBPFunctionLib::WriteStringToFile(FString FilePath, FString Data, bool& bOutSuccess, FString& OutInfoMessage)
{
	if (!FFileHelper::SaveStringToFile(Data, *FilePath))
	{
		bOutSuccess = false;
		OutInfoMessage = FString::Printf(TEXT("failed to write file: '%s'"), *FilePath);
		return;
	}
	bOutSuccess = true;
	OutInfoMessage = FString::Printf(TEXT("wrote file: %s"), *FilePath);
}

void UBPFunctionLib::GetABContentLocation(FString& OutContentLocation)
{
	UABSettings* Settings = GetMutableDefault<UABSettings>();
	if (Settings != nullptr)
	{
		OutContentLocation =Settings->UnrealContentLocation;
	}
}

void UBPFunctionLib::SetABContentLocation(FString& ContentLocation)
{
	UABSettings* Settings = GetMutableDefault<UABSettings>();
	if (Settings != nullptr)
	{
		Settings->UnrealContentLocation = ContentLocation;
		Settings->SaveConfig();
	}
}

TSharedPtr<FJsonObject> UBPFunctionLib::ReadJson(FString FilePath, bool& bOutSuccess, FString& OutInfoMessage)
{
	FString StringData = ReadStringFromFile(FilePath, bOutSuccess, OutInfoMessage);
	if (!bOutSuccess)
	{
		return nullptr;
	}
	TSharedPtr<FJsonObject> ReturnObj;

	if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(StringData), ReturnObj))
	{
		bOutSuccess = false;
		OutInfoMessage = FString::Printf(TEXT("failed to read json object: %s"), *StringData);
		return nullptr;
	}
	bOutSuccess = true;
	OutInfoMessage = FString::Printf(TEXT("json read success from %s"), *FilePath);
	return ReturnObj;
}

void UBPFunctionLib::WriteJson(FString FilePath, TSharedPtr<FJsonObject> JsonObject, bool& bOutSuccess,	FString& OutInfoMessage)
{
	FString JsonString;
	if (!FJsonSerializer::Serialize(JsonObject.ToSharedRef(), TJsonWriterFactory<>::Create(&JsonString, 0)))
	{
		bOutSuccess = false;
		OutInfoMessage = FString::Printf(TEXT("failed to write json file: %s"), *FilePath);
		return;
	}
	WriteStringToFile(FilePath, JsonString, bOutSuccess, OutInfoMessage);
	if (!bOutSuccess)
	{
		return;
	}
	bOutSuccess = true;
	OutInfoMessage = FString::Printf(TEXT("wrote json to file: %s"),*FilePath);
}
