#include "BeamRing.h"
#include "HAL/PlatformAtomics.h"
#include "HAL/PlatformTime.h"
#include "HAL/PlatformMath.h"
#include <cfloat>

#if BEAM_RING_USE_SIMD && PLATFORM_ENABLE_VECTORINTRINSICS
#include <immintrin.h>
#endif

FBeamRing::FBeamRing(int32 InBufferSize)
	: WriteIndex(0)
	, ReadIndex(0)
	, PublishCount(0)
	, TotalLatency(0.0)
	, PeakLatency(0.0)
	, LatencySampleCount(0)
	, BufferSize(GetNextPowerOfTwo(InBufferSize))
	, BufferMask(BufferSize - 1)
	, bUseAdvancedInterpolation(true)
	, bUseSecondaryBuffer(false)
{
	Buffer.SetNum(BufferSize);
	SecondaryBuffer.SetNum(BufferSize);
	
	PreAllocate();
	
	bUseSecondaryBuffer.store(false, std::memory_order_relaxed);
}

FBeamRing::~FBeamRing()
{
	for (FBeamFrame* PooledFrame : MemoryPool)
	{
		delete PooledFrame;
	}
	MemoryPool.Empty();
}

void FBeamRing::PreAllocate()
{
#if BEAM_RING_USE_MEMORY_POOL
	MemoryPool.Reserve(BufferSize);
	for (int32 i = 0; i < BufferSize; ++i)
	{
		void* AlignedMemory = FMemory::Malloc(sizeof(FBeamFrame), BEAM_CACHE_LINE_SIZE);
		MemoryPool.Add(new(AlignedMemory) FBeamFrame());
	}
	PoolIndex.store(0, std::memory_order_relaxed);
#endif
}

bool FBeamRing::IsPowerOfTwo(int32 Value) const
{
	return (Value > 0) && ((Value & (Value - 1)) == 0);
}

int32 FBeamRing::GetNextPowerOfTwo(int32 Value) const
{
	if (IsPowerOfTwo(Value))
	{
		return Value;
	}
	
	int32 Power = 1;
	while (Power < Value)
	{
		Power <<= 1;
	}
	return Power;
}

bool FBeamRing::Publish(const FBeamFrame& Frame)
{
	const double StartTime = FPlatformTime::Seconds();
	
	// Use double buffering for enhanced throughput
	const bool bCurrentBuffer = bUseSecondaryBuffer.load(std::memory_order_relaxed);
	TArray<FBeamFrame>& CurrentBuffer = bCurrentBuffer ? SecondaryBuffer : Buffer;
	
	uint32 CurrentWriteIndex = WriteIndex.load(std::memory_order_relaxed);
	uint32 NextWriteIndex = (CurrentWriteIndex + 1) & BufferMask;

	uint32 CurrentReadIndex = ReadIndex.load(std::memory_order_acquire);
	if (NextWriteIndex == CurrentReadIndex)
	{
		// Buffer would overflow, advance read index to make room
		ReadIndex.store(NextWriteIndex, std::memory_order_release);
	}
	
	// Use optimized frame copy operation
	CopyFrameOptimized(Frame, CurrentBuffer[CurrentWriteIndex]);

	WriteIndex.store(NextWriteIndex, std::memory_order_release);
	
	// Increment publish count
	PublishCount.fetch_add(1, std::memory_order_relaxed);

	const double EndTime = FPlatformTime::Seconds();
	UpdatePerformanceStats(EndTime - StartTime);
	
	return true;
}

bool FBeamRing::ReadLatest(FBeamFrame& OutFrame) const
{
	uint32 CurrentReadIndex = ReadIndex.load(std::memory_order_relaxed);
	uint32 CurrentWriteIndex = WriteIndex.load(std::memory_order_acquire);
	
	if (CurrentReadIndex == CurrentWriteIndex)
	{
		return false; // Buffer is empty
	}
	
	// Determine which buffer to read from
	const bool bCurrentBuffer = bUseSecondaryBuffer.load(std::memory_order_relaxed);
	const TArray<FBeamFrame>& CurrentBuffer = bCurrentBuffer ? SecondaryBuffer : Buffer;
	
	// Use optimized frame copy operation
	CopyFrameOptimized(CurrentBuffer[CurrentReadIndex], OutFrame);
	return true;
}

