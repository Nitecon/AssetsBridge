// Copyright 2023 Nitecon Studios LLC. All rights reserved.


#include "BridgeManager.h"

#include "AssetsBridgeTools.h"
#include "ActorFactories/ActorFactory.h"
#include "ActorFactories/ActorFactoryBlueprint.h"
#include "EditorAssetLibrary.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "PackageTools.h"
#include "Engine/StaticMesh.h"
#include "Exporters/Exporter.h"
#include "Exporters/FbxExportOption.h"
#include "Factories/FbxFactory.h"
#include "Materials/MaterialInstance.h"
#include "UnrealEd/Private/FbxExporter.h"
#include "Editor/UnrealEd/Public/AssetImportTask.h"
#include "AssetToolsModule.h"

UBridgeManager::UBridgeManager()
{
}

void UBridgeManager::ExecuteSwap(TArray<AActor*> SelectList, TArray<FAssetData> ContentList, bool& bIsSuccessful, FString& OutMessage)
{
	if (SelectList.Num() < 1)
	{
		bIsSuccessful = false;
		OutMessage = "You must select at least 1 item in the level";
		return;
	}
	if (ContentList.Num() < 1)
	{
		bIsSuccessful = false;
		OutMessage = "You must select at least 1 from the content browser to replace the selected items with";
		return;
	}
	for (auto Asset : ContentList)
	{
		if (UClass* AssetClass = Asset.GetClass())
		{
			UActorFactory* Factory = nullptr;

			if (AssetClass->IsChildOf(UBlueprint::StaticClass()))
			{
				Factory = GEditor->FindActorFactoryByClass(UActorFactoryBlueprint::StaticClass());
			}
			else
			{
				const TArray<UActorFactory*>& ActorFactories = GEditor->ActorFactories;
				for (int32 FactoryIdx = 0; FactoryIdx < ActorFactories.Num(); FactoryIdx++)
				{
					UActorFactory* ActorFactory = ActorFactories[FactoryIdx];
					// Check if the actor can be created using this factory, making sure to check for an asset to be assigned from the selector
					FText ErrorMessage;
					if (ActorFactory->CanCreateActorFrom(Asset, ErrorMessage))
					{
						Factory = ActorFactory;
						break;
					}
				}
			}

			if (Factory)
			{
				GEditor->ReplaceSelectedActors(Factory, Asset);
			}
		}
	}
	bIsSuccessful = true;
	OutMessage = "Operation Succeeded.";
}

bool UBridgeManager::IsSystemPath(FString Path)
{
	if (Path.StartsWith("/Engine"))
	{
		return true;
	}
	return false;
}


