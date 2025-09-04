/*=============================================================================
    BeamEyeTrackerTypes.h: Core data structures and enumerations for Beam SDK.

    Defines fundamental data types and structures for eye tracking integration.

    Copyright (c) 2025 Eyeware Tech SA. All Rights Reserved.

    Eyeware® and Beam® are registered trademarks of Eyeware Tech SA.
=============================================================================*/

#pragma once
#include "CoreMinimal.h"
#include "BeamEyeTrackerTypes.generated.h"

/** Shared structs/enums for Beam tracking data */

// System health status
UENUM()
enum class EBeamHealth : uint8
{
	Ok UMETA(DisplayName = "OK"),
	Warning UMETA(DisplayName = "Warning"),
	AppNotRunning UMETA(DisplayName = "Beam App Not Running"),
	DllMissing UMETA(DisplayName = "DLL Missing"),
	NoData UMETA(DisplayName = "No Data"),
	Recovering UMETA(DisplayName = "Recovering"),
	Error UMETA(DisplayName = "Error")
};

// Data source types
UENUM()
enum class EBeamDataSourceType : uint8
{
	Live UMETA(DisplayName = "Live"),
	File UMETA(DisplayName = "File"),
	Network UMETA(DisplayName = "Network"),
	Recorded UMETA(DisplayName = "Recorded"),
	Synthetic UMETA(DisplayName = "Synthetic")
};

/** Tracking status enumeration for individual tracking elements */
UENUM()
enum class EBeamTrackingStatus : uint8
{
	NotTracked UMETA(DisplayName = "Not Tracked"),
	Tracked UMETA(DisplayName = "Tracked"),
	TrackedWithLowConfidence UMETA(DisplayName = "Tracked With Low Confidence")
};

/** Data reception status for system connectivity */
UENUM()
enum class EBeamDataReceptionStatus : uint8
{
	NotReceiving UMETA(DisplayName = "Not Receiving"),
	Receiving UMETA(DisplayName = "Receiving"),
	AttemptingToStart UMETA(DisplayName = "Attempting To Start")
};

/** Eye selection for per-eye data access */
UENUM()
enum class EBeamEye : uint8
{
	Left UMETA(DisplayName = "Left Eye"),
	Right UMETA(DisplayName = "Right Eye")
};

/** Simple gaze point structure for screen coordinates */
USTRUCT(BlueprintType)
struct FGazePoint 
{ 
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, Category = "Gaze")
	bool bValid = false;
	
		UPROPERTY(BlueprintReadWrite, Category = "Gaze")
	FVector2D Screen01 = FVector2D::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "Gaze")
	FVector2D ScreenPx = FVector2D::ZeroVector;
	
	/** Timestamp when this gaze point was captured (milliseconds) */
	UPROPERTY(BlueprintReadWrite, Category = "Gaze Point", meta = (ToolTip = "Timestamp when this gaze point was captured (milliseconds)"))
	double TimestampMs = 0.0;
	
	/** Confidence value from 0.0 (no confidence) to 1.0 (full confidence) */
	UPROPERTY(BlueprintReadWrite, Category = "Gaze Point", meta = (ToolTip = "Confidence value from 0.0 (no confidence) to 1.0 (full confidence)"))
	double Confidence = 0.0;
};

/** Simple head pose structure for 3D head tracking */
USTRUCT(BlueprintType)
struct FHeadPose 
{ 
	GENERATED_BODY()
	
	/** Head position in centimeters in document coordinate space */
	UPROPERTY(BlueprintReadWrite, Category = "Head Pose", meta = (ToolTip = "Head position in centimeters in document coordinate space"))
	FVector PositionCm = FVector::ZeroVector;
	
	/** Head rotation in degrees (pitch, yaw, roll) */
	UPROPERTY(BlueprintReadWrite, Category = "Head Pose", meta = (ToolTip = "Head rotation in degrees (pitch, yaw, roll)"))
	FRotator Rotation = FRotator::ZeroRotator;
	
	/** Timestamp when this head pose was captured (milliseconds) */
	UPROPERTY(BlueprintReadWrite, Category = "Head Pose", meta = (ToolTip = "Timestamp when this head pose was captured (milliseconds)"))
	double TimestampMs = 0.0;
	
	/** Confidence value from 0.0 (no confidence) to 1.0 (full confidence) */
	UPROPERTY(BlueprintReadWrite, Category = "Head Pose", meta = (ToolTip = "Confidence value from 0.0 (no confidence) to 1.0 (full confidence)"))
	double Confidence = 0.0;

	/** Session UID for tracking consecutive frames (increments when tracking is lost and regained) */
	UPROPERTY(BlueprintReadWrite, Category = "Head Pose", meta = (ToolTip = "Session UID for tracking consecutive frames (increments when tracking is lost and regained)"))
	int64 TrackSessionUID = 0;
};

/**
 * Complete frame containing gaze and head data.
 * 
 * Represents a complete tracking frame with both gaze and head pose
 * information, along with frame identification and timing data
 * for synchronization and analysis.
 */
USTRUCT(BlueprintType)
struct FBeamFrame 
{ 
	GENERATED_BODY()
	
	/** Gaze point data for this frame */
	UPROPERTY(BlueprintReadWrite, Category = "Beam Frame", meta = (ToolTip = "Gaze point data for this frame"))
	FGazePoint Gaze;
	