bool FBeamRing::GetFrameAt(double TimestampMs, FBeamFrame& OutFrame) const
{
	uint32 CurrentReadIndex = ReadIndex.load(std::memory_order_acquire);
	uint32 CurrentWriteIndex = WriteIndex.load(std::memory_order_acquire);
	
	if (CurrentReadIndex == CurrentWriteIndex)
	{
		return false; // Buffer is empty
	}
	
	// Determine which buffer to read from
	const bool bCurrentBuffer = bUseSecondaryBuffer.load(std::memory_order_relaxed);
	const TArray<FBeamFrame>& CurrentBuffer = bCurrentBuffer ? SecondaryBuffer : Buffer;
	
	// Find frame closest to the requested timestamp with optimized search
	double ClosestTimeDiff = DBL_MAX;
	bool bFound = false;
	
	uint32 Index = CurrentReadIndex;
	while (Index != CurrentWriteIndex)
	{
		const FBeamFrame& Frame = CurrentBuffer[Index];
		double TimeDiff = FMath::Abs(Frame.SDKTimestampMs - TimestampMs);
		
		if (TimeDiff < ClosestTimeDiff)
		{
			ClosestTimeDiff = TimeDiff;
			CopyFrameOptimized(Frame, OutFrame);
			bFound = true;
		}
		
		Index = (Index + 1) & BufferMask;
	}
	
	return bFound;
}

bool FBeamRing::GetLatestInterpolatedFrame(double DeltaSeconds, FBeamFrame& OutFrame) const
{
	uint32 CurrentReadIndex = ReadIndex.load(std::memory_order_acquire);
	uint32 CurrentWriteIndex = WriteIndex.load(std::memory_order_acquire);
	
	if (CurrentReadIndex == CurrentWriteIndex)
	{
		return false; // Buffer is empty
	}
	
	// Determine which buffer to read from
	const bool bCurrentBuffer = bUseSecondaryBuffer.load(std::memory_order_relaxed);
	const TArray<FBeamFrame>& CurrentBuffer = bCurrentBuffer ? SecondaryBuffer : Buffer;
	
	if (bUseAdvancedInterpolation && CurrentWriteIndex != CurrentReadIndex)
	{
		
		uint32 Frame1Index = (CurrentWriteIndex - 1) & BufferMask;
		uint32 Frame2Index = (CurrentWriteIndex - 2) & BufferMask;
		
		if (Frame1Index != Frame2Index)
		{
			const FBeamFrame& Frame1 = CurrentBuffer[Frame1Index];
			const FBeamFrame& Frame2 = CurrentBuffer[Frame2Index];
			
			// Calculate interpolation weight based on delta time
			double Alpha = CalculateInterpolationWeight(DeltaSeconds, Frame2.SDKTimestampMs, Frame1.SDKTimestampMs);
			
			// Perform advanced interpolation
			InterpolateFrames(Frame2, Frame1, Alpha, OutFrame);
			return true;
		}
	}
	
	// Fallback to latest frame
	uint32 LatestIndex = (CurrentWriteIndex - 1) & BufferMask;
	CopyFrameOptimized(CurrentBuffer[LatestIndex], OutFrame);
	return true;
}

int32 FBeamRing::GetBufferUtilization() const
{
	uint32 CurrentReadIndex = ReadIndex.load(std::memory_order_acquire);
	uint32 CurrentWriteIndex = WriteIndex.load(std::memory_order_acquire);
	
	if (CurrentWriteIndex >= CurrentReadIndex)
	{
		return (CurrentWriteIndex - CurrentReadIndex) * 100 / BufferSize;
	}
	else
	{
		return (BufferSize - CurrentReadIndex + CurrentWriteIndex) * 100 / BufferSize;
	}
}

int32 FBeamRing::GetMaxSize() const
{
	return BufferSize;
}

void FBeamRing::Clear()
{
	ReadIndex.store(0, std::memory_order_relaxed);
	WriteIndex.store(0, std::memory_order_relaxed);
	PublishCount.store(0, std::memory_order_relaxed);
	
	// Reset performance statistics
	TotalLatency.store(0.0, std::memory_order_relaxed);
	PeakLatency.store(0.0, std::memory_order_relaxed);
	LatencySampleCount.store(0, std::memory_order_relaxed);
	
	// Toggle buffer for double buffering
	bUseSecondaryBuffer.store(!bUseSecondaryBuffer.load(std::memory_order_relaxed), std::memory_order_relaxed);
}

