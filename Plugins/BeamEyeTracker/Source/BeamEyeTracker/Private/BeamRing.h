/*=============================================================================
    BeamRing.h: High-performance SPSC ring buffer for Beam SDK.

    Provides a lock-free single-producer, single-consumer ring buffer
    with SIMD optimization, memory pooling, advanced interpolation,
    cache-friendly layout, and comprehensive performance statistics.

    Copyright (c) 2025 Eyeware Tech SA. All Rights Reserved.

    Eyeware® and Beam® are registered trademarks of Eyeware Tech SA.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "BeamEyeTrackerTypes.h"
#include "HAL/Platform.h"
#include "HAL/PlatformAtomics.h"
#include "HAL/Platform.h"
#include <atomic>

// Performance optimization flags
#define BEAM_RING_USE_SIMD 1
#define BEAM_RING_USE_MEMORY_POOL 1
#define BEAM_RING_USE_ADVANCED_INTERPOLATION 1
#define BEAM_RING_USE_CACHE_ALIGNMENT 1
#define BEAM_RING_USE_DOUBLE_BUFFERING 1

// Cache line size for x86 architectures (64 bytes)
#define BEAM_CACHE_LINE_SIZE 64

/** High-performance, lock-free SPSC ring buffer for Beam eye tracking data */
class FBeamRing
{
public:
	FBeamRing(int32 InBufferSize);
	~FBeamRing();

	// Core ring buffer operations
	bool Publish(const FBeamFrame& Frame);
	bool ReadLatest(FBeamFrame& OutFrame) const;
	bool GetFrameAt(double TimestampMs, FBeamFrame& OutFrame) const;
	bool GetLatestInterpolatedFrame(double DeltaSeconds, FBeamFrame& OutFrame) const;
	
	// Buffer management
	void Clear();
	int32 GetBufferUtilization() const;
	int32 GetMaxSize() const;
	
	// Performance optimization features
	void PreAllocate();
	void SetAdvancedInterpolation(bool bEnable);
	void GetPerformanceStats(int32& OutFrameCount, double& OutAverageLatency, double& OutPeakLatency) const;

private:
	// Cache-aligned atomic indices to prevent false sharing
	alignas(BEAM_CACHE_LINE_SIZE) std::atomic<uint32> WriteIndex;
	alignas(BEAM_CACHE_LINE_SIZE) std::atomic<uint32> ReadIndex;
	alignas(BEAM_CACHE_LINE_SIZE) std::atomic<uint32> PublishCount;
	
	// Performance tracking (aligned to prevent false sharing)
	alignas(BEAM_CACHE_LINE_SIZE) std::atomic<double> TotalLatency;
	alignas(BEAM_CACHE_LINE_SIZE) std::atomic<double> PeakLatency;
	alignas(BEAM_CACHE_LINE_SIZE) std::atomic<int32> LatencySampleCount;
	
	// Buffer configuration
	const int32 BufferSize;
	const uint32 BufferMask;
	
	// Frame storage with cache-aligned layout
	TArray<FBeamFrame> Buffer;
	
	// Advanced features
	bool bUseAdvancedInterpolation;
	
	// Memory pooling for performance
	TArray<FBeamFrame*> MemoryPool;
	std::atomic<int32> PoolIndex;
	
	// Double buffering support
	TArray<FBeamFrame> SecondaryBuffer;
	std::atomic<bool> bUseSecondaryBuffer;
	
	// Utility functions
	bool IsPowerOfTwo(int32 Value) const;
	int32 GetNextPowerOfTwo(int32 Value) const;
	
	// Performance optimization functions
	void CopyFrameOptimized(const FBeamFrame& Source, FBeamFrame& Destination) const;
	void InterpolateFrames(const FBeamFrame& Frame1, const FBeamFrame& Frame2, 
						   double Alpha, FBeamFrame& OutInterpolatedFrame) const;
	double CalculateInterpolationWeight(double TargetTime, double Frame1Time, double Frame2Time) const;
	void UpdatePerformanceStats(double Latency);
	
	// SIMD optimization helpers
	void ProcessFrameBatchSIMD(const TArray<FBeamFrame>& InputFrames, TArray<FBeamFrame>& OutputFrames) const;
};

/*=============================================================================
    End of BeamRing.h
=============================================================================*/
