/*=============================================================================
    BeamEyeTracker.h: Declares the Beam Eye Tracker plugin module.

    Provides the core module interface for the Beam Eye Tracker plugin,
    handling module lifecycle management and initialization.

    Copyright (c) 2025 Eyeware Tech SA. All Rights Reserved.

    Eyeware® and Beam® are registered trademarks of Eyeware Tech SA.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * Core plugin module for Beam Eye Tracker.
 * 
 * Handles module lifecycle management including startup and shutdown
 * operations for the Beam Eye Tracker plugin system.
 */
class BEAMEYETRACKER_API FBeamEyeTrackerModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

/*=============================================================================
    End of BeamEyeTracker.h
============================================================================*/
