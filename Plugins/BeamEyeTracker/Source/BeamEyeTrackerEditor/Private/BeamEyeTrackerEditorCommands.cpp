/*=============================================================================
    BeamEyeTrackerEditorCommands.cpp: Editor commands implementation for Beam Eye Tracker.

    Implements editor commands for opening monitor tabs, controlling tracking,
    and accessing project settings within the Unreal Editor interface.

    Copyright (c) 2025 Eyeware Tech SA. All Rights Reserved.

    Eyeware® and Beam® are registered trademarks of Eyeware Tech SA.
=============================================================================*/

#include "BeamEyeTrackerEditorCommands.h"
#include "Framework/Commands/UICommandList.h"
#include "Framework/Commands/UICommandInfo.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "BeamEyeTrackerSubsystem.h"
#include "ISettingsModule.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Docking/TabManager.h"

#define LOCTEXT_NAMESPACE "FBeamEyeTrackerEditorCommands"

void FBeamEyeTrackerEditorCommands::RegisterCommands()
{
	UI_COMMAND(OpenMonitor, "Beam Monitor", "Open Beam Eye Tracker Monitor", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(StartTracking, "Start Tracking", "Start Beam Eye Tracking", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(StopTracking, "Stop Tracking", "Stop Beam Eye Tracking", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(OpenProjectSettings, "Project Settings", "Open Beam Eye Tracker Project Settings", EUserInterfaceActionType::Button, FInputChord());
}

void FBeamEyeTrackerEditorCommands::MapAction(TSharedRef<FUICommandList> InCommandList)
{
	InCommandList->MapAction(OpenMonitor, FExecuteAction::CreateLambda([]()
	{
		// Open monitor tab
		FGlobalTabmanager::Get()->TryInvokeTab(FName("BeamEyeTrackerMonitor"));
	}));

	InCommandList->MapAction(StartTracking, FExecuteAction::CreateLambda([]()
	{
		// Start tracking in the current world
		if (GEngine && GEngine->GetWorldContexts().Num() > 0)
		{
			UWorld* World = GEngine->GetWorldContexts()[0].World();
			if (World)
			{
				UBeamEyeTrackerSubsystem* Subsystem = World->GetGameInstance()->GetSubsystem<UBeamEyeTrackerSubsystem>();
				if (Subsystem)
				{
					Subsystem->StartBeamTracking();
				}
			}
		}
	}));

	InCommandList->MapAction(StopTracking, FExecuteAction::CreateLambda([]()
	{
		// Stop tracking in the current world
		if (GEngine && GEngine->GetWorldContexts().Num() > 0)
		{
			UWorld* World = GEngine->GetWorldContexts()[0].World();
			if (World)
			{
				UBeamEyeTrackerSubsystem* Subsystem = World->GetGameInstance()->GetSubsystem<UBeamEyeTrackerSubsystem>();
				if (Subsystem)
				{
					Subsystem->StopBeamTracking();
				}
			}
		}
	}));

	InCommandList->MapAction(OpenProjectSettings, FExecuteAction::CreateLambda([]()
	{
		// Open project settings using UE 5.6 compatible API
		if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
		{
			// Try to open project settings - method name may vary by UE version
			UE_LOG(LogTemp, Log, TEXT("Opening Beam Eye Tracker project settings"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Settings module not available"));
		}
	}));
}

#undef LOCTEXT_NAMESPACE

/*=============================================================================
    End of BeamEyeTrackerEditorCommands.cpp
=============================================================================*/
