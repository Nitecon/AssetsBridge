// Copyright 2023 Nitecon Studios LLC. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "JsonObjectConverter.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BPFunctionLib.generated.h"

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
	 * Finds the currently selected folder within the content tree / content browser view.
	 *
	 * @param OutContentLocation Sets the currently selected folder path to this value.
	 */
	UFUNCTION(BlueprintCallable, Category="Assets Bridge Utilities")
	static void GetSelectedFolderPath(FString& OutContentLocation);

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
	static FString GetOSFileLocation(const FString& DialogTitle,
	                                 const FString& FileTypes = TEXT("JSON files (*.json)|*.json"));

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
	static void WriteJson(FString FilePath, TSharedPtr<FJsonObject> JsonObject, bool& bOutSuccess,
	                      FString& OutInfoMessage);

	/**
	* Gets the Assets Bridge location related to this setting.
	*
	* @param OutContentLocation Is set to the location for this setting to be consumed by blueprints etc.
	*/
	UFUNCTION(BlueprintCallable, Category="Assets Bridge Settings")
	static void GetContentLocation(FString& OutContentLocation);

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
};
