#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "BeamEyeTrackerSubsystem.h"
#include "BeamEyeTrackerComponent.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/SpinBox.h"
#include "Components/EditableTextBox.h"
#include "Components/Slider.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "Components/GridPanel.h"

#include "TimerManager.h"
#include "BeamEyeTrackerMonitorWidget.generated.h"

/**
 * Pure C++ UMG Widget for Beam Eye Tracker Monitoring
 * 
 * This widget provides a complete C++ implementation that users can:
 * 1. Use directly in C++ code by creating instances programmatically
 * 2. Extend and customize by inheriting from this class
 * 3. Create Blueprint children from for visual editing
 * 4. Use as a base class for their own implementations
 * 
 * Features:
 * - Real-time eye tracking data display with configurable update rates
 * - Control buttons for tracking, recording, and playback operations
 * - Configurable settings (polling rate, smoothing, filter parameters)
 * - Status monitoring and health checks with visual indicators
 * - File path management for recordings and playback
 * - Automatic widget binding using meta = (BindWidget) system
 * 
 * Usage in Blueprint:
 * 1. Create a Widget Blueprint that inherits from this class
 * 2. Add UI elements with names matching the BindWidget properties
 * 3. Override NativeConstruct() or use Blueprint events for initialization
 * 4. Call control functions from Blueprint events or UI buttons
 * 
 * Usage in C++:
 * 1. Create widget instance: CreateWidget<UBeamEyeTrackerMonitorWidget>(World)
 * 2. Add to viewport: AddToViewport() or AddToPlayerScreen()
 * 3. Call control functions directly or bind to UI events
 * 4. Use getter functions to access current tracking data
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Beam Eye Tracker Monitor"))
class BEAMEYETRACKEREDITOR_API UBeamEyeTrackerMonitorWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UBeamEyeTrackerMonitorWidget(const FObjectInitializer& ObjectInitializer);

	// ============================================================================
	// BLUEPRINT-CALLABLE CONTROL FUNCTIONS
	// ============================================================================
	
	/** Starts eye tracking - initializes the Beam SDK and begins data collection */
	UFUNCTION(BlueprintCallable, Category = "Beam Eye Tracker|Control", meta = (DisplayName = "Start Tracking", ToolTip = "Starts eye tracking. Initializes the Beam SDK and begins data collection. Call this when you want to begin tracking user's eye movements and head pose."))
	void StartTracking();

	/** Stops eye tracking - shuts down the Beam SDK and stops data collection */
	UFUNCTION(BlueprintCallable, Category = "Beam Eye Tracker|Control", meta = (DisplayName = "Stop Tracking", ToolTip = "Stops eye tracking. Shuts down the Beam SDK and stops data collection. Call this when you want to stop tracking or before shutting down."))
	void StopTracking();

	/** Resets eye tracking calibration - clears all calibration data for fresh start */
	UFUNCTION(BlueprintCallable, Category = "Beam Eye Tracker|Control", meta = (DisplayName = "Reset Calibration", ToolTip = "Resets eye tracking calibration. Clears all calibration data and returns system to uncalibrated state. Use this when switching users or if calibration quality degrades."))
	void ResetCalibration();

	/** Starts recording tracking data - captures all tracking data to CSV file for analysis */
	UFUNCTION(BlueprintCallable, Category = "Beam Eye Tracker|Recording", meta = (DisplayName = "Start Recording", ToolTip = "Starts recording tracking data. Captures all tracking data to CSV file for analysis. FilePath is optional - uses default if empty. Returns true if recording started successfully."))
	void StartRecording(const FString& FilePath = TEXT(""));

	/** Stops recording tracking data - completes the recording session and saves the file */
	UFUNCTION(BlueprintCallable, Category = "Beam Eye Tracker|Recording", meta = (DisplayName = "Stop Recording", ToolTip = "Stops recording tracking data. Completes the current recording session and saves the file. Call this when you want to stop capturing data."))
	void StopRecording();

	/** Starts playback of recorded data - replays previously recorded tracking data */
	UFUNCTION(BlueprintCallable, Category = "Beam Eye Tracker|Playback", meta = (DisplayName = "Start Playback", ToolTip = "Starts playback of recorded data. Replays previously recorded tracking data from CSV file. FilePath is optional - uses default if empty. Useful for development and testing."))
	void StartPlayback(const FString& FilePath = TEXT(""));

	/** Stops playback of recorded data - ends the playback session */
	UFUNCTION(BlueprintCallable, Category = "Beam Eye Tracker|Playback", meta = (DisplayName = "Stop Playback", ToolTip = "Stops playback of recorded data. Ends the current playback session. Call this when you want to stop replaying data."))
	void StopPlayback();

	// ============================================================================
	// BLUEPRINT-CALLABLE GETTER FUNCTIONS
	// ============================================================================
	
	/** Checks if eye tracking is currently active - useful for UI state management */
	UFUNCTION(BlueprintPure, Category = "Beam Eye Tracker|Status", meta = (DisplayName = "Is Tracking", ToolTip = "Checks if eye tracking is currently active. Returns true when tracking is running and data is being collected. Use this to update UI elements or gate gameplay features."))
	bool IsTracking() const;

	/** Checks if recording is currently active - useful for UI state management */
	UFUNCTION(BlueprintPure, Category = "Beam Eye Tracker|Status", meta = (DisplayName = "Is Recording", ToolTip = "Checks if recording is currently active. Returns true when recording is in progress. Use this to update UI elements or prevent starting multiple recordings."))
	bool IsRecording() const;

	/** Checks if playback is currently active - useful for UI state management */
	UFUNCTION(BlueprintPure, Category = "Beam Eye Tracker|Status", meta = (DisplayName = "Is Playing Back", ToolTip = "Checks if playback is currently active. Returns true when playback is in progress. Use this to update UI elements or prevent starting multiple playbacks."))
	bool IsPlayingBack() const;

	/** Gets current tracking FPS - useful for performance monitoring and debugging */
	UFUNCTION(BlueprintPure, Category = "Beam Eye Tracker|Status", meta = (DisplayName = "Get Current FPS", ToolTip = "Gets current tracking FPS. Returns the actual frame rate of tracking data. Use this for performance monitoring, debugging, or displaying real-time statistics to users."))
	float GetCurrentFPS() const;

	/** Gets current buffer utilization - indicates data processing efficiency */
	UFUNCTION(BlueprintPure, Category = "Beam Eye Tracker|Status", meta = (DisplayName = "Get Buffer Utilization", ToolTip = "Gets current buffer utilization. Returns percentage of buffer that is filled with data. High utilization may indicate performance issues or data processing bottlenecks."))
	int32 GetBufferUtilization() const;

	/** Gets current gaze point coordinates - returns screen-space gaze position */
	UFUNCTION(BlueprintPure, Category = "Beam Eye Tracker|Data", meta = (DisplayName = "Get Current Gaze Point", ToolTip = "Gets current gaze point coordinates. Returns screen-space gaze position as FVector2D. Use this for UI interactions, gaze-based gameplay, or visual feedback."))
	FVector2D GetCurrentGazePoint() const;

	/** Gets current head position - returns 3D head position in centimeters */
	UFUNCTION(BlueprintPure, Category = "Beam Eye Tracker|Data", meta = (DisplayName = "Get Current Head Position", ToolTip = "Gets current head position. Returns 3D head position in centimeters relative to the tracking reference frame. Use this for VR applications or head pose tracking."))
	FVector GetCurrentHeadPosition() const;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	/** Update the monitor display */
	void UpdateMonitor();
	
	/** Initialize widget bindings */
	void InitializeWidgetBindings();
	
	/** Initialize default values */
	void InitializeDefaults();

	// Event handlers (Blueprint-callable)
	UFUNCTION()
	void OnStartTrackingClicked();
	
	UFUNCTION()
	void OnStopTrackingClicked();
	
	UFUNCTION()
	void OnResetCalibrationClicked();
	
	UFUNCTION()
	void OnStartRecordingClicked();
	
	UFUNCTION()
	void OnStopRecordingClicked();
	
	UFUNCTION()
	void OnStartPlaybackClicked();
	
	UFUNCTION()
	void OnStopPlaybackClicked();
	
	UFUNCTION()
	void OnDebugHUDToggled(bool bIsChecked);
	
	UFUNCTION()
	void OnPollingRateChanged(float NewValue);
	
	UFUNCTION()
	void OnSmoothingToggled(bool bIsChecked);
	
	UFUNCTION()
	void OnMinCutoffChanged(float NewValue);
	
	UFUNCTION()
	void OnBetaChanged(float NewValue);
	
	UFUNCTION()
	void OnRecordingPathChanged(const FText& NewText);
	
	UFUNCTION()
	void OnPlaybackPathChanged(const FText& NewText);

	// Status text getters
	FText GetTrackingStatusText() const;
	FText GetHealthStatusText() const;
	FText GetFPSText() const;
	FText GetGazePointText() const;
	FText GetBufferUtilizationText() const;
	FText GetHeadPoseText() const;
	FText GetCalibrationStatusText() const;
	FText GetRecordingStatusText() const;
	FText GetPlaybackStatusText() const;