FExportAsset UBridgeManager::DuplicateAndSwap(FExportAsset InAsset, bool& bIsSuccessful, FString& OutMessage)
{
	FExportAsset OutAsset;
	UStaticMesh* Mesh = Cast<UStaticMesh>(InAsset.Model);
	if (Mesh)
	{
		FString SourcePackagePath = UAssetsBridgeTools::GetPathWithoutExt(Mesh->GetPathName());
		FString TargetPath = UAssetsBridgeTools::GetSystemPathAsAssetPath(SourcePackagePath);
		UObject* DuplicateObject = UEditorAssetLibrary::DuplicateAsset(SourcePackagePath, TargetPath);
		if (DuplicateObject == nullptr)
		{
			bIsSuccessful = false;
			OutMessage = FString::Printf(TEXT("Cannot duplicate: %s to %s, does it already exist?"), *SourcePackagePath, *TargetPath);
			return {};
		}
		UStaticMesh* DuplicateMesh = Cast<UStaticMesh>(DuplicateObject);
		if (DuplicateMesh)
		{
			OutAsset.Model = DuplicateMesh;
			OutAsset.InternalPath = UAssetsBridgeTools::GetPathWithoutExt(DuplicateMesh->GetPathName()).Replace(TEXT("/Game"), TEXT(""));
			OutAsset.ShortName = UAssetsBridgeTools::GetPathWithoutExt(DuplicateMesh->GetName());
			TArray<FMaterialSlot> DupeMats;
			// now we duplicate each of the materials:
			auto StaticMats = Mesh->GetStaticMaterials();
			for (FStaticMaterial SrcMat : StaticMats)
			{
				FMaterialSlot DupeMaterial;
				FString SourceMaterialPath = UAssetsBridgeTools::GetPathWithoutExt(SrcMat.MaterialInterface->GetPathName());
				auto MatIdx = Mesh->GetMaterialIndex(FName(SrcMat.MaterialSlotName));
				DupeMaterial.Idx = MatIdx;
				DupeMaterial.Name = SrcMat.MaterialSlotName.ToString();
				FString TargetMatPath = UAssetsBridgeTools::GetSystemPathAsAssetPath(SourceMaterialPath);
				UObject* DuplicateMat = UEditorAssetLibrary::DuplicateAsset(SourceMaterialPath, TargetMatPath);
				if (DuplicateMat == nullptr)
				{
					bIsSuccessful = false;
					OutMessage = FString::Printf(TEXT("Cannot duplicate: %s to %s, does it already exist?"), *SourcePackagePath, *TargetPath);
					return {};
				}
				UMaterialInstance* NewMat = Cast<UMaterialInstance>(DuplicateMat);
				if (NewMat != nullptr)
				{
					DuplicateMesh->SetMaterial(MatIdx, NewMat);
					DupeMaterial.InternalPath = UAssetsBridgeTools::GetPathWithoutExt(NewMat->GetPathName());
				}
				DupeMats.Add(DupeMaterial);
			}
			OutAsset.Model = DuplicateMesh;
		}
		FAssetData AssetData = UAssetsBridgeTools::GetAssetDataFromPath(DuplicateMesh->GetPathName());
		TArray<FAssetData> AssetItems;
		AssetItems.Add(AssetData);
		ExecuteSwap(UAssetsBridgeTools::GetWorldSelection(), AssetItems, bIsSuccessful, OutMessage);
	}

	return OutAsset;
}

bool UBridgeManager::HasMatchingExport(TArray<FExportAsset> Assets, FAssetData InAsset)
{
	for (FExportAsset ExAsset : Assets)
	{
		if (ExAsset.Model->GetPathName().Equals(InAsset.GetAsset()->GetPathName()))
		{
			return true;
		}
	}
	return false;
}

FString UBridgeManager::ComputeTransformChecksum(FWorldData& Object)
{
	// Serialize the object data to a memory buffer
	TArray<uint8> ObjectData;
	FMemoryWriter Writer(ObjectData);
	Object.Serialize(Writer);

	// Compute the SHA-1 hash of the object data
	FSHA1 Sha1;
	Sha1.Update(ObjectData.GetData(), ObjectData.Num());
	Sha1.Final();
	uint8 Hash[FSHA1::DigestSize];
	Sha1.GetHash(Hash);

	// Convert the hash to a hexadecimal string
	FString HexHash;
	for (int i = 0; i < FSHA1::DigestSize; i++)
	{
		HexHash += FString::Printf(TEXT("%02x"), Hash[i]);
	}

	return HexHash;
}