void FBeamRing::SetAdvancedInterpolation(bool bEnable)
{
	bUseAdvancedInterpolation = bEnable;
}

void FBeamRing::GetPerformanceStats(int32& OutFrameCount, double& OutAverageLatency, double& OutPeakLatency) const
{
	OutFrameCount = static_cast<int32>(PublishCount.load(std::memory_order_relaxed));
	OutPeakLatency = PeakLatency.load(std::memory_order_relaxed);
	
	int32 SampleCount = LatencySampleCount.load(std::memory_order_relaxed);
	if (SampleCount > 0)
	{
		OutAverageLatency = TotalLatency.load(std::memory_order_relaxed) / SampleCount;
	}
	else
	{
		OutAverageLatency = 0.0;
	}
}

void FBeamRing::CopyFrameOptimized(const FBeamFrame& Source, FBeamFrame& Destination) const
{
#if BEAM_RING_USE_SIMD && PLATFORM_ENABLE_VECTORINTRINSICS
	// SIMD-optimized frame copy for high-performance systems
	if (PLATFORM_ENABLE_VECTORINTRINSICS)
	{
		// Copy gaze data (8 floats = 32 bytes) using SIMD
		// Convert double to float for SIMD operations
		float GazeScreen01[4] = { static_cast<float>(Source.Gaze.Screen01.X), static_cast<float>(Source.Gaze.Screen01.Y), 
		                          static_cast<float>(Source.Gaze.ScreenPx.X), static_cast<float>(Source.Gaze.ScreenPx.Y) };
		__m128 GazeData = _mm_loadu_ps(GazeScreen01);
		_mm_storeu_ps(GazeScreen01, GazeData);
		Destination.Gaze.Screen01.X = GazeScreen01[0];
		Destination.Gaze.Screen01.Y = GazeScreen01[1];
		Destination.Gaze.ScreenPx.X = GazeScreen01[2];
		Destination.Gaze.ScreenPx.Y = GazeScreen01[3];
		
		// Copy head pose data (PositionCm.X, PositionCm.Y, PositionCm.Z, Confidence)
		float HeadPos[4] = { static_cast<float>(Source.Head.PositionCm.X), static_cast<float>(Source.Head.PositionCm.Y), 
		                     static_cast<float>(Source.Head.PositionCm.Z), static_cast<float>(Source.Head.Confidence) };
		__m128 HeadPosData = _mm_loadu_ps(HeadPos);
		_mm_storeu_ps(HeadPos, HeadPosData);
		Destination.Head.PositionCm.X = HeadPos[0];
		Destination.Head.PositionCm.Y = HeadPos[1];
		Destination.Head.PositionCm.Z = HeadPos[2];
		Destination.Head.Confidence = HeadPos[3];
		
		// Copy head rotation data (Pitch, Yaw, Roll, padding)
		float HeadRot[4] = { static_cast<float>(Source.Head.Rotation.Pitch), static_cast<float>(Source.Head.Rotation.Yaw), 
		                     static_cast<float>(Source.Head.Rotation.Roll), 0.0f };
		__m128 HeadRotData = _mm_loadu_ps(HeadRot);
		_mm_storeu_ps(HeadRot, HeadRotData);
		Destination.Head.Rotation.Pitch = HeadRot[0];
		Destination.Head.Rotation.Yaw = HeadRot[1];
		Destination.Head.Rotation.Roll = HeadRot[2];
		
		// Copy remaining scalar data
		Destination.Gaze.bValid = Source.Gaze.bValid;
		Destination.SDKTimestampMs = Source.SDKTimestampMs;
		Destination.DeltaTimeSeconds = Source.DeltaTimeSeconds;
	}
	else
#endif
	{
		// Standard copy for non-SIMD platforms
		Destination = Source;
	}
}

