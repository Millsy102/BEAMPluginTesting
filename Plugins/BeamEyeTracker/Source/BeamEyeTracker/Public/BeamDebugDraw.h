/*=============================================================================
    BeamDebugDraw.h: Debug drawing utilities for Beam Eye Tracker.

    Provides comprehensive debug visualization including status panels,
    gaze crosshairs, gaze trails, and 3D ray visualization for
    development and testing purposes.

    Copyright (c) 2025 Eyeware Tech SA. All Rights Reserved.

    Eyeware® and Beam® are registered trademarks of Eyeware Tech SA.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "BeamEyeTrackerTypes.h"

// Forward declarations
class UCanvas;
class UBeamEyeTrackerSubsystem;

// Debug Drawing Class

class BEAMEYETRACKER_API FBeamDebugDraw
{
public:
	// Main debug HUD drawing function
	static void DrawDebugHUD(UCanvas* Canvas, UBeamEyeTrackerSubsystem* Subsystem);

private:
	// Individual drawing functions
	static void DrawStatusPanel(UCanvas* Canvas, UBeamEyeTrackerSubsystem* Subsystem, const FVector2D& ViewportSize);
	static void DrawGazeCrosshair(UCanvas* Canvas, UBeamEyeTrackerSubsystem* Subsystem, const FVector2D& ViewportSize);
	static void DrawGazeTrail(UCanvas* Canvas, UBeamEyeTrackerSubsystem* Subsystem, const FVector2D& ViewportSize);
	static void DrawGazeRay(UCanvas* Canvas, UBeamEyeTrackerSubsystem* Subsystem, const FVector2D& ViewportSize);

	// Utility functions
	static FString GetHealthStatusString(EBeamHealth Health);
	static FLinearColor GetHealthColor(EBeamHealth Health);
};

/*=============================================================================
    End of BeamDebugDraw.h
=============================================================================*/

