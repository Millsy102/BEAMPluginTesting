/*=============================================================================
    BeamCalibrationWidget.h: Advanced Calibration Widget for Beam SDK.

    Provides an interactive calibration interface with visual feedback,
    quality assessment, and real-time monitoring. Guides users through
    the calibration process with immediate feedback on calibration quality.

    Copyright (c) 2025 Eyeware Tech SA. All Rights Reserved.

    Eyeware® and Beam® are registered trademarks of Eyeware Tech SA.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BeamEyeTrackerTypes.h"
#include "BeamCalibrationWidget.generated.h"

class UImage;
class UTextBlock;
class UProgressBar;
class UButton;
class UCanvasPanel;
class UBeamEyeTrackerSubsystem;
class UBeamAnalyticsSubsystem;

/** Advanced Calibration Widget for Beam Eye Tracker */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Beam Calibration Widget"))
class BEAMEYETRACKER_API UBeamCalibrationWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UBeamCalibrationWidget(const FObjectInitializer& ObjectInitializer);

    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
    virtual void NativeDestruct() override;

    // UI Elements (BindWidget)
    UPROPERTY(meta = (BindWidget))
    class UCanvasPanel* CalibrationCanvas;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* StatusText;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* QualityText;

    UPROPERTY(meta = (BindWidget))
    class UProgressBar* QualityProgressBar;

    UPROPERTY(meta = (BindWidget))
    class UButton* StartCalibrationButton;

    UPROPERTY(meta = (BindWidget))
    class UButton* StopCalibrationButton;

    UPROPERTY(meta = (BindWidget))
    class UButton* AssessQualityButton;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* InstructionsText;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* StatisticsText;

    // Calibration Control
    UFUNCTION(BlueprintCallable, Category = "Beam|Calibration", meta = (DisplayName = "Start Calibration", ToolTip = "Begin the calibration process"))
    void StartCalibration();

    UFUNCTION(BlueprintCallable, Category = "Beam|Calibration", meta = (DisplayName = "Stop Calibration", ToolTip = "Stop the current calibration process"))
    void StopCalibration();

    UFUNCTION(BlueprintCallable, Category = "Beam|Calibration", meta = (DisplayName = "Assess Quality", ToolTip = "Assess the current calibration quality"))
    void AssessQuality();

    UFUNCTION(BlueprintCallable, Category = "Beam|Calibration", meta = (DisplayName = "Reset Calibration", ToolTip = "Reset all calibration data"))
    void ResetCalibration();

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Beam|Config", meta = (DisplayName = "Set Calibration Points", ToolTip = "Set the number and positions of calibration points"))
    void SetCalibrationPoints(const TArray<FVector2D>& Points);

    UFUNCTION(BlueprintCallable, Category = "Beam|Config", meta = (DisplayName = "Set Point Duration", ToolTip = "Set how long each calibration point is displayed"))
    void SetPointDuration(float Duration);

    UFUNCTION(BlueprintCallable, Category = "Beam|Config", meta = (DisplayName = "Get Calibration Settings", ToolTip = "Get current calibration configuration"))
    void GetCalibrationSettings(TArray<FVector2D>& OutPoints, float& OutDuration) const;

    // Data Access
    UFUNCTION(BlueprintCallable, Category = "Beam|Data", meta = (DisplayName = "Get Calibration Quality", ToolTip = "Get current calibration quality data"))
    FCalibrationQuality GetCalibrationQuality() const;

    UFUNCTION(BlueprintCallable, Category = "Beam|Data", meta = (DisplayName = "Get Calibration Score", ToolTip = "Get overall calibration quality score"))
    float GetCalibrationScore() const;

    UFUNCTION(BlueprintCallable, Category = "Beam|Data", meta = (DisplayName = "Is Calibrated", ToolTip = "Check if the system is currently calibrated"))
    bool IsCalibrated() const;

    // Events
    UFUNCTION(BlueprintImplementableEvent, Category = "Beam|Events", meta = (DisplayName = "On Calibration Started", ToolTip = "Called when calibration begins"))
    void OnCalibrationStarted();

    UFUNCTION(BlueprintImplementableEvent, Category = "Beam|Events", meta = (DisplayName = "On Calibration Completed", ToolTip = "Called when calibration is completed"))
    void OnCalibrationCompleted();

    UFUNCTION(BlueprintImplementableEvent, Category = "Beam|Events", meta = (DisplayName = "On Calibration Failed", ToolTip = "Called when calibration fails"))
    void OnCalibrationFailed();

    UFUNCTION(BlueprintImplementableEvent, Category = "Beam|Events", meta = (DisplayName = "On Quality Assessed", ToolTip = "Called when quality assessment is complete"))
    void OnQualityAssessed(const FCalibrationQuality& Quality);

protected:
    // Blueprint events
    UFUNCTION(BlueprintImplementableEvent, Category = "Beam|Events", meta = (DisplayName = "On Point Activated", ToolTip = "Called when a calibration point becomes active"))
    void OnPointActivated(int32 PointIndex, const FVector2D& Position);

    UFUNCTION(BlueprintImplementableEvent, Category = "Beam|Events", meta = (DisplayName = "On Point Completed", ToolTip = "Called when a calibration point is completed"))
    void OnPointCompleted(int32 PointIndex, float Quality);

    UFUNCTION(BlueprintImplementableEvent, Category = "Beam|Events", meta = (DisplayName = "On Progress Updated", ToolTip = "Called when calibration progress updates"))
    void OnProgressUpdated(float Progress, int32 CurrentPoint, int32 TotalPoints);

private:
    UPROPERTY()
    UBeamEyeTrackerSubsystem* BeamSubsystem;

    UPROPERTY()
    UBeamAnalyticsSubsystem* AnalyticsSubsystem;

    // Calibration state
    bool bCalibrationActive;
    int32 CurrentPointIndex;
    float PointStartTime;
    float PointDuration;
    float TotalCalibrationTime;

    // Configuration
    TArray<FVector2D> CalibrationPoints;
    TArray<float> PointQualities;
    TArray<UImage*> PointImages;

    // Statistics
    int32 SuccessfulPoints;
    int32 FailedPoints;
    float AverageQuality;
    float LastCalibrationTime;

    // Calibration quality data
    FCalibrationQuality CurrentCalibrationQuality;

    // Helper functions
    void UpdateCalibrationDisplay();
    void ActivateNextPoint();
    void CompleteCurrentPoint();
    void CalculateOverallQuality();
    void UpdateStatistics();
    void CreatePointVisuals();
    void DestroyPointVisuals();
    void UpdateInstructions();
    void UpdateStatusDisplay();
    float CalculatePointQuality(const FVector2D& TargetPoint, const FVector2D& GazePoint);
    bool IsGazeOnTarget(const FVector2D& TargetPoint, const FVector2D& GazePoint, float Threshold = 0.05f);
};

/*=============================================================================
    End of BeamCalibrationWidget.h
=============================================================================*/
