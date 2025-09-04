/*=============================================================================
    BeamBlueprintLibrary.h: Enhanced Blueprint library for Beam SDK.

    Provides simplified, user-friendly functions for easy eye tracking
    integration. Features one-line initialization, simplified data access,
    input validation, error handling, and comprehensive tooltips.

    Copyright (c) 2025 Eyeware Tech SA. All Rights Reserved.

    Eyeware® and Beam® are registered trademarks of Eyeware Tech SA.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BeamBlueprintLibrary.generated.h"

class UObject;
class UBeamEyeTrackerSubsystem;
class AActor;
class UWidget;

/** Enhanced Blueprint library for easy eye tracking integration */
UCLASS(meta = (DisplayName = "Beam Eye Tracking Library"))
class BEAMEYETRACKER_API UBeamBlueprintLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    
    UFUNCTION(BlueprintCallable, Category = "Beam", meta = (WorldContext = "WorldContextObject"))
    static bool InitializeEyeTracking(const UObject* WorldContextObject);

    UFUNCTION(BlueprintPure, Category = "Beam", meta = (WorldContext = "WorldContextObject"))
    static bool IsEyeTrackingAvailable(const UObject* WorldContextObject);

    UFUNCTION(BlueprintPure, Category = "Beam", meta = (WorldContext = "WorldContextObject"))
    static UBeamEyeTrackerSubsystem* GetEyeTrackingSubsystem(const UObject* WorldContextObject);

    // Data access functions
    UFUNCTION(BlueprintPure, Category = "Beam", meta = (WorldContext = "WorldContextObject"))
    static FVector2D GetGazePoint2D(const UObject* WorldContextObject);

    UFUNCTION(BlueprintPure, Category = "Beam", meta = (WorldContext = "WorldContextObject"))
    static FVector2D GetGazePointPixels(const UObject* WorldContextObject);

    UFUNCTION(BlueprintPure, Category = "Beam", meta = (WorldContext = "WorldContextObject"))
    static FVector GetHeadPosition(const UObject* WorldContextObject);

    UFUNCTION(BlueprintPure, Category = "Beam", meta = (WorldContext = "WorldContextObject"))
    static FRotator GetHeadRotation(const UObject* WorldContextObject);

    UFUNCTION(BlueprintPure, Category = "Beam", meta = (WorldContext = "WorldContextObject"))
    static FTransform GetHeadPoseTransform(const UObject* WorldContextObject);

    	//~ Begin Advanced Data Access
	/** Get tracking confidence (0-1, where 1 is high confidence) */
	UFUNCTION(BlueprintPure, Category = "Beam|Data", meta = (WorldContext = "WorldContextObject"))
	static float GetTrackingConfidence(const UObject* WorldContextObject);

	/** Get current tracking FPS */
	UFUNCTION(BlueprintPure, Category = "Beam|Data", meta = (WorldContext = "WorldContextObject"))
	static float GetTrackingFPS(const UObject* WorldContextObject);

	/** Check if user is currently looking at a specific actor */
	UFUNCTION(BlueprintPure, Category = "Beam|Interaction", meta = (WorldContext = "WorldContextObject"))
	static bool IsLookingAtActor(const UObject* WorldContextObject, AActor* TargetActor, float MaxDistance = 1000.0f);

	/** Get the actor closest to the current gaze point */
	UFUNCTION(BlueprintPure, Category = "Beam|Interaction", meta = (WorldContext = "WorldContextObject"))
	static AActor* GetClosestActorToGaze(const UObject* WorldContextObject, TArray<AActor*> ActorList, float MaxDistance = 1000.0f);
	//~ End Advanced Data Access

    //~ Begin Control Functions
    /** Start eye tracking */
    UFUNCTION(BlueprintCallable, Category = "Beam|Control", meta = (WorldContext = "WorldContextObject"))
    static bool StartEyeTracking(const UObject* WorldContextObject);

    /** Stop eye tracking */
    UFUNCTION(BlueprintCallable, Category = "Beam|Control", meta = (WorldContext = "WorldContextObject"))
    static void StopEyeTracking(const UObject* WorldContextObject);

    /** Check if eye tracking is currently active */
    UFUNCTION(BlueprintPure, Category = "Beam|Control", meta = (WorldContext = "WorldContextObject"))
    static bool IsEyeTrackingActive(const UObject* WorldContextObject);
    //~ End Control Functions

    //~ Begin Utility Functions
    /** Convert screen coordinates to world position */
    UFUNCTION(BlueprintPure, Category = "Beam|Utility")
    static FVector ScreenToWorldPosition(const FVector2D& ScreenPosition, float Distance = 1000.0f);

    /** Convert world position to screen coordinates */
    UFUNCTION(BlueprintPure, Category = "Beam|Utility")
    static FVector2D WorldToScreenPosition(const FVector& WorldPosition);

    /** Get distance between two 3D points */
    UFUNCTION(BlueprintPure, Category = "Beam|Utility")
    static float GetDistance3D(const FVector& PointA, const FVector& PointB);

    /** Get distance between two 2D points */
    UFUNCTION(BlueprintPure, Category = "Beam|Utility")
    static float GetDistance2D(const FVector2D& PointA, const FVector2D& PointB);
    //~ End Utility Functions

    //~ Begin Validation & Error Handling
    /** Validate gaze point data */
    UFUNCTION(BlueprintPure, Category = "Beam|Validation")
    static bool IsValidGazePoint(const FVector2D& GazePoint);

    /** Get error message for eye tracking issues */
    UFUNCTION(BlueprintPure, Category = "Beam|Validation", meta = (WorldContext = "WorldContextObject"))
    static FString GetErrorMessage(const UObject* WorldContextObject);

    /** Check if calibration is needed */
    UFUNCTION(BlueprintPure, Category = "Beam|Validation", meta = (WorldContext = "WorldContextObject"))
    static bool NeedsCalibration(const UObject* WorldContextObject);
    //~ End Validation & Error Handling

private:
    /** Helper function to get subsystem safely */
    static UBeamEyeTrackerSubsystem* GetSubsystemSafe(const UObject* WorldContextObject);
    
    /** Helper function to validate world context */
    static bool ValidateWorldContext(const UObject* WorldContextObject, FString& OutErrorMessage);
    
    /** Helper function to get player camera info */
    static bool GetPlayerCameraInfo(const UObject* WorldContextObject, FVector& OutLocation, FRotator& OutRotation);
};

/*=============================================================================
    End of BeamBlueprintLibrary.h
=============================================================================*/

