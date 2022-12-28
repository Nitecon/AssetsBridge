// Copyright 2023 Nitecon Studios LLC. All rights reserved.


#include "BridgeManager.h"

#include "ABSettings.h"
#include "UnrealEd/Private/FbxExporter.h"
#include "AssetsBridge.h"
#include "BPFunctionLib.h"
#include "ActorFactories/ActorFactory.h"
#include "ActorFactories/ActorFactoryBlueprint.h"
#include "Animation/SkeletalMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Exporters/Exporter.h"

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

TArray<FExportAsset> UBridgeManager::GetMeshData(AActor* Actor, bool& bIsSuccessful, FString& OutMessage)
{
	TArray<FExportAsset> Result;
	TArray<UStaticMeshComponent*> Components;
	Actor->GetComponents<UStaticMeshComponent>(Components);
	for (const auto Mesh : Components)
	{
		if (Mesh->GetStaticMesh() != nullptr)
		{
			FString ContentLocation;
			UBPFunctionLib::GetContentLocation(ContentLocation);
			FString ItemPath = Mesh->GetStaticMesh().GetPath();
			FString AssetPath;
			UBPFunctionLib::GetAssetsLocation(AssetPath);
			FString RelativeContentPath;
			FString ShortName;
			FString Discard;
			FPaths::Split(Mesh->GetStaticMesh().GetPath(),RelativeContentPath,ShortName,Discard);
			//FPaths::MakePathRelativeTo(RelativeContentPath, *FPaths::ProjectContentDir());
			RelativeContentPath = RelativeContentPath.Replace(TEXT("/Game"), TEXT(""));
			IFileManager::Get().MakeDirectory(*FPaths::Combine(AssetPath,RelativeContentPath), true );
			// If it starts with Engine or LevelPrototyping I need to ask the user for a new Path & Name as we can't replace engine items.
			FExportAsset ItemData;
			ItemData.InternalPath = ItemPath;
			ItemData.Model = Cast<UObject>(Mesh->GetStaticMesh());
			ItemData.ShortName = ShortName;
			ItemData.RelativeExportPath = RelativeContentPath;
			ItemData.ExportLocation = FPaths::Combine(AssetPath,RelativeContentPath, ShortName.Append(".fbx"));
			Result.Add(ItemData);
		}
	}

	bIsSuccessful = true;
	OutMessage = "Operation Succeeded.";
	return Result;
}

void UBridgeManager::GenerateExport(TArray<AActor*> AssetList, bool& bIsSuccessful, FString& OutMessage)
{
	UnFbx::FFbxExporter* Exporter = UnFbx::FFbxExporter::GetInstance();
	bool bIsCanceled = false;
	bool bExportAll;
	INodeNameAdapter NodeNameAdapter;
	Exporter->FillExportOptions(false, false, UExporter::CurrentFilename, bIsCanceled, bExportAll);
	Exporter->SetExportOptionsOverride(nullptr);
	FBridgeExport ExportData;
	ExportData.Operation = "UnrealExport";
	for (auto Actor : AssetList)
	{
		auto MeshDataArray = GetMeshData(Actor, bIsSuccessful, OutMessage);
		for (auto Item : MeshDataArray)
		{
			bool bDidExport = false;
			if (Item.Model->IsA(UStaticMesh::StaticClass()))
			{
				UStaticMesh* Mesh = Cast<UStaticMesh>(Item.Model);
				if (Mesh != nullptr)
				{
					Exporter->CreateDocument();
					Exporter->ExportStaticMesh(Mesh);
					Exporter->WriteToFile(*Item.ExportLocation);
					Exporter->CloseDocument();
					bDidExport = true;
				}
			}
			if (Item.Model->IsA(USkeletalMesh::StaticClass()))
			{
				USkeletalMesh* Mesh = Cast<USkeletalMesh>(Item.Model);
				if (Mesh != nullptr)
				{
					Exporter->CreateDocument();
					Exporter->ExportSkeletalMesh(Mesh);
					Exporter->WriteToFile(*Item.ExportLocation);
					Exporter->CloseDocument();
					bDidExport = true;
				}
			}
			if (bDidExport)
			{
				FBridgeExportElement ExItem;
				ExItem.ExportLocation = *Item.ExportLocation;
				ExItem.InternalPath = *Item.InternalPath;
				ExItem.ObjectType = "StaticMesh";
				ExItem.ShortName = *Item.ShortName;
				ExportData.Objects.Add(ExItem);
			}
		}
	}
	Exporter->DeleteInstance();
	UBPFunctionLib::WriteBridgeExportFile(ExportData, bIsSuccessful, OutMessage);
}

void UBridgeManager::GenerateImport(bool& bIsSuccessful, FString& OutMessage)
{
	UE_LOG(LogTemp, Warning, TEXT("Starting import"))
	bIsSuccessful = true;
	OutMessage = "Operation Succeeded.";
}
