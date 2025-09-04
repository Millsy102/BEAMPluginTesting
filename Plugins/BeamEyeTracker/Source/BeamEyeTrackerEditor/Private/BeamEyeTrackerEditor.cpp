/*=============================================================================
    BeamEyeTrackerEditor.cpp: Editor module implementation for Beam Eye Tracker.

    Implements editor integration including toolbar buttons, menu entries,
    project settings, and monitor tabs for comprehensive development support.

    Copyright (c) 2025 Eyeware Tech SA. All Rights Reserved.

    Eyeware® and Beam® are registered trademarks of Eyeware Tech SA.
=============================================================================*/

#include "BeamEyeTrackerEditor.h"
#include "BeamEyeTrackerEditorStyle.h"
#include "BeamEyeTrackerEditorCommands.h"
#include "BeamEyeTrackerSettings.h"
#include "BeamEyeTrackerSubsystem.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "Framework/MultiBox/MultiBoxDefs.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Docking/TabManager.h"
#include "Styling/AppStyle.h"
#include "Editor/WorkspaceMenuStructure/Public/WorkspaceMenuStructure.h"
#include "Editor/WorkspaceMenuStructure/Public/WorkspaceMenuStructureModule.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Text/STextBlock.h"
#include "PropertyEditorModule.h"
#include "ISettingsModule.h"
#include "Styling/AppStyle.h"
#include "ComponentCustomization.h"
// MonitorTab.h removed - using UMG widget approach instead
#include "BeamK2Nodes.h"

#define LOCTEXT_NAMESPACE "FBeamEyeTrackerEditorModule"

// Define the static member
const FName FBeamEyeTrackerEditorModule::BeamEyeTrackerMonitorTabName(TEXT("BeamEyeTrackerMonitor"));

void FBeamEyeTrackerEditorModule::StartupModule()
{
	// Register the editor style
	FBeamEyeTrackerEditorStyle::Initialize();
	FBeamEyeTrackerEditorStyle::ReloadTextures();

	// Register the editor commands
	FBeamEyeTrackerEditorCommands::Register();
	
	// Register the editor commands with the toolbar
	PluginCommands = MakeShareable(new FUICommandList);
	FBeamEyeTrackerEditorCommands::Get().MapAction(PluginCommands.ToSharedRef());

	// Register the toolbar extension
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	
	// Add toolbar button
	TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender());
	ToolbarExtender->AddToolBarExtension(
		"Settings",
		EExtensionHook::After,
		PluginCommands,
		FToolBarExtensionDelegate::CreateRaw(this, &FBeamEyeTrackerEditorModule::AddToolbarButton)
	);
	LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);

	// Add menu item
	TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
	MenuExtender->AddMenuExtension(
		"LevelEditor",
		EExtensionHook::After,
		PluginCommands,
		FMenuExtensionDelegate::CreateRaw(this, &FBeamEyeTrackerEditorModule::AddMenuEntry)
	);
	LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);

	// Register project settings
	RegisterProjectSettings();

	// Register the monitor tab spawner
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		BeamEyeTrackerMonitorTabName,
		FOnSpawnTab::CreateRaw(this, &FBeamEyeTrackerEditorModule::OnSpawnMonitorTab)
	)
	.SetDisplayName(LOCTEXT("BeamEyeTrackerMonitorTab", "Beam Eye Tracker Monitor"))
	.SetTooltipText(LOCTEXT("BeamEyeTrackerMonitorTabTooltip", "Open the Beam Eye Tracker Monitor"))
	.SetGroup(WorkspaceMenu::GetMenuStructure().GetDeveloperToolsMiscCategory())
	.SetIcon(FSlateIcon(FBeamEyeTrackerEditorStyle::GetStyleSetName(), "BeamEyeTrackerEditor.MonitorIcon"));

	// Register component customization
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomClassLayout(
		"BeamEyeTrackerComponent",
		FOnGetDetailCustomizationInstance::CreateStatic(&FBeamEyeTrackerComponentCustomization::MakeInstance)
	);

	// Register K2 nodes for Blueprint support
	UBeamK2Nodes::RegisterNodes();
}

