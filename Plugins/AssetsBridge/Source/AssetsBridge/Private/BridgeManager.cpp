// Copyright 2023 Nitecon Studios LLC. All rights reserved.


#include "BridgeManager.h"

#include "AssetImportTask.h"
#include "AssetsBridgeTools.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "ActorFactories/ActorFactory.h"
#include "ActorFactories/ActorFactoryBlueprint.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "EditorAssetLibrary.h"
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
	if (Path.StartsWith("/Game/LevelPrototyping"))
	{
		return true;
	}
	if (Path.StartsWith("/Engine"))
	{
		return true;
	}
	if (Path.StartsWith("/LevelPrototyping"))
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
			TArray<FExportMaterial> DupeMats;
			// now we duplicate each of the materials:
			auto StaticMats = Mesh->GetStaticMaterials();
			for (FStaticMaterial SrcMat : StaticMats)
			{
				FExportMaterial DupeMaterial;
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

void UBridgeManager::StartExport(bool& bIsSuccessful, FString& OutMessage)
{
	auto Selection = UAssetsBridgeTools::GetWorldSelection();
	if (Selection.Num() == 0)
	{
		bIsSuccessful = false;
		OutMessage = FString(TEXT("Please select at least one item in the level to continue."));
		return;
	}
	TArray<FExportAsset> ExportArray;
	for (auto Actor : Selection)
	{
		auto MeshData = UAssetsBridgeTools::GetMeshData(Actor, bIsSuccessful, OutMessage);
		if (!bIsSuccessful)
		{
			return;
		}
		for (auto MeshInfo : MeshData)
		{
			if (IsSystemPath(MeshInfo.InternalPath))
			{
				// Start duplication and swap, then add to array.
				FExportAsset NewItem = DuplicateAndSwap(MeshInfo, bIsSuccessful, OutMessage);
				if (!bIsSuccessful)
				{
					return;
				}
				ExportArray.Add(NewItem);
			}
			else
			{
				ExportArray.Add(MeshInfo);
			}
		}
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
		FBridgeExportElement ExItem;
		

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
			ExItem.ObjectType = "StaticMesh";
			ExItem.ExportLocation = *Item.ExportLocation;
			FString InternalBase = Item.InternalPath.Replace(TEXT("/Game"), TEXT(""));
			ExItem.InternalPath = UAssetsBridgeTools::GetPathWithoutExt(InternalBase);
			ExItem.ShortName = *Item.ShortName;
			//TArray<UMaterialInterface*> Materials = Mesh->Materials_DEPRECATED;
			// GetNumMaterials is now missing so we'll have to iterate over the mats manually to a limit of 64 as max for fbx.
			TArray<FStaticMaterial> Materials = Mesh->GetStaticMaterials();
			for (auto mat : Materials)
			{
				FMaterialSlot NewSlotMat;
				NewSlotMat.Name = mat.MaterialSlotName.ToString();
				NewSlotMat.InternalPath = UAssetsBridgeTools::GetPathWithoutExt(mat.MaterialInterface.GetPath());
				ExItem.ObjectMaterials.Add(NewSlotMat);
			}
			Exporter->CreateDocument();
			Exporter->ExportStaticMesh(Mesh, &Materials);
			Exporter->WriteToFile(*Item.ExportLocation);
			Exporter->CloseDocument();
			//UGLTFExporter::ExportToGLTF(Mesh, *Item.ExportLocation.Replace(TEXT(".fbx"), TEXT(".gltf")));
			bDidExport = true;
		}

		USkeletalMesh* SkeleMesh = Cast<USkeletalMesh>(Item.Model);
		if (SkeleMesh != nullptr)
		{
			ExItem.ObjectType = "SkeletalMesh";
			ExItem.ExportLocation = *Item.ExportLocation;
			FString InternalBase = Item.InternalPath.Replace(TEXT("/Game"), TEXT(""));
			ExItem.InternalPath = UAssetsBridgeTools::GetPathWithoutExt(InternalBase);
			ExItem.ShortName = *Item.ShortName;
			Exporter->CreateDocument();
			Exporter->ExportSkeletalMesh(SkeleMesh);
			Exporter->WriteToFile(*Item.ExportLocation);
			Exporter->CloseDocument();
			bDidExport = true;
		}
		if (bDidExport)
		{
			ExportData.Objects.Add(ExItem);
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

	UnFbx::FFbxImporter* Importer = UnFbx::FFbxImporter::GetInstance();

	for (auto Item : BridgeData.Objects)
	{
		TArray<UObject*> ReturnObjects;
		if (Item.ObjectType.Equals("StaticMesh"))
		{
			bool bPostMapMaterials = false;
			FString InternalName = FPaths::Combine(FString("/Game"), *Item.InternalPath, *Item.ShortName);
			UE_LOG(LogTemp, Warning, TEXT("Creating Package: %s"), *InternalName)
			UPackage* Package = CreatePackage(*InternalName);
			if (!ensure(Package))
			{
				// Failed to create the package to hold this asset for some reason
				continue;
			}
			UFbxFactory* Factory = NewObject<UFbxFactory>(UFbxFactory::StaticClass(), FName("Factory"), RF_NoFlags);
			Factory->ResetState();
			if (Factory == nullptr)
			{
				bIsSuccessful = false;
				OutMessage = FString::Printf(TEXT("cannot instantiate import factory."));
				return;
			}
			bool bImportWasCancelled = false;
			UClass* ImportAssetType = Factory->ResolveSupportedClass();
			UObject* Result;
			UAssetImportTask* ImportTask = NewObject<UAssetImportTask>(UAssetImportTask::StaticClass());
			ImportTask->bAutomated = true;
			ImportTask->bReplaceExisting = true;
			ImportTask->Options = Factory->ImportUI;
			Factory->SetAssetImportTask(ImportTask);
			Factory->ImportUI->bImportMaterials = true;
			
			if (Item.ObjectMaterials.Num() > 0)
			{
				Factory->ImportUI->bImportMaterials = false;
				bPostMapMaterials = false;
			}
			else
			{
				Factory->ImportUI->bImportMaterials = true;
				bPostMapMaterials = false;
				
			}
			//Factory->ImportUI->bImportMaterials = false;
			Result = Factory->ImportObject(ImportAssetType, Package, FName(*Item.ShortName), RF_Public | RF_Standalone | RF_Transactional, Item.ExportLocation, nullptr, bImportWasCancelled);
			// Do not report any error if the operation was canceled.
			if (!bImportWasCancelled)
			{
				if (Result)
				{
					UStaticMesh* ResultMesh = Cast<UStaticMesh>(Result);
					if (ResultMesh)
					{
						if (bPostMapMaterials)
						{
							for (auto Mat : Item.ObjectMaterials)
							{
								UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, *Mat.InternalPath);
								if (Material)
								{
									ResultMesh->SetMaterial(Mat.Idx, Material);
								}
							}
						}
						
					}

					ReturnObjects.Add(Result);

					// Notify the asset registry
					FAssetRegistryModule::AssetCreated(Result);
					GEditor->BroadcastObjectReimported(Result);
					bIsSuccessful = true;
				}
				else
				{
					FString Msg = FString::Printf(TEXT("Failed to import '%s'. Failed to create asset '%s'.\nPlease see Output Log for details."), *Item.ExportLocation, *Item.ShortName);
					const FText Message = FText::FromString(Msg);
					if (!Factory->IsAutomatedImport())
					{
						FMessageDialog::Open(EAppMsgType::Ok, Message);
					}
					UE_LOG(LogTemp, Warning, TEXT("%s"), *Message.ToString());
				}
			}
			if (ReturnObjects.Num())
			{
				FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
				ContentBrowserModule.Get().SyncBrowserToAssets(ReturnObjects, /*bAllowLockedBrowsers=*/true);
			}
		}
	}

	bIsSuccessful = true;
	OutMessage = FString::Printf(TEXT("Operation was successful"));
}
