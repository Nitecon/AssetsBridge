// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "AssetsBridge.generated.h"

class FToolBarBuilder;
class FMenuBuilder;
class AStaticMeshActor;

UENUM(BlueprintType)
enum class EBridgeType : uint8 {
	Unknown = 0 UMETA(DisplayName = "Unsupported"), 
	StaticMesh = 1 UMETA(DisplayName = "Static Mesh"),
	SkeletalMesh = 2  UMETA(DisplayName = "Skeletal Mesh"),
};

USTRUCT(BlueprintType)
struct FBridgeAssets
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EBridgeType AssetType = EBridgeType::Unknown;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UStaticMesh* StaticMesh = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class USkeletalMesh* SkeletalMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString InternalPath = "";
	
};

class FAssetsBridgeModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	/** This function will be bound to Command (by default it will bring up plugin window) */
	void SwapButtonClicked();
	void ExportButtonClicked();
	void ImportButtonClicked();
	void OpenSettingsMenu();
	
	FReply SaveAssetsLocation();
	
private:

	void RegisterMenus();

	//Used to load our userWidget blueprint (which is a slate wrapper)
	void LoadUserWidget();
	TArray<FBridgeAssets> GetSelectedUserContext();

	TArray<AStaticMeshActor *> GetSelectedStaticMeshes();
	

	class UUserWidget* CreatedWidget = nullptr;
	
	TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs);

private:
	TSharedPtr<class FUICommandList> PluginCommands;
};
