/*=============================================================================
    BeamRecording.h: Recording and playback system for Beam SDK.

    Handles recording and playback of gaze/head pose sessions using
    .beamrec binary format. Provides deterministic playback suitable
    for CI testing and development purposes with comprehensive controls.

    Copyright (c) 2025 Eyeware Tech SA. All Rights Reserved.

    Eyeware® and Beam® are registered trademarks of Eyeware Tech SA.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "BeamEyeTrackerTypes.h"

/** Handles recording and playback of gaze/head pose sessions */
class BEAMEYETRACKER_API FBeamRecording
{
public:
	/** Recording header structure for .beamrec files */
	struct FRecordingHeader
	{
		uint32 Magic = 0x4245414D; // "BEAM"
		uint32 Version = 1;
		uint32 FrameCount = 0;
		uint64 StartTimestamp = 0;
		uint64 EndTimestamp = 0;
		uint32 Reserved[4] = {0};
	};

	/** Frame record structure for storing tracking data */
	struct FFrameRecord
	{
		uint64 Timestamp = 0;
		FVector2D GazeScreen01 = FVector2D::ZeroVector;
		FVector2D GazeScreenPx = FVector2D::ZeroVector;
		float GazeConfidence = 0.0f;
		FVector HeadPosition = FVector::ZeroVector;
		FRotator HeadRotation = FRotator::ZeroRotator;
		float HeadConfidence = 0.0f;
		uint32 Reserved[2] = {0};
	};

public:
	FBeamRecording();
	~FBeamRecording();

	/** Starts recording gaze data to file */
	bool StartRecording(const FString& FilePath);
	
	/** Stops recording and saves the file */
	void StopRecording();
	
	/** Checks if currently recording to a file */
	bool IsRecording() const { return bIsRecording; }
	
	/** Records a single frame of tracking data */
	void RecordFrame(const FBeamFrame& Frame);
	
	/** Starts playback from a recorded file */
	bool StartPlayback(const FString& FilePath);
	
	/** Stops playback and closes the file */
	void StopPlayback();
	
	/** Checks if currently playing back a recording */
	bool IsPlayingBack() const { return bIsPlayingBack; }
	
	/** Gets the next frame for playback */
	bool GetNextFrame(FBeamFrame& OutFrame);
	
	/** Gets playback progress as a value from 0.0 to 1.0 */
	float GetPlaybackProgress() const;
	
	/** Seeks to a specific time in the recording for playback control */
	bool SeekToTime(double TimestampMs);
	
	/** Gets recording information from a .beamrec file */
	bool GetRecordingInfo(const FString& FilePath, FRecordingHeader& OutHeader, int32& OutFrameCount) const;
	
	/** Checks if a file is a valid .beamrec recording file */
	bool IsValidRecordingFile(const FString& FilePath) const;

private:
	/** Recording state flags */
	bool bIsRecording = false;
	bool bIsPlayingBack = false;
	
	/** File handles for recording and playback operations */
	IFileHandle* RecordingFile = nullptr;
	IFileHandle* PlaybackFile = nullptr;
	
	/** Recording data structures */
	FRecordingHeader RecordingHeader;
	TArray<FFrameRecord> FrameBuffer;
	
	/** Playback data structures */
	FRecordingHeader PlaybackHeader;
	TArray<FFrameRecord> PlaybackFrames;
	int32 CurrentPlaybackIndex = 0;
	
	/** File path for current recording/playback session */
	FString CurrentFilePath;
	
	/** Writes header information to a recording file */
	bool WriteHeader(IFileHandle* File, const FRecordingHeader& Header);
	
	/** Reads header information from a recording file */
	bool ReadHeader(IFileHandle* File, FRecordingHeader& OutHeader);
	
	/** Writes a single frame to a recording file */
	bool WriteFrame(IFileHandle* File, const FFrameRecord& Frame);
	
	/** Reads a single frame from a recording file */
	bool ReadFrame(IFileHandle* File, FFrameRecord& OutFrame);
	
	/** Converts BeamFrame to FrameRecord for storage */
	FFrameRecord ConvertFrameToRecord(const FBeamFrame& Frame) const;
	
	/** Converts FrameRecord to BeamFrame for playback */
	FBeamFrame ConvertRecordToFrame(const FFrameRecord& Record) const;
	
	/** Closes all file handles and cleans up resources */
	void CloseFiles();
};

/*=============================================================================
    End of BeamRecording.h
=============================================================================*/
