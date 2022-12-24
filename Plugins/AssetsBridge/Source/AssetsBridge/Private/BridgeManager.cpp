// Copyright 2023 Nitecon Studios LLC. All rights reserved.


#include "BridgeManager.h"
#include "UnrealEd/Private/FbxExporter.h"
#include "AssetsBridge.h"
#include "ActorFactories/ActorFactory.h"
#include "ActorFactories/ActorFactoryBlueprint.h"

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

void UBridgeManager::GenerateExport(TArray<FBridgeAssets> AssetList, bool& bIsSuccessful, FString& OutMessage)
{
	for (auto asset : AssetList)
	{
		UE_LOG(LogTemp, Warning, TEXT("Exporting [%s] as %s"), *asset.InternalPath, EnumToString(asset.AssetType))
	}

	/*UnFbx::FFbxExporter* Exporter = UnFbx::FFbxExporter::GetInstance();
	bool bIsCanceled;
	bool bExportAll;
	Exporter->FillExportOptions(false, false, *FString(TEXT("")),bIsCanceled, bExportAll );*/
	bIsSuccessful = true;
	OutMessage = "Operation Succeeded.";
}

void UBridgeManager::GenerateImport(bool& bIsSuccessful, FString& OutMessage)
{
	UE_LOG(LogTemp, Warning, TEXT("Starting import"))
	bIsSuccessful = true;
	OutMessage = "Operation Succeeded.";
}