void UBridgeManager::StartExport(bool& bIsSuccessful, FString& OutMessage)
{
	TArray<FExportAsset> ExportArray;
	TArray<FAssetData> SelectedAssets;
	UAssetsBridgeTools::GetSelectedContentBrowserItems(SelectedAssets);
	TArray<FAssetDetails> Selection = UAssetsBridgeTools::GetWorldSelectedAssets();
	if (Selection.Num() == 0 && SelectedAssets.Num() == 0)
	{
		bIsSuccessful = false;
		OutMessage = FString(TEXT("Please select at least one item in the level / content browser to export."));
		return;
	}
	if (Selection.Num() > 0)
	{
		for (auto SelItem : Selection)
		{
			//SelectedAsset.ObjectAsset.PackagePath.ToString())
			FExportAsset ExpItem = UAssetsBridgeTools::GetExportInfo(SelItem.ObjectAsset, bIsSuccessful, OutMessage);
			if (!bIsSuccessful)
			{
				return;
			}
			AActor* ItemActor = Cast<AActor>(SelItem.WorldObject);
			if (ItemActor)
			{
				UE_LOG(LogTemp, Warning, TEXT("Found this to be an actual world actor"))
				FRotator Rotator = ItemActor->GetActorTransform().GetRotation().Rotator();
				FWorldData ActorWorldInfo;
				ActorWorldInfo.Location = ItemActor->GetActorLocation();
				ActorWorldInfo.Rotation = FVector(Rotator.Roll, Rotator.Pitch, Rotator.Yaw);
				ActorWorldInfo.Scale = ItemActor->GetActorScale();
				ExpItem.WorldData = ActorWorldInfo;
				// Create a checksum from the world data to set as ObjectID
				//ExpItem.ObjectID = ComputeTransformChecksum(ExpItem.WorldData);
				ExpItem.ObjectID = ItemActor->GetName();
				//ExpItem.ObjectID = 
			}
			ExportArray.Add(ExpItem);
		}
		// we only have world selections so convert to assets and export with world context
	}
	if (SelectedAssets.Num() > 0)
	{
		for (auto CAsset : SelectedAssets)
		{
			// If a content browser item matches an item in the export array we can skip it as it should be the same item with world context. else add
			if (!HasMatchingExport(ExportArray, CAsset))
			{
				ExportArray.Add(UAssetsBridgeTools::GetExportInfo(CAsset, bIsSuccessful, OutMessage));
				if (!bIsSuccessful)
				{
					return;
				}
			}
		}
	}

	if (bIsSuccessful)
	{
		/*for (auto ExItem : ExportArray)
		{
			// Create a formatted FString
			FString InternalPath = FString::Printf(TEXT("/Game%s/%s"), *ExItem.InternalPath, *ExItem.ShortName);
			UE_LOG(LogTemp, Warning, TEXT("Exporting %s to %s"), *InternalPath, *ExItem.ExportLocation);
			ExportObject(InternalPath, ExItem.ExportLocation, bIsSuccessful, OutMessage);
			if (!bIsSuccessful)
			{
				return;
			}
		}*/
		GenerateExport(ExportArray, bIsSuccessful, OutMessage);
	}
}


void UBridgeManager::GenerateExport(TArray<FExportAsset> MeshDataArray, bool& bIsSuccessful, FString& OutMessage)
{
	UnFbx::FFbxExporter* Exporter = UnFbx::FFbxExporter::GetInstance();
	bool bIsCanceled = false;
	bool bExportAll;
	INodeNameAdapter NodeNameAdapter;
	Exporter->FillExportOptions(false, false, UExporter::CurrentFilename, bIsCanceled, bExportAll);
	UFbxExportOption* ExportOptions = Exporter->GetExportOptions();
	ExportOptions->FbxExportCompatibility = EFbxExportCompatibility::FBX_2020;
	ExportOptions->bForceFrontXAxis = false;
	ExportOptions->bASCII = false;
	ExportOptions->LevelOfDetail = false;
	ExportOptions->SaveOptions();
	Exporter->SetExportOptionsOverride(ExportOptions);
	FBridgeExport ExportData;
	ExportData.Operation = "UnrealExport";
	for (auto Item : MeshDataArray)
	{
		bool bDidExport = false;
		//FBridgeExportElement ExItem;
		// Create the distination directory if it doesn't already exist
		FString ItemPath = FPaths::GetPath(*Item.ExportLocation);
		if (!IFileManager::Get().DirectoryExists(*ItemPath))
		{
			const bool bTree = true;
			if (!IFileManager::Get().MakeDirectory(*ItemPath, bTree))
			{
				UE_LOG(LogTemp, Error, TEXT("%s. The destination directory could not be created."), *ItemPath);
				bIsSuccessful = false;
				OutMessage = FString::Printf(TEXT("%s. The destination directory could not be created."), *ItemPath);
				return;
			}
		}
		// TODO: just use Item.Model to run export task in the future.
		UStaticMesh* Mesh = Cast<UStaticMesh>(Item.Model);
		if (Mesh != nullptr)
		{
			Exporter->CreateDocument();
			Exporter->ExportStaticMesh(Mesh, &Mesh->GetStaticMaterials());
			Exporter->WriteToFile(*Item.ExportLocation);
			Exporter->CloseDocument();
			//UGLTFExporter::ExportToGLTF(Mesh, *Item.ExportLocation.Replace(TEXT(".fbx"), TEXT(".gltf")));
			bDidExport = true;
		}
		USkeletalMesh* SkeleMesh = Cast<USkeletalMesh>(Item.Model);
		if (SkeleMesh != nullptr)
		{
			Exporter->CreateDocument();
			Exporter->ExportSkeletalMesh(SkeleMesh);
			Exporter->WriteToFile(*Item.ExportLocation);
			Exporter->CloseDocument();
			//UGLTFExporter::ExportToGLTF(Mesh, *Item.ExportLocation.Replace(TEXT(".fbx"), TEXT(".gltf")));
			bDidExport = true;
		}
		if (bDidExport)
		{
			ExportData.Objects.Add(Item);
		}
	}
	Exporter->DeleteInstance();
	UAssetsBridgeTools::WriteBridgeExportFile(ExportData, bIsSuccessful, OutMessage);
}


