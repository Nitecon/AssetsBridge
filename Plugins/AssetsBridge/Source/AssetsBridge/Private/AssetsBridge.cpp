// Copyright Epic Games, Inc. All Rights Reserved.

#include "AssetsBridge.h"

#include "ABSettings.h"
#include "AssetsBridgeStyle.h"
#include "AssetsBridgeCommands.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"
#include "ISettingsModule.h"
#include "Selection.h"
#include "Blueprint/UserWidget.h"
#include "Engine/StaticMeshActor.h"
#include "UnrealEd/Private/FbxExporter.h"


static const FName AssetsBridgeTabName("Assets Bridge Configuration");

#define LOCTEXT_NAMESPACE "FAssetsBridgeModule"

void FAssetsBridgeModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FAssetsBridgeStyle::Initialize();
	FAssetsBridgeStyle::ReloadTextures();

	FAssetsBridgeCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FAssetsBridgeCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FAssetsBridgeModule::OpenSettingsMenu),
		FCanExecuteAction());
	
	PluginCommands->MapAction(
		FAssetsBridgeCommands::Get().ContentSwapAction,
		FExecuteAction::CreateRaw(this, &FAssetsBridgeModule::SwapButtonClicked),
		FCanExecuteAction());
	
	PluginCommands->MapAction(
		FAssetsBridgeCommands::Get().ContentExportAction,
		FExecuteAction::CreateRaw(this, &FAssetsBridgeModule::ExportButtonClicked),
		FCanExecuteAction());
	
	PluginCommands->MapAction(
		FAssetsBridgeCommands::Get().ContentImportAction,
		FExecuteAction::CreateRaw(this, &FAssetsBridgeModule::ImportButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FAssetsBridgeModule::RegisterMenus));
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(AssetsBridgeTabName, FOnSpawnTab::CreateRaw(this, &FAssetsBridgeModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FAssetsBridgeTabTitle", "AssetsBridge"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);

	if(ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings("Project", "Plugins", "AssetsBridge",
			LOCTEXT("RuntimeSettingsName", "Assets Bridge Settings"), LOCTEXT("RuntimeSettingsDescription", "Setup path locations for assets bridge"),
			GetMutableDefault<UABSettings>());
	}
}

void FAssetsBridgeModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	if(ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "AssetsBridge");
	}

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FAssetsBridgeStyle::Shutdown();

	FAssetsBridgeCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(AssetsBridgeTabName);
}

FReply FAssetsBridgeModule::SaveAssetsLocation()
{
	return FReply::Handled();
}

TSharedRef<SDockTab> FAssetsBridgeModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	//Load our user widget from blueprint.
	LoadUserWidget();

	FText WidgetText = FText::Format(
		LOCTEXT("WindowWidgetText", "Add code to {0} in {1} to override this window's contents"),
		FText::FromString(TEXT("FWrapperHelperTempateModule::OnSpawnPluginTab")),
		FText::FromString(TEXT("WrapperHelperTempate.cpp"))
		);

	//Declare our DockTab and assign it to the template
	TSharedRef<SDockTab> NewDockTab = SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			// Put your tab content here!
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(WidgetText)
			]
		];
	
	//If we have loaded our widget, assign it
	if (CreatedWidget)
	{
		//Log our widget's name
		UE_LOG(LogTemp, Warning, TEXT("Created Widget: %s"), *CreatedWidget->GetName())
		//Store our widget
		TSharedRef<SWidget> UserSlateWidget = CreatedWidget->TakeWidget();

		//Assign our widget to our tab's content
		NewDockTab->SetContent(UserSlateWidget);

	}

	//Return our tab
	return NewDockTab;
}

