/*=============================================================================
    BeamEyeTrackingComponent.h: Enhanced eye tracking component for Beam SDK.

    Provides a simple, out-of-the-box way to add eye tracking to any actor.
    Automatically initializes eye tracking on BeginPlay and manages updates
    throughout the actor's lifetime with performance optimization options.

    Copyright (c) 2025 Eyeware Tech SA. All Rights Reserved.

    Eyeware® and Beam® are registered trademarks of Eyeware Tech SA.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BeamEyeTrackerSubsystem.h"
#include "BeamEyeTrackingComponent.generated.h"

/** Enhanced eye tracking component that automatically manages eye tracking lifecycle */
UCLASS(ClassGroup = "Beam Eye Tracking", meta = (BlueprintSpawnableComponent), DisplayName = "Beam Eye Tracking Component")
class BEAMEYETRACKER_API UBeamEyeTrackingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UBeamEyeTrackingComponent();

    //~ Begin UActorComponent Interface
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    //~ End UActorComponent Interface

    /** Auto-initializes eye tracking on BeginPlay */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam|Setup", 
               meta = (DisplayName = "Auto Initialize", ToolTip = "Automatically initialize eye tracking when component begins play"))
    bool bAutoInitialize = true;

    /** Automatically updates gaze data */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam|Setup",
               meta = (DisplayName = "Auto Update", ToolTip = "Automatically update gaze data during tick"))
    bool bAutoUpdate = true;

    /** Update frequency in Hz (1-1000) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam|Setup", 
               meta = (ClampMin = "1", ClampMax = "1000", DisplayName = "Update Frequency (Hz)", 
                       ToolTip = "How often to update gaze data. Higher values = more responsive but more CPU usage"))
    float UpdateFrequency = 60.0f;

    /** Enable performance optimization mode */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam|Performance",
               meta = (DisplayName = "Performance Mode", ToolTip = "Enable performance optimizations for high-frequency tracking"))
    bool bPerformanceMode = false;

    /** Maximum update frequency when in performance mode */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam|Performance",
               meta = (EditCondition = "bPerformanceMode", ClampMin = "30", ClampMax = "1000", 
                       DisplayName = "Max Performance Frequency", ToolTip = "Maximum update frequency in performance mode"))
    float MaxPerformanceFrequency = 120.0f;

    /** Enable debug visualization */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam|Debug",
               meta = (DisplayName = "Debug Visualization", ToolTip = "Show debug information and gaze visualization"))
    bool bDebugVisualization = false;

public:
    /** Manually initialize eye tracking */
    UFUNCTION(BlueprintCallable, Category = "Beam|Control",
              meta = (DisplayName = "Initialize Eye Tracking", ToolTip = "Manually initialize eye tracking. Called automatically if Auto Initialize is enabled"))
    bool InitializeEyeTracking();

    /** Manually stop eye tracking */
    UFUNCTION(BlueprintCallable, Category = "Beam|Control",
              meta = (DisplayName = "Stop Eye Tracking", ToolTip = "Manually stop eye tracking and cleanup resources"))
    void StopEyeTracking();

    /** Check if eye tracking is active */
    UFUNCTION(BlueprintPure, Category = "Beam|Status",
              meta = (DisplayName = "Is Eye Tracking Active", ToolTip = "Returns true if eye tracking is currently active and receiving data"))
    bool IsEyeTrackingActive() const;

    /** Get current gaze point in screen coordinates */
    UFUNCTION(BlueprintPure, Category = "Beam|Data",
              meta = (DisplayName = "Get Gaze Point 2D", ToolTip = "Get current gaze point in screen coordinates (0-1 range)"))
    FVector2D GetGazePoint2D() const;

    /** Get current gaze point in world coordinates */
    UFUNCTION(BlueprintPure, Category = "Beam|Data",
              meta = (DisplayName = "Get Gaze Point 3D", ToolTip = "Get current gaze point projected into world space"))
    FVector GetGazePoint3D() const;

    /** Get current head pose transform */
    UFUNCTION(BlueprintPure, Category = "Beam|Data",
              meta = (DisplayName = "Get Head Pose", ToolTip = "Get current head position and rotation"))
    FTransform GetHeadPose() const;

    /** Get tracking confidence (0-1) */
    UFUNCTION(BlueprintPure, Category = "Beam|Data",
              meta = (DisplayName = "Get Tracking Confidence", ToolTip = "Get confidence level of current tracking data (0 = no confidence, 1 = high confidence)"))
    float GetTrackingConfidence() const;

    /** Get current tracking FPS */
    UFUNCTION(BlueprintPure, Category = "Beam|Performance",
              meta = (DisplayName = "Get Tracking FPS", ToolTip = "Get current frames per second of eye tracking data"))
    float GetTrackingFPS() const;

    /** Set update frequency dynamically */
    UFUNCTION(BlueprintCallable, Category = "Beam|Control",
              meta = (DisplayName = "Set Update Frequency", ToolTip = "Change update frequency during runtime"))
    void SetUpdateFrequency(float NewFrequency);

    /** Enable/disable performance mode dynamically */
    UFUNCTION(BlueprintCallable, Category = "Beam|Control",
              meta = (DisplayName = "Set Performance Mode", ToolTip = "Enable or disable performance optimization mode"))
    void SetPerformanceMode(bool bEnable);

protected:
    /** Called when eye tracking is successfully initialized */
    UFUNCTION(BlueprintImplementableEvent, Category = "Beam|Events",
              meta = (DisplayName = "On Eye Tracking Initialized", ToolTip = "Called when eye tracking is successfully initialized"))
    void OnEyeTrackingInitialized();

    /** Called when eye tracking is stopped */
    UFUNCTION(BlueprintImplementableEvent, Category = "Beam|Events",
              meta = (DisplayName = "On Eye Tracking Stopped", ToolTip = "Called when eye tracking is stopped"))
    void OnEyeTrackingStopped();

    /** Called when new gaze data is received */
    UFUNCTION(BlueprintImplementableEvent, Category = "Beam|Events",
              meta = (DisplayName = "On Gaze Data Updated", ToolTip = "Called when new gaze data is received"))
    void OnGazeDataUpdated(const FVector2D& GazePoint2D, const FVector& GazePoint3D, float Confidence);

    /** Called when tracking confidence changes significantly */
    UFUNCTION(BlueprintImplementableEvent, Category = "Beam|Events",
              meta = (DisplayName = "On Confidence Changed", ToolTip = "Called when tracking confidence changes by more than 0.1"))
    void OnConfidenceChanged(float NewConfidence, float OldConfidence);

private:
    /** Reference to the eye tracking subsystem */
    UPROPERTY()
    UBeamEyeTrackerSubsystem* BeamSubsystem;

    /** Timer handle for update frequency control */
    FTimerHandle UpdateTimer;

    /** Current tracking status */
    bool bTrackingActive;

    /** Last received gaze data */
    FVector2D LastGazePoint2D;
    FVector LastGazePoint3D;
    FTransform LastHeadPose;
    float LastConfidence;

    /** Performance tracking */
    float LastUpdateTime;
    int32 FrameCount;
    float CurrentFPS;

    /** Update gaze data from subsystem */
    void UpdateGazeData();

    /** Calculate current FPS */
    void UpdateFPS();

    /** Check if it's time to update based on frequency */
    bool ShouldUpdate() const;

    /** Apply performance optimizations */
    void ApplyPerformanceOptimizations();

    /** Validate and clamp update frequency */
    float ValidateUpdateFrequency(float Frequency) const;
};

/*=============================================================================
    End of BeamEyeTrackingComponent.h
=============================================================================*/
