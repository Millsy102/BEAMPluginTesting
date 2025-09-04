// Implements config loading & editor integration for settings

#include "BeamEyeTrackerSettings.h"

UBeamEyeTrackerSettings::UBeamEyeTrackerSettings()
{
	
	// Default profile: Balanced performance and quality for general development
	FBeamProfile DefaultProfile;
	DefaultProfile.Name = TEXT("Default");
	DefaultProfile.PollingHz = 120;        // 120 Hz polling for smooth tracking
	DefaultProfile.bEnableSmoothing = true; // Enable smoothing for reduced jitter
	DefaultProfile.MinCutoff = 1.0f;       // Moderate smoothing aggressiveness
	DefaultProfile.Beta = 0.2f;            // Balanced adaptation speed
	DefaultProfile.TraceDistance = 5000.0f; // 50m trace distance for world interactions
	DefaultProfile.Flags = 0;

	// High Quality profile: Maximum quality for research and precision applications
	FBeamProfile HighQualityProfile;
	HighQualityProfile.Name = TEXT("High Quality");
	HighQualityProfile.PollingHz = 240;    // 240 Hz for maximum temporal resolution
	HighQualityProfile.bEnableSmoothing = true; // Enable smoothing for clean data
	HighQualityProfile.MinCutoff = 0.5f;   // Aggressive smoothing for stability
	HighQualityProfile.Beta = 0.1f;        // Slow adaptation for consistency
	HighQualityProfile.TraceDistance = 10000.0f; // 100m trace distance for large worlds
	HighQualityProfile.Flags = 0;

	// Performance profile: Optimized for performance-critical applications
	FBeamProfile PerformanceProfile;
	PerformanceProfile.Name = TEXT("Performance");
	PerformanceProfile.PollingHz = 60;     // 60 Hz for reduced CPU usage
	PerformanceProfile.bEnableSmoothing = false; // Disable smoothing for speed
	PerformanceProfile.MinCutoff = 1.0f;   // Default cutoff (unused when smoothing disabled)
	PerformanceProfile.Beta = 0.2f;        // Default beta (unused when smoothing disabled)
	PerformanceProfile.TraceDistance = 2500.0f; // 25m trace distance for performance
	PerformanceProfile.Flags = 0;

	// Add all profiles to the settings collection
	Profiles.Add(DefaultProfile);
	Profiles.Add(HighQualityProfile);
	Profiles.Add(PerformanceProfile);
}

// Editor Integration

FName UBeamEyeTrackerSettings::GetCategoryName() const
{
	return TEXT("Plugins");
}

// Profile Management

const FBeamProfile* UBeamEyeTrackerSettings::GetActiveProfile() const
{
	// First check if the active profile exists
	for (const FBeamProfile& Profile : Profiles)
	{
		if (Profile.Name == ActiveProfile)
		{
			return &Profile;
		}
	}

	// If not found, return the first profile or nullptr if none exist
	// This ensures we always have a fallback profile
	return Profiles.Num() > 0 ? &Profiles[0] : nullptr;
}

void UBeamEyeTrackerSettings::ApplyActiveProfile(UBeamEyeTrackerSettings* TargetSettings) const
{
	if (!TargetSettings)
	{
		return;
	}

	const FBeamProfile* ActiveProfilePtr = GetActiveProfile();
	if (!ActiveProfilePtr)
	{
		return;
	}

	// Apply profile values to target settings
	// This allows runtime switching between different configuration presets
	TargetSettings->PollingHz = ActiveProfilePtr->PollingHz;
	TargetSettings->bEnableSmoothing = ActiveProfilePtr->bEnableSmoothing;
	TargetSettings->MinCutoff = ActiveProfilePtr->MinCutoff;
	TargetSettings->Beta = ActiveProfilePtr->Beta;
	TargetSettings->TraceDistance = ActiveProfilePtr->TraceDistance;
}

