#include "BeamFilters.h"
#include "HAL/PlatformTime.h"
#include "HAL/PlatformMath.h"
#include "Math/UnrealMathUtility.h"

// Performance optimization flags
#define BEAM_FILTERS_USE_SIMD 1
#define BEAM_FILTERS_USE_FAST_MATH 1
#define BEAM_FILTERS_USE_APPROXIMATIONS 1

#if BEAM_FILTERS_USE_SIMD && PLATFORM_ENABLE_VECTORINTRINSICS
#include <immintrin.h>
#endif

// Fast math approximations for performance
#if BEAM_FILTERS_USE_FAST_MATH
static constexpr double FAST_PI = 3.14159265358979323846;
static constexpr double FAST_2PI = 6.28318530717958647692;
static constexpr double FAST_INV_2PI = 0.15915494309189533577;
#endif

// FOneEuroFilter Implementation

FOneEuroFilter::FOneEuroFilter(const FOneEuroFilterParams& InParams)
	: Params(InParams)
	, LastValue(FVector2D::ZeroVector)
	, LastDerivative(FVector2D::ZeroVector)
	, bInitialized(false)
{
}

FVector2D FOneEuroFilter::Filter(const FVector2D& Input, double DeltaTimeSeconds)
{
	if (!bInitialized)
	{
		LastValue = Input;
		LastDerivative = FVector2D::ZeroVector;
		bInitialized = true;
		return Input;
	}

#if BEAM_FILTERS_USE_SIMD && PLATFORM_ENABLE_VECTORINTRINSICS
	// SIMD-optimized filtering for high-performance systems
	if (PLATFORM_ENABLE_VECTORINTRINSICS)
	{
		// Calculate alpha for value filtering
		const double AlphaValue = CalculateAlpha(Params.MinCutoff, DeltaTimeSeconds);
		
		// Load vectors into SIMD registers
		__m128d InputVec = _mm_set_pd(Input.Y, Input.X);
		__m128d LastValueVec = _mm_set_pd(LastValue.Y, LastValue.X);
		__m128d AlphaVec = _mm_set1_pd(AlphaValue);
		__m128d OneMinusAlphaVec = _mm_set1_pd(1.0 - AlphaValue);
		
		// Apply low-pass filter: Alpha * Input + (1-Alpha) * LastValue
		__m128d FilteredValueVec = _mm_add_pd(
			_mm_mul_pd(AlphaVec, InputVec),
			_mm_mul_pd(OneMinusAlphaVec, LastValueVec)
		);
		
		// Extract filtered values
		double FilteredArray[2];
		_mm_store_pd(FilteredArray, FilteredValueVec);
		FVector2D FilteredValue(FilteredArray[0], FilteredArray[1]);
		
		// Calculate derivative using SIMD
		__m128d DeltaTimeVec = _mm_set1_pd(DeltaTimeSeconds);
		__m128d DerivativeVec = _mm_div_pd(
			_mm_sub_pd(FilteredValueVec, LastValueVec),
			DeltaTimeVec
		);
		
		// Apply derivative filtering
		const double AlphaDerivative = CalculateAlpha(Params.Beta, DeltaTimeSeconds);
		__m128d AlphaDerivVec = _mm_set1_pd(AlphaDerivative);
		__m128d OneMinusAlphaDerivVec = _mm_set1_pd(1.0 - AlphaDerivative);
		
		__m128d LastDerivVec = _mm_set_pd(LastDerivative.Y, LastDerivative.X);
		__m128d FilteredDerivativeVec = _mm_add_pd(
			_mm_mul_pd(AlphaDerivVec, DerivativeVec),
			_mm_mul_pd(OneMinusAlphaDerivVec, LastDerivVec)
		);
		
		// Extract filtered derivative
		double DerivArray[2];
		_mm_store_pd(DerivArray, FilteredDerivativeVec);
		FVector2D FilteredDerivative(DerivArray[0], DerivArray[1]);

		LastValue = FilteredValue;
		LastDerivative = FilteredDerivative;
		
		return FilteredValue;
	}
	else
#endif
	{
		// Standard implementation for non-SIMD platforms
		// Calculate alpha for value filtering
		const double AlphaValue = CalculateAlpha(Params.MinCutoff, DeltaTimeSeconds);
		
		// Apply low-pass filter to value
		const FVector2D FilteredValue(
			LowPassFilter(Input.X, AlphaValue),
			LowPassFilter(Input.Y, AlphaValue)
		);
		
		// Calculate alpha for derivative filtering
		const double AlphaDerivative = CalculateAlpha(Params.Beta, DeltaTimeSeconds);
		
		// Calculate derivative
		const FVector2D Derivative(
			(FilteredValue.X - LastValue.X) / DeltaTimeSeconds,
			(FilteredValue.Y - LastValue.Y) / DeltaTimeSeconds
		);
		
		// Apply low-pass filter to derivative
		const FVector2D FilteredDerivative(
			LowPassFilterDerivative(Derivative.X, AlphaDerivative),
			LowPassFilterDerivative(Derivative.Y, AlphaDerivative)
		);

		LastValue = FilteredValue;
		LastDerivative = FilteredDerivative;
		
		return FilteredValue;
	}
}

