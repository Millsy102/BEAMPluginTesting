/*=============================================================================
    BeamFilters.h: Data filtering algorithms and utilities for Beam SDK.

    Provides various filtering algorithms to reduce noise and jitter in
    eye tracking data. Includes One-Euro filter for adaptive smoothing
    and EMA filter for simple exponential moving average smoothing.

    Copyright (c) 2025 Eyeware Tech SA. All Rights Reserved.

    Eyeware® and Beam® are registered trademarks of Eyeware Tech SA.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "BeamEyeTrackerTypes.h"
#include "BeamFilters.generated.h"

/**
 * Signal processing filters (One Euro, EMA) for gaze/head pose smoothing.
 * 
 * Provides various filtering algorithms to reduce noise and jitter in
 * eye tracking data. Includes One-Euro filter for adaptive smoothing
 * and EMA filter for simple exponential moving average smoothing.
 */

// One-Euro filter parameters
USTRUCT(BlueprintType)
struct BEAMEYETRACKER_API FOneEuroFilterParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter", meta = (ClampMin = "0.1", ClampMax = "100.0"))
	float MinCutoff = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float Beta = 0.007f;

	/** Data rate in Hz */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter", meta = (ClampMin = "1.0", ClampMax = "1000.0"))
	float DataRate = 120.0f;

	/** Default constructor */
	FOneEuroFilterParams() = default;

	/** Constructor with parameters */
	FOneEuroFilterParams(float InMinCutoff, float InBeta, float InDataRate)
		: MinCutoff(InMinCutoff), Beta(InBeta), DataRate(InDataRate) {}
};

// EMA filter parameters
USTRUCT(BlueprintType)
struct BEAMEYETRACKER_API FEmaFilterParams
{
	GENERATED_BODY()

	/** Smoothing factor (0.0 = no smoothing, 1.0 = maximum smoothing) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Alpha = 0.1f;

	/** Enable adaptive smoothing based on data confidence */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
	bool bAdaptive = true;

	/** Minimum confidence threshold for adaptive smoothing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinConfidence = 0.5f;

	/** Default constructor */
	FEmaFilterParams() = default;

	/** Constructor with parameters */
	FEmaFilterParams(float InAlpha, bool InAdaptive = true, float InMinConfidence = 0.5f)
		: Alpha(InAlpha), bAdaptive(InAdaptive), MinConfidence(InMinConfidence) {}
};

// Filter types
UENUM()
enum class EBeamFilterType : uint8
{
	None UMETA(DisplayName = "No Filtering"),
	EMA UMETA(DisplayName = "Exponential Moving Average"),
	OneEuro UMETA(DisplayName = "One-Euro Filter")
};

/**
 * One-Euro filter implementation for gaze smoothing.
 * 
 * Implements the One-Euro filter algorithm which provides adaptive
 * smoothing based on signal velocity. Automatically adjusts smoothing
 * strength based on how fast the signal is changing.
 */
class BEAMEYETRACKER_API FOneEuroFilter
{
public:
	FOneEuroFilter(const FOneEuroFilterParams& InParams = FOneEuroFilterParams{});
	
	/** Filters a 2D gaze point using One-Euro algorithm */
	FVector2D Filter(const FVector2D& Input, double DeltaTimeSeconds);
	
	/** Resets the filter state to initial values */
	void Reset();
	
	/** Updates filter parameters at runtime */
	void UpdateParams(const FOneEuroFilterParams& NewParams);

private:
	FOneEuroFilterParams Params;
	FVector2D LastValue;
	FVector2D LastDerivative;
	bool bInitialized;
	
	/** Applies low-pass filter to value component */
	double LowPassFilter(double Input, double Alpha);
	
	/** Applies low-pass filter to derivative component */
	double LowPassFilterDerivative(double Input, double Alpha);
	
	/** Calculates alpha value for low-pass filter based on cutoff frequency */
	double CalculateAlpha(double Cutoff, double DeltaTime);
};

/**
 * EMA filter implementation for simple smoothing.
 * 
 * Provides exponential moving average smoothing for various data types.
 * Can be configured with adaptive smoothing that adjusts strength
 * based on input signal stability.
 */
class BEAMEYETRACKER_API FEmaFilter
{
public:
	FEmaFilter(const FEmaFilterParams& InParams = FEmaFilterParams{});
	
	/** Filters a 2D gaze point using EMA algorithm */
	FVector2D Filter(const FVector2D& Input);
	
	/** Filters a 3D vector using EMA algorithm */
	FVector Filter(const FVector& Input);
	
	/** Filters a rotator using EMA algorithm */
	FRotator Filter(const FRotator& Input);
	
	/** Resets the filter state to initial values */
	void Reset();
	
	/** Updates filter parameters at runtime */
	void UpdateParams(const FEmaFilterParams& NewParams);

private:
	FEmaFilterParams Params;
	FVector2D LastGazeValue;
	FVector LastPositionValue;
	FRotator LastRotationValue;
	bool bInitialized;
	
	/** Calculates adaptive alpha based on input stability for 2D vectors */
	double CalculateAdaptiveAlpha(const FVector2D& Input);
	
	/** Calculates adaptive alpha based on input stability for 3D vectors */
	double CalculateAdaptiveAlpha(const FVector& Input);
	
	/** Calculates adaptive alpha based on input stability for rotators */
	double CalculateAdaptiveAlpha(const FRotator& Input);
};

/**
 * Main filter manager for Beam eye tracking data.
 * 
 * Coordinates multiple filtering algorithms and applies them to
 * tracking frames. Automatically manages filter instances and
 * provides a unified interface for all filtering operations.
 */
class BEAMEYETRACKER_API FBeamFilters
{
public:
	FBeamFilters();
	
	/** Sets the active filter type for data processing */
	void SetFilterType(EBeamFilterType InFilterType);
	
	/** Gets the current filter type being used */
	EBeamFilterType GetFilterType() const { return CurrentFilterType; }
	
	/** Applies active filters to a tracking frame */
	void ApplyFilters(FBeamFrame& Frame, double DeltaTimeSeconds);
	
	/** Resets all filter states to initial values */
	void Reset();
	
	/** Updates One-Euro filter parameters at runtime */
	void UpdateOneEuroParams(const FOneEuroFilterParams& Params);
	
	/** Updates EMA filter parameters at runtime */
	void UpdateEmaParams(const FEmaFilterParams& Params);

private:
	/** Current filter type being used for data processing */
	EBeamFilterType CurrentFilterType = EBeamFilterType::OneEuro;
	
	/** One-Euro filter instance for adaptive smoothing */
	TUniquePtr<FOneEuroFilter> OneEuroFilter;
	
	/** EMA filter instance for simple smoothing */
	TUniquePtr<FEmaFilter> EmaFilter;
	
	/** Initializes filter instances based on current type */
	void InitializeFilters();
};

/*=============================================================================
    End of BeamFilters.h
=============================================================================*/
