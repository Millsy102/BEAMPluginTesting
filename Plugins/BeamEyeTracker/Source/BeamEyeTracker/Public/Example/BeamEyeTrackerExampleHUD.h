/*=============================================================================
    BeamEyeTrackerExampleHUD.h: Example HUD with Beam SDK integration.

    Demonstrates how to integrate eye tracking into a custom HUD system
    with real-time status panels, gaze crosshairs, gaze trails, and
    performance metrics for comprehensive debugging and visualization.

    Copyright (c) 2025 Eyeware Tech SA. All Rights Reserved.

    Eyeware® and Beam® are registered trademarks of Eyeware Tech SA.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BeamEyeTrackerTypes.h"
#include "BeamEyeTrackerExampleHUD.generated.h"

// Forward declarations
class UBeamEyeTrackerSubsystem;
class UBeamEyeTrackerComponent;

// Example HUD with Beam Eye Tracker Integration

UCLASS(BlueprintType, Blueprintable)
class BEAMEYETRACKER_API ABeamEyeTrackerExampleHUD : public AHUD
{
	GENERATED_BODY()

public:
	ABeamEyeTrackerExampleHUD();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Draw HUD
	virtual void DrawHUD() override;

	// HUD update timer
	UPROPERTY(BlueprintReadOnly, Category="Beam Eye Tracker")
	FTimerHandle HUDUpdateTimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Beam Eye Tracker")
	float HUDUpdateInterval = 0.1f;

	// HUD visibility
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Beam Eye Tracker")
	bool bShowStatusPanel = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Beam Eye Tracker")
	bool bShowGazeCrosshair = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Beam Eye Tracker")
	bool bShowGazeTrail = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Beam Eye Tracker")
	bool bShowPerformanceMetrics = true;

	// HUD positioning
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Beam Eye Tracker")
	FVector2D StatusPanelPosition = FVector2D(20.0f, 20.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Beam Eye Tracker")
	FVector2D StatusPanelSize = FVector2D(300.0f, 200.0f);

	// Current tracking data
	UPROPERTY(BlueprintReadOnly, Category="Beam Eye Tracker")
	bool bIsTracking = false;

	UPROPERTY(BlueprintReadOnly, Category="Beam Eye Tracker")
	EBeamHealth CurrentHealth = EBeamHealth::Error;

	UPROPERTY(BlueprintReadOnly, Category="Beam Eye Tracker")
	float CurrentFPS = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Beam Eye Tracker")
	float CurrentBufferUtilization = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Beam Eye Tracker")
	FGazePoint CurrentGazePoint;

	UPROPERTY(BlueprintReadOnly, Category="Beam Eye Tracker")
	FHeadPose CurrentHeadPose;

	UPROPERTY(BlueprintReadOnly, Category="Beam Eye Tracker")
	bool bIsRecording = false;

	UPROPERTY(BlueprintReadOnly, Category="Beam Eye Tracker")
	bool bIsPlayingBack = false;

	// Gaze trail history
	UPROPERTY(BlueprintReadOnly, Category="Beam Eye Tracker")
	TArray<FVector2D> GazeTrailPoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Beam Eye Tracker")
	int32 MaxTrailPoints = 30;

private:
	// HUD update functions
	void UpdateHUDData();
	void DrawStatusPanel();
	void DrawGazeCrosshair();
	void DrawGazeTrail();
	void DrawPerformanceMetrics();

	// Utility functions
	FString GetHealthStatusString(EBeamHealth Health) const;
	FLinearColor GetHealthColor(EBeamHealth Health) const;
	FString GetTrackingStatusString() const;
	FLinearColor GetTrackingStatusColor() const;
};

/*=============================================================================
    End of BeamEyeTrackerExampleHUD.h
=============================================================================*/