//This function does the magic of loading the entire widget from disk into an object.
//Once we've loaded it, we then create a widget and store it to replace our slate UI with.
void FAssetsBridgeModule::LoadUserWidget()
{

	UE_LOG(LogTemp, Warning, TEXT("Function: LoadUserWidget()"));

	//This path is contained within the WrapperHelperTemplate Content folder
	UClass* LoadedWidget = LoadObject<UClass>(NULL, TEXT("/AssetsBridge/BPW_Settings.BPW_Settings_C"), NULL, LOAD_None, NULL);
	if (LoadedWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("User Widget loaded successfully"));
		
		//If successfully loaded, store it for later use.
		CreatedWidget = CreateWidget(GEditor->GetEditorWorldContext().World(), LoadedWidget);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Unable to loacate BP Widget inside our plugin's content folder."));
	}
}

TArray<AStaticMeshActor *> FAssetsBridgeModule::GetSelectedStaticMeshes()
{
	//IAssetRegistry &AssetRegistry = FModuleManager::GetModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
	//FAssetData InstanceData = AssetRegistry.GetAssetByObjectPath(FSoftObjectPath(InstancePath));
	//UMaterialInstanceConstant *MaterialInstance = Cast<UMaterialInstanceConstant>(UEditorAssetLibrary::LoadAsset(InstancePath));

	USelection *SelectedActors = GEditor->GetSelectedActors();
	TArray<AActor *> Actors;
	TArray<AStaticMeshActor *> SelectedStaticMeshes;

	TArray<ULevel *> UniqueLevels;
	for (FSelectionIterator Iter(*SelectedActors); Iter; ++Iter)
	{
		AActor *Actor = Cast<AActor>(*Iter);
		TArray<UStaticMeshComponent *> Components;
		Actor->GetComponents(Components);

		if (Components.Num() > 0)
		{
			AStaticMeshActor *SelectedStaticMesh = Cast<AStaticMeshActor>(Actor);
			if (SelectedStaticMesh)
			{
				SelectedStaticMeshes.Add(SelectedStaticMesh);
			}
		}

		for (int32 i = 0; i < Components.Num(); i++)
		{
			UStaticMeshComponent *MeshComponent = Components[i];
			int32 mCnt = MeshComponent->GetNumMaterials();
			/*for (int j = 0; j < mCnt; j++)
			{

				MeshComponent->SetMaterial(j, MaterialInstance);
			}*/
		}
	}

	return SelectedStaticMeshes;
}

TArray<FBridgeAssets> FAssetsBridgeModule::GetSelectedUserContext()
{
	// Try to get static meshes first:
	TArray<FBridgeAssets> Result;
	auto smActors = GetSelectedStaticMeshes();
	if (smActors.Num() > 0)
	{
		for (auto sm : smActors)
		{
			FBridgeAssets Item;
			UStaticMeshComponent* comp = sm->GetStaticMeshComponent();
			if (comp != nullptr && comp->GetStaticMesh() != nullptr)
			{
				Item.StaticMesh =sm->GetStaticMeshComponent()->GetStaticMesh();
				Item.AssetType = EBridgeType::StaticMesh;
				Result.Add(Item);
			}
		}
	}
	return Result;
}

void FAssetsBridgeModule::SwapButtonClicked()
{
	// If content settings are not set show the tab else run the command
	//FGlobalTabmanager::Get()->TryInvokeTab(AssetsBridgeTabName);
	// Put your "OnButtonClicked" stuff here
	FString outContent;
	auto assets = GetSelectedUserContext();
	for (auto item : assets)
	{
		if (item.AssetType == EBridgeType::StaticMesh)
		{
			outContent.Append(item.StaticMesh->GetPathName());
		}
	}
	FText DialogText = FText::Format(
							LOCTEXT("PluginButtonDialogText", "Add code to {0} in {1} to override this button's actions, Selected asset: {2}"),
							FText::FromString(TEXT("FAssetsBridgeModule::PluginButtonClicked()")),
							FText::FromString(TEXT("AssetBridge.cpp")),
							FText::FromString(outContent)
					   );
	FMessageDialog::Open(EAppMsgType::Ok, DialogText);

	
	/*UnFbx::FFbxExporter* Exporter = UnFbx::FFbxExporter::GetInstance();
	bool bIsCanceled;
	bool bExportAll;
	Exporter->FillExportOptions(false, false, *FString(TEXT("")),bIsCanceled, bExportAll );*/
	
}