private:
	/** Timer handle for updates */
	FTimerHandle UpdateTimerHandle;
	
	/** Update interval in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam Eye Tracker|Settings", meta = (AllowPrivateAccess = "true"))
	float UpdateInterval = 0.1f;
	
	/** Auto-start tracking when widget is created */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam Eye Tracker|Settings", meta = (AllowPrivateAccess = "true"))
	bool bAutoStartTracking = false;
	
	/** Show debug information */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam Eye Tracker|Settings", meta = (AllowPrivateAccess = "true"))
	bool bShowDebugInfo = true;
	
	/** Widget references for updating */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* StatusText;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* GazeDataText;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* HeadDataText;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* HealthText;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* FPSText;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* BufferText;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* CalibrationText;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* RecordingText;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlaybackText;
	
	/** Control widgets */
	UPROPERTY(meta = (BindWidget))
	UCheckBox* DebugHUDCheckBox;
	
	UPROPERTY(meta = (BindWidget))
	USpinBox* PollingRateSpinBox;
	
	UPROPERTY(meta = (BindWidget))
	UCheckBox* SmoothingCheckBox;
	
	UPROPERTY(meta = (BindWidget))
	USpinBox* MinCutoffSpinBox;
	
	UPROPERTY(meta = (BindWidget))
	USpinBox* BetaSpinBox;
	
	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* RecordingPathTextBox;
	
	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* PlaybackPathTextBox;
	
	/** Current recording path */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam Eye Tracker|Paths", meta = (AllowPrivateAccess = "true"))
	FString CurrentRecordingPath;
	
	/** Current playback path */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam Eye Tracker|Paths", meta = (AllowPrivateAccess = "true"))
	FString CurrentPlaybackPath;
	
	/** Default recording directory */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam Eye Tracker|Paths", meta = (AllowPrivateAccess = "true"))
	FString DefaultRecordingDirectory = TEXT("BeamRecordings");
};
