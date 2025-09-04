/*=============================================================================
    Commands.cpp: Editor commands implementation for Beam Eye Tracker.

    Implements editor commands for calibration, recentering, overlay toggling,
    and monitor tab access within the Unreal Editor interface.

    Copyright (c) 2025 Eyeware Tech SA. All Rights Reserved.

    Eyeware® and Beam® are registered trademarks of Eyeware Tech SA.
=============================================================================*/

#include "Commands.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "FBeamCommands"

void FBeamCommands::RegisterCommands()
{
	UI_COMMAND(Calibrate, "Calibrate", "Start eye tracking calibration", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(Recenter, "Recenter", "Start camera recentering", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(ToggleOverlay, "Toggle Overlay", "Toggle debug overlay visibility", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(OpenMonitorTab, "Open Monitor", "Open the Beam monitoring tab", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE

/*=============================================================================
    End of Commands.cpp
=============================================================================*/
