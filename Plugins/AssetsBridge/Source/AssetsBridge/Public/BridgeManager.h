// Copyright 2023 Nitecon Studios LLC. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "BridgeManager.generated.h"

/**
 * 
 */
UCLASS()
class ASSETSBRIDGE_API UBridgeManager : public UObject
{
public:
	GENERATED_BODY()

	/** Generic constructor */
	UBridgeManager();

	/**
	 * This function is responsible for creating the export bundle that will be saved and made available for external 3D application.
	 * 
	 * @param SelectList the array of items currently selected within the level
	 * @param ContentList the array of items currently selected within the content browser.
	 * @param bIsSuccessful indicates whether operation was successful
	 * @param OutMessage provides verbose information on the status of the operation.
	 */
	UFUNCTION(BlueprintCallable, Category="Assets Bridge Exports")
	static void ExecuteSwap(TArray<AActor*> SelectList, TArray<FAssetData> ContentList, bool &bIsSuccessful, FString &OutMessage);

	/**
	 * This function is responsible for creating the export bundle that will be saved and made available for external 3D application.
	 * 
	 * @param AssetList contains the array of all assets to be exported to the scene.
	 * @param bIsSuccessful indicates whether operation was successful
	 * @param OutMessage provides verbose information on the status of the operation.
	 */
	UFUNCTION(BlueprintCallable, Category="Assets Bridge Exports")
	static void GenerateExport(TArray<FBridgeAssets> AssetList, bool &bIsSuccessful, FString &OutMessage);

	/**
	 * This function is responsible for reading the manifest and importing the associated mesh in level or multiple meshes to asset library.
	 * 
	 * @param bIsSuccessful indicates whether operation was successful
	 * @param OutMessage provides verbose information on the status of the operation.
	 */
	UFUNCTION(BlueprintCallable, Category="Assets Bridge Exports")
	static void GenerateImport(bool &bIsSuccessful, FString &OutMessage);
};
