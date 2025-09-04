/*=============================================================================
    BeamDebugDraw.h: Debug drawing utilities for Beam Eye Tracker.

    Provides helper functions for visualizing gaze rays, head pose, and
    debug overlays with consistent formatting and color schemes.

    Copyright (c) 2025 Eyeware Tech SA. All Rights Reserved.

    Eyeware® and Beam® are registered trademarks of Eyeware Tech SA.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "BeamEyeTrackerTypes.h"

#if BEAM_FEATURE_DEBUG_OVERLAY

class FCanvas;
class UFont;

/** Helper functions to visualize gaze rays, head pose, and debug overlays */
class BEAMEYETRACKER_API FBeamDebugDraw
{
public:
    	/** Draw a gaze crosshair at the specified position */
    static void DrawGazeCrosshair(FCanvas* Canvas, const FVector2D& Position, float Size, 
        const FLinearColor& Color, float Thickness = 2.0f);
    
    	/** Draw a compact status panel showing system health and tracking information */
    static void DrawStatusPanel(FCanvas* Canvas, const FVector2D& Anchor01, const FVector2D& ViewportSize,
        EBeamHealth Health, float FPS, int32 PollHz, const FString& Profile, 
        const FString& Source, const FString& SDKVersion, bool bAppRunning);
    
    /** Draw a gaze trail showing recent gaze point history */
    static void DrawTrail(FCanvas* Canvas, const TArray<FVector2D>& TrailPoints,
        const FLinearColor& Color, float Thickness = 1.0f, int32 MaxPoints = 256);
    
    /** Get color for health status display */
    static FLinearColor GetHealthColor(EBeamHealth Health);
    
    /** Get text for health status display */
    static FString GetHealthText(EBeamHealth Health);
    
    /** Get the default font for debug text rendering */
    static UFont* GetDebugFont();
    
private:
    /** Draw a simple line segment */
    static void DrawLine(FCanvas* Canvas, const FVector2D& Start, const FVector2D& End,
        const FLinearColor& Color, float Thickness);
    
    /** Draw text with consistent formatting */
    static void DrawText(FCanvas* Canvas, const FVector2D& Position, const FString& Text,
        const FLinearColor& Color, float Scale = 1.0f);
};

#endif // BEAM_FEATURE_DEBUG_OVERLAY

/*=============================================================================
    End of BeamDebugDraw.h
=============================================================================*/