void FOneEuroFilter::Reset()
{
	LastValue = FVector2D::ZeroVector;
	LastDerivative = FVector2D::ZeroVector;
	bInitialized = false;
}

void FOneEuroFilter::UpdateParams(const FOneEuroFilterParams& NewParams)
{
	Params = NewParams;
}

double FOneEuroFilter::LowPassFilter(double Input, double Alpha)
{
	return Alpha * Input + (1.0 - Alpha) * LastValue.X; // Use X component as approximation
}

double FOneEuroFilter::LowPassFilterDerivative(double Input, double Alpha)
{
	return Alpha * Input + (1.0 - Alpha) * LastDerivative.X; // Use X component as approximation
}

double FOneEuroFilter::CalculateAlpha(double Cutoff, double DeltaTime)
{
#if BEAM_FILTERS_USE_FAST_MATH
	// Fast approximation using pre-computed constants
	const double Tau = FAST_INV_2PI / Cutoff;
#else
	const double Tau = 1.0 / (FAST_2PI * Cutoff);
#endif
	return 1.0 / (1.0 + Tau / DeltaTime);
}

// FEmaFilter Implementation

FEmaFilter::FEmaFilter(const FEmaFilterParams& InParams)
	: Params(InParams)
	, LastGazeValue(FVector2D::ZeroVector)
	, LastPositionValue(FVector::ZeroVector)
	, LastRotationValue(FRotator::ZeroRotator)
	, bInitialized(false)
{
}

FVector2D FEmaFilter::Filter(const FVector2D& Input)
{
	if (!bInitialized)
	{
		LastGazeValue = Input;
		bInitialized = true;
		return Input;
	}
	
	const double Alpha = Params.bAdaptive ? CalculateAdaptiveAlpha(Input) : Params.Alpha;
	
#if BEAM_FILTERS_USE_SIMD && PLATFORM_ENABLE_VECTORINTRINSICS
	if (PLATFORM_ENABLE_VECTORINTRINSICS)
	{
		// SIMD-optimized filtering
		__m128d InputVec = _mm_set_pd(Input.Y, Input.X);
		__m128d LastValueVec = _mm_set_pd(LastGazeValue.Y, LastGazeValue.X);
		__m128d AlphaVec = _mm_set1_pd(Alpha);
		__m128d OneMinusAlphaVec = _mm_set1_pd(1.0 - Alpha);
		
		// Apply EMA filter: Alpha * Input + (1-Alpha) * LastValue
		__m128d FilteredValueVec = _mm_add_pd(
			_mm_mul_pd(AlphaVec, InputVec),
			_mm_mul_pd(OneMinusAlphaVec, LastValueVec)
		);
		
		// Extract and store result
		double FilteredArray[2];
		_mm_store_pd(FilteredArray, FilteredValueVec);
		FVector2D FilteredValue(FilteredArray[0], FilteredArray[1]);
		
		LastGazeValue = FilteredValue;
		return FilteredValue;
	}
	else
#endif
	{
		// Standard implementation
		const FVector2D FilteredValue(
			FMath::Lerp(LastGazeValue.X, Input.X, Alpha),
			FMath::Lerp(LastGazeValue.Y, Input.Y, Alpha)
		);
		
		LastGazeValue = FilteredValue;
		return FilteredValue;
	}
}

