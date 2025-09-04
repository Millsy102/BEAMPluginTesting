/*=============================================================================
    BeamDebugCVars.h: Console variables for Beam SDK debugging.

    Declares console variables for comprehensive debugging control including
    HUD toggles, drawing options, position settings, and performance controls.

    Copyright (c) 2025 Eyeware Tech SA. All Rights Reserved.

    Eyeware® and Beam® are registered trademarks of Eyeware Tech SA.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "HAL/Platform.h"

// Debug Console Variables

class FBeamDebugCVars
{
public:
	// Main debug HUD toggle
	static TAutoConsoleVariable<bool> CVarDebugHUD;
	
	// Individual drawing toggles
	static TAutoConsoleVariable<bool> CVarDrawText;
	static TAutoConsoleVariable<bool> CVarDrawGaze;
	static TAutoConsoleVariable<bool> CVarDrawTrail;
	static TAutoConsoleVariable<bool> CVarDrawRay;
	
	// Position and layout
	static TAutoConsoleVariable<float> CVarAnchorX;
	static TAutoConsoleVariable<float> CVarAnchorY;
	
	// Trail settings
	static TAutoConsoleVariable<int32> CVarSampleWindow;
	static TAutoConsoleVariable<float> CVarTrailLength;
	static TAutoConsoleVariable<float> CVarTrailWidth;
	
	// Crosshair settings
	static TAutoConsoleVariable<float> CVarCrosshairSize;
	static TAutoConsoleVariable<float> CVarCrosshairThickness;
	
	// Performance settings
	static TAutoConsoleVariable<float> CVarUpdateInterval;
	static TAutoConsoleVariable<bool> CVarEnableProfiling;
	
	// Utility functions
	static bool IsDebugHUDEnabled();
	static bool IsDrawGazeEnabled();
	static bool IsDrawRayEnabled();
	static bool IsDrawTextEnabled();
	static bool IsDrawTrailEnabled();
	static int32 GetSampleWindow();
	static FVector2D GetAnchorPosition();
};

/*=============================================================================
    End of BeamDebugCVars.h
=============================================================================*/


