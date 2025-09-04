/*=============================================================================
    Commands.h: Editor commands for Beam Eye Tracker.

    Defines editor commands for calibration, recentering, overlay toggling,
    and monitor tab access within the Unreal Editor interface.

    Copyright (c) 2025 Eyeware Tech SA. All Rights Reserved.

    Eyeware® and Beam® are registered trademarks of Eyeware Tech SA.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"

/**
 * Defines commands for the Beam Eye Tracker editor integration
 */
class FBeamCommands : public TCommands<FBeamCommands>
{
public:
	FBeamCommands()
		: TCommands<FBeamCommands>(
			TEXT("BeamEditor"), // Context name for fast lookup
			NSLOCTEXT("BeamEditor", "BeamEditorCommands", "Beam Editor Commands"), // Localized context name for displaying
			NAME_None, // No parent context
			FAppStyle::GetAppStyleSetName() // Icon Style Set - Updated for UE 5.6
		)
	{
	}

	// TCommand<> interface
	virtual void RegisterCommands() override;

	// Beam Commands
	TSharedPtr<FUICommandInfo> Calibrate;
	TSharedPtr<FUICommandInfo> Recenter;
	TSharedPtr<FUICommandInfo> ToggleOverlay;
	TSharedPtr<FUICommandInfo> OpenMonitorTab;
};

/*=============================================================================
    End of Commands.h
=============================================================================*/