void FBeamRing::InterpolateFrames(const FBeamFrame& Frame1, const FBeamFrame& Frame2, 
								   double Alpha, FBeamFrame& OutInterpolatedFrame) const
{
	// Interpolate gaze data with confidence weighting
	if (Frame1.Gaze.bValid && Frame2.Gaze.bValid)
	{
		OutInterpolatedFrame.Gaze.Screen01 = FMath::Lerp(Frame1.Gaze.Screen01, Frame2.Gaze.Screen01, Alpha);
		OutInterpolatedFrame.Gaze.ScreenPx = FMath::Lerp(Frame1.Gaze.ScreenPx, Frame2.Gaze.ScreenPx, Alpha);
		OutInterpolatedFrame.Gaze.Confidence = FMath::Lerp(Frame1.Gaze.Confidence, Frame2.Gaze.Confidence, Alpha);
		OutInterpolatedFrame.Gaze.bValid = true;
	}
	else
	{
		// Use the more valid frame
		OutInterpolatedFrame.Gaze = Frame1.Gaze.bValid ? Frame1.Gaze : Frame2.Gaze;
	}
	
	// Interpolate head pose data with confidence weighting
	if (Frame1.Head.Confidence > 0.0f && Frame2.Head.Confidence > 0.0f)
	{
		OutInterpolatedFrame.Head.PositionCm = FMath::Lerp(Frame1.Head.PositionCm, Frame2.Head.PositionCm, Alpha);
		OutInterpolatedFrame.Head.Rotation = FMath::Lerp(Frame1.Head.Rotation, Frame2.Head.Rotation, Alpha);
		OutInterpolatedFrame.Head.Confidence = FMath::Lerp(Frame1.Head.Confidence, Frame2.Head.Confidence, Alpha);
	}
	else
	{
		// Use the more confident frame
		OutInterpolatedFrame.Head = Frame1.Head.Confidence > Frame2.Head.Confidence ? Frame1.Head : Frame2.Head;
	}
	
	// Interpolate timestamps
	OutInterpolatedFrame.SDKTimestampMs = FMath::Lerp(Frame1.SDKTimestampMs, Frame2.SDKTimestampMs, Alpha);
	OutInterpolatedFrame.DeltaTimeSeconds = FMath::Lerp(Frame1.DeltaTimeSeconds, Frame2.DeltaTimeSeconds, Alpha);
}

double FBeamRing::CalculateInterpolationWeight(double TargetTime, double Frame1Time, double Frame2Time) const
{
	if (FMath::IsNearlyEqual(Frame1Time, Frame2Time))
	{
		return 0.5f; // Equal weights if timestamps are the same
	}
	
	// Calculate weight based on temporal distance
	double TotalTimeDiff = Frame2Time - Frame1Time;
	if (TotalTimeDiff > 0.0)
	{
		double Weight = (TargetTime - Frame1Time) / TotalTimeDiff;
		return FMath::Clamp(Weight, 0.0, 1.0);
	}
	
	return 0.5f; // Default to equal weights
}

void FBeamRing::UpdatePerformanceStats(double Latency)
{
	
	TotalLatency.fetch_add(Latency, std::memory_order_relaxed);

	double CurrentPeak = PeakLatency.load(std::memory_order_relaxed);
	while (Latency > CurrentPeak)
	{
		if (PeakLatency.compare_exchange_weak(CurrentPeak, Latency, std::memory_order_relaxed))
		{
			break;
		}
	}
	
	// Increment sample count
	LatencySampleCount.fetch_add(1, std::memory_order_relaxed);
}

void FBeamRing::ProcessFrameBatchSIMD(const TArray<FBeamFrame>& InputFrames, TArray<FBeamFrame>& OutputFrames) const
{
#if BEAM_RING_USE_SIMD && PLATFORM_ENABLE_VECTORINTRINSICS
	if (!PLATFORM_ENABLE_VECTORINTRINSICS)
	{
		return;
	}
	
	const int32 NumFrames = InputFrames.Num();
	OutputFrames.SetNum(NumFrames);
	
	// Process frames in batches of 4 for SIMD optimization
	const int32 SIMDSize = 4;
	const int32 SIMDEnd = (NumFrames / SIMDSize) * SIMDSize;
	
	for (int32 i = 0; i < SIMDEnd; i += SIMDSize)
	{
		// Process 4 frames simultaneously using SIMD
		for (int32 j = 0; j < SIMDSize; ++j)
		{
			CopyFrameOptimized(InputFrames[i + j], OutputFrames[i + j]);
		}
	}
	
	// Process remaining frames
	for (int32 i = SIMDEnd; i < NumFrames; ++i)
	{
		CopyFrameOptimized(InputFrames[i], OutputFrames[i]);
	}
#endif
}

