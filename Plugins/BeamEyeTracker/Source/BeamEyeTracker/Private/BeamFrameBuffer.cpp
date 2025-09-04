// Implements thread-safe frame buffer for Beam data

#include "BeamFrameBuffer.h"
#include "BeamEyeTrackerTypes.h"
#include "HAL/PlatformTime.h"
#include "HAL/PlatformMath.h"

// Performance optimization constants
static constexpr int32 OPTIMAL_BUFFER_SIZE = 64;
static constexpr double INTERPOLATION_THRESHOLD = 0.016; // 16ms threshold
static constexpr double MAX_FRAME_AGE_SECONDS = 1.0; // 1 second max age

FBeamFrameBuffer::FBeamFrameBuffer(int32 InBufferSize)
	: BufferSize(FMath::Max(InBufferSize, OPTIMAL_BUFFER_SIZE))
	, BufferMask(BufferSize - 1)
	, WriteIndex(0)
	, ReadIndex(0)
	, bIsFull(false)
{
	BufferSize = FMath::RoundUpToPowerOfTwo(BufferSize);
	BufferMask = BufferSize - 1;
	
	Buffer.SetNum(BufferSize);
}

FBeamFrameBuffer::~FBeamFrameBuffer()
{
}

bool FBeamFrameBuffer::Publish(const FBeamFrame& Frame)
{
	const double CurrentTime = FPlatformTime::Seconds();
	
	if (bIsFull && (CurrentTime - Buffer[ReadIndex & BufferMask].UETimestampSeconds) < INTERPOLATION_THRESHOLD)
	{
		return false;
	}
	
	FTimestampedFrame TimestampedFrame;
	TimestampedFrame.Frame = Frame;
	TimestampedFrame.Timestamp = CurrentTime;
	TimestampedFrame.UETimestampSeconds = FPlatformTime::Seconds();
	
	const int32 WriteIdx = WriteIndex.fetch_add(1, std::memory_order_relaxed);
	Buffer[WriteIdx & BufferMask] = TimestampedFrame;
	
	if (WriteIdx - ReadIndex.load(std::memory_order_relaxed) >= BufferSize)
	{
		bIsFull.store(true, std::memory_order_relaxed);
	}
	
	return true;
}

bool FBeamFrameBuffer::ReadLatest(FBeamFrame& OutFrame)
{
	const int32 CurrentReadIndex = ReadIndex.load(std::memory_order_relaxed);
	const int32 CurrentWriteIndex = WriteIndex.load(std::memory_order_relaxed);
	
	if (CurrentReadIndex >= CurrentWriteIndex)
	{
		return false; // No frames available
	}

	const int32 LatestIndex = CurrentWriteIndex - 1;
	OutFrame = Buffer[LatestIndex & BufferMask].Frame;
	
	return true;
}

bool FBeamFrameBuffer::GetFrameAt(double Timestamp, FBeamFrame& OutFrame, double Tolerance)
{
	const int32 CurrentReadIndex = ReadIndex.load(std::memory_order_relaxed);
	const int32 CurrentWriteIndex = WriteIndex.load(std::memory_order_relaxed);
	
	if (CurrentReadIndex >= CurrentWriteIndex)
	{
		return false; // No frames available
	}
	
	// Search for frame closest to timestamp
	int32 BestIndex = -1;
	double BestDifference = MAX_dbl;
	
	for (int32 i = CurrentReadIndex; i < CurrentWriteIndex; ++i)
	{
		const double FrameTime = Buffer[i & BufferMask].Timestamp;
		const double Difference = FMath::Abs(FrameTime - Timestamp);
		
		if (Difference <= Tolerance && Difference < BestDifference)
		{
			BestDifference = Difference;
			BestIndex = i;
		}
	}
	
	if (BestIndex >= 0)
	{
		OutFrame = Buffer[BestIndex & BufferMask].Frame;
		return true;
	}
	
	return false;
}

bool FBeamFrameBuffer::GetLatestInterpolatedFrame(FBeamFrame& OutFrame)
{
	const int32 CurrentReadIndex = ReadIndex.load(std::memory_order_relaxed);
	const int32 CurrentWriteIndex = WriteIndex.load(std::memory_order_relaxed);
	
	if (CurrentReadIndex >= CurrentWriteIndex)
	{
		return false; // No frames available
	}

	const int32 LatestIndex = CurrentWriteIndex - 1;
	OutFrame = Buffer[LatestIndex & BufferMask].Frame;
	
	// Simple interpolation if we have multiple frames
	if (CurrentWriteIndex - CurrentReadIndex > 1)
	{
		const int32 PreviousIndex = LatestIndex - 1;
		const FTimestampedFrame& PreviousFrame = Buffer[PreviousIndex & BufferMask];
		const FTimestampedFrame& LatestFrame = Buffer[LatestIndex & BufferMask];
		
		const double Alpha = 0.5; // Simple 50/50 blend
		
		// Interpolate gaze point
		if (PreviousFrame.Frame.Gaze.bValid && LatestFrame.Frame.Gaze.bValid)
		{
			OutFrame.Gaze.Screen01 = FMath::Lerp(PreviousFrame.Frame.Gaze.Screen01, LatestFrame.Frame.Gaze.Screen01, Alpha);
			OutFrame.Gaze.ScreenPx = FMath::Lerp(PreviousFrame.Frame.Gaze.ScreenPx, LatestFrame.Frame.Gaze.ScreenPx, Alpha);
			OutFrame.Gaze.Confidence = FMath::Lerp(PreviousFrame.Frame.Gaze.Confidence, LatestFrame.Frame.Gaze.Confidence, Alpha);
		}
		
		// Interpolate head pose
		if (PreviousFrame.Frame.Head.Confidence > 0.0 && LatestFrame.Frame.Head.Confidence > 0.0)
		{
			OutFrame.Head.PositionCm = FMath::Lerp(PreviousFrame.Frame.Head.PositionCm, LatestFrame.Frame.Head.PositionCm, Alpha);
			OutFrame.Head.Rotation = FMath::Lerp(PreviousFrame.Frame.Head.Rotation, LatestFrame.Frame.Head.Rotation, Alpha);
			OutFrame.Head.Confidence = FMath::Lerp(PreviousFrame.Frame.Head.Confidence, LatestFrame.Frame.Head.Confidence, Alpha);
		}
	}
	
	return true;
}

void FBeamFrameBuffer::Clear()
{
	WriteIndex.store(0, std::memory_order_relaxed);
	ReadIndex.store(0, std::memory_order_relaxed);
	bIsFull.store(false, std::memory_order_relaxed);
}

int32 FBeamFrameBuffer::GetSize() const
{
	return BufferSize;
}

int32 FBeamFrameBuffer::GetCount() const
{
	const int32 CurrentReadIndex = ReadIndex.load(std::memory_order_relaxed);
	const int32 CurrentWriteIndex = WriteIndex.load(std::memory_order_relaxed);
	return FMath::Max(0, CurrentWriteIndex - CurrentReadIndex);
}

bool FBeamFrameBuffer::IsEmpty() const
{
	return GetCount() == 0;
}

bool FBeamFrameBuffer::IsFull() const
{
	return bIsFull.load(std::memory_order_relaxed);
}

