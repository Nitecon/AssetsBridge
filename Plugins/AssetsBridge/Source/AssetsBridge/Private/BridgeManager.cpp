// Copyright 2023 Nitecon Studios LLC. All rights reserved.


#include "BridgeManager.h"

#include "AssetsBridge.h"

UBridgeManager::UBridgeManager()
{
}

void UBridgeManager::GenerateExport(TArray<FBridgeAssets> AssetList, bool& bIsSuccessful, FString& OutMessage)
{
	for (auto asset : AssetList)
	{
		UE_LOG(LogTemp, Warning, TEXT("new asset to be exported"))
	}
	/*UnFbx::FFbxExporter* Exporter = UnFbx::FFbxExporter::GetInstance();
	bool bIsCanceled;
	bool bExportAll;
	Exporter->FillExportOptions(false, false, *FString(TEXT("")),bIsCanceled, bExportAll );*/
}

