/*=============================================================================
    BeamEyeTrackerCommands.h: Console commands and CVars for Beam SDK.

    Binds custom Editor UI/console commands for comprehensive Beam
    functionality including tracking control, data management, debugging,
    and configuration. All commands are prefixed with "Beam." for organization.

    Copyright (c) 2025 Eyeware Tech SA. All Rights Reserved.

    Eyeware® and Beam® are registered trademarks of Eyeware Tech SA.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"

/** Binds custom Editor UI/console commands for Beam Eye Tracker functionality */
class BEAMEYETRACKER_API FBeamEyeTrackerCommands
{
public:
	/** Initialize console commands and CVars */
	static void Initialize();

	/** Shutdown console commands and CVars */
	static void Shutdown();

	/** Register all console commands */
	static void RegisterCommands();

	/** Unregister all console commands */
	static void UnregisterCommands();

private:
	/** Console command handlers */
	
	/** Registers 'Beam.Start' command to start eye tracking */
	static void StartTracking(const TArray<FString>& Args, UWorld* World, FOutputDevice& Ar);
	
	/** Registers 'Beam.Stop' command to stop eye tracking */
	static void StopTracking(const TArray<FString>& Args, UWorld* World, FOutputDevice& Ar);
	
	/** Registers 'Beam.Dump' command to dump current tracking status */
	static void DumpStatus(const TArray<FString>& Args, UWorld* World, FOutputDevice& Ar);
	
	/** Registers 'Beam.DumpFrames' command to dump recent frame data */
	static void DumpFrames(const TArray<FString>& Args, UWorld* World, FOutputDevice& Ar);
	
	/** Registers 'Beam.SetDataSource' command to change data source */
	static void SetDataSource(const TArray<FString>& Args, UWorld* World, FOutputDevice& Ar);
	
	/** Registers 'Beam.SetFilter' command to configure signal filters */
	static void SetFilter(const TArray<FString>& Args, UWorld* World, FOutputDevice& Ar);
	
	/** Registers 'Beam.StartRecording' command to begin data recording */
	static void StartRecording(const TArray<FString>& Args, UWorld* World, FOutputDevice& Ar);
	
	/** Registers 'Beam.StopRecording' command to stop data recording */
	static void StopRecording(const TArray<FString>& Args, UWorld* World, FOutputDevice& Ar);
	
	/** Registers 'Beam.StartPlayback' command to begin data playback */
	static void StartPlayback(const TArray<FString>& Args, UWorld* World, FOutputDevice& Ar);
	
	/** Registers 'Beam.StopPlayback' command to stop data playback */
	static void StopPlayback(const TArray<FString>& Args, UWorld* World, FOutputDevice& Ar);
	
	/** Registers 'Beam.SetProfile' command to change tracking profile */
	static void SetProfile(const TArray<FString>& Args, UWorld* World, FOutputDevice& Ar);
	
	/** Registers 'Beam.ToggleDebugHUD' command to toggle debug overlay */
	static void ToggleDebugHUD(const TArray<FString>& Args, UWorld* World, FOutputDevice& Ar);
	
	/** Registers 'Beam.ToggleTracing' command to toggle gaze ray tracing */
	static void ToggleTracing(const TArray<FString>& Args, UWorld* World, FOutputDevice& Ar);
	
	/** Registers 'Beam.ExportCSV' command to export data to CSV format */
	static void ExportCSV(const TArray<FString>& Args, UWorld* World, FOutputDevice& Ar);

	/** CVar declarations */
	
	/** Toggle for enabling debug HUD overlay */
	static TAutoConsoleVariable<bool> CVarEnableDebugHUD;
	
	/** Toggle for enabling gaze ray tracing */
	static TAutoConsoleVariable<bool> CVarEnableTracing;
	
	/** Polling frequency in Hz for tracking data */
	static TAutoConsoleVariable<int32> CVarPollingFrequency;
	
	/** Type of signal filter to apply (0=None, 1=OneEuro, 2=EMA) */
	static TAutoConsoleVariable<int32> CVarFilterType;
	
	/** Toggle for enabling signal smoothing */
	static TAutoConsoleVariable<bool> CVarEnableSmoothing;
	
	/** Smoothing gain factor for filter calculations */
	static TAutoConsoleVariable<float> CVarSmoothingGain;
	
	/** Toggle for enabling data recording */
	static TAutoConsoleVariable<bool> CVarEnableRecording;
	
	/** Directory path for saving recorded data files */
	static TAutoConsoleVariable<FString> CVarRecordingDirectory;
	
	/** Toggle for enabling synthetic data generation */
	static TAutoConsoleVariable<bool> CVarEnableSyntheticData;
	
	/** Frequency in Hz for synthetic data generation */
	static TAutoConsoleVariable<int32> CVarSyntheticDataFrequency;
};

/*=============================================================================
    End of BeamEyeTrackerCommands.h
=============================================================================*/