	/** Head pose data for this frame */
	UPROPERTY(BlueprintReadWrite, Category = "Beam Frame", meta = (ToolTip = "Head pose data for this frame"))
	FHeadPose Head;
	
	/** Unique frame identifier for tracking frame sequence */
	UPROPERTY(BlueprintReadWrite, Category = "Beam Frame", meta = (ToolTip = "Unique frame identifier for tracking frame sequence"))
	int64 FrameId = 0;
	
	/** SDK timestamp in milliseconds from the Beam system */
	UPROPERTY(BlueprintReadWrite, Category = "Beam Frame", meta = (ToolTip = "SDK timestamp in milliseconds from the Beam system"))
	double SDKTimestampMs = 0.0;
	
	/** Unreal Engine timestamp in seconds for synchronization */
	UPROPERTY(BlueprintReadWrite, Category = "Beam Frame", meta = (ToolTip = "Unreal Engine timestamp in seconds for synchronization"))
	double UETimestampSeconds = 0.0;
	
	/** Frame delta time in seconds for frame rate calculations */
	UPROPERTY(BlueprintReadWrite, Category = "Beam Frame", meta = (ToolTip = "Frame delta time in seconds for frame rate calculations"))
	double DeltaTimeSeconds = 0.0;
};

/**
 * World ray projection result for 3D gaze interaction.
 * 
 * Contains the result of projecting a gaze point into 3D world space,
 * providing origin, direction, and distance information for ray casting.
 */
USTRUCT()
struct FBeamWorldRay
{
	GENERATED_BODY()
	
	/** Origin point of the gaze ray in world space */
	UPROPERTY(meta = (ToolTip = "Origin point of the gaze ray in world space"))
	FVector Origin = FVector::ZeroVector;
	
	/** Direction vector of the gaze ray in world space */
	UPROPERTY(meta = (ToolTip = "Direction vector of the gaze ray in world space"))
	FVector Direction = FVector::ZeroVector;
	
	/** Distance from origin to the projected gaze point */
	UPROPERTY(meta = (ToolTip = "Distance from origin to the projected gaze point"))
	double Distance = 0.0;
	
	/** Whether this world ray projection is valid */
	UPROPERTY(meta = (ToolTip = "Whether this world ray projection is valid"))
	bool bValid = false;
};

/**
 * Interpolation settings for smooth data rendering.
 * 
 * Configures how the system interpolates between tracking frames
 * to provide smooth visual updates even with variable frame rates.
 */
USTRUCT()
struct FInterpolationSettings
{
	GENERATED_BODY()
	
	/** Whether to enable frame interpolation for smooth rendering */
	UPROPERTY(EditAnywhere, Category = "Interpolation", meta = (ToolTip = "Whether to enable frame interpolation for smooth rendering"))
	bool bEnableInterpolation = true;
	
	/** Maximum buffer size for storing frames for interpolation */
	UPROPERTY(EditAnywhere, Category = "Interpolation", meta = (ToolTip = "Maximum buffer size for storing frames for interpolation"))
	int32 MaxBufferSize = 64;
	
	/** Maximum time window in milliseconds for interpolation */
	UPROPERTY(EditAnywhere, Category = "Interpolation", meta = (ToolTip = "Maximum time window in milliseconds for interpolation"))
	double MaxInterpolationTimeMs = 100.0;
};

// Advanced Gaze Analytics
USTRUCT(BlueprintType)
struct BEAMEYETRACKER_API FGazeAnalytics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam|Analytics")
    float AverageFixationDuration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam|Analytics")
    float SaccadeVelocity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam|Analytics")
    int32 FixationCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam|Analytics")
    float ScanPathLength = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam|Analytics")
    TArray<FVector2D> FixationPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam|Analytics")
    float TimeStamp = 0.0f;

    FGazeAnalytics() = default;
};

// Calibration Quality Data
USTRUCT(BlueprintType)
struct BEAMEYETRACKER_API FCalibrationQuality
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam|Calibration")
    float OverallScore = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam|Calibration")
    float LeftEyeScore = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam|Calibration")
    float RightEyeScore = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam|Calibration")
    TArray<FVector2D> CalibrationPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam|Calibration")
    TArray<float> PointScores;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam|Calibration")
    float LastCalibrationTime = 0.0f;

    FCalibrationQuality() = default;
};

// Performance Metrics
USTRUCT(BlueprintType)
struct BEAMEYETRACKER_API FBeamPerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam|Performance")
    float AverageFrameTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam|Performance")
    float MinFrameTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam|Performance")
    float MaxFrameTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam|Performance")
    float CPUUsage = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam|Performance")
    float MemoryUsage = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam|Performance")
    int32 DroppedFrames = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam|Performance")
    float TimeStamp = 0.0f;

    FBeamPerformanceMetrics() = default;
};

// Gaze Interaction Data
USTRUCT(BlueprintType)
struct BEAMEYETRACKER_API FGazeInteraction
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam|Interaction")
    AActor* TargetActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam|Interaction")
    UPrimitiveComponent* TargetComponent = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam|Interaction")
    FVector WorldLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam|Interaction")
    FVector2D ScreenLocation = FVector2D::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam|Interaction")
    float InteractionTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam|Interaction")
    float Confidence = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beam|Interaction")
    bool bIsValid = false;

    FGazeInteraction() = default;
};

/*=============================================================================
    End of BeamEyeTrackerTypes.h
=============================================================================*/
