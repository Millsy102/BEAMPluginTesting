/*=============================================================================
    BeamDebugHUD.h: Debug HUD for Beam Eye Tracker.

    Provides comprehensive debug overlay for development and testing.
    Includes status displays, performance metrics, and interactive controls.

    Copyright (c) 2025 Eyeware Tech SA. All Rights Reserved.

    Eyeware® and Beam® are registered trademarks of Eyeware Tech SA.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BeamEyeTrackerSubsystem.h"
#include "BeamEyeTrackerTypes.h"
#include "BeamDebugCVars.h"
#include "BeamDebugHUD.generated.h"

// Debug HUD Widget

UCLASS(BlueprintType, Blueprintable)
class BEAMEYETRACKER_API UBeamDebugHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	UBeamDebugHUD(const FObjectInitializer& ObjectInitializer);

	// Widget lifecycle
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Beam Debug HUD")
	void UpdateStatusDisplay();

	UFUNCTION(BlueprintCallable, Category = "Beam Debug HUD")
	void UpdateGazeDisplay();

	UFUNCTION(BlueprintCallable, Category = "Beam Debug HUD")
	void UpdatePerformanceDisplay();

	UFUNCTION(BlueprintPure, Category = "Beam Debug HUD")
	FText GetTrackingStatusText() const;

	UFUNCTION(BlueprintPure, Category = "Beam Debug HUD")
	FText GetHealthStatusText() const;

	UFUNCTION(BlueprintPure, Category = "Beam Debug HUD")
	FText GetFPSText() const;

	UFUNCTION(BlueprintPure, Category = "Beam Debug HUD")
	FText GetBufferUtilizationText() const;

	UFUNCTION(BlueprintPure, Category = "Beam Debug HUD")
	FText GetGazePositionText() const;

	UFUNCTION(BlueprintPure, Category = "Beam Debug HUD")
	FText GetHeadPoseText() const;

	UFUNCTION(BlueprintPure, Category = "Beam Debug HUD")
	FText GetCalibrationStatusText() const;

	UFUNCTION(BlueprintPure, Category = "Beam Debug HUD")
	FText GetRecordingStatusText() const;

	UFUNCTION(BlueprintPure, Category = "Beam Debug HUD")
	FText GetPlaybackStatusText() const;

	// UI event handlers
	UFUNCTION(BlueprintCallable, Category = "Beam Debug HUD")
	void OnToggleTracking();

	UFUNCTION(BlueprintCallable, Category = "Beam Debug HUD")
	void OnResetCalibration();

	UFUNCTION(BlueprintCallable, Category = "Beam Debug HUD")
	void OnToggleRecording();

	UFUNCTION(BlueprintCallable, Category = "Beam Debug HUD")
	void OnTogglePlayback();

	UFUNCTION(BlueprintCallable, Category = "Beam Debug HUD")
	void OnToggleDebugHUD();

	UFUNCTION(BlueprintCallable, Category = "Beam Debug HUD")
	void OnToggleGazeCrosshair();

	UFUNCTION(BlueprintCallable, Category = "Beam Debug HUD")
	void OnToggleGazeTrail();

	UFUNCTION(BlueprintCallable, Category = "Beam Debug HUD")
	void OnToggleGazeRay();

protected:
	// Subsystem reference
	UPROPERTY(BlueprintReadOnly, Category = "Beam Debug HUD")
	UBeamEyeTrackerSubsystem* BeamSubsystem;

	UPROPERTY(BlueprintReadOnly, Category = "Beam Debug HUD")
	FTimerHandle UpdateTimerHandle;

	UPROPERTY(BlueprintReadOnly, Category = "Beam Debug HUD")
	float UpdateInterval;

	// Current status data
	UPROPERTY(BlueprintReadOnly, Category = "Beam Debug HUD")
	bool bIsTracking;

	UPROPERTY(BlueprintReadOnly, Category = "Beam Debug HUD")
	EBeamHealth CurrentHealth;

	UPROPERTY(BlueprintReadOnly, Category = "Beam Debug HUD")
	float CurrentFPS;

	UPROPERTY(BlueprintReadOnly, Category = "Beam Debug HUD")
	float CurrentBufferUtilization;

	UPROPERTY(BlueprintReadOnly, Category = "Beam Debug HUD")
	FGazePoint CurrentGazePoint;

	UPROPERTY(BlueprintReadOnly, Category = "Beam Debug HUD")
	FHeadPose CurrentHeadPose;

	UPROPERTY(BlueprintReadOnly, Category = "Beam Debug HUD")
	bool bIsRecording;

	UPROPERTY(BlueprintReadOnly, Category = "Beam Debug HUD")
	bool bIsPlayingBack;

	// Debug visualization toggles
	UPROPERTY(BlueprintReadWrite, Category = "Beam Debug HUD")
	bool bShowGazeCrosshair;

	UPROPERTY(BlueprintReadWrite, Category = "Beam Debug HUD")
	bool bShowGazeTrail;

	UPROPERTY(BlueprintReadWrite, Category = "Beam Debug HUD")
	bool bShowGazeRay;

private:
	// Helper functions
	void RefreshSubsystemReference();
	void StartUpdateTimer();
	void StopUpdateTimer();
	void OnUpdateTimer();
};

/*=============================================================================
    End of BeamDebugHUD.h
=============================================================================*/