void FBeamEyeTrackerEditorModule::ShutdownModule()
{
	// Unregister the editor style
	FBeamEyeTrackerEditorStyle::Shutdown();

	// Unregister the editor commands
	FBeamEyeTrackerEditorCommands::Unregister();

	// Unregister the monitor tab spawner
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(BeamEyeTrackerMonitorTabName);

	// Unregister component customization
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.UnregisterCustomClassLayout("BeamEyeTrackerComponent");
	}

	// Unregister K2 nodes
	UBeamK2Nodes::UnregisterNodes();
}

void FBeamEyeTrackerEditorModule::AddToolbarButton(FToolBarBuilder& Builder)
{
	Builder.AddToolBarButton(
		FBeamEyeTrackerEditorCommands::Get().OpenMonitor,
		NAME_None,
		LOCTEXT("BeamEyeTrackerMonitor", "Beam Monitor"),
		LOCTEXT("BeamEyeTrackerMonitorTooltip", "Open Beam Eye Tracker Monitor"),
		FSlateIcon(FBeamEyeTrackerEditorStyle::GetStyleSetName(), "BeamEyeTrackerEditor.MonitorIcon")
	);

	Builder.AddSeparator();

	Builder.AddToolBarButton(
		FBeamEyeTrackerEditorCommands::Get().StartTracking,
		NAME_None,
		LOCTEXT("StartTracking", "Start"),
		LOCTEXT("StartTrackingTooltip", "Start Beam Eye Tracking"),
		FSlateIcon(FBeamEyeTrackerEditorStyle::GetStyleSetName(), "BeamEyeTrackerEditor.StartIcon")
	);

	Builder.AddToolBarButton(
		FBeamEyeTrackerEditorCommands::Get().StopTracking,
		NAME_None,
		LOCTEXT("StopTracking", "Stop"),
		LOCTEXT("StopTrackingTooltip", "Stop Beam Eye Tracking"),
		FSlateIcon(FBeamEyeTrackerEditorStyle::GetStyleSetName(), "BeamEyeTrackerEditor.StopIcon")
	);

	Builder.AddSeparator();

	Builder.AddToolBarButton(
		FBeamEyeTrackerEditorCommands::Get().OpenProjectSettings,
		NAME_None,
		LOCTEXT("ProjectSettings", "Settings"),
		LOCTEXT("ProjectSettingsTooltip", "Open Beam Eye Tracker Project Settings"),
		FSlateIcon(FBeamEyeTrackerEditorStyle::GetStyleSetName(), "BeamEyeTrackerEditor.SettingsIcon")
	);
}

void FBeamEyeTrackerEditorModule::AddMenuEntry(FMenuBuilder& Builder)
{
	Builder.BeginSection("BeamEyeTracker", LOCTEXT("BeamEyeTracker", "Beam Eye Tracker"));
	{
		Builder.AddMenuEntry(FBeamEyeTrackerEditorCommands::Get().OpenMonitor);
		Builder.AddMenuEntry(FBeamEyeTrackerEditorCommands::Get().StartTracking);
		Builder.AddMenuEntry(FBeamEyeTrackerEditorCommands::Get().StopTracking);
		Builder.AddSeparator();
		Builder.AddMenuEntry(FBeamEyeTrackerEditorCommands::Get().OpenProjectSettings);
	}
	Builder.EndSection();
}

void FBeamEyeTrackerEditorModule::RegisterProjectSettings()
{
	// Register the project settings
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings("Project", "Plugins", "Beam Eye Tracker",
			LOCTEXT("BeamEyeTrackerSettingsName", "Beam Eye Tracker"),
			LOCTEXT("BeamEyeTrackerSettingsDescription", "Configure Beam Eye Tracker plugin settings"),
			GetMutableDefault<UBeamEyeTrackerSettings>()
		);
	}
}

TSharedRef<SDockTab> FBeamEyeTrackerEditorModule::OnSpawnMonitorTab(const FSpawnTabArgs& SpawnTabArgs)
{
	// Note: The old Slate widget has been replaced with UMG widget
	// This tab spawner is kept for backward compatibility but now shows a placeholder
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("MonitorTabPlaceholder", "Beam Eye Tracker Monitor\n\nThis tab has been replaced with the UMG-based monitor widget.\nUse the widget in your game or create a Widget Blueprint based on UBeamEyeTrackerMonitorWidget."))
			.Justification(ETextJustify::Center)
		];
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FBeamEyeTrackerEditorModule, BeamEyeTrackerEditor)

/*=============================================================================
    End of BeamEyeTrackerEditor.cpp
=============================================================================*/