void FAssetsBridgeModule::ExportButtonClicked()
{
	// If content settings are not set show the tab else run the command
	//FGlobalTabmanager::Get()->TryInvokeTab(AssetsBridgeTabName);
	// Put your "OnButtonClicked" stuff here
	FString outContent;
	auto assets = GetSelectedUserContext();
	for (auto item : assets)
	{
		if (item.AssetType == EBridgeType::StaticMesh)
		{
			outContent.Append(item.StaticMesh->GetPathName());
		}
	}
	FText DialogText = FText::Format(
							LOCTEXT("PluginButtonDialogText", "Add code to {0} in {1} to override this button's actions, Selected asset: {2}"),
							FText::FromString(TEXT("FAssetsBridgeModule::PluginButtonClicked()")),
							FText::FromString(TEXT("AssetBridge.cpp")),
							FText::FromString(outContent)
					   );
	FMessageDialog::Open(EAppMsgType::Ok, DialogText);

	
	UnFbx::FFbxExporter* Exporter = UnFbx::FFbxExporter::GetInstance();
	bool bIsCanceled;
	bool bExportAll;
	Exporter->FillExportOptions(false, false, *FString(TEXT("")),bIsCanceled, bExportAll );
	
}

void FAssetsBridgeModule::ImportButtonClicked()
{
	// If content settings are not set show the tab else run the command
	//FGlobalTabmanager::Get()->TryInvokeTab(AssetsBridgeTabName);
	// Put your "OnButtonClicked" stuff here
	FString outContent;
	auto assets = GetSelectedUserContext();
	for (auto item : assets)
	{
		if (item.AssetType == EBridgeType::StaticMesh)
		{
			outContent.Append(item.StaticMesh->GetPathName());
		}
	}
	FText DialogText = FText::Format(
							LOCTEXT("PluginButtonDialogText", "Add code to {0} in {1} to override this button's actions, Selected asset: {2}"),
							FText::FromString(TEXT("FAssetsBridgeModule::PluginButtonClicked()")),
							FText::FromString(TEXT("AssetBridge.cpp")),
							FText::FromString(outContent)
					   );
	FMessageDialog::Open(EAppMsgType::Ok, DialogText);

	
	UnFbx::FFbxExporter* Exporter = UnFbx::FFbxExporter::GetInstance();
	bool bIsCanceled;
	bool bExportAll;
	Exporter->FillExportOptions(false, false, *FString(TEXT("")),bIsCanceled, bExportAll );
	
}

void FAssetsBridgeModule::OpenSettingsMenu()
{
	// If content settings are not set show the tab else run the command
	FGlobalTabmanager::Get()->TryInvokeTab(AssetsBridgeTabName);
	
}

void FAssetsBridgeModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FAssetsBridgeCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FAssetsBridgeCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
	/*{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FAssetsBridgeCommands::Get().ContentSwapAction, PluginCommands);
		}
	}*/

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
			{
				FToolMenuEntry& SwapEntry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FAssetsBridgeCommands::Get().ContentSwapAction));
				SwapEntry.SetCommandList(PluginCommands);
				FToolMenuEntry& ExportEntry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FAssetsBridgeCommands::Get().ContentExportAction));
				ExportEntry.SetCommandList(PluginCommands);
				FToolMenuEntry& ImportEntry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FAssetsBridgeCommands::Get().ContentImportAction));
				ImportEntry.SetCommandList(PluginCommands);
			}
		}
	}
}

FText FAssetsBridgeModule::GetAssetsLocation() const
{
	if(ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		UABSettings* Settings = GetMutableDefault<UABSettings>();
		if (Settings != nullptr)
		{
			return FText::FromString(Settings->UnrealContentLocation);
		}
	}
	return FText::FromString("Unknown");
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FAssetsBridgeModule, AssetsBridge)