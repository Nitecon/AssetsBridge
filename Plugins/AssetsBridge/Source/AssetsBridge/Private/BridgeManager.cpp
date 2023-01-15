// Copyright 2023 Nitecon Studios LLC. All rights reserved.


#include "BridgeManager.h"

#include "AssetsBridgeTools.h"
#include "ActorFactories/ActorFactory.h"
#include "ActorFactories/ActorFactoryBlueprint.h"
#include "EditorAssetLibrary.h"
#include "IAssetTools.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "PackageTools.h"
#include "Engine/StaticMesh.h"
#include "Exporters/Exporter.h"
#include "Factories/FbxFactory.h"
#include "Materials/MaterialInstance.h"
#include "UnrealEd/Private/FbxExporter.h"

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
				ExpItem.WorldData = FTransform(ItemActor->GetActorRotation(), ItemActor->GetActorLocation(), ItemActor->GetActorScale3D());
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
	Exporter->SetExportOptionsOverride(nullptr);
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
		FString ImportPackageName = FString("/Game") + Item.InternalPath + FString("/") + Item.ShortName;
		ImportPackageName = UPackageTools::SanitizePackageName(ImportPackageName);
		bool bHasExisting = false;
		FString ExistingName;
		if (HasExistingPackageAtPath(ImportPackageName))
		{
			bHasExisting = true;
			UE_LOG(LogTemp, Warning, TEXT("Package already exists at %s"), *ImportPackageName)
			ExistingName = MoveExistingPackage(ImportPackageName, bIsSuccessful, OutMessage);
			if (!bIsSuccessful)
			{
				return;
			}
			UE_LOG(LogTemp, Warning, TEXT("Moved existing package %s to: %s"),*ImportPackageName, *ExistingName);
		}
		UE_LOG(LogTemp, Warning, TEXT("Ready to process %s"), *ImportPackageName);
		//UObject* Obj = ImportFBX(Item.InternalPath, Item.ShortName, Item.ExportLocation, bIsSuccessful, OutMessage);
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
	FString PackageName = FPackageName::ObjectPathToPackageName(InPath);
	return FPackageName::DoesPackageExist(PackageName);
}

FString UBridgeManager::MoveExistingPackage(FString InPath, bool& bIsSuccessful, FString& OutMessage)
{
	FString PackageName = FPackageName::ObjectPathToPackageName(InPath);
	UPackage* ExistingPackage = FindPackage(nullptr, *PackageName);
	FString NewPackageName = PackageName + "_old";
	if (ExistingPackage != nullptr)
	{
		TArray<FAssetData> Assets;
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		AssetRegistryModule.Get().GetAssetsByPackageName(FName(PackageName), Assets);
		UPackage* PrimPackage = nullptr;
		TArray<UObject*> AssetsToMove;
		for (const FAssetData& Asset : Assets)
		{
			FAssetRenameManager::RenameAssets(const TArray<FAssetRenameData>& AssetsAndNames)
			UObject* Obj = Asset.GetAsset();
			Obj->Rename(*Obj->GetName().Append("_old"), ExistingPackage, REN_DontCreateRedirectors | REN_DoNotDirty | REN_NonTransactional);
			AssetsToMove.Add(Obj);
			PrimPackage = Obj->GetPackage();
		}
		//ExistingPackage->Rename(*NewPackageName, GetTransientPackage(), REN_DontCreateRedirectors | REN_DoNotDirty | REN_NonTransactional);
		UPackageTools::SavePackagesForObjects(AssetsToMove);
		UPackageTools::ReloadPackages(TArray<UPackage*>{PrimPackage});
	}
	return NewPackageName;
}

UObject* UBridgeManager::ImportFBX(FString InternalPath, FString AssetName, FString ExternalFile, bool& bIsSuccessful, FString& OutMessage)
{
	FString LogMessage;
	UObject* ImportedObject = nullptr;
	//ExternalFile.Replace(TEXT("\""), TEXT("/"));

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	UFbxFactory* FbxFactory = NewObject<UFbxFactory>();
	FbxFactory->AddToRoot();
	//FbxFactory->EnableShowOption();

	FString FbxFileName = FPaths::GetBaseFilename(ExternalFile);
	FString ImportPackageName = FString("/Game") + InternalPath + FString("/") + FbxFileName;
	ImportPackageName = UPackageTools::SanitizePackageName(ImportPackageName);
	bool bIsReImportTask = false;
	EObjectFlags Flags;
	UStaticMesh* ExistingMesh = nullptr;
	UPackage* ExistingPackage = nullptr;
	FString ExistingImportPackageName = ImportPackageName;
	AssetName.Append("Foo");
	
	if (FPackageName::DoesPackageExist(ImportPackageName))
	{
		bIsReImportTask = true;
		ExistingMesh = FindObject<UStaticMesh>(ExistingPackage, *AssetName);
		if (ExistingMesh != nullptr)
		{
			GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->CloseAllEditorsForAsset(ExistingMesh);
		}

		ImportPackageName.Append("_New");
		LogMessage = ImportPackageName + FString(" already exists!, import will use: ") + ImportPackageName;
		UE_LOG(LogTemp, Warning, TEXT("Ex: %s"), *LogMessage);
	}
	UPackage* NewPackage = CreatePackage(*ImportPackageName);
	NewPackage->FullyLoad();

	Flags = RF_Standalone | RF_Public | RF_Transient;
	FbxFactory->ImportUI->bIsObjImport = true;
	bool bImportCancelled = false;
	//ImportedObject = FbxFactory->ImportObject(UStaticMesh::StaticClass(),ModelPackage, FName(*FbxFileName),RF_Standalone | RF_Public,ExternalFile,nullptr,bImportCancelled);
	ImportedObject = FbxFactory->ImportObject(UStaticMesh::StaticClass(), NewPackage, FName(*FbxFileName), Flags, ExternalFile, nullptr, bImportCancelled);

	if (bImportCancelled)
	{
		if (NewPackage != nullptr)
		{
			bIsSuccessful = false;
			OutMessage = "Import was canceled";
			// @todo clean up created package
			NewPackage->ConditionalBeginDestroy();
			FbxFactory->RemoveFromRoot();
			FbxFactory->ConditionalBeginDestroy();
			return nullptr;
		}
	}

	ReplaceRefs(ExistingImportPackageName, NewPackage, bIsSuccessful, OutMessage);
	if (bIsSuccessful)
	{
		TArray<FAssetData> Assets;
		AssetRegistryModule.Get().GetAssetsByPackageName(*ExistingImportPackageName, Assets);
		for (const FAssetData& Asset : Assets)
		{
			bIsSuccessful = UEditorAssetLibrary::DeleteAsset(Asset.GetObjectPathString());
			FAssetRegistryModule::AssetDeleted(Asset.GetAsset());
			if (!bIsSuccessful)
			{
				OutMessage = "Could not remove existing object...";
				bIsSuccessful = false;
				return nullptr;
			}
		}
		CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
	}
	// Unload the old package.
	
	if (bIsSuccessful)
	{
		if (bIsReImportTask)
		{
			//TODO: ModelPackage->Rename(*ExistingImportPackageName);
			FString PackageName = NewPackage->GetName();
		}
		UPackageTools::SavePackagesForObjects(TArray<UObject*>{ImportedObject});
		OutMessage = "Renamed successfully";
	}
	else
	{
		bIsSuccessful = false;
		OutMessage = "Could not rename new asset to old";
	}
	FbxFactory->RemoveFromRoot();
	FbxFactory->ConditionalBeginDestroy();
	FbxFactory = nullptr;
	OutMessage = "Import success";
	return ImportedObject;
}

