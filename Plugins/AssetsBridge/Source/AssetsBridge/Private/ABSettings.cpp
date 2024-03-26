// Copyright 2023 Nitecon Studios LLC. All rights reserved.


#include "ABSettings.h"

#include "ISettingsModule.h"

UABSettings::UABSettings(const FObjectInitializer& obj)
{
	UnrealContentLocation = TEXT("Content/Assets");
	AssetLocationOnDisk = TEXT("");
}