void UBridgeManager::GenerateImport(bool& bIsSuccessful, FString& OutMessage)
{
	UE_LOG(LogTemp, Warning, TEXT("Starting import"))
	FBridgeExport BridgeData = UAssetsBridgeTools::ReadBridgeExportFile(bIsSuccessful, OutMessage);
	if (!bIsSuccessful)
	{
		return;
	}

	for (auto Item : BridgeData.Objects)
	{
		FString FbxFileName = FPaths::GetBaseFilename(Item.ExportLocation);
		// if item internalpath does not start with / prepend it
		if (!Item.InternalPath.StartsWith("/"))
		{
			Item.InternalPath = "/" + Item.InternalPath;
		}
		FString ImportPackageName = FString("/Game") + Item.InternalPath + FString("/") + Item.ShortName;
		ImportPackageName = UPackageTools::SanitizePackageName(ImportPackageName);
		bool bHasExisting = false;
		FString ExistingName;
		if (HasExistingPackageAtPath(ImportPackageName))
		{
			UStaticMesh* ExistingMesh = FindObject<UStaticMesh>(nullptr, *ImportPackageName);
			if (ExistingMesh != nullptr)
			{
				UE_LOG(LogTemp, Warning, TEXT("Found existing mesh, closing all related editors"))
				GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->CloseAllEditorsForAsset(ExistingMesh);
			}

			bHasExisting = true;
		}
		ImportAsset(Item.ExportLocation, ImportPackageName, bIsSuccessful, OutMessage);
		if (!bIsSuccessful)
		{
			return;
		}
	}
	bIsSuccessful = true;

	OutMessage = FString::Printf(TEXT("Operation was successful"));
}

void UBridgeManager::ReplaceRefs(FString OldPackageName, UPackage* NewPackage, bool& bIsSuccessful, FString& OutMessage)
{
	// move assets from the old package to the new package
	TArray<UObject*> Assets;
	GetObjectsWithOuter(FindPackage(nullptr, *OldPackageName), Assets);
	for (UObject* Asset : Assets)
	{
		Asset->Rename(nullptr, NewPackage, REN_DontCreateRedirectors | REN_DoNotDirty | REN_NonTransactional);
	}

	// replace all references to the old package with references to the new package
	TArray<UObject*> Objects;
	GetObjectsOfClass(UObject::StaticClass(), Objects);
	if (Objects.Num() > 0)
	{
		for (UObject* Obj : Objects)
		{
			if (Obj != nullptr && Obj->GetOuter() != nullptr)
			{
				if (Obj->GetOuter()->GetName() == OldPackageName)
				{
					Obj->Rename(nullptr, NewPackage, REN_DontCreateRedirectors | REN_DoNotDirty | REN_NonTransactional);
				}
			}
		}
	}
	// remove the old package from the asset registry
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistrySingleton = AssetRegistryModule.Get();
	TArray<FAssetData> AssetsData;
	AssetRegistrySingleton.GetAssetsByPackageName(*OldPackageName, AssetsData);
	for (const FAssetData& Asset : AssetsData)
	{
		bIsSuccessful = UEditorAssetLibrary::DeleteAsset(Asset.GetObjectPathString());
		if (!bIsSuccessful)
		{
			OutMessage = "Could not delete asset";
			return;
		}
	}
	bIsSuccessful = true;
	OutMessage = "References Replaced";
}

bool UBridgeManager::HasExistingPackageAtPath(FString InPath)
{
	const FString PackageName = FPackageName::ObjectPathToPackageName(InPath);
	return FPackageName::DoesPackageExist(PackageName);
}

