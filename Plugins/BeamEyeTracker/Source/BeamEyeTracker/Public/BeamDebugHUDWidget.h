/*=============================================================================
    BeamDebugHUDWidget.h: UMG-based Debug HUD for Beam Eye Tracker.

    Provides real-time debugging information and visual feedback for gaze point
    visualization, head pose tracking, performance metrics, and connection status.
    Users can create Blueprint children for custom styling or extend with
    additional debug features.

    Copyright (c) 2025 Eyeware Tech SA. All Rights Reserved.

    Eyeware® and Beam® are registered trademarks of Eyeware Tech SA.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "BeamEyeTrackerSubsystem.h"
#include "BeamEyeTrackerComponent.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "TimerManager.h"
#include "BeamDebugHUDWidget.generated.h"

/** UMG-based Debug HUD for Beam Eye Tracker */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Beam Debug HUD"))
class BEAMEYETRACKER_API UBeamDebugHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UBeamDebugHUDWidget(const FObjectInitializer& ObjectInitializer);

	// Blueprint-callable functions for external control
	UFUNCTION(BlueprintCallable, Category = "Beam Eye Tracker|Debug")
	void ShowDebugHUD();

	UFUNCTION(BlueprintCallable, Category = "Beam Eye Tracker|Debug")
	void HideDebugHUD();

	UFUNCTION(BlueprintCallable, Category = "Beam Eye Tracker|Debug")
	void ToggleDebugHUD();

	UFUNCTION(BlueprintCallable, Category = "Beam Eye Tracker|Debug")
	void SetGazeCrosshairVisible(bool bVisible);

	UFUNCTION(BlueprintCallable, Category = "Beam Eye Tracker|Debug")
	void SetHeadPoseIndicatorVisible(bool bVisible);

	UFUNCTION(BlueprintCallable, Category = "Beam Eye Tracker|Debug")
	void SetPerformanceMetricsVisible(bool bVisible);

	// Blueprint-callable getters
	UFUNCTION(BlueprintPure, Category = "Beam Eye Tracker|Debug")
	bool IsDebugHUDVisible() const;

	UFUNCTION(BlueprintPure, Category = "Beam Eye Tracker|Debug")
	FVector2D GetCurrentGazeScreenPosition() const;

	UFUNCTION(BlueprintPure, Category = "Beam Eye Tracker|Debug")
	FVector GetCurrentHeadPosition() const;

	UFUNCTION(BlueprintPure, Category = "Beam Eye Tracker|Debug")
	float GetCurrentGazeConfidence() const;

	UFUNCTION(BlueprintPure, Category = "Beam Eye Tracker|Debug")
	float GetCurrentHeadConfidence() const;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	/** Update the debug display */
	void UpdateDebugDisplay();
	
	/** Update gaze crosshair position */
	void UpdateGazeCrosshair();
	
	/** Update head pose indicator */
	void UpdateHeadPoseIndicator();
	
	/** Update performance metrics */
	void UpdatePerformanceMetrics();
	
	/** Update connection status */
	void UpdateConnectionStatus();
	
	/** Initialize widget bindings */
	void InitializeWidgetBindings();

private:
	/** Debug HUD visibility */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam Eye Tracker|Debug", meta = (AllowPrivateAccess = "true"))
	bool bDebugHUDVisible = true;
	
	/** Show gaze crosshair */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam Eye Tracker|Debug", meta = (AllowPrivateAccess = "true"))
	bool bShowGazeCrosshair = true;
	
	/** Show head pose indicator */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam Eye Tracker|Debug", meta = (AllowPrivateAccess = "true"))
	bool bShowHeadPoseIndicator = true;
	
	/** Show performance metrics */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam Eye Tracker|Debug", meta = (AllowPrivateAccess = "true"))
	bool bShowPerformanceMetrics = true;
	
	/** Show connection status */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam Eye Tracker|Debug", meta = (AllowPrivateAccess = "true"))
	bool bShowConnectionStatus = true;
	
	/** Update interval in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam Eye Tracker|Debug", meta = (AllowPrivateAccess = "true"))
	float UpdateInterval = 0.016f; // 60 FPS update rate
	
	/** Gaze crosshair size */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam Eye Tracker|Debug", meta = (AllowPrivateAccess = "true"))
	float GazeCrosshairSize = 20.0f;
	
	/** Head pose indicator size */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam Eye Tracker|Debug", meta = (AllowPrivateAccess = "true"))
	float HeadPoseIndicatorSize = 15.0f;
	
	/** Timer handle for updates */
	FTimerHandle UpdateTimerHandle;
	
	/** Widget references for updating */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* StatusText;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* GazeDataText;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* HeadDataText;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* PerformanceText;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ConnectionText;
	
	/** Visual elements */
	UPROPERTY(meta = (BindWidget))
	UImage* GazeCrosshair;
	
	UPROPERTY(meta = (BindWidget))
	UImage* HeadPoseIndicator;
	
	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* DebugCanvas;
	
	UPROPERTY(meta = (BindWidget))
	UOverlay* DebugOverlay;
	
	/** Current gaze data */
	FVector2D CurrentGazeScreenPos;
	FVector2D CurrentGazeScreen01;
	float CurrentGazeConfidence;
	bool bGazeValid;
	
	/** Current head pose data */
	FVector CurrentHeadPosition;
	FRotator CurrentHeadRotation;
	float CurrentHeadConfidence;
	bool bHeadValid;
	
	/** Performance data */
	float CurrentFPS;
	int32 CurrentBufferUtilization;
	double CurrentLatency;
	
	/** Connection data */
	bool bIsConnected;
	bool bIsTracking;
	FString ConnectionStatus;
};

/*=============================================================================
    End of BeamDebugHUDWidget.h
=============================================================================*/
