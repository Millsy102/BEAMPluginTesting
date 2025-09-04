/*=============================================================================
    BeamEyeTrackerEditorStyle.h: Editor styling for Beam Eye Tracker.

    Provides custom Slate styling and visual customization for the
    editor interface including icons, colors, and layout preferences.

    Copyright (c) 2025 Eyeware Tech SA. All Rights Reserved.

    Eyeware® and Beam® are registered trademarks of Eyeware Tech SA.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateStyle.h"

class FBeamEyeTrackerEditorStyle
{
public:
	static void Initialize();
	static void Shutdown();
	static void ReloadTextures();
	static const ISlateStyle& Get();
	static FName GetStyleSetName();

private:
	static TSharedRef<class FSlateStyleSet> Create();
	static TSharedPtr<class FSlateStyleSet> StyleInstance;
};

/*=============================================================================
    End of BeamEyeTrackerEditorStyle.h
=============================================================================*/
