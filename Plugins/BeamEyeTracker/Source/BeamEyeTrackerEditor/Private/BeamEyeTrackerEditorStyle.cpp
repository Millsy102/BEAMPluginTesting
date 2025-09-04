/*=============================================================================
    BeamEyeTrackerEditorStyle.cpp: Editor styling implementation for Beam Eye Tracker.

    Implements custom Slate styling and visual customization for the
    editor interface including icons, colors, and layout preferences.

    Copyright (c) 2025 Eyeware Tech SA. All Rights Reserved.

    Eyeware® and Beam® are registered trademarks of Eyeware Tech SA.
=============================================================================*/

#include "BeamEyeTrackerEditorStyle.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/SlateStyleRegistry.h"
#include "Styling/SlateTypes.h"
#include "Styling/CoreStyle.h"
#include "Styling/AppStyle.h"
#include "Slate/SlateGameResources.h"
#include "Styling/SlateStyle.h"

TSharedPtr<FSlateStyleSet> FBeamEyeTrackerEditorStyle::StyleInstance = nullptr;

void FBeamEyeTrackerEditorStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FBeamEyeTrackerEditorStyle::Shutdown()
{
	if (StyleInstance.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
		StyleInstance.Reset();
	}
}

void FBeamEyeTrackerEditorStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FBeamEyeTrackerEditorStyle::Get()
{
	return *StyleInstance;
}

FName FBeamEyeTrackerEditorStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("BeamEyeTrackerEditorStyle"));
	return StyleSetName;
}

TSharedRef<FSlateStyleSet> FBeamEyeTrackerEditorStyle::Create()
{
	TSharedRef<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet(GetStyleSetName()));
	Style->SetContentRoot(FPaths::ProjectPluginsDir() / TEXT("BeamEyeTracker/Resources"));

	// Add default icons (using text-based icons for now)
	Style->Set("BeamEyeTrackerEditor.MonitorIcon", new FSlateColorBrush(FLinearColor::White));
	Style->Set("BeamEyeTrackerEditor.StartIcon", new FSlateColorBrush(FLinearColor::Green));
	Style->Set("BeamEyeTrackerEditor.StopIcon", new FSlateColorBrush(FLinearColor::Red));
	Style->Set("BeamEyeTrackerEditor.SettingsIcon", new FSlateColorBrush(FLinearColor::Blue));

	return Style;
}

/*=============================================================================
    End of BeamEyeTrackerEditorStyle.cpp
=============================================================================*/