FVector FEmaFilter::Filter(const FVector& Input)
{
	if (!bInitialized)
	{
		LastPositionValue = Input;
		bInitialized = true;
		return Input;
	}
	
	const double Alpha = Params.bAdaptive ? CalculateAdaptiveAlpha(Input) : Params.Alpha;
	
#if BEAM_FILTERS_USE_SIMD && PLATFORM_ENABLE_VECTORINTRINSICS
	if (PLATFORM_ENABLE_VECTORINTRINSICS)
	{
		// SIMD-optimized 3D vector filtering
		__m128 InputVec = _mm_set_ps(0.0f, Input.Z, Input.Y, Input.X);
		__m128 LastValueVec = _mm_set_ps(0.0f, LastPositionValue.Z, LastPositionValue.Y, LastPositionValue.X);
		__m128 AlphaVec = _mm_set1_ps(static_cast<float>(Alpha));
		__m128 OneMinusAlphaVec = _mm_set1_ps(static_cast<float>(1.0 - Alpha));
		
		// Apply EMA filter
		__m128 FilteredValueVec = _mm_add_ps(
			_mm_mul_ps(AlphaVec, InputVec),
			_mm_mul_ps(OneMinusAlphaVec, LastValueVec)
		);
		
		// Extract result
		float FilteredArray[4];
		_mm_store_ps(FilteredArray, FilteredValueVec);
		FVector FilteredValue(FilteredArray[0], FilteredArray[1], FilteredArray[2]);
		
		LastPositionValue = FilteredValue;
		return FilteredValue;
	}
	else
#endif
	{
		// Standard implementation
		const FVector FilteredValue(
			FMath::Lerp(LastPositionValue.X, Input.X, Alpha),
			FMath::Lerp(LastPositionValue.Y, Input.Y, Alpha),
			FMath::Lerp(LastPositionValue.Z, Input.Z, Alpha)
		);
		
		LastPositionValue = FilteredValue;
		return FilteredValue;
	}
}

FRotator FEmaFilter::Filter(const FRotator& Input)
{
	if (!bInitialized)
	{
		LastRotationValue = Input;
		bInitialized = true;
		return Input;
	}
	
	const double Alpha = Params.bAdaptive ? CalculateAdaptiveAlpha(Input) : Params.Alpha;
	
#if BEAM_FILTERS_USE_SIMD && PLATFORM_ENABLE_VECTORINTRINSICS
	if (PLATFORM_ENABLE_VECTORINTRINSICS)
	{
		// SIMD-optimized rotation filtering
		__m128 InputVec = _mm_set_ps(0.0f, Input.Roll, Input.Yaw, Input.Pitch);
		__m128 LastValueVec = _mm_set_ps(0.0f, LastRotationValue.Roll, LastRotationValue.Yaw, LastRotationValue.Pitch);
		__m128 AlphaVec = _mm_set1_ps(static_cast<float>(Alpha));
		__m128 OneMinusAlphaVec = _mm_set1_ps(static_cast<float>(1.0 - Alpha));
		
		// Apply EMA filter
		__m128 FilteredValueVec = _mm_add_ps(
			_mm_mul_ps(AlphaVec, InputVec),
			_mm_mul_ps(OneMinusAlphaVec, LastValueVec)
		);
		
		// Extract result
		float FilteredArray[4];
		_mm_store_ps(FilteredArray, FilteredValueVec);
		FRotator FilteredValue(FilteredArray[0], FilteredArray[1], FilteredArray[2]);
		
		LastRotationValue = FilteredValue;
		return FilteredValue;
	}
	else
#endif
	{
		// Standard implementation
		const FRotator FilteredValue(
			FMath::Lerp(LastRotationValue.Pitch, Input.Pitch, Alpha),
			FMath::Lerp(LastRotationValue.Yaw, Input.Yaw, Alpha),
			FMath::Lerp(LastRotationValue.Roll, Input.Roll, Alpha)
		);
		
		LastRotationValue = FilteredValue;
		return FilteredValue;
	}
}

