/*=============================================================================
    BeamAnalyticsSubsystem.h: Advanced Analytics Subsystem for Beam SDK.

    Provides comprehensive analytics, calibration monitoring, and performance metrics
    for eye tracking applications. Works alongside the main tracking subsystem
    to deliver insights into user behavior and system performance.

    Copyright (c) 2025 Eyeware Tech SA. All Rights Reserved.

    Eyeware® and Beam® are registered trademarks of Eyeware Tech SA.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "BeamEyeTrackerTypes.h"
#include "BeamAnalyticsSubsystem.generated.h"

class UBeamEyeTrackerSubsystem;

/** Advanced Analytics Subsystem for Beam Eye Tracker */
UCLASS(DisplayName = "Beam Analytics Subsystem")
class BEAMEYETRACKER_API UBeamAnalyticsSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBeamAnalyticsSubsystem();

    // Lifecycle
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Gaze Analytics
    UFUNCTION(BlueprintCallable, Category = "Beam|Analytics", meta = (DisplayName = "Start Gaze Analytics", ToolTip = "Begin collecting gaze analytics data"))
    void StartGazeAnalytics();

    UFUNCTION(BlueprintCallable, Category = "Beam|Analytics", meta = (DisplayName = "Stop Gaze Analytics", ToolTip = "Stop collecting gaze analytics data"))
    void StopGazeAnalytics();

    UFUNCTION(BlueprintCallable, Category = "Beam|Analytics", meta = (DisplayName = "Get Gaze Analytics", ToolTip = "Get current gaze analytics data"))
    FGazeAnalytics GetGazeAnalytics() const;

    UFUNCTION(BlueprintCallable, Category = "Beam|Analytics", meta = (DisplayName = "Reset Gaze Analytics", ToolTip = "Clear all collected gaze analytics data"))
    void ResetGazeAnalytics();

    // Calibration Quality
    UFUNCTION(BlueprintCallable, Category = "Beam|Calibration", meta = (DisplayName = "Get Calibration Quality", ToolTip = "Get current calibration quality assessment"))
    FCalibrationQuality GetCalibrationQuality() const;

    UFUNCTION(BlueprintCallable, Category = "Beam|Calibration", meta = (DisplayName = "Assess Calibration", ToolTip = "Perform a comprehensive calibration assessment"))
    void AssessCalibration();

    UFUNCTION(BlueprintCallable, Category = "Beam|Calibration", meta = (DisplayName = "Get Calibration Score", ToolTip = "Get overall calibration quality score (0-100)"))
    float GetCalibrationScore() const;

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Beam|Performance", meta = (DisplayName = "Start Performance Monitoring", ToolTip = "Begin monitoring system performance metrics"))
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Beam|Performance", meta = (DisplayName = "Stop Performance Monitoring", ToolTip = "Stop monitoring system performance metrics"))
    void StopPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Beam|Performance", meta = (DisplayName = "Get Performance Metrics", ToolTip = "Get current performance metrics"))
    FBeamPerformanceMetrics GetPerformanceMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Beam|Performance", meta = (DisplayName = "Get Performance Score", ToolTip = "Get overall performance score (0-100)"))
    float GetPerformanceScore() const;

    // Data Export
    UFUNCTION(BlueprintCallable, Category = "Beam|Export", meta = (DisplayName = "Export Analytics Data", ToolTip = "Export analytics data to CSV file"))
    bool ExportAnalyticsData(const FString& FilePath);

    UFUNCTION(BlueprintCallable, Category = "Beam|Export", meta = (DisplayName = "Export Performance Data", ToolTip = "Export performance data to CSV file"))
    bool ExportPerformanceData(const FString& FilePath);

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Beam|Config", meta = (DisplayName = "Set Analytics Settings", ToolTip = "Configure analytics collection settings"))
    void SetAnalyticsSettings(float SamplingRate, float MinFixationDuration, float MaxGapTime);

    UFUNCTION(BlueprintCallable, Category = "Beam|Config", meta = (DisplayName = "Get Analytics Settings", ToolTip = "Get current analytics configuration"))
    void GetAnalyticsSettings(float& OutSamplingRate, float& OutMinFixationDuration, float& OutMaxGapTime) const;

    // Events
    UFUNCTION(BlueprintImplementableEvent, Category = "Beam|Events", meta = (DisplayName = "On Analytics Updated", ToolTip = "Called when analytics data is updated"))
    void OnAnalyticsUpdated(const FGazeAnalytics& Analytics);

    UFUNCTION(BlueprintImplementableEvent, Category = "Beam|Events", meta = (DisplayName = "On Calibration Assessed", ToolTip = "Called when calibration assessment is complete"))
    void OnCalibrationAssessed(const FCalibrationQuality& Quality);

    UFUNCTION(BlueprintImplementableEvent, Category = "Beam|Events", meta = (DisplayName = "On Performance Updated", ToolTip = "Called when performance metrics are updated"))
    void OnPerformanceUpdated(const FBeamPerformanceMetrics& Metrics);

protected:
    // Blueprint events
    UFUNCTION(BlueprintImplementableEvent, Category = "Beam|Events", meta = (DisplayName = "On Analytics Started", ToolTip = "Called when analytics collection begins"))
    void OnAnalyticsStarted();

    UFUNCTION(BlueprintImplementableEvent, Category = "Beam|Events", meta = (DisplayName = "On Analytics Stopped", ToolTip = "Called when analytics collection stops"))
    void OnAnalyticsStopped();

    UFUNCTION(BlueprintImplementableEvent, Category = "Beam|Events", meta = (DisplayName = "On Performance Monitoring Started", ToolTip = "Called when performance monitoring begins"))
    void OnPerformanceMonitoringStarted();

    UFUNCTION(BlueprintImplementableEvent, Category = "Beam|Events", meta = (DisplayName = "On Performance Monitoring Stopped", ToolTip = "Called when performance monitoring stops"))
    void OnPerformanceMonitoringStopped();

private:
    UPROPERTY()
    UBeamEyeTrackerSubsystem* BeamSubsystem;

    // Analytics state
    bool bAnalyticsActive;
    bool bPerformanceMonitoringActive;
    
    // Configuration
    float SamplingRate;
    float MinFixationDuration;
    float MaxGapTime;

    // Data storage
    FGazeAnalytics CurrentAnalytics;
    FCalibrationQuality CurrentCalibrationQuality;
    FBeamPerformanceMetrics CurrentPerformanceMetrics;

    // Internal tracking
    TArray<FVector2D> GazeHistory;
    TArray<float> GazeTimestamps;
    float LastUpdateTime;

    // Helper functions
    void UpdateGazeAnalytics();
    void UpdatePerformanceMetrics();
    void CalculateFixations();
    void CalculateSaccades();
    float CalculateDistance(const FVector2D& Point1, const FVector2D& Point2);
    void CleanupOldData();
};

/*=============================================================================
    End of BeamAnalyticsSubsystem.h
=============================================================================*/
