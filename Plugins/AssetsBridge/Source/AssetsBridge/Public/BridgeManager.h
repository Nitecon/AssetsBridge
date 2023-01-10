// Copyright 2023 Nitecon Studios LLC. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "BridgeManager.generated.h"

USTRUCT(BlueprintType)
struct FExportMaterial
{
	GENERATED_BODY()

	/** Name of the material / slot name. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name = "";
	
	/** Name of the material / slot name. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Idx = 0;
	
	/** Where to find it in the content library. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString InternalPath = "";

	
};

USTRUCT(BlueprintType)
struct FExportAsset
{
	GENERATED_BODY()

	/** mesh pointer for it will be set here. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UObject* Model = nullptr;

	/** List of materials used by the model. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FExportMaterial> Materials;

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

	/**
	 * This functions checks to see if the actor path is part of "Engine" content, so it can be duplicated first.
	 * @param Path This is the path that is to be evaluated whether it is within system directories.
	 * @return Returns true if it is deemed to be an engine item false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category="Assets Bridge Tools")
	static bool IsSystemPath(FString Path);

	

	/**
	 * This function is responsible for duplicating the engine selected items and swapping them in the level with the new items.
	 * @param InAsset uses the engine information to create a duplicate and switch it out.
	 * @param bIsSuccessful indicates whether operation was successful
	 * @param OutMessage provides verbose information on the status of the operation.
	 * @return Returns the updated FExportAsset with the new duplicated path.
	 */
	UFUNCTION(BlueprintCallable, Category="Assets Bridge Tools")
	static FExportAsset DuplicateAndSwap(FExportAsset InAsset, bool& bIsSuccessful, FString& OutMessage);


	/**
	 * This function is responsible for checking to see if we have something selected duplicating engine content and assigning the array to GenerateExport.
	 * 
	 * @param bIsSuccessful indicates whether operation was successful
	 * @param OutMessage provides verbose information on the status of the operation.
	 */
	UFUNCTION(BlueprintCallable, Category="Assets Bridge Exports")
	static void StartExport(bool &bIsSuccessful, FString &OutMessage);


	/**
	 * This function is responsible for creating the export bundle that will be saved and made available for external 3D application.
	 * 
	 * @param AssetList contains the array of all assets to be exported to the scene.
	 * @param bIsSuccessful indicates whether operation was successful
	 * @param OutMessage provides verbose information on the status of the operation.
	 */
	static void GenerateExport(TArray<FExportAsset> AssetList, bool& bIsSuccessful, FString& OutMessage);

	/**
	 * This function is responsible for reading the manifest and importing the associated mesh in level or multiple meshes to asset library.
	 * 
	 * @param bIsSuccessful indicates whether operation was successful
	 * @param OutMessage provides verbose information on the status of the operation.
	 */
	UFUNCTION(BlueprintCallable, Category="Assets Bridge Exports")
	static void GenerateImport(bool &bIsSuccessful, FString &OutMessage);

private:
};