void FEmaFilter::Reset()
{
	LastGazeValue = FVector2D::ZeroVector;
	LastPositionValue = FVector::ZeroVector;
	LastRotationValue = FRotator::ZeroRotator;
	bInitialized = false;
}

void FEmaFilter::UpdateParams(const FEmaFilterParams& NewParams)
{
	Params = NewParams;
}

double FEmaFilter::CalculateAdaptiveAlpha(const FVector2D& Input)
{
#if BEAM_FILTERS_USE_APPROXIMATIONS
	// Fast distance calculation using Manhattan distance approximation
	const double DeltaX = FMath::Abs(Input.X - LastGazeValue.X);
	const double DeltaY = FMath::Abs(Input.Y - LastGazeValue.Y);
	const double Distance = DeltaX + DeltaY; // Manhattan distance approximation
#else
	// Accurate Euclidean distance
	const double Distance = FVector2D::Distance(Input, LastGazeValue);
#endif

	const double NormalizedDistance = FMath::Clamp(Distance / 100.0, 0.0, 1.0);
	
	// Higher smoothing for larger changes
	return FMath::Lerp(Params.Alpha, Params.Alpha * 0.5, NormalizedDistance);
}

double FEmaFilter::CalculateAdaptiveAlpha(const FVector& Input)
{
#if BEAM_FILTERS_USE_APPROXIMATIONS
	// Fast distance calculation using Manhattan distance approximation
	const double DeltaX = FMath::Abs(Input.X - LastPositionValue.X);
	const double DeltaY = FMath::Abs(Input.Y - LastPositionValue.Y);
	const double DeltaZ = FMath::Abs(Input.Z - LastPositionValue.Z);
	const double Distance = DeltaX + DeltaY + DeltaZ; // Manhattan distance approximation
#else
	// Accurate Euclidean distance
	const double Distance = FVector::Distance(Input, LastPositionValue);
#endif

	const double NormalizedDistance = FMath::Clamp(Distance / 100.0, 0.0, 1.0);
	
	// Higher smoothing for larger changes
	return FMath::Lerp(Params.Alpha, Params.Alpha * 0.5, NormalizedDistance);
}

double FEmaFilter::CalculateAdaptiveAlpha(const FRotator& Input)
{
#if BEAM_FILTERS_USE_APPROXIMATIONS
	// Fast rotation distance calculation using absolute differences
	const double DeltaPitch = FMath::Abs(Input.Pitch - LastRotationValue.Pitch);
	const double DeltaYaw = FMath::Abs(Input.Yaw - LastRotationValue.Yaw);
	const double DeltaRoll = FMath::Abs(Input.Roll - LastRotationValue.Roll);
	const double Distance = DeltaPitch + DeltaYaw + DeltaRoll; // Sum of absolute differences
#else
	// Accurate rotation distance
	const double Distance = FVector::Distance(Input.Vector(), LastRotationValue.Vector());
#endif

	const double NormalizedDistance = FMath::Clamp(Distance / 2.0, 0.0, 1.0);
	
	// Higher smoothing for larger changes
	return FMath::Lerp(Params.Alpha, Params.Alpha * 0.5, NormalizedDistance);
}

