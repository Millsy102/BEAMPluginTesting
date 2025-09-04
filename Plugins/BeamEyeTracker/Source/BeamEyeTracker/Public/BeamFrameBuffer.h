/*=============================================================================
    BeamFrameBuffer.h: Lock-free ring buffer for Beam SDK frame storage.

    High-performance lock-free frame buffer for Beam Eye Tracker using
    atomic operations and ring buffer for thread-safe operation.
    Designed for single-producer, single-consumer (SPSC) scenarios.

    Copyright (c) 2025 Eyeware Tech SA. All Rights Reserved.

    Eyeware® and Beam® are registered trademarks of Eyeware Tech SA.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "BeamEyeTrackerTypes.h"
#include <atomic>

// Forward declaration for timestamped frame
struct FTimestampedFrame;

/** Ring buffer for storing Beam SDK frames */
class BEAMEYETRACKER_API FBeamFrameBuffer
{
public:
	FBeamFrameBuffer(int32 InBufferSize = 64);
	~FBeamFrameBuffer();

	/** Publishes a new frame to the buffer (producer thread only) */
	bool Publish(const FBeamFrame& Frame);

	/** Reads the latest frame from the buffer (consumer thread only) */
	bool ReadLatest(FBeamFrame& OutFrame);

	/** Gets frame at specific timestamp (closest match) */
	bool GetFrameAt(double Timestamp, FBeamFrame& OutFrame, double Tolerance = 0.016);

	/** Gets latest interpolated frame for smooth rendering */
	bool GetLatestInterpolatedFrame(FBeamFrame& OutFrame);

	/** Clears all frames and resets buffer state */
	void Clear();

	/** Gets current buffer size */
	int32 GetSize() const;

	/** Gets current frame count */
	int32 GetCount() const;

	/** Checks if buffer is empty (no frames available) */
	bool IsEmpty() const;

	/** Checks if buffer is full (all slots occupied) */
	bool IsFull() const;

private:
	/** Frame storage (ring buffer implementation) */
	TArray<FTimestampedFrame> Buffer;

	/** Buffer size (automatically adjusted to power of 2) */
	int32 BufferSize;

	/** Buffer mask for efficient modulo operations (size - 1) */
	int32 BufferMask;

	/** Atomic write index for thread-safe publishing */
	std::atomic<int32> WriteIndex{0};

	/** Atomic read index for thread-safe consumption */
	std::atomic<int32> ReadIndex{0};

	/** Atomic flag for buffer full state */
	std::atomic<bool> bIsFull{false};
};

// Timestamped frame structure
struct FTimestampedFrame
{
	FBeamFrame Frame;
	double Timestamp;
	double UETimestampSeconds;
};

/*=============================================================================
    End of BeamFrameBuffer.h
=============================================================================*/
