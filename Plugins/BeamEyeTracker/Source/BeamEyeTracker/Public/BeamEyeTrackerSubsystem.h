/*=============================================================================
    BeamEyeTrackerSubsystem.h: Subsystem for integrating Eyeware Beam SDK.

    Provides optimized, stable, and Blueprint-accessible eye tracking features.

    Copyright (c) 2025 Eyeware Tech SA. All Rights Reserved.

    Eyeware® and Beam® are registered trademarks of Eyeware Tech SA.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "BeamEyeTrackerTypes.h"
#include "BeamFilters.h"
#include "BeamEyeTrackerSubsystem.generated.h"

// Forward declarations for private implementation classes
class IBeamDataSource;
class FBeamRing;
class FBeamRecording;
class FBeamTrace;

	// Forward declarations for UObject classes
	class UBeamEyeTrackerSettings;
	class UBeamEyeTrackerComponent;

/** Game-instance subsystem for global access to Beam eye tracking functionality */
UCLASS()
class BEAMEYETRACKER_API UBeamEyeTrackerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UBeamEyeTrackerSubsystem();

	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Starts beam tracking and initializes the data source */
	UFUNCTION(BlueprintCallable, Category = "BEAM|Tracking", meta = (DisplayName = "Start Beam Tracking", ToolTip = "Starts beam tracking and initializes the data source"))
	bool StartBeamTracking();

	/** Stops beam tracking and cleans up resources */
	UFUNCTION(BlueprintCallable, Category = "BEAM|Tracking", meta = (DisplayName = "Stop Beam Tracking", ToolTip = "Stops beam tracking and cleans up resources"))
	void StopBeamTracking();

	/** Checks if beam tracking is currently active */
	UFUNCTION(BlueprintPure, Category = "BEAM|Tracking", meta = (DisplayName = "Is Beam Tracking", ToolTip = "Checks if beam tracking is currently active"))
	bool IsBeamTracking() const;

	// Data access - vary naming patterns
	UFUNCTION(BlueprintCallable, Category = "Beam")
	bool FetchCurrentFrame(FBeamFrame& OutFrame) const;

	/** Gets frame data at a specific timestamp */
	UFUNCTION(BlueprintCallable, Category = "BEAM|Tracking", meta = (DisplayName = "Get Frame At", ToolTip = "Gets frame data at a specific timestamp"))
	bool GetFrameAt(double TimestampMs, FBeamFrame& OutFrame) const;

	/** Gets latest interpolated frame for smooth rendering */
	UFUNCTION(BlueprintCallable, Category = "BEAM|Tracking", meta = (DisplayName = "Get Latest Interpolated Frame", ToolTip = "Gets latest interpolated frame for smooth rendering"))
	bool GetLatestInterpolatedFrame(double DeltaSeconds, FBeamFrame& OutFrame) const;

	// Vary function names - don't use "Get" for everything
	UFUNCTION(BlueprintCallable, Category = "Beam")
	FGazePoint CurrentGaze() const;

	UFUNCTION(BlueprintCallable, Category = "Beam")
	FHeadPose HeadPosition() const;

	UFUNCTION(BlueprintCallable, Category = "Beam")
	FString GetSDKVersion() const;

	/** Checks if Beam application is running */
	UFUNCTION(BlueprintPure, Category = "BEAM|Tracking", meta = (DisplayName = "Is Beam App Running", ToolTip = "Checks if Beam application is running"))
	bool IsBeamAppRunning() const;

	/** Gets the current health status of the eye tracker */
	UFUNCTION(BlueprintPure, Category = "BEAM|Status", meta = (DisplayName = "Get Health Status", ToolTip = "Returns the current health status of the eye tracker"))
	EBeamHealth GetBeamHealth() const { return CurrentHealth; }

	/** Gets the current data source type */
	UFUNCTION(BlueprintPure, Category = "BEAM|Status", meta = (DisplayName = "Get Data Source Type", ToolTip = "Returns the current data source type"))
	EBeamDataSourceType GetDataSourceType() const { return DataSourceType; }

	/** Gets the last error message if any */
	UFUNCTION(BlueprintPure, Category = "BEAM|Status", meta = (DisplayName = "Get Last Error", ToolTip = "Returns the last error message if any"))
	FString GetLastErrorMessage() const { return LastErrorMessage; }

	/** Gets current system health status */
	// Phase 2: Advanced Analytics and Performance Features
	/** Gets comprehensive performance metrics for the tracking system */
	UFUNCTION(BlueprintCallable, Category = "BEAM|Analytics", meta = (DisplayName = "Get Performance Metrics", ToolTip = "Gets comprehensive performance metrics including frame times, CPU usage, and memory"))
	FBeamPerformanceMetrics GetPerformanceMetrics() const;

	/** Gets gaze analytics data including fixations and saccades */
	UFUNCTION(BlueprintCallable, Category = "BEAM|Analytics", meta = (DisplayName = "Get Gaze Analytics", ToolTip = "Gets gaze analytics data including fixation duration, saccade velocity, and scan patterns"))
	FGazeAnalytics GetGazeAnalytics() const;

	/** Gets detailed calibration quality assessment */
	UFUNCTION(BlueprintCallable, Category = "BEAM|Analytics", meta = (DisplayName = "Get Calibration Quality", ToolTip = "Gets detailed calibration quality assessment with per-eye scores and point analysis"))
	FCalibrationQuality GetCalibrationQuality() const;

	/** Exports tracking data to CSV format for analysis */
	UFUNCTION(BlueprintCallable, Category = "BEAM|Export", meta = (DisplayName = "Export Tracking Data", ToolTip = "Exports tracking data to CSV format for external analysis"))
	bool ExportTrackingData(const FString& FilePath, float DurationSeconds = 60.0f);

	/** Gets system resource usage statistics */
	UFUNCTION(BlueprintCallable, Category = "BEAM|System", meta = (DisplayName = "Get System Resources", ToolTip = "Gets current system resource usage including CPU, memory, and GPU"))
	void GetSystemResources(float& OutCPUUsage, float& OutMemoryUsage, float& OutGPUUsage) const;

	/** Gets current system health status */
	UFUNCTION(BlueprintPure, Category = "BEAM|Status", meta = (DisplayName = "Get Health", ToolTip = "Gets current system health status"))
	EBeamHealth GetHealth() const;

	/** Sets data source type and optionally file path */
	UFUNCTION(BlueprintCallable, Category = "BEAM|Tracking", meta = (DisplayName = "Set Data Source Type", ToolTip = "Sets data source type and optionally file path"))
	void SetDataSourceType(EBeamDataSourceType NewType, const FString& FilePath = TEXT(""));

	/** Checks if an actor has a tagged beam component */
	UFUNCTION(BlueprintPure, Category = "BEAM|Tracking", meta = (DisplayName = "Has Tagged Beam Component", ToolTip = "Checks if an actor has a tagged beam component"))
	static bool HasTaggedBeamComponent(const AActor* Actor, FName Tag);

	/** Gets the settings object for configuration */
	UFUNCTION(BlueprintPure, Category = "BEAM|Tracking", meta = (DisplayName = "Get Settings", ToolTip = "Gets the settings object for configuration"))
	const UBeamEyeTrackerSettings* GetSettings() const { return Settings; }

	UFUNCTION(BlueprintCallable, Category = "Beam")
	bool ProjectGazeToWorld(const APlayerController* PlayerController, FVector& OutRayOrigin, FVector& OutRayDirection) const;

	// Buffer and performance functions
	UFUNCTION(BlueprintPure, Category = "Beam")
	float GetBufferUtilization() const;

	UFUNCTION(BlueprintPure, Category = "Beam")
	float GetTrackingFPS() const;

	// Alias function - keep simple
	UFUNCTION(BlueprintPure, Category = "Beam")
	float GetCurrentFPS() const { return GetTrackingFPS(); }

	// Debug and settings functions
	UFUNCTION(BlueprintCallable, Category = "Beam")
	void SetDebugHUDEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Beam")
	void SetPollingRate(int32 NewRateHz);

	UFUNCTION(BlueprintCallable, Category = "Beam")
	void SetSmoothingEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Beam")
	void SetMinCutoff(float NewMinCutoff);

	UFUNCTION(BlueprintCallable, Category = "Beam")
	void SetBeta(float NewBeta);

	UFUNCTION(BlueprintPure, Category = "Beam")
	bool IsCalibrated() const;

	UFUNCTION(BlueprintCallable, Category = "Beam")
	void OpenBeamDownloads() const;

	UFUNCTION(BlueprintPure, Category = "Beam")
	EBeamFilterType GetFilterType() const;

	UFUNCTION(BlueprintCallable, Category = "Beam")
	void SetFilterType(EBeamFilterType NewFilterType);

	UFUNCTION(BlueprintCallable, Category = "Beam")
	void ApplyRuntimeSettings(const UBeamEyeTrackerSettings* NewSettings);

	UFUNCTION(BlueprintPure, Category = "Beam")
	void GetEffectiveRuntimeSettings(int32& OutPollingHz, bool& OutEnableSmoothing, float& OutMinCutoff, float& OutBeta, float& OutTraceDistance) const;

	// Camera and viewport functions
	UFUNCTION(BlueprintCallable, Category = "Beam")
	bool StartCameraRecentering();

	UFUNCTION(BlueprintCallable, Category = "Beam")
	void EndCameraRecentering();

	UFUNCTION(BlueprintCallable, Category = "Beam")
	void UpdateViewportGeometry(int32 ViewportWidth, int32 ViewportHeight);

	UFUNCTION(BlueprintCallable, Category = "Beam")
	void AutoUpdateViewport();

	// Calibration functions
	UFUNCTION(BlueprintCallable, Category = "Beam")
	bool StartCalibration(const FString& ProfileId = TEXT("Default"));

	UFUNCTION(BlueprintCallable, Category = "Beam")
	void StopCalibration();

	UFUNCTION(BlueprintCallable, Category = "Beam")
	void ResetCalibration();

	UFUNCTION(BlueprintPure, Category = "Beam")
	bool IsCalibrating() const;

	UFUNCTION(BlueprintCallable, Category = "Beam")
	bool StartRecording(const FString& FilePath = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "Beam")
	void StopRecording();

	UFUNCTION(BlueprintPure, Category = "Beam")
	bool IsRecording() const;

	UFUNCTION(BlueprintCallable, Category = "Beam")
	bool StartPlayback(const FString& FilePath);

	UFUNCTION(BlueprintCallable, Category = "Beam")
	void StopPlayback();

	UFUNCTION(BlueprintPure, Category = "Beam")
	bool IsPlayingBack() const;

	/** Gets the current viewport dimensions - essential for coordinate mapping */
	UFUNCTION(BlueprintPure, Category = "BEAM|Viewport", meta = (DisplayName = "Get Viewport Dimensions", ToolTip = "Returns the current viewport dimensions. Essential for accurate coordinate mapping. All parameters are output parameters that get filled with current values. Use this to check viewport size before calling UpdateViewportGeometry()."))
	void GetViewportDimensions(int32& OutWidth, int32& OutHeight) const;

	/** Updates viewport dimensions for proper coordinate mapping - convenience function */
	UFUNCTION(BlueprintCallable, Category = "BEAM|Viewport", meta = (DisplayName = "Update Viewport Dimensions", ToolTip = "Updates viewport dimensions for proper coordinate mapping. Convenience function that automatically detects current viewport size. Call this in BeginPlay() or when viewport changes. Equivalent to calling AutoUpdateViewport()."))
	void UpdateViewportDimensions();

