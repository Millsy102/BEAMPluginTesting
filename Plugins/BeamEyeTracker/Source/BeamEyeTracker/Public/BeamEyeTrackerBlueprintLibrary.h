/*=============================================================================
    BeamEyeTrackerBlueprintLibrary.h: Blueprint Function Library for Beam SDK.

    Provides easy-to-use Blueprint nodes for eye-tracking functionality.
    No C++ knowledge required - perfect for beginners and visual scripting.

    Copyright (c) 2025 Eyeware Tech SA. All Rights Reserved.

    Eyeware® and Beam® are registered trademarks of Eyeware Tech SA.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BeamEyeTrackerTypes.h"
#include "BeamEyeTrackerBlueprintLibrary.generated.h"

class UBeamEyeTrackerSubsystem;
class UBeamEyeTrackerComponent;
class UObject;
class UWorld;

/** Blueprint Function Library for Beam Eye Tracker */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Beam Eye Tracker"))
class BEAMEYETRACKER_API UBeamEyeTrackerBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
		// CORE SYSTEM FUNCTIONS
	
	/** Get the Beam Eye Tracker subsystem for the current world */
	UFUNCTION(BlueprintCallable, Category = "Beam Eye Tracker|Core", meta = (WorldContext = "WorldContextObject"))
	static UBeamEyeTrackerSubsystem* GetBeamEyeTrackerSubsystem(const UObject* WorldContextObject);

	/** Check if Beam eye tracking is available and working */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Beam Eye Tracker|Core", meta = (WorldContext = "WorldContextObject"))
	static bool IsBeamEyeTrackingAvailable(const UObject* WorldContextObject);

	/** Start Beam eye tracking */
	UFUNCTION(BlueprintCallable, Category = "Beam Eye Tracker|Core", meta = (WorldContext = "WorldContextObject"))
	static bool StartBeamEyeTracking(const UObject* WorldContextObject);

	/** Stop Beam eye tracking */
	UFUNCTION(BlueprintCallable, Category = "Beam Eye Tracker|Core", meta = (WorldContext = "WorldContextObject"))
	static void StopBeamEyeTracking(const UObject* WorldContextObject);

		// GAZE TRACKING FUNCTIONS
	
	/** Get the latest gaze point from the camera */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Beam Eye Tracker|Gaze", meta = (WorldContext = "WorldContextObject"))
	static FGazePoint CurrentGaze(const UObject* WorldContextObject);

	/** Get gaze point at a specific time (for playback) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Beam Eye Tracker|Gaze", meta = (WorldContext = "WorldContextObject"))
	static FGazePoint GetGazePointAtTime(const UObject* WorldContextObject, float TimeSeconds);

	/** Check if current gaze point is valid */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Beam Eye Tracker|Gaze", meta = (WorldContext = "WorldContextObject"))
	static bool IsGazePointValid(const UObject* WorldContextObject);

	/** Get gaze confidence level (0.0 to 1.0) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Beam Eye Tracker|Gaze", meta = (WorldContext = "WorldContextObject"))
	static float GetGazeConfidence(const UObject* WorldContextObject);

		// HEAD POSE FUNCTIONS
	
	/** Get the latest head pose from the camera */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Beam Eye Tracker|Head Pose", meta = (WorldContext = "WorldContextObject"))
	static FHeadPose HeadPosition(const UObject* WorldContextObject);

	/** Get head pose at a specific time (for playback) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Beam Eye Tracker|Head Pose", meta = (WorldContext = "WorldContextObject"))
	static FHeadPose GetHeadPoseAtTime(const UObject* WorldContextObject, float TimeSeconds);

	/** Check if current head pose is valid */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Beam Eye Tracker|Head Pose", meta = (WorldContext = "WorldContextObject"))
	static bool IsHeadPoseValid(const UObject* WorldContextObject);

	/** Get head pose confidence level (0.0 to 1.0) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Beam Eye Tracker|Head Pose", meta = (WorldContext = "WorldContextObject"))
	static float GetHeadPoseConfidence(const UObject* WorldContextObject);

		// CALIBRATION FUNCTIONS
	
	/** Start calibration process */
	UFUNCTION(BlueprintCallable, Category = "Beam Eye Tracker|Calibration", meta = (WorldContext = "WorldContextObject"))
	static bool StartCalibration(const UObject* WorldContextObject);

	/** Stop calibration process */
	UFUNCTION(BlueprintCallable, Category = "Beam Eye Tracker|Calibration", meta = (WorldContext = "WorldContextObject"))
	static void StopCalibration(const UObject* WorldContextObject);

	/** Check if calibration is in progress */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Beam Eye Tracker|Calibration", meta = (WorldContext = "WorldContextObject"))
	static bool IsCalibrating(const UObject* WorldContextObject);

	/** Check if calibration is complete and valid */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Beam Eye Tracker|Calibration", meta = (WorldContext = "WorldContextObject"))
	static bool IsCalibrated(const UObject* WorldContextObject);

	/** Get calibration quality score (0.0 to 1.0) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Beam Eye Tracker|Calibration", meta = (WorldContext = "WorldContextObject"))
	static float GetCalibrationQuality(const UObject* WorldContextObject);

		// RECORDING & PLAYBACK FUNCTIONS
	
	/** Start recording eye tracking data */
	UFUNCTION(BlueprintCallable, Category = "Beam Eye Tracker|Recording", meta = (WorldContext = "WorldContextObject"))
	static bool StartRecording(const UObject* WorldContextObject, const FString& FilePath);

	/** Stop recording eye tracking data */
	UFUNCTION(BlueprintCallable, Category = "Beam Eye Tracker|Recording", meta = (WorldContext = "WorldContextObject"))
	static void StopRecording(const UObject* WorldContextObject);

	/** Check if currently recording */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Beam Eye Tracker|Recording", meta = (WorldContext = "WorldContextObject"))
	static bool IsRecording(const UObject* WorldContextObject);

	/** Start playback of recorded eye tracking data */
	UFUNCTION(BlueprintCallable, Category = "Beam Eye Tracker|Playback", meta = (WorldContext = "WorldContextObject"))
	static bool StartPlayback(const UObject* WorldContextObject, const FString& FilePath);

	/** Stop playback of recorded eye tracking data */
	UFUNCTION(BlueprintCallable, Category = "Beam Eye Tracker|Playback", meta = (WorldContext = "WorldContextObject"))
	static void StopPlayback(const UObject* WorldContextObject);

	/** Check if currently playing back */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Beam Eye Tracker|Playback", meta = (WorldContext = "WorldContextObject"))
	static bool IsPlayingBack(const UObject* WorldContextObject);

		// UTILITY FUNCTIONS
	
	/** Convert screen coordinates to world coordinates */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Beam Eye Tracker|Utility", meta = (WorldContext = "WorldContextObject"))
	static FVector ProjectGazeToWorld(const UObject* WorldContextObject, const FVector2D& ScreenPosition, float Distance = 1000.0f);

	/** Get the SDK version information */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Beam Eye Tracker|Utility", meta = (WorldContext = "WorldContextObject"))
	static FString GetSDKVersion(const UObject* WorldContextObject);

	/** Get connection quality (0.0 to 1.0) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Beam Eye Tracker|Utility", meta = (WorldContext = "WorldContextObject"))
	static float GetConnectionQuality(const UObject* WorldContextObject);

	/** Check if the camera is connected and working */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Beam Eye Tracker|Utility", meta = (WorldContext = "WorldContextObject"))
	static bool IsCameraConnected(const UObject* WorldContextObject);

		// ADVANCED FUNCTIONS (for power users)
	
	/** Get raw frame data for advanced processing */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Beam Eye Tracker|Advanced", meta = (WorldContext = "WorldContextObject"))
	static FBeamFrame GetLatestRawFrame(const UObject* WorldContextObject);

	/** Get frame data at a specific time (for playback) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Beam Eye Tracker|Advanced", meta = (WorldContext = "WorldContextObject"))
	static FBeamFrame GetRawFrameAtTime(const UObject* WorldContextObject, float TimeSeconds);
};

/*=============================================================================
    End of BeamEyeTrackerBlueprintLibrary.h
=============================================================================*/


