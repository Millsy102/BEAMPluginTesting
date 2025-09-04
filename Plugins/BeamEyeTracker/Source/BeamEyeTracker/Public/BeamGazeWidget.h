/*=============================================================================
    BeamGazeWidget.h: Enhanced UMG widget for real-time gaze visualization.

    Provides a complete visual interface for monitoring eye tracking data,
    including gaze point visualization, tracking status, and performance metrics.
    Features configurable display modes and automatic data binding.

    Copyright (c) 2025 Eyeware Tech SA. All Rights Reserved.

    Eyeware® and Beam® are registered trademarks of Eyeware Tech SA.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BeamEyeTrackerSubsystem.h"
#include "BeamGazeWidget.generated.h"

class UImage;
class UTextBlock;
class UProgressBar;
class UHorizontalBox;
class UVerticalBox;

/** Display mode for the gaze widget */
UENUM(BlueprintType)
enum class EDisplayMode : uint8
{
    /** Compact mode - minimal information */
    Compact UMETA(DisplayName = "Compact"),
    
    /** Detailed mode - standard information */
    Detailed UMETA(DisplayName = "Detailed"),
    
    /** Expert mode - all information */
    Expert UMETA(DisplayName = "Expert")
};

/** Widget display status for eye tracking system */
UENUM(BlueprintType)
enum class EBeamWidgetStatus : uint8
{
    /** Not tracking */
    NotTracking UMETA(DisplayName = "Not Tracking"),
    
    /** Initializing */
    Initializing UMETA(DisplayName = "Initializing"),
    
    /** Tracking active */
    Tracking UMETA(DisplayName = "Tracking"),
    
    /** Tracking with low confidence */
    LowConfidence UMETA(DisplayName = "Low Confidence"),
    
    /** Error state */
    Error UMETA(DisplayName = "Error")
};

/** Enhanced UMG widget for real-time gaze visualization and eye tracking status */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Beam Gaze Widget"))
class BEAMEYETRACKER_API UBeamGazeWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UBeamGazeWidget(const FObjectInitializer& ObjectInitializer);

    //~ Begin UUserWidget Interface
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
    virtual void NativeDestruct() override;
    //~ End UUserWidget Interface

    /** Shows gaze point on screen */
    UPROPERTY(meta = (BindWidget))
    class UImage* GazeIndicator;

    /** Displays tracking status */
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* StatusText;

    /** Shows calibration quality */
    UPROPERTY(meta = (BindWidget))
    class UProgressBar* CalibrationQuality;

    /** Shows current FPS */
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* FPSText;

    /** Shows tracking confidence */
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* ConfidenceText;

    /** Shows head pose information */
    UPROPERTY(meta = (BindWidget))
    class UVerticalBox* HeadPoseInfo;

    /** Shows gaze coordinates */
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* GazeCoordinatesText;

    /** Shows performance metrics */
    UPROPERTY(meta = (BindWidget))
    class UVerticalBox* PerformanceMetrics;

    /** Shows error messages */
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* ErrorText;

public:
    /** Enable/disable gaze visualization */
    UFUNCTION(BlueprintCallable, Category = "Beam|Display",
              meta = (DisplayName = "Set Gaze Visualization", ToolTip = "Enable or disable the gaze point indicator"))
    void SetGazeVisualization(bool bEnable);

    /** Set display mode (compact, detailed, expert) */
    UFUNCTION(BlueprintCallable, Category = "Beam|Display",
              meta = (DisplayName = "Set Display Mode", ToolTip = "Change the amount of information displayed"))
    void SetDisplayMode(EDisplayMode Mode);

    /** Set update frequency for widget updates */
    UFUNCTION(BlueprintCallable, Category = "Beam|Performance",
              meta = (DisplayName = "Set Update Frequency", ToolTip = "How often the widget updates (Hz)"))
    void SetUpdateFrequency(float Frequency);

    /** Get current tracking status */
    UFUNCTION(BlueprintPure, Category = "Beam|Status",
              meta = (DisplayName = "Get Tracking Status", ToolTip = "Get current eye tracking status"))
    EBeamWidgetStatus GetTrackingStatus() const;

    /** Get current gaze point */
    UFUNCTION(BlueprintPure, Category = "Beam|Data",
              meta = (DisplayName = "Get Current Gaze", ToolTip = "Get current gaze point in screen coordinates"))
    FVector2D GetCurrentGaze() const;

    /** Get current confidence level */
    UFUNCTION(BlueprintPure, Category = "Beam|Data",
              meta = (DisplayName = "Get Current Confidence", ToolTip = "Get current tracking confidence (0-1)"))
    float GetCurrentConfidence() const;

protected:
    /** Called when tracking status changes */
    UFUNCTION(BlueprintImplementableEvent, Category = "Beam|Events",
              meta = (DisplayName = "On Tracking Status Changed", ToolTip = "Called when eye tracking status changes"))
    void OnTrackingStatusChanged(EBeamWidgetStatus NewStatus, EBeamWidgetStatus OldStatus);

    /** Called when new gaze data is received */
    UFUNCTION(BlueprintImplementableEvent, Category = "Beam|Events",
              meta = (DisplayName = "On Gaze Data Updated", ToolTip = "Called when new gaze data is received"))
    void OnGazeDataUpdated(const FVector2D& GazePoint, float Confidence);

    /** Called when confidence changes significantly */
    UFUNCTION(BlueprintImplementableEvent, Category = "Beam|Events",
              meta = (DisplayName = "On Confidence Changed", ToolTip = "Called when confidence changes by more than threshold"))
    void OnConfidenceChanged(float NewConfidence, float OldConfidence);

private:
    /** Reference to the eye tracking subsystem */
    UPROPERTY()
    UBeamEyeTrackerSubsystem* BeamSubsystem;

    /** Current display mode */
    EDisplayMode CurrentDisplayMode;

    /** Update frequency in Hz */
    float UpdateFrequency;

    /** Last update time */
    float LastUpdateTime;

    /** Current tracking status */
    EBeamWidgetStatus CurrentTrackingStatus;

    /** Current gaze data */
    FVector2D CurrentGazePoint;
    float CurrentConfidence;
    float CurrentFPS;

    /** Performance tracking */
    float LastFPSUpdateTime;
    int32 FrameCount;

    /** Update all widget elements */
    void UpdateWidget();

    /** Update gaze indicator position */
    void UpdateGazeIndicator();

    /** Update status text */
    void UpdateStatusText();

    /** Update calibration quality bar */
    void UpdateCalibrationQuality();

    /** Update FPS display */
    void UpdateFPSDisplay();

    /** Update confidence display */
    void UpdateConfidenceDisplay();

    /** Update head pose information */
    void UpdateHeadPoseInfo();

    /** Update gaze coordinates */
    void UpdateGazeCoordinates();

    /** Update performance metrics */
    void UpdatePerformanceMetrics();

    /** Update error display */
    void UpdateErrorDisplay();

    /** Check if it's time to update */
    bool ShouldUpdate() const;

    /** Calculate FPS */
    void CalculateFPS();

    /** Get status text color based on tracking status */
    FLinearColor GetStatusColor(EBeamWidgetStatus Status) const;

    /** Get confidence color based on confidence level */
    FLinearColor GetConfidenceColor(float Confidence) const;
};

/*=============================================================================
    End of BeamGazeWidget.h
=============================================================================*/
