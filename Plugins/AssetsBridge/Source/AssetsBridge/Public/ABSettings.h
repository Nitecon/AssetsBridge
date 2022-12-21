// Copyright 2023 Nitecon Studios LLC. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ABSettings.generated.h"

/**
 * 
 */
UCLASS(config = AssetsBridge)
class ASSETSBRIDGE_API UABSettings : public UObject
{
	GENERATED_BODY()
public:
	UABSettings(const FObjectInitializer& obj);

	UPROPERTY(Config, EditAnywhere, Category = "ContentFolder")
	FString UnrealContentLocation;

	UPROPERTY(Config, EditAnywhere, Category = "AssetsLocation")
	FString AssetLocationOnDisk;
	
	UPROPERTY(Config, EditAnywhere, Category = "AssetBridgeCacheLocation")
	FString AssetBridgeCacheLocation;

};
