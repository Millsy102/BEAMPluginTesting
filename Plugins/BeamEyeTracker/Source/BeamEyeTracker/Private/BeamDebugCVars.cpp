// Registers Beam console variables (CVars)

#include "BeamDebugCVars.h"
#include "HAL/IConsoleManager.h"

// CVar Definitions

TAutoConsoleVariable<bool> FBeamDebugCVars::CVarDebugHUD(
	TEXT("beam.debug.hud"),
	true,
	TEXT("Enable/disable the Beam debug HUD overlay"),
	ECVF_Default
);

// Individual drawing toggles
TAutoConsoleVariable<bool> FBeamDebugCVars::CVarDrawText(
	TEXT("beam.debug.text"),
	true,
	TEXT("Enable/disable text status panel in debug HUD"),
	ECVF_Default
);

TAutoConsoleVariable<bool> FBeamDebugCVars::CVarDrawGaze(
	TEXT("beam.debug.gaze"),
	true,
	TEXT("Enable/disable gaze crosshair in debug HUD"),
	ECVF_Default
);

TAutoConsoleVariable<bool> FBeamDebugCVars::CVarDrawTrail(
	TEXT("beam.debug.trail"),
	true,
	TEXT("Enable/disable gaze trail in debug HUD"),
	ECVF_Default
);

TAutoConsoleVariable<bool> FBeamDebugCVars::CVarDrawRay(
	TEXT("beam.debug.ray"),
	false,
	TEXT("Enable/disable 3D gaze ray visualization"),
	ECVF_Default
);

// Position and layout
TAutoConsoleVariable<float> FBeamDebugCVars::CVarAnchorX(
	TEXT("beam.debug.anchor.x"),
	0.8f,
	TEXT("X anchor position for debug HUD (0.0 to 1.0)"),
	ECVF_Default
);

TAutoConsoleVariable<float> FBeamDebugCVars::CVarAnchorY(
	TEXT("beam.debug.anchor.y"),
	0.1f,
	TEXT("Y anchor position for debug HUD (0.0 to 1.0)"),
	ECVF_Default
);

// Trail settings
TAutoConsoleVariable<int32> FBeamDebugCVars::CVarSampleWindow(
	TEXT("beam.debug.trail.samples"),
	30,
	TEXT("Number of samples to use for gaze trail"),
	ECVF_Default
);

TAutoConsoleVariable<float> FBeamDebugCVars::CVarTrailLength(
	TEXT("beam.debug.trail.length"),
	100.0f,
	TEXT("Length of gaze trail in pixels"),
	ECVF_Default
);

TAutoConsoleVariable<float> FBeamDebugCVars::CVarTrailWidth(
	TEXT("beam.debug.trail.width"),
	50.0f,
	TEXT("Width of gaze trail in pixels"),
	ECVF_Default
);

// Crosshair settings
TAutoConsoleVariable<float> FBeamDebugCVars::CVarCrosshairSize(
	TEXT("beam.debug.crosshair.size"),
	20.0f,
	TEXT("Size of gaze crosshair in pixels"),
	ECVF_Default
);

TAutoConsoleVariable<float> FBeamDebugCVars::CVarCrosshairThickness(
	TEXT("beam.debug.crosshair.thickness"),
	2.0f,
	TEXT("Thickness of gaze crosshair lines"),
	ECVF_Default
);

// Performance settings
TAutoConsoleVariable<float> FBeamDebugCVars::CVarUpdateInterval(
	TEXT("beam.debug.update.interval"),
	0.1f,
	TEXT("Update interval for debug HUD in seconds"),
	ECVF_Default
);

TAutoConsoleVariable<bool> FBeamDebugCVars::CVarEnableProfiling(
	TEXT("beam.debug.profiling"),
	false,
	TEXT("Enable/disable debug HUD performance profiling"),
	ECVF_Default
);

// Utility Functions

bool FBeamDebugCVars::IsDebugHUDEnabled()
{
	return CVarDebugHUD.GetValueOnGameThread() != 0;
}

bool FBeamDebugCVars::IsDrawGazeEnabled()
{
	return CVarDrawGaze.GetValueOnGameThread() != 0;
}

bool FBeamDebugCVars::IsDrawRayEnabled()
{
	return CVarDrawRay.GetValueOnGameThread() != 0;
}

bool FBeamDebugCVars::IsDrawTextEnabled()
{
	return CVarDrawText.GetValueOnGameThread() != 0;
}

bool FBeamDebugCVars::IsDrawTrailEnabled()
{
	return CVarDrawTrail.GetValueOnGameThread() != 0;
}

int32 FBeamDebugCVars::GetSampleWindow()
{
	return CVarSampleWindow.GetValueOnGameThread();
}

FVector2D FBeamDebugCVars::GetAnchorPosition()
{
	return FVector2D(CVarAnchorX.GetValueOnGameThread(), CVarAnchorY.GetValueOnGameThread());
}

