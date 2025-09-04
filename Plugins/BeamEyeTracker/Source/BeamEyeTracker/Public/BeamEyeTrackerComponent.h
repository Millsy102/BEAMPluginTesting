/*=============================================================================
    BeamEyeTrackerComponent.h: Actor component for Beam eye tracking integration.

    Attach this component to an Actor to enable eye tracking features.
    Provides easy access to gaze and head pose data with proper Details Panel
    organization and Blueprint delegates.

    Copyright (c) 2025 Eyeware Tech SA. All Rights Reserved.

    Eyeware® and Beam® are registered trademarks of Eyeware Tech SA.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BeamEyeTrackerTypes.h"
#include "BeamFilters.h"
#include "BeamFrameBuffer.h"
#include "BeamEyeTrackerComponent.generated.h"

class UBeamEyeTrackerSubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGazeUpdated, const FGazePoint&, GazePoint);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHeadPoseUpdated, const FHeadPose&, HeadPose);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBeamHealthChanged, EBeamHealth, Health);

/** Attach this component to an Actor to enable eye tracking features */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BEAMEYETRACKER_API UBeamEyeTrackerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBeamEyeTrackerComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Connection settings
	UPROPERTY(EditAnywhere, Category = "Beam")
	bool bAutoStart = true;

	UPROPERTY(EditAnywhere, Category = "Beam", meta = (ClampMin = "30", ClampMax = "144", Units = "Hz"))
	int32 PollingHz = 120;

	// Filtering settings
	UPROPERTY(EditAnywhere, Category = "Beam")
	bool bEnableSmoothing = true;

	UPROPERTY(EditAnywhere, Category = "Beam", meta = (ClampMin = "0.1", ClampMax = "5.0", EditCondition = "bEnableSmoothing"))
	float MinCutoff = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Beam", meta = (ClampMin = "0.0", ClampMax = "2.0", EditCondition = "bEnableSmoothing"))
	float Beta = 0.0f;

	// Projection settings
	UPROPERTY(EditAnywhere, Category = "Beam")
	bool bProjectFromOwnerCamera = false;

	UPROPERTY(EditAnywhere, Category = "Beam", meta = (ClampMin = "100", ClampMax = "100000", EditCondition = "bProjectFromOwnerCamera", Units = "cm"))
	float TraceDistance = 5000.0f;

	// **BEAM|Debug Group** (BEAM|Debug)
	/** If true, enables debug HUD overlay for development */
	UPROPERTY(EditAnywhere, Category = "BEAM|Debug", meta = (DisplayPriority = "4", EditCondition = "bEnableDebugHUD", ToolTip = "If true, enables debug HUD overlay for development"))
	bool bEnableDebugHUD = false;

	/** If true, shows gaze crosshair on screen */
	UPROPERTY(EditAnywhere, Category = "BEAM|Debug", meta = (DisplayPriority = "4", EditCondition = "bEnableDebugHUD", ToolTip = "If true, shows gaze crosshair on screen"))
	bool bShowGazeCrosshair = true;

	/** If true, shows 3D gaze ray in world */
	UPROPERTY(EditAnywhere, Category = "BEAM|Debug", meta = (DisplayPriority = "4", EditCondition = "bEnableDebugHUD", ToolTip = "If true, shows 3D gaze ray in world"))
	bool bShowGazeRay = false;

	/** If true, shows status panel with tracking info */
	UPROPERTY(EditAnywhere, Category = "BEAM|Debug", meta = (DisplayPriority = "4", EditCondition = "bEnableDebugHUD", ToolTip = "If true, shows status panel with tracking info"))
	bool bShowStatusPanel = true;

	/** If true, shows gaze trail (recent gaze points) */
	UPROPERTY(EditAnywhere, Category = "BEAM|Debug", meta = (DisplayPriority = "4", EditCondition = "bEnableDebugHUD", ToolTip = "If true, shows gaze trail (recent gaze points)"))
	bool bShowGazeTrail = false;

	/** Number of points in gaze trail (10-1000) */
	UPROPERTY(EditAnywhere, Category = "BEAM|Debug", meta = (DisplayPriority = "4", EditCondition = "bShowGazeTrail", ClampMin = "10", ClampMax = "1000", ToolTip = "Number of points in gaze trail (10-1000)"))
	int32 GazeTrailLength = 180;

	/** Status panel anchor position X (0.0-1.0) */
	UPROPERTY(EditAnywhere, Category = "BEAM|Debug", meta = (DisplayPriority = "4", EditCondition = "bShowStatusPanel", ClampMin = "0.0", ClampMax = "1.0", ToolTip = "Status panel anchor position X (0.0-1.0)"))
	float StatusPanelAnchorX = 0.8f;

	/** Status panel anchor position Y (0.0-1.0) */
	UPROPERTY(EditAnywhere, Category = "BEAM|Debug", meta = (DisplayPriority = "4", EditCondition = "bShowStatusPanel", ClampMin = "0.0", ClampMax = "1.0", ToolTip = "Status panel anchor position Y (0.0-1.0)"))
	float StatusPanelAnchorY = 0.1f;

	// **BEAM|Data Quality Group** (BEAM|Data Quality)
	/** Minimum confidence threshold for valid gaze data (0.0-1.0) */
	UPROPERTY(EditAnywhere, Category = "BEAM|Data Quality", meta = (DisplayPriority = "5", ClampMin = "0.0", ClampMax = "1.0", ToolTip = "Minimum confidence threshold for valid gaze data (0.0-1.0)"))
	float MinGazeConfidence = 0.5f;

	/** Minimum confidence threshold for valid head pose data (0.0-1.0) */
	UPROPERTY(EditAnywhere, Category = "BEAM|Data Quality", meta = (DisplayPriority = "5", ClampMin = "0.0", ClampMax = "1.0", ToolTip = "Minimum confidence threshold for valid head pose data (0.0-1.0)"))
	float MinHeadPoseConfidence = 0.3f;

	/** Maximum age of gaze data before considering it stale (seconds) */
	UPROPERTY(EditAnywhere, Category = "BEAM|Data Quality", meta = (DisplayPriority = "5", ClampMin = "0.1", ClampMax = "5.0", Units = "s", ToolTip = "Maximum age of gaze data before considering it stale (seconds)"))
	float MaxGazeAgeSeconds = 0.5f;

	/** If true, enables data validation to filter out invalid samples */
	UPROPERTY(EditAnywhere, Category = "BEAM|Data Quality", meta = (DisplayPriority = "5", ToolTip = "If true, enables data validation to filter out invalid samples"))
	bool bEnableDataValidation = true;

	// **BEAM|Advanced Filtering Group** (BEAM|Advanced Filtering)
	/** If true, enables outlier detection and removal */
	UPROPERTY(EditAnywhere, Category = "BEAM|Advanced Filtering", meta = (DisplayPriority = "6", ToolTip = "If true, enables outlier detection and removal"))
	bool bEnableOutlierDetection = false;

	/** Outlier detection threshold (standard deviations) */
	UPROPERTY(EditAnywhere, Category = "BEAM|Advanced Filtering", meta = (DisplayPriority = "6", ClampMin = "1.0", ClampMax = "5.0", EditCondition = "bEnableOutlierDetection", ToolTip = "Outlier detection threshold (standard deviations)"))
	float OutlierThreshold = 2.5f;

	/** If true, enables adaptive smoothing based on confidence */
	UPROPERTY(EditAnywhere, Category = "BEAM|Advanced Filtering", meta = (DisplayPriority = "6", ToolTip = "If true, enables adaptive smoothing based on confidence"))
	bool bEnableAdaptiveSmoothing = false;

	/** Smoothing strength multiplier for low confidence data */
	UPROPERTY(EditAnywhere, Category = "BEAM|Advanced Filtering", meta = (DisplayPriority = "6", ClampMin = "1.0", ClampMax = "5.0", EditCondition = "bEnableAdaptiveSmoothing", ToolTip = "Smoothing strength multiplier for low confidence data"))
	float LowConfidenceSmoothingMultiplier = 2.0f;

	// **BEAM|Performance Group** (BEAM|Performance)
	/** Frame buffer size for data storage (must be power of 2) */
	UPROPERTY(EditAnywhere, Category = "BEAM|Performance", meta = (DisplayPriority = "7", ClampMin = "16", ClampMax = "1024", ToolTip = "Frame buffer size for data storage (must be power of 2)"))
	int32 FrameBufferSize = 64;

	/** If true, enables frame interpolation for smooth updates */
	UPROPERTY(EditAnywhere, Category = "BEAM|Performance", meta = (DisplayPriority = "7", ToolTip = "If true, enables frame interpolation for smooth updates"))
	bool bEnableFrameInterpolation = true;

	/** Maximum interpolation time window (milliseconds) */
	UPROPERTY(EditAnywhere, Category = "BEAM|Performance", meta = (DisplayPriority = "7", ClampMin = "16", ClampMax = "500", EditCondition = "bEnableFrameInterpolation", Units = "ms", ToolTip = "Maximum interpolation time window (milliseconds)"))
	float MaxInterpolationTimeMs = 100.0f;

	/** If true, enables adaptive polling based on frame rate */
	UPROPERTY(EditAnywhere, Category = "BEAM|Performance", meta = (DisplayPriority = "7", ToolTip = "If true, enables adaptive polling based on frame rate"))
	bool bEnableAdaptivePolling = false;

	// **BEAM|Events Group** (BEAM|Events)
	/** If true, enables gaze change notifications */
	UPROPERTY(EditAnywhere, Category = "BEAM|Events", meta = (DisplayPriority = "8", ToolTip = "If true, enables gaze change notifications"))
	bool bEnableGazeChangeNotifications = true;

	/** Minimum gaze movement threshold for notifications (pixels) */
	UPROPERTY(EditAnywhere, Category = "BEAM|Events", meta = (DisplayPriority = "8", ClampMin = "1", ClampMax = "100", EditCondition = "bEnableGazeChangeNotifications", ToolTip = "Minimum gaze movement threshold for notifications (pixels)"))
	int32 GazeChangeThresholdPixels = 10;

	/** If true, enables head pose change notifications */
	UPROPERTY(EditAnywhere, Category = "BEAM|Events", meta = (DisplayPriority = "8", ToolTip = "If true, enables head pose change notifications"))
	bool bEnableHeadPoseChangeNotifications = false;

	/** Head pose change threshold (degrees) */
	UPROPERTY(EditAnywhere, Category = "BEAM|Events", meta = (DisplayPriority = "8", ClampMin = "1.0", ClampMax = "45.0", EditCondition = "bEnableHeadPoseChangeNotifications", Units = "deg", ToolTip = "Head pose change threshold (degrees)"))
	float HeadPoseChangeThresholdDegrees = 5.0f;

	// **BEAM|Advanced SDK Group** (BEAM|Advanced SDK)
	/** If true, enables foveated rendering support */
	UPROPERTY(EditAnywhere, Category = "BEAM|Advanced SDK", meta = (DisplayPriority = "9", ToolTip = "If true, enables foveated rendering support"))
	bool bEnableFoveatedRendering = false;

	/** If true, enables immersive HUD features */
	UPROPERTY(EditAnywhere, Category = "BEAM|Advanced SDK", meta = (DisplayPriority = "9", ToolTip = "If true, enables immersive HUD features"))
	bool bEnableImmersiveHUD = false;

	/** If true, enables sim game camera controls */
	UPROPERTY(EditAnywhere, Category = "BEAM|Advanced SDK", meta = (DisplayPriority = "9", ToolTip = "If true, enables sim game camera controls"))
	bool bEnableSimGameCamera = false;

	/** Camera sensitivity for head tracking */
	UPROPERTY(EditAnywhere, Category = "BEAM|Advanced SDK", meta = (DisplayPriority = "9", ClampMin = "0.1", ClampMax = "5.0", EditCondition = "bEnableSimGameCamera", ToolTip = "Camera sensitivity for head tracking"))
	float CameraSensitivity = 1.0f;

	// **BEAM|Integration Group** (BEAM|Integration)
	/** If true, enables compatibility mode for specific engines/frameworks */
	UPROPERTY(EditAnywhere, Category = "BEAM|Integration", meta = (DisplayPriority = "10", ToolTip = "If true, enables compatibility mode for specific engines/frameworks"))
	bool bEnableCompatibilityMode = false;

	/** Custom application identifier for SDK */
	UPROPERTY(EditAnywhere, Category = "BEAM|Integration", meta = (DisplayPriority = "10", EditCondition = "bEnableCompatibilityMode", ToolTip = "Custom application identifier for SDK"))
	FString CustomAppIdentifier = TEXT("UnrealEngine");

	// **BEAM|Analytics Group** (BEAM|Analytics)
	/** If true, enables detailed logging */
	UPROPERTY(EditAnywhere, Category = "BEAM|Analytics", meta = (DisplayPriority = "11", ToolTip = "If true, enables detailed logging"))
	bool bEnableDetailedLogging = false;

	/** If true, enables performance metrics collection */
	UPROPERTY(EditAnywhere, Category = "BEAM|Analytics", meta = (DisplayPriority = "11", ToolTip = "If true, enables performance metrics collection"))
	bool bEnablePerformanceMetrics = false;

	// **Blueprint Events**
	/** Called when gaze data is updated */
	UPROPERTY(BlueprintAssignable, Category = "BEAM|Events")
	FOnGazeUpdated OnGazeUpdated;

	/** Called when head pose data is updated */
	UPROPERTY(BlueprintAssignable, Category = "BEAM|Events")
	FOnHeadPoseUpdated OnHeadPoseUpdated;

	/** Called when Beam health status changes */
	UPROPERTY(BlueprintAssignable, Category = "BEAM|Events")
	FOnBeamHealthChanged OnBeamHealthChanged;

	// **Public API Functions**
	/** Gets the latest gaze point */
	UFUNCTION(BlueprintPure, Category = "BEAM|Tracking", meta = (DisplayName = "Get Latest Gaze Point", ToolTip = "Gets the latest gaze point from the eye tracker"))
	FGazePoint CurrentGaze() const;

	/** Gets the latest head pose */
	UFUNCTION(BlueprintPure, Category = "BEAM|Tracking", meta = (DisplayName = "Get Latest Head Pose", ToolTip = "Gets the latest head pose from the eye tracker"))
	FHeadPose HeadPosition() const;

	/** Gets the latest complete frame */
	UFUNCTION(BlueprintPure, Category = "BEAM|Tracking", meta = (DisplayName = "Get Latest Frame", ToolTip = "Gets the latest complete frame with all tracking data"))
	bool FetchCurrentFrame(FBeamFrame& OutFrame) const;

	/** Gets gaze ray in world space (if bProjectFromOwnerCamera is true) */
	UFUNCTION(BlueprintCallable, Category = "BEAM|Tracking", meta = (DisplayName = "Get Gaze World Ray", ToolTip = "Gets gaze ray in world space from the owner's camera"))
	bool GetGazeWorldRay(FVector& OutOrigin, FVector& OutDirection) const;

	/** Checks if tracking is active */
	UFUNCTION(BlueprintPure, Category = "BEAM|Tracking", meta = (DisplayName = "Is Tracking Active", ToolTip = "Checks if eye tracking is currently active"))
	bool IsTrackingActive() const;

	/** Gets current Beam health status */
	UFUNCTION(BlueprintPure, Category = "BEAM|Tracking", meta = (DisplayName = "Get Beam Health", ToolTip = "Gets the current health status of the Beam system"))
	EBeamHealth GetBeamHealth() const;

	/** Gets current data source type */
	UFUNCTION(BlueprintPure, Category = "BEAM|Tracking", meta = (DisplayName = "Get Data Source Type", ToolTip = "Gets the current data source type being used"))
	EBeamDataSourceType GetDataSourceType() const;

	/** Checks if debug HUD is currently enabled */
	UFUNCTION(BlueprintPure, Category = "BEAM|Debug", meta = (DisplayName = "Is Debug HUD Enabled", ToolTip = "Checks if debug HUD is currently enabled"))
	bool IsDebugHUDEnabled() const;

	/** Toggles debug HUD on/off at runtime */
	UFUNCTION(BlueprintCallable, Category = "BEAM|Debug", meta = (DisplayName = "Toggle Debug HUD", ToolTip = "Toggles debug HUD on/off at runtime"))
	void ToggleDebugHUD();

	/** Enables debug HUD at runtime */
	UFUNCTION(BlueprintCallable, Category = "BEAM|Debug", meta = (DisplayName = "Enable Debug HUD", ToolTip = "Enables debug HUD at runtime"))
	void EnableDebugHUD();

	/** Disables debug HUD at runtime */
	UFUNCTION(BlueprintCallable, Category = "BEAM|Debug", meta = (DisplayName = "Disable Debug HUD", ToolTip = "Disables debug HUD at runtime"))
	void DisableDebugHUDRuntime();

	/** Gets filtered gaze point based on quality settings */
	UFUNCTION(BlueprintPure, Category = "BEAM|Tracking", meta = (DisplayName = "Get Filtered Gaze Point", ToolTip = "Gets filtered gaze point based on quality settings"))
	FGazePoint GetFilteredGazePoint() const;

	/** Gets filtered head pose based on quality settings */
	UFUNCTION(BlueprintPure, Category = "BEAM|Tracking", meta = (DisplayName = "Get Filtered Head Pose", ToolTip = "Gets filtered head pose based on quality settings"))
	FHeadPose GetFilteredHeadPose() const;

	/** Checks if current data meets quality thresholds */
	UFUNCTION(BlueprintPure, Category = "BEAM|Tracking", meta = (DisplayName = "Is Data Quality Acceptable", ToolTip = "Checks if current data meets quality thresholds"))
	bool IsDataQualityAcceptable() const;

	/** Gets current buffer utilization percentage */
	UFUNCTION(BlueprintPure, Category = "BEAM|Tracking", meta = (DisplayName = "Get Buffer Utilization", ToolTip = "Gets current buffer utilization percentage"))
	float GetBufferUtilization() const;

	/** Resets all filters and smoothing */
	UFUNCTION(BlueprintCallable, Category = "BEAM|Tracking", meta = (DisplayName = "Reset Filters", ToolTip = "Resets all filters and smoothing to default state"))
	void ResetFilters();

	/** Applies project default settings to this component */
	UFUNCTION(BlueprintCallable, Category = "BEAM|Settings", meta = (DisplayName = "Apply Project Defaults", ToolTip = "Applies project default settings to this component"))
	void ApplyProjectDefaults();

	/** Gets effective settings combining component and project defaults */
	UFUNCTION(BlueprintPure, Category = "BEAM|Settings", meta = (DisplayName = "Get Effective Settings", ToolTip = "Gets effective settings combining component and project defaults"))
	void GetEffectiveSettings(float& OutPollingHz, bool& OutEnableSmoothing, float& OutMinCutoff, float& OutBeta, float& OutTraceDistance) const;

	/** Checks if user is currently being tracked */
	UFUNCTION(BlueprintPure, Category = "BEAM|Tracking", meta = (DisplayName = "Is User Tracked", ToolTip = "Checks if user is currently being tracked"))
	bool IsUserTracked() const;

	/** Gets current gaze point data */
	UFUNCTION(BlueprintPure, Category = "BEAM|Tracking", meta = (DisplayName = "Get Current Gaze Point", ToolTip = "Gets current gaze point data"))
	bool GetCurrentGazePoint(FGazePoint& OutGazePoint) const;

	/** Updates component settings at runtime */
	UFUNCTION(BlueprintCallable, Category = "BEAM|Tracking", meta = (DisplayName = "Update Component Settings", ToolTip = "Updates component settings at runtime"))
	void UpdateComponentSettings();

	// **BEAM|Actions Group** (BEAM|Actions)
	/** Update projection settings from current component configuration */
	UFUNCTION(BlueprintCallable, Category = "BEAM|Actions", meta = (ToolTip = "Update projection settings from current component configuration"))
	void UpdateProjectionSettings();

	/** Test gaze ray tracing with current settings */
	UFUNCTION(BlueprintCallable, Category = "BEAM|Actions", meta = (ToolTip = "Test gaze ray tracing with current settings"))
	void TestGazeRay();

	/** Get current head pose if available */
	UFUNCTION(BlueprintCallable, Category = "BEAM|Data", meta = (ToolTip = "Get current head pose if available"))
	bool GetCurrentHeadPose(FHeadPose& OutHeadPose) const;

