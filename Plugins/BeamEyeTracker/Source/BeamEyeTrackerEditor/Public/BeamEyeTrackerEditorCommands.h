/*=============================================================================
    BeamEyeTrackerEditorCommands.h: Editor commands for Beam Eye Tracker.

    Defines editor commands for opening monitor tabs, controlling tracking,
    and accessing project settings within the Unreal Editor interface.

    Copyright (c) 2025 Eyeware Tech SA. All Rights Reserved.

    Eyeware® and Beam® are registered trademarks of Eyeware Tech SA.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"

class FBeamEyeTrackerEditorCommands : public TCommands<FBeamEyeTrackerEditorCommands>
{
public:
	FBeamEyeTrackerEditorCommands()
		: TCommands<FBeamEyeTrackerEditorCommands>(
			TEXT("BeamEyeTrackerEditor"), // Context name for fast lookup
			NSLOCTEXT("BeamEyeTrackerEditor", "BeamEyeTrackerEditorCommands", "Beam Eye Tracker Editor Commands"), // Localized context name for displaying
			NAME_None, // No parent context
			FAppStyle::GetAppStyleSetName() // Icon Style Set
			)
	{
	}

	// TCommand<> interface
	virtual void RegisterCommands() override;

	/** Maps the commands to the UI command list */
	void MapAction(TSharedRef<FUICommandList> InCommandList);

public:
	TSharedPtr<FUICommandInfo> OpenMonitor;
	TSharedPtr<FUICommandInfo> StartTracking;
	TSharedPtr<FUICommandInfo> StopTracking;
	TSharedPtr<FUICommandInfo> OpenProjectSettings;
};

/*=============================================================================
    End of BeamEyeTrackerEditorCommands.h
=============================================================================*/
