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
	 * @param AssetList contains the array of all assets to be exported to the scene.
	 * @param bIsSuccessful indicates whether operation was successful
	 * @param OutMessage provides verbose information on the status of the operation.
	 */
	UFUNCTION(BlueprintCallable, Category="Assets Bridge Exports")
	void GenerateExport(TArray<FBridgeAssets> AssetList, bool &bIsSuccessful, FString &OutMessage);
};