private:
	/** Reference to the subsystem */
	UPROPERTY()
	UBeamEyeTrackerSubsystem* Subsystem = nullptr;

	/** Previous health status for change detection */
	EBeamHealth PreviousHealth = EBeamHealth::Error;

	/** Previous gaze point for change detection */
	FGazePoint PreviousGazePoint;

	/** Previous head pose for change detection */
	FHeadPose PreviousHeadPose;

	/** One-Euro filter for gaze smoothing */
	TUniquePtr<FOneEuroFilter> GazeFilter;

	/** One-Euro filter for head pose smoothing */
	TUniquePtr<FOneEuroFilter> HeadPoseFilter;

	/** Frame buffer for storing recent data */
	TUniquePtr<FBeamFrameBuffer> ComponentFrameBuffer;

	/** Cached frame from current tick to avoid redundant subsystem calls */
	FBeamFrame CachedFrame;

	/** Flag indicating if cached frame is valid */
	bool bHasValidCachedFrame = false;
	
	/** Batch processing buffer for performance optimization */
	TArray<FBeamFrame> BatchFrameBuffer;
	
	/** Lock-free queues for high-performance data exchange */
	TUniquePtr<TQueue<FBeamFrame, EQueueMode::Mpsc>> FrameInputQueue;
	TUniquePtr<TQueue<FBeamFrame, EQueueMode::Spsc>> FrameOutputQueue;
	
	/** Triple buffering system for enhanced throughput */
	TUniquePtr<TArray<FBeamFrame>> TripleBuffer[3];
	std::atomic<int32> WriteBufferIndex;
	std::atomic<int32> ReadBufferIndex;
	std::atomic<int32> DisplayBufferIndex;

		/** Performance metrics tracking */
	struct FPerformanceMetrics
	{
		float AverageTickTime = 0.0f;
		float FrameRate = 0.0f;
		int32 FramesProcessed = 0;
		double LastMetricsUpdate = 0.0;
		float AverageProcessingTime = 0.0f;
		float PeakProcessingTime = 0.0f;
	} PerformanceMetrics;

	/** Check if this component should auto-start tracking */
	bool ShouldAutoStartTracking() const;

	/** Update viewport geometry if needed */
	void UpdateViewportGeometry();

	/** Initialize filters and buffers */
	void InitializeFiltersAndBuffers();

	/** Update debug HUD if enabled */
	void UpdateDebugHUD();

	/** Disable debug HUD when component is destroyed */
	void DisableDebugHUD();

	/** Apply gaze smoothing if enabled */
	void ApplyGazeSmoothing(FBeamFrame& Frame);

	/** Deproject gaze point to world coordinates */
	bool DeprojectGazeToWorld(const FGazePoint& GazePoint, FVector& OutWorldLocation, FVector& OutWorldDirection) const;

	/** Apply data quality filtering */
	void ApplyDataQualityFiltering(FBeamFrame& Frame);

	/** Apply outlier detection and removal */
	void ApplyOutlierDetection(FBeamFrame& Frame);

	/** Apply adaptive smoothing based on confidence */
	void ApplyAdaptiveSmoothing(FBeamFrame& Frame);

	/** Check if data meets quality thresholds */
	bool MeetsQualityThresholds(const FBeamFrame& Frame) const;

	/** Update buffer size if needed */
	void UpdateBufferSize();

	/** Update performance metrics */
	void UpdatePerformanceMetrics(float DeltaTime);
	
	/** Update processing performance metrics */
	void UpdateProcessingMetrics(double ProcessingTimeMs);
	
	/** Process batch frames for performance optimization */
	void ProcessBatchFrames();
	
	/** Process batch frames using SIMD optimizations */
	void ProcessBatchFramesSIMD();
	
	/** Process batch frames using standard optimizations */
	void ProcessBatchFramesStandard();
	
	/** Process a single frame with optimizations */
	void ProcessFrameOptimized(FBeamFrame& Frame);
	
	/** Initialize lock-free queues for high-performance data exchange */
	void InitializeLockFreeQueues();
	
	/** Initialize triple buffering system */
	void InitializeTripleBuffering();
	
	/** Process frames using lock-free queue operations */
	void ProcessFramesLockFree();
	
	/** Update performance metrics with Unreal Insights integration */
	void UpdatePerformanceMetricsWithProfiling(float DeltaTime);
	
	/** Advanced performance profiling and bottleneck identification */
	void UpdateAdvancedPerformanceProfiling(float DeltaTime);
	
	/** Check for performance alerts and violations */
	void CheckPerformanceAlerts();
	
	/** Analyze performance bottlenecks in real-time */
	void AnalyzePerformanceBottlenecks();
	
	/** Log advanced performance metrics for debugging */
	void LogAdvancedPerformanceMetrics();
	
	/** Process frame batch using SIMD optimizations */
	void ProcessFrameBatchSIMD(const TArray<FBeamFrame>& InputFrames, TArray<FBeamFrame>& OutputFrames);
	
	/** Internal SIMD processing for frame batches */
	void ProcessFrameBatchSIMDInternal(const FBeamFrame* InputFrames, FBeamFrame* OutputFrames, int32 Count);
	
	/** SIMD-optimized frame copy operation */
	void CopyFrameOptimizedSIMD(const FBeamFrame& Source, FBeamFrame& Destination);

	/** Broadcast health change event if needed */
	void BroadcastHealthChangeIfNeeded();
};

/*=============================================================================
    End of BeamEyeTrackerComponent.h
=============================================================================*/


