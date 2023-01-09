// Copyright 2023 Nitecon Studios LLC. All rights reserved.


#include "BPFunctionLib.h"

#include "ABSettings.h"
#include "BridgeManager.h"
#include "ContentBrowserModule.h"
#include "EditorDirectories.h"
#include "IContentBrowserSingleton.h"
#include "Selection.h"
#include "Developer/DesktopPlatform/Public/IDesktopPlatform.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonSerializer.h"

void UBPFunctionLib::StartExport(TArray<FExportAsset> ChangedList, TArray<FExportAsset> ReadyList, bool& bIsSuccessful, FString& OutMessage)
{
	TArray<FExportAsset> AssetList;
	for (auto Item : ChangedList)
	{
		AssetList.Add(Item);
	}
	for (auto Item : ReadyList)
	{
		AssetList.Add(Item);
	}
	
	UBridgeManager::GenerateExport(AssetList, bIsSuccessful, OutMessage);
	if (!bIsSuccessful)
	{
		FText DialogText = FText::FromString(OutMessage);
		FMessageDialog::Open(EAppMsgType::Ok, DialogText);
	}
}

FString UBPFunctionLib::GetExportPathFromInternal(FString NewInternalPath, FString NewName)
{
	FString AssetHome;
	GetAssetsLocation(AssetHome);
	//TODO: Strip Engine / Game / Other folders from the start.
	FString NewExportPath = FPaths::Combine(AssetHome, NewInternalPath, NewName.Append(".fbx"));
	UE_LOG(LogTemp, Warning, TEXT("Adding new export path: %s"), *NewExportPath)
	return NewExportPath;
}

void UBPFunctionLib::CloseExportTab()
{
	static const FName AssetsBridgeExpConfig("Assets Bridge Export Configuration");

}

FBridgeExport UBPFunctionLib::ReadBridgeExportFile(bool& bOutSuccess, FString& OutMessage)
{
	FString AssetBase;
	GetAssetsLocation(AssetBase);
	FString JsonFilePath = FPaths::Combine(AssetBase, "AssetBridge.json");
	// Try to read generic text into json object
	TSharedPtr<FJsonObject> JSONObject = ReadJson(JsonFilePath, bOutSuccess, OutMessage);
	{
		if (!bOutSuccess)
		{
			return FBridgeExport();
		}
	}
	FBridgeExport ReturnData;
	if (!FJsonObjectConverter::JsonObjectToUStruct<FBridgeExport>(JSONObject.ToSharedRef(), &ReturnData))
	{
		bOutSuccess = false;
		OutMessage = FString::Printf(TEXT("Invalid json detected for this operation on file: %s"), *JsonFilePath);
		return FBridgeExport();
	}
	bOutSuccess = true;
	OutMessage = FString::Printf(TEXT("Operation Succeded"));
	return ReturnData;
}

void UBPFunctionLib::WriteBridgeExportFile(FBridgeExport Data, bool& bOutSuccess, FString& OutMessage)
{
	TSharedPtr<FJsonObject> JsonObject = FJsonObjectConverter::UStructToJsonObject(Data);
	if (JsonObject == nullptr)
	{
		bOutSuccess = false;
		OutMessage = FString::Printf(TEXT("Invalid struct received, cannot convert to json"));
		return;
	}
	FString BridgeName = "AssetBridge.json";
	FString AssetBase;
	GetAssetsLocation(AssetBase);
	FString JsonFilePath = FPaths::Combine(AssetBase, BridgeName);
	WriteJson(JsonFilePath, JsonObject, bOutSuccess, OutMessage);
}

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

void UBPFunctionLib::GetSelectedContentItems(TArray<FAssetData>& SelectedAssets)
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	IContentBrowserSingleton& ContentBrowserSingleton = ContentBrowserModule.Get();

	ContentBrowserSingleton.GetSelectedAssets(SelectedAssets);
}

FString UBPFunctionLib::GetOSDirectoryLocation(const FString& DialogTitle)
{
	if (IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get())
	{
		FString DestinationFolder;
		const void* ParentWindowHandle = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);
		const FString DefaultLocation(FEditorDirectories::Get().GetLastDirectory(ELastDirectory::GENERIC_IMPORT));

		const bool bFolderSelected = DesktopPlatform->OpenDirectoryDialog(
			ParentWindowHandle,
			DialogTitle,
			DefaultLocation,
			DestinationFolder
		);

		if (bFolderSelected)
		{
			FEditorDirectories::Get().SetLastDirectory(ELastDirectory::GENERIC_EXPORT, DestinationFolder);
			return FPaths::ConvertRelativePathToFull(DestinationFolder);
		}
	}
	return FString("Unknown");
}

