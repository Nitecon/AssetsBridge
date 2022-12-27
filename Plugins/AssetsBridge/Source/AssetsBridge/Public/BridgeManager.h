// Copyright 2023 Nitecon Studios LLC. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "BridgeManager.generated.h"

USTRUCT(BlueprintType)
struct FExportAsset
{
	GENERATED_BODY()

	/** mesh pointer for it will be set here. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UObject* Model = nullptr;

	/** Where to find it in the content library. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString InternalPath = "";

	/** Name of the actual file for use in export. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RelativeExportPath = "";
	
	/** Name of the actual file for use in export. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ShortName = "";
	
	/** Location of where to export. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ExportLocation = "";

	/** Location of where to export. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString StringType = "StaticMesh";
	
};

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

	UFUNCTION(BlueprintCallable, Category="Assets Bridge Utilities")
	static TArray<FExportAsset> GetMeshData(AActor* Actor, bool& bIsSuccessful, FString& OutMessage);

	/**
	 * This function is responsible for creating the export bundle that will be saved and made available for external 3D application.
	 * 
	 * @param AssetList contains the array of all assets to be exported to the scene.
	 * @param bIsSuccessful indicates whether operation was successful
	 * @param OutMessage provides verbose information on the status of the operation.
	 */
	UFUNCTION(BlueprintCallable, Category="Assets Bridge Exports")
	static void GenerateExport(TArray<AActor*> AssetList, bool& bIsSuccessful, FString& OutMessage);

	/**
	 * This function is responsible for reading the manifest and importing the associated mesh in level or multiple meshes to asset library.
	 * 
	 * @param bIsSuccessful indicates whether operation was successful
	 * @param OutMessage provides verbose information on the status of the operation.
	 */
	UFUNCTION(BlueprintCallable, Category="Assets Bridge Exports")
	static void GenerateImport(bool &bIsSuccessful, FString &OutMessage);
};
