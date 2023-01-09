// Copyright 2023 Nitecon Studios LLC. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "JsonObjectConverter.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BPFunctionLib.generated.h"


USTRUCT(BlueprintType)
struct FMaterialSlot
{
	GENERATED_BODY()

	/** Name of the material / slot name. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name = "";

	/** Material index */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Idx = 0;
	
	/** Where to find it in the content library. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString InternalPath = "";

	
};

USTRUCT(BlueprintType, Category="JSON")
struct FBridgeExportElement
{
	GENERATED_BODY()

public:
	/** Name of the actual file for use in export. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="JSON")
	FString ShortName = "";

	/** Where to find it in the content library. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="JSON")
	FString InternalPath = "";

	/** Location of where to export. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="JSON")
	FString ExportLocation = "";

	/** Location of where to export. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="JSON")
	FString ObjectType = "StaticMesh";

	/** Material information for the object. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="JSON")
	TArray<FMaterialSlot> ObjectMaterials;

	/* Todo: ADD Checksum at some point...*/
	
};

USTRUCT(BlueprintType, Category="JSON")
struct FBridgeExport
{
	GENERATED_BODY()

public:
	/** Name of the actual file for use in export. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="JSON")
	FString Operation = "UnrealExport";

	/** Where to find it in the content library. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="JSON")
	TArray<FBridgeExportElement> Objects;

};



class UStruct;
/**
 * 
 */
UCLASS()
class ASSETSBRIDGE_API UBPFunctionLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Reads transforms the selected content and ensures a proper path is selected for it.
	 *
	 * @param AssetList Use the provided asset list to ensure non engine content.
	 * @param bIsSuccessful Provides boolean whether operation succeeded.
	 * @param OutMessage Provides more verbose information on the operation.
	 */
	UFUNCTION(BlueprintCallable, Category="Assets Bridge Utilities")
	static void StartExport(TArray<FExportAsset> ChangedList, TArray<FExportAsset> ReadyList,  bool& bIsSuccessful, FString& OutMessage);

	/**
	 * Combines name and new internal path and stitches it onto the user provided export base.
	 *
	 * @param NewInternalPath The new location where the asset will be residing.
	 * @param NewName The new name for the asset as provided by the user.
	 * @return Returns the full export path.
	 */
	UFUNCTION(BlueprintCallable, Category="Assets Bridge Utilities")
	static FString GetExportPathFromInternal(FString NewInternalPath, FString NewName);

	UFUNCTION(BlueprintCallable, Category="Assets Bridge Utilities")
	static void CloseExportTab();

	/**
	 * Reads a JSON file into a FBridgeExportElement Structure and returns the content as a string.
	 *
	 * @param bOutSuccess Provides boolean whether operation succeeded.
	 * @param OutMessage Provides more verbose information on the operation.
	 *
	 * @return Returns a list of Bridge Export Elements which are read from a file..
	 */
	UFUNCTION(BlueprintCallable, Category="JSON")
	static FBridgeExport ReadBridgeExportFile(bool& bOutSuccess, FString& OutMessage);

	/**
		 * Writes a JSON file from a Array of FBridgeExportElement Structure.
		 *
		 * @param Data Contains the data that is to be converted over.
		 * @param bOutSuccess Provides boolean whether operation succeeded.
		 * @param OutMessage Provides more verbose information on the operation.
		 */
	UFUNCTION(BlueprintCallable, Category="JSON")
	static void WriteBridgeExportFile(FBridgeExport Data, bool& bOutSuccess, FString& OutMessage);
	
	/**
	 * Finds the currently selected folder within the content tree / content browser view.
	 *
	 * @param OutContentLocation Sets the currently selected folder path to this value.
	 */
	UFUNCTION(BlueprintCallable, Category="Assets Bridge Utilities")
	static void GetSelectedFolderPath(FString& OutContentLocation);

	/**
	 * Finds the currently selected items within the content tree / content browser view.
	 *
	 * @param SelectedAssets Appends the array with the currently selected items.
	 */
	UFUNCTION(BlueprintCallable, Category="Assets Bridge Utilities")
	static void GetSelectedContentItems(TArray<FAssetData>& SelectedAssets);

	/**
	 * Opens a dialog for the user to browse and select a directory on disk.
	 *
	 * @param DialogTitle Title to add to the dialog box to establish intent for the dialog box.
	 *
	 * @return Provides an FString to be used which contains the location of the Directory.
	 */
	UFUNCTION(BlueprintCallable, Category="Assets Bridge Utilities")
	static FString GetOSDirectoryLocation(const FString& DialogTitle);

	/**
	 * Opens a dialog for the user to browse and select a file on disk.
	 *
	 * @param DialogTitle Title to add to the dialog box to establish intent for the dialog box.
	 * @param FileTypes List of files to filter on in the format TEXT("JSON files (*.json)|*.json").
	 *
	 * @return Provides an FString to be used which contains the location of the file.
	 */
	UFUNCTION(BlueprintCallable, Category="Assets Bridge Utilities")
	static FString GetOSFileLocation(const FString& DialogTitle, const FString& FileTypes = TEXT("JSON files (*.json)|*.json"));