FString UBPFunctionLib::GetOSFileLocation(const FString& DialogTitle, const FString& FileTypes)
{
	if (IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get())
	{
		FString DestinationFolder;
		TArray<FString> OutFiles;
		const void* ParentWindowHandle = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);
		const FString DefaultLocation(FEditorDirectories::Get().GetLastDirectory(ELastDirectory::GENERIC_IMPORT));

		const bool bFolderSelected = DesktopPlatform->OpenFileDialog(
			ParentWindowHandle,
			DialogTitle,
			DefaultLocation,
			TEXT(""),
			FileTypes,
			EFileDialogFlags::None,
			OutFiles
		);

		if (bFolderSelected && OutFiles.Num() > 0)
		{
			FEditorDirectories::Get().SetLastDirectory(ELastDirectory::GENERIC_EXPORT, DestinationFolder);
			return FPaths::ConvertRelativePathToFull(OutFiles[0]);
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

void UBPFunctionLib::WriteJson(FString FilePath, TSharedPtr<FJsonObject> JsonObject, bool& bOutSuccess,
                               FString& OutInfoMessage)
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
	OutInfoMessage = FString::Printf(TEXT("wrote json to file: %s"), *FilePath);
}

void UBPFunctionLib::GetContentLocation(FString& OutContentLocation)
{
	UABSettings* Settings = GetMutableDefault<UABSettings>();
	if (Settings != nullptr)
	{
		OutContentLocation = Settings->UnrealContentLocation;
	}
}

FString UBPFunctionLib::GetContentLocation()
{
	UABSettings* Settings = GetMutableDefault<UABSettings>();
	if (Settings != nullptr)
	{
		return Settings->UnrealContentLocation;
	}
	return FString();
}

TArray<AActor*> UBPFunctionLib::GetWorldSelection()
{
	TArray<AActor*> OutActors;
	// TODO: Add filter for static /skeletal meshes only.
	USelection* SelectedActors = GEditor->GetSelectedActors();
	for (FSelectionIterator Iter(*SelectedActors); Iter; ++Iter)
	{
		AActor* Actor = Cast<AActor>(*Iter);
		TArray<UStaticMeshComponent*> Components;
		Actor->GetComponents(Components);
		if (Components.Num() > 0)
		{
			OutActors.Add(Actor);
		}
	}
	return OutActors;
}

void UBPFunctionLib::SetContentLocation(FString InLocation)
{
	UABSettings* Settings = GetMutableDefault<UABSettings>();
	if (Settings != nullptr)
	{
		Settings->UnrealContentLocation = InLocation;
		Settings->SaveConfig();
	}
}

void UBPFunctionLib::GetAssetsLocation(FString& OutContentLocation)
{
	UABSettings* Settings = GetMutableDefault<UABSettings>();
	if (Settings != nullptr)
	{
		OutContentLocation = Settings->AssetLocationOnDisk;
	}
}

void UBPFunctionLib::SetAssetsLocation(FString InLocation)
{
	UABSettings* Settings = GetMutableDefault<UABSettings>();
	if (Settings != nullptr)
	{
		Settings->AssetLocationOnDisk = InLocation;
		Settings->SaveConfig();
	}
}

void UBPFunctionLib::GetBridgeWorkingDir(FString& OutContentLocation)
{
	UABSettings* Settings = GetMutableDefault<UABSettings>();
	if (Settings != nullptr)
	{
		OutContentLocation = Settings->AssetBridgeWorkingDir;
	}
}

void UBPFunctionLib::SetBridgeWorkingDir(FString InLocation)
{
	UABSettings* Settings = GetMutableDefault<UABSettings>();
	if (Settings != nullptr)
	{
		Settings->AssetBridgeWorkingDir = InLocation;
		Settings->SaveConfig();
	}
}

TArray<FExportAsset> UBPFunctionLib::GetMeshData(AActor* Actor, bool& bIsSuccessful, FString& OutMessage)
{
	TArray<FExportAsset> Result;
	TArray<UStaticMeshComponent*> Components;
	Actor->GetComponents<UStaticMeshComponent>(Components);
	for (const auto Mesh : Components)
	{
		if (Mesh->GetStaticMesh() != nullptr)
		{
			FString ContentLocation;
			GetContentLocation(ContentLocation);
			FString ItemPath = Mesh->GetStaticMesh().GetPath();
			FString AssetPath;
			GetAssetsLocation(AssetPath);
			FString RelativeContentPath;
			FString ShortName;
			FString Discard;
			FPaths::Split(Mesh->GetStaticMesh().GetPath(), RelativeContentPath, ShortName, Discard);
			//FPaths::MakePathRelativeTo(RelativeContentPath, *FPaths::ProjectContentDir());
			RelativeContentPath = RelativeContentPath.Replace(TEXT("/Game"), TEXT(""));
			IFileManager::Get().MakeDirectory(*FPaths::Combine(AssetPath, RelativeContentPath), true);
			// If it starts with Engine or LevelPrototyping I need to ask the user for a new Path & Name as we can't replace engine items.
			FExportAsset ItemData;
			ItemData.InternalPath = ItemPath;
			ItemData.Model = Cast<UObject>(Mesh->GetStaticMesh());
			if (Mesh->GetStaticMesh() != nullptr)
			{
				TArray<FStaticMaterial> Materials = Mesh->GetStaticMesh()->GetStaticMaterials();
				for (auto Mat : Materials)
				{
					FExportMaterial NewMat;
					NewMat.Name = Mat.MaterialSlotName.ToString();
					NewMat.InternalPath = Mat.MaterialInterface.GetPath();
					NewMat.Idx = Mesh->GetStaticMesh()->GetMaterialIndex(FName(NewMat.Name));
					ItemData.Materials.Add(NewMat);
					UE_LOG(LogTemp, Warning, TEXT("Adding material: %s"), *NewMat.Name)
				}
			}
			ItemData.ShortName = ShortName;
			ItemData.RelativeExportPath = RelativeContentPath;
			FString FileName = ShortName.Append(".fbx");
			FString ExportLoc = FPaths::Combine(AssetPath, RelativeContentPath, FileName);
			//UE_LOG(LogTemp, Warning, TEXT("Adding file for export: %s"), *ExportLoc)
			ItemData.ExportLocation = ExportLoc;
			Result.Add(ItemData);
		}
	}

	bIsSuccessful = true;
	OutMessage = "Operation Succeeded.";
	return Result;
}