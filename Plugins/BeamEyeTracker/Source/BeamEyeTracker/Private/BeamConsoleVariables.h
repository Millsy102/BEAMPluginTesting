/*=============================================================================
    BeamConsoleVariables.h: Console variables for Beam SDK debugging.

    Defines console commands and cvars for runtime tuning of tracking
    parameters, debug visualization, and troubleshooting capabilities.
    All variables are prefixed with "beam." for easy identification.

    Copyright (c) 2025 Eyeware Tech SA. All Rights Reserved.

    Eyeware® and Beam® are registered trademarks of Eyeware Tech SA.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"

/** Defines console commands and cvars for debugging Beam Eye Tracker functionality */
class BEAMEYETRACKER_API FBeamConsoleVariables
{
public:
    /** Sync console variables with project settings on startup */
    static void SyncWithProjectSettings();
};

/*=============================================================================
    End of BeamConsoleVariables.h
=============================================================================*/
