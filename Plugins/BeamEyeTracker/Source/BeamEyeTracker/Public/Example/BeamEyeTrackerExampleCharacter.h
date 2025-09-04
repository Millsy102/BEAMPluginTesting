/*=============================================================================
    BeamEyeTrackerExampleCharacter.h: Example Character with Beam SDK integration.

    Demonstrates how to integrate eye tracking into a character system
    with input actions, camera components, head tracking settings, and
    comprehensive tracking status monitoring for development and testing.

    Copyright (c) 2025 Eyeware Tech SA. All Rights Reserved.

    Eyeware® and Beam® are registered trademarks of Eyeware Tech SA.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "BeamEyeTrackerTypes.h"
#include "Engine/Font.h"
#include "BeamEyeTrackerExampleCharacter.generated.h"

// Forward declarations
class UInputMappingContext;
class UInputAction;
class USpringArmComponent;
class UCameraComponent;
class UBeamEyeTrackerComponent;
class UBeamEyeTrackerSubsystem;

// Example Character with Beam Eye Tracker Integration

UCLASS(config=Game)
class BEAMEYETRACKER_API ABeamEyeTrackerExampleCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ABeamEyeTrackerExampleCharacter();

	// Input actions
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
	UInputAction* StartEyeTrackingAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
	UInputAction* StopEyeTrackingAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
	UInputAction* ResetCalibrationAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
	UInputAction* ToggleDebugHUDAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
	UInputAction* ToggleRecordingAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input)
	UInputAction* TogglePlaybackAction;

protected:
	// Camera components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	UCameraComponent* FollowCamera;

	// Beam Eye Tracker component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Beam Eye Tracker")
	UBeamEyeTrackerComponent* BeamEyeTrackerComponent;

	// Head tracking settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Beam Eye Tracker")
	float HeadTrackingSensitivity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Beam Eye Tracker")
	float HeadTrackingSmoothing = 5.0f;

	// HUD update timer
	UPROPERTY(BlueprintReadOnly, Category="Beam Eye Tracker")
	FTimerHandle HUDUpdateTimerHandle;

	// Current tracking status
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

	// Debug font for HUD drawing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Beam Eye Tracker")
	UFont* DebugFont;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintPure, Category="Beam Eye Tracker")
	EBeamHealth GetTrackingHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintPure, Category="Beam Eye Tracker")
	FGazePoint GetCurrentGazePoint() const { return CurrentGazePoint; }

	UFUNCTION(BlueprintPure, Category="Beam Eye Tracker")
	FHeadPose GetCurrentHeadPose() const { return CurrentHeadPose; }

	UFUNCTION(BlueprintPure, Category="Beam Eye Tracker")
	bool IsEyeTrackingActive() const { return bIsTracking; }

	UFUNCTION(BlueprintPure, Category="Beam Eye Tracker")
	FBeamFrame GetCurrentFrame() const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Input functions
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	// Beam Eye Tracker functions
	UFUNCTION(BlueprintCallable, Category="Beam Eye Tracker")
	void StartEyeTracking();

	UFUNCTION(BlueprintCallable, Category="Beam Eye Tracker")
	void StopEyeTracking();

	UFUNCTION(BlueprintCallable, Category="Beam Eye Tracker")
	void ResetCalibration();

	UFUNCTION(BlueprintCallable, Category="Beam Eye Tracker")
	void ToggleDebugHUD();

	UFUNCTION(BlueprintCallable, Category="Beam Eye Tracker")
	void ToggleRecording();

	UFUNCTION(BlueprintCallable, Category="Beam Eye Tracker")
	void TogglePlayback();

	// HUD and tracking functions
	UFUNCTION(BlueprintCallable, Category="Beam Eye Tracker")
	void UpdateHUD();

	UFUNCTION(BlueprintCallable, Category="Beam Eye Tracker")
	void ApplyHeadTracking(const FHeadPose& HeadPose);

	// Debug drawing
	UFUNCTION(BlueprintCallable, Category="Beam Eye Tracker")
	void DrawDebugHUD(UCanvas* Canvas);
};

/*=============================================================================
    End of BeamEyeTrackerExampleCharacter.h
=============================================================================*/

