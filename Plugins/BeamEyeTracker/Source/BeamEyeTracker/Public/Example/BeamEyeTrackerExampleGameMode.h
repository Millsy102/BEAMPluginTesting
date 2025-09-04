/*=============================================================================
    BeamEyeTrackerExampleGameMode.h: Example Game Mode with Beam SDK integration.

    Demonstrates how to integrate eye tracking into a game mode system
    with automatic initialization, tracking control, calibration management,
    and comprehensive debugging features for development and testing.

    Copyright (c) 2025 Eyeware Tech SA. All Rights Reserved.

    Eyeware® and Beam® are registered trademarks of Eyeware Tech SA.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BeamEyeTrackerTypes.h"
#include "BeamEyeTrackerExampleGameMode.generated.h"

// Forward declarations
class UBeamEyeTrackerSubsystem;
class ABeamEyeTrackerExampleCharacter;
class ABeamEyeTrackerExampleHUD;

// Example Game Mode with Beam Eye Tracker Integration

UCLASS(BlueprintType, Blueprintable)
class BEAMEYETRACKER_API ABeamEyeTrackerExampleGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ABeamEyeTrackerExampleGameMode();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Game mode overrides
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void InitGameState() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	// Beam Eye Tracker integration
	UFUNCTION(BlueprintCallable, Category="Beam Eye Tracker")
	void InitializeBeamEyeTracker();

	UFUNCTION(BlueprintCallable, Category="Beam Eye Tracker")
	void StartBeamTracking();

	UFUNCTION(BlueprintCallable, Category="Beam Eye Tracker")
	void StopBeamTracking();

	UFUNCTION(BlueprintCallable, Category="Beam Eye Tracker")
	void ResetBeamCalibration();

	UFUNCTION(BlueprintCallable, Category="Beam Eye Tracker")
	void ToggleBeamDebugHUD();

	UFUNCTION(BlueprintCallable, Category="Beam Eye Tracker")
	void StartBeamRecording();

	UFUNCTION(BlueprintCallable, Category="Beam Eye Tracker")
	void StopBeamRecording();

	UFUNCTION(BlueprintCallable, Category="Beam Eye Tracker")
	void StartBeamPlayback();

	UFUNCTION(BlueprintCallable, Category="Beam Eye Tracker")
	void StopBeamPlayback();

	// Game state management
	UFUNCTION(BlueprintCallable, Category="Game Mode")
	void PauseGame();

	UFUNCTION(BlueprintCallable, Category="Game Mode")
	void ResumeGame();

	UFUNCTION(BlueprintCallable, Category="Game Mode")
	void RestartGame();

	// Performance monitoring
	UFUNCTION(BlueprintCallable, Category="Game Mode")
	void EnablePerformanceMonitoring();

	UFUNCTION(BlueprintCallable, Category="Game Mode")
	void DisablePerformanceMonitoring();

	UPROPERTY(BlueprintReadOnly, Category="Beam Eye Tracker")
	FTimerHandle UpdateTimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Beam Eye Tracker")
	float UpdateInterval = 0.1f;

	// Auto-start tracking
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Beam Eye Tracker")
	bool bAutoStartTracking = true;

	// Show debug HUD by default
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Beam Eye Tracker")
	bool bShowDebugHUDByDefault = true;

	// Enable performance monitoring by default
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Beam Eye Tracker")
	bool bEnablePerformanceMonitoring = true;

	// Current game state
	UPROPERTY(BlueprintReadOnly, Category="Game Mode")
	bool bGamePaused = false;

	UPROPERTY(BlueprintReadOnly, Category="Game Mode")
	bool bPerformanceMonitoringEnabled = false;

	// Beam Eye Tracker status
	UPROPERTY(BlueprintReadOnly, Category="Beam Eye Tracker")
	bool bBeamTrackingActive = false;

	UPROPERTY(BlueprintReadOnly, Category="Beam Eye Tracker")
	EBeamHealth BeamHealth = EBeamHealth::Error;

	UPROPERTY(BlueprintReadOnly, Category="Beam Eye Tracker")
	float BeamFPS = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Beam Eye Tracker")
	float BeamBufferUtilization = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Beam Eye Tracker")
	bool bBeamRecording = false;

	UPROPERTY(BlueprintReadOnly, Category="Beam Eye Tracker")
	bool bBeamPlayingBack = false;

private:
	
	void UpdateBeamEyeTracker();
	void UpdatePerformanceMetrics();

	// Utility functions
	void LogBeamStatus();
	void HandleBeamError(const FString& ErrorMessage);
};

/*=============================================================================
    End of BeamEyeTrackerExampleGameMode.h
=============================================================================*/