private:
	bool bIsTracking = false;

	EBeamHealth CurrentHealth = EBeamHealth::AppNotRunning;

	/** Current data source type */
	EBeamDataSourceType DataSourceType = EBeamDataSourceType::Live;

	/** Last error message */
	FString LastErrorMessage;

	/** Settings object */
	UPROPERTY()
	UBeamEyeTrackerSettings* Settings = nullptr;

	/** Frame buffer for data storage */
	FBeamRing* FrameBuffer;

	/** Data source interface */
	IBeamDataSource* DataSource;

	/** Gaze smoothing filter */
	FBeamFilters* Filters;

	/** Recording system (dev only) */
	FBeamRecording* Recording;

	/** Tracing system */
	FBeamTrace* Tracing;

	/** Background thread for polling */
	FRunnableThread* PollingThread;

	/** Stop flag for the polling thread */
	FThreadSafeBool bStopPolling;

	/** Performance tracking */
	double LastFrameTime = 0.0;
	double FrameTimeSum = 0.0;
	int32 FrameCount = 0;

	/** Watchdog recovery state */
	double LastDataTime = 0.0;
	int32 ConsecutiveFailures = 0;
	double RecoveryBackoffTime = 0.0;

	/** Calibration state */
	bool bIsCalibrating = false;
	FString CurrentCalibrationProfile;

	/** Filter configuration */
	EBeamFilterType CurrentFilterType = EBeamFilterType::None;

	/** Recording state */
	bool bIsRecording = false;
	FString RecordingFilePath;
	FArchive* RecordingFile;
	double RecordingStartTime = 0.0;

	/** Playback state */
	bool bIsPlayingBack = false;
	FString PlaybackFilePath;
	TArray<FString> PlaybackLines;
	int32 CurrentPlaybackLine = 0;
	double PlaybackStartTime = 0.0;

	/** Start the background polling thread */
	void StartPollingThread();

	/** Stop the background polling thread */
	void StopPollingThread();

	/** Main polling loop function */
	static uint32 PollingThreadFunction(void* Param);

	/** Update health status based on data source */
	void UpdateHealthStatus();

	/** Create data source of specified type */
	IBeamDataSource* CreateDataSource(EBeamDataSourceType Type, const FString& FilePath = TEXT(""));

	/** Apply smoothing to gaze data if enabled */
	void ApplyGazeSmoothing(FBeamFrame& Frame);

	/** Watchdog recovery logic */
	void UpdateWatchdogRecovery();

	/** DLL safety verification */
	bool VerifyDLLSafety();

	/** Initialize tracing system */
	void InitializeTracing();

	/** Update performance metrics */
	void UpdatePerformanceMetrics();
	
	/** Record a frame to the recording file if recording is active */
	void RecordFrame(const FBeamFrame& Frame);

};

/*=============================================================================
    End of BeamEyeTrackerSubsystem.h
=============================================================================*/