UObject* UBridgeManager::ImportAsset(FString InSourcePath, FString InDestPath, bool& bIsSuccessful, FString& OutMessage)
{
	UAssetImportTask* ImportTask = CreateImportTask(InSourcePath, InDestPath, nullptr, nullptr, bIsSuccessful, OutMessage);
	if (!bIsSuccessful)
	{
		return nullptr;
	}
	UObject* RetAss = ProcessTask(ImportTask, bIsSuccessful, OutMessage);
	if (!bIsSuccessful)
	{
		return nullptr;
	}
	bIsSuccessful = true;
	OutMessage = "Asset Imported";
	return RetAss;
}

UObject* UBridgeManager::ProcessTask(UAssetImportTask* ImportTask, bool& bIsSuccessful, FString& OutMessage)
{
	if (ImportTask == nullptr)
	{
		bIsSuccessful = false;
		OutMessage = "Could not process task";
		return nullptr;
	}
	FAssetToolsModule* AssetTools = FModuleManager::LoadModulePtr<FAssetToolsModule>("AssetTools");
	if (AssetTools == nullptr)
	{
		bIsSuccessful = false;
		OutMessage = "Could not load asset tools module";
		return nullptr;
	}
	AssetTools->Get().ImportAssetTasks({ImportTask});
	if (ImportTask->GetObjects().Num() == 0)
	{
		bIsSuccessful = false;
		OutMessage = "Could not process task";
		return nullptr;
	}
	UObject* ImportedObject = StaticLoadObject(UObject::StaticClass(), nullptr, *FPaths::Combine(ImportTask->DestinationPath, ImportTask->DestinationName));
	if (ImportedObject == nullptr)
	{
		bIsSuccessful = false;
		OutMessage = "Import partially successful but returned invalid object";
		return nullptr;
	}
	bIsSuccessful = true;
	OutMessage = "Import success";
	return ImportedObject;
}

UAssetImportTask* UBridgeManager::CreateImportTask(FString InSourcePath, FString InDestPath, UFactory* InFactory,
                                                   UObject* ExtraOpts, bool& bIsSuccessful, FString& OutMessage)
{
	UAssetImportTask* ResTask = NewObject<UAssetImportTask>();
	if (ResTask == nullptr)
	{
		bIsSuccessful = false;
		OutMessage = "Could not create asset import task";
		return nullptr;
	}
	ResTask->Filename = InSourcePath;
	ResTask->DestinationPath = FPaths::GetPath(InDestPath);
	ResTask->DestinationName = FPaths::GetCleanFilename(InDestPath);

	ResTask->bSave = false;
	ResTask->bAutomated = true;
	ResTask->bAsync = false;
	ResTask->bReplaceExisting = true;
	ResTask->bReplaceExistingSettings = false;
	// should we bring in the FBXFactory options to handle additional capabilities like materials / textures?
	/*UFbxFactory* FbxFactory = NewObject<UFbxFactory>();
	FbxFactory->AddToRoot();
	FbxFactory->ImportUI->bImportMaterials = false;
	FbxFactory->ImportUI->bImportTextures = false;
	FbxFactory->ImportUI->bIsObjImport = true;
	FbxFactory->ImportUI->bIsReimport = bisReImport;
	FbxFactory->ImportUI->StaticMeshImportData->ImportRotation = FRotator(0, -90, 0); // X = Roll , Y = Pitch, Z = Yaw
	ResTask->Factory = FbxFactory;*/

	bIsSuccessful = true;
	OutMessage = "Task Created";
	return ResTask;
}

void UBridgeManager::ExportObject(FString InObjInternalPath, FString InDestPath, bool& bIsSuccessful, FString& OutMessage)
{
	FAssetToolsModule* AssetTools = FModuleManager::LoadModulePtr<FAssetToolsModule>("AssetTools");
	if (AssetTools == nullptr)
	{
		bIsSuccessful = false;
		OutMessage = "Could not load asset tools module";
		AssetTools = nullptr;
		return;
	}
	AssetTools->Get().ExportAssets(TArray<FString>{InObjInternalPath}, FPaths::GetPath(InDestPath));
	bIsSuccessful = true;
	OutMessage = "Export success";
}
