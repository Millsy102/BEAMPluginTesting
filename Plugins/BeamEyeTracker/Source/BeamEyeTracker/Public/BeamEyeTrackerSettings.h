/*=============================================================================
    BeamEyeTrackerSettings.h: Project settings and configuration for Beam SDK.

    Provides comprehensive configuration with profiles and advanced options for
    eye tracking functionality. Organized for easy access in the editor.

    Copyright (c) 2025 Eyeware Tech SA. All Rights Reserved.

    Eyeware® and Beam® are registered trademarks of Eyeware Tech SA.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Internationalization/Internationalization.h"
#include "BeamEyeTrackerTypes.h"
#include "BeamFilters.h"
#include "BeamEyeTrackerSettings.generated.h"

/**
 * Profile structure for Beam Eye Tracker configuration presets.
 * 
 * Defines named configuration profiles that can be applied to quickly
 * switch between different tracking setups and filter configurations.
 */
USTRUCT(BlueprintType)
struct FBeamProfile
{
	GENERATED_BODY()

	/** Profile name for identification */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile", meta = (ToolTip = "Profile name for identification"))
	FName Name = TEXT("Default");

	/** Polling frequency in Hz for data collection */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile", meta = (ClampMin = "15", ClampMax = "240", UIMin = "30", UIMax = "144", Units = "Hz", ToolTip = "Polling frequency in Hz for data collection"))
	int32 PollingHz = 120;

	/** If true, enables One-Euro smoothing for gaze data */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile", meta = (ToolTip = "If true, enables One-Euro smoothing for gaze data"))
	bool bEnableSmoothing = true;

	/** One-Euro filter minimum cutoff frequency for smoothing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile", meta = (EditCondition = "bEnableSmoothing", ClampMin = "0.1", ClampMax = "5.0", UIMin = "0.2", UIMax = "2.0", ToolTip = "One-Euro filter minimum cutoff frequency for smoothing"))
	float MinCutoff = 1.0f;

	/** One-Euro filter beta parameter for adaptive smoothing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile", meta = (EditCondition = "bEnableSmoothing", ClampMin = "0.0", ClampMax = "2.0", UIMin = "0.0", UIMax = "1.0", ToolTip = "One-Euro filter beta parameter for adaptive smoothing"))
	float Beta = 0.2f;

	/** Default max distance for gaze line traces in centimeters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile", meta = (ClampMin = "100", ClampMax = "100000", UIMin = "500", UIMax = "20000", Units = "cm", ToolTip = "Default max distance for gaze line traces in centimeters"))
	float TraceDistance = 5000.0f;

	/** Profile flags for additional configuration options */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile", meta = (ToolTip = "Profile flags for additional configuration options"))
	int32 Flags = 0;

	FBeamProfile() = default;
};

/**
 * Project-wide config for Beam eye tracking functionality.
 * 
 * Provides comprehensive configuration with profiles and advanced options.
 * Organized for easy access in the editor. All settings are configurable
 * at the project level and can be overridden via command line arguments.
 */
UCLASS(config = Engine, defaultconfig, meta = (DisplayName = "Beam Eye Tracker"))
class BEAMEYETRACKER_API UBeamEyeTrackerSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UBeamEyeTrackerSettings();

	// UDeveloperSettings interface
	virtual FName GetCategoryName() const override;

	// Profile Management
	const FBeamProfile* GetActiveProfile() const;
	void ApplyActiveProfile(UBeamEyeTrackerSettings* TargetSettings) const;

	// Core Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core", meta = (ClampMin = "15", ClampMax = "240", UIMin = "30", UIMax = "144", Units = "Hz", ToolTip = "Polling frequency in Hz for data collection"))
	int32 PollingHz = 120;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core", meta = (ToolTip = "If true, enables One-Euro smoothing for gaze data"))
	bool bEnableSmoothing = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core", meta = (EditCondition = "bEnableSmoothing", ClampMin = "0.1", ClampMax = "5.0", UIMin = "0.2", UIMax = "2.0", ToolTip = "One-Euro filter minimum cutoff frequency for smoothing"))
	float MinCutoff = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core", meta = (EditCondition = "bEnableSmoothing", ClampMin = "0.0", ClampMax = "2.0", UIMin = "0.0", UIMax = "1.0", ToolTip = "One-Euro filter beta parameter for adaptive smoothing"))
	float Beta = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core", meta = (ClampMin = "100", ClampMax = "100000", UIMin = "500", UIMax = "20000", Units = "cm", ToolTip = "Default max distance for gaze line traces in centimeters"))
	float TraceDistance = 5000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core", meta = (ToolTip = "Application name for Beam SDK initialization"))
	FString ApplicationName = TEXT("BeamEyeTracker");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core", meta = (ToolTip = "Auto-start tracking when PIE begins"))
	bool bAutoStartOnPIE = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core", meta = (ToolTip = "Auto-start tracking when the subsystem initializes"))
	bool bAutoStart = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core", meta = (ToolTip = "Active profile name"))
	FName ActiveProfile = TEXT("Default");

private:
	UPROPERTY()
	TArray<FBeamProfile> Profiles;
};

/*=============================================================================
    End of BeamEyeTrackerSettings.h
=============================================================================*/
