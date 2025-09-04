/*=============================================================================
    BeamEyeTrackerEditor.h: Editor module for Beam Eye Tracker.

    Provides editor integration including toolbar buttons, menu entries,
    project settings, and monitor tabs for comprehensive development support.

    Copyright (c) 2025 Eyeware Tech SA. All Rights Reserved.

    Eyeware® and Beam® are registered trademarks of Eyeware Tech SA.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/MultiBox/MultiBoxDefs.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Framework/Commands/UICommandList.h"
#include "BeamEyeTrackerSettings.h"

class FBeamEyeTrackerEditorStyle;
class FBeamEyeTrackerEditorCommands;
// Removed Slate widget reference - now using UMG widget approach
class FBeamEyeTrackerComponentCustomization;

class BEAMEYETRACKEREDITOR_API FBeamEyeTrackerEditorModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/** This function will be bound to Command. */
	void PluginButtonClicked();
	
	/** This function will be bound to Command. */
	void OpenProjectSettings();

private:
	void RegisterProjectSettings();
	void AddToolbarButton(FToolBarBuilder& Builder);
	void AddMenuEntry(FMenuBuilder& Builder);
	TSharedRef<SDockTab> OnSpawnMonitorTab(const FSpawnTabArgs& SpawnTabArgs);

private:
	TSharedPtr<FBeamEyeTrackerEditorStyle> StyleSet;
	TSharedPtr<FUICommandList> PluginCommands;
	
	/** The name of the monitor tab */
	static const FName BeamEyeTrackerMonitorTabName;
};

/*=============================================================================
    End of BeamEyeTrackerEditor.h
=============================================================================*/