	/**
	 * Reads a file and returns the content as a string.
	 *
	 * @param FilePath Location for the file to be read on disk.
	 * @param bOutSuccess Provides boolean whether operation succeeded.
	 * @param OutInfoMessage Provides more verbose information on the operation.
	 *
	 * @return Provides an FString to be used which contains the content of the file.
	 */
	UFUNCTION(BlueprintCallable, Category="Assets Bridge Utilities")
	static FString ReadStringFromFile(FString FilePath, bool& bOutSuccess, FString& OutInfoMessage);

	/**
	 * Sets the Assets Bridge content location where assets will be stored (root assets folder)
	 *
	 * @param FilePath Location for the file to be written on disk.
	 * @param Data the contents to be written to the file.
	 * @param bOutSuccess Provides boolean whether operation succeeded.
	 * @param OutInfoMessage Provides more verbose information on the operation.
	 */
	UFUNCTION(BlueprintCallable, Category="Assets Bridge Utilities")
	static void WriteStringToFile(FString FilePath, FString Data, bool& bOutSuccess, FString& OutInfoMessage);


	/**
	 * Open a json file read it's content and convert it to a json object
	 *
	 * @param FilePath	Location of the json file on disk.
	 * @param bOutSuccess Returns true of operation is successful.
	 * @param OutInfoMessage Verbose information on the current operation.
	 *
	 * @return The JsonObject content of your json file.
	 */
	static TSharedPtr<FJsonObject> ReadJson(FString FilePath, bool& bOutSuccess, FString& OutInfoMessage);

	/**
	* Open a json file read it's content and convert it to a json object
	*
	* @param FilePath	Location of the json file on disk.
	* @param JsonObject Object to write to file
	* @param bOutSuccess Returns true of operation is successful.
	* @param OutInfoMessage Verbose information on the current operation.
	*/
	static void WriteJson(FString FilePath, TSharedPtr<FJsonObject> JsonObject, bool& bOutSuccess, FString& OutInfoMessage);

	/**
	* Gets the Assets Bridge location related to this setting.
	*
	* @param OutContentLocation Is set to the location for this setting to be consumed by blueprints etc.
	*/
	UFUNCTION(BlueprintCallable, Category="Assets Bridge Settings")
	static void GetContentLocation(FString& OutContentLocation);

	/**
	* Gets the Assets Bridge location related to this setting.
	*
	* @return provides the location for this setting .
	*/
	static FString GetContentLocation();

	/**
	 * Returns the user selected list of items to be exported or interacted with, returns only items that are static meshes or skeletal meshes.
	 */
	UFUNCTION(BlueprintCallable, Category="AssetsBridge Utilities")
	static TArray<AActor*> GetWorldSelection();

	/**
	* Sets the Assets Bridge config option for the related setting.
	*
	* @param InLocation Sets the provided string to the value of the content location.
	*/
	UFUNCTION(BlueprintCallable, Category="Assets Bridge Settings")
	static void SetContentLocation(FString InLocation);

	/**
	* Gets the Assets Bridge location related to this setting.
	*
	* @param OutContentLocation Is set to the location for this setting to be consumed by blueprints etc.
	*/
	UFUNCTION(BlueprintCallable, Category="Assets Bridge Settings")
	static void GetAssetsLocation(FString& OutContentLocation);

	/**
	* Sets the Assets Bridge config option for the related setting.
	*
	* @param InLocation Sets the provided string to the value of the content location.
	*/
	UFUNCTION(BlueprintCallable, Category="Assets Bridge Settings")
	static void SetAssetsLocation(FString InLocation);

	/**
	* Gets the Assets Bridge location related to this setting.
	*
	* @param OutContentLocation Is set to the location for this setting to be consumed by blueprints etc.
	*/
	UFUNCTION(BlueprintCallable, Category="Assets Bridge Settings")
	static void GetBridgeWorkingDir(FString& OutContentLocation);

	/**
	* Sets the Assets Bridge config option for the related setting.
	*
	* @param InLocation Sets the provided string to the value of the content location.
	*/
	UFUNCTION(BlueprintCallable, Category="Assets Bridge Settings")
	static void SetBridgeWorkingDir(FString InLocation);

	
	UFUNCTION(BlueprintCallable, Category="Assets Bridge Utilities")
	static TArray<FExportAsset> GetMeshData(AActor* Actor, bool& bIsSuccessful, FString& OutMessage);

	template <typename T>
	static FString EnumToString(const FString& enumName, const T value)
	{
		UEnum* pEnum = FindObject<UEnum>(ANY_PACKAGE, *enumName);
		return *(pEnum ? pEnum->GetNameStringByIndex(static_cast<uint8>(value)) : "null");
	}
};
