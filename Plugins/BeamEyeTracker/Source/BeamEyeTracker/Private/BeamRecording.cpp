// Implements recording and playback of gaze/head pose sessions

#include "BeamRecording.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "BeamLogging.h"

FBeamRecording::FBeamRecording()
	: bIsRecording(false)
	, bIsPlayingBack(false)
	, RecordingFile(nullptr)
	, PlaybackFile(nullptr)
	, CurrentPlaybackIndex(0)
{
}

FBeamRecording::~FBeamRecording()
{
	if (bIsRecording)
	{
		StopRecording();
	}
	if (bIsPlayingBack)
	{
		StopPlayback();
	}
}

bool FBeamRecording::StartRecording(const FString& FilePath)
{
	if (bIsRecording)
	{
		UE_LOG(LogBeam, Warning, TEXT("Already recording to %s"), *FilePath);
		return false;
	}

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	RecordingFile = PlatformFile.OpenWrite(*FilePath);
	
	if (!RecordingFile)
	{
		UE_LOG(LogBeam, Error, TEXT("Failed to create recording file: %s"), *FilePath);
		return false;
	}

	RecordingHeader.Magic = 0x4245414D;
	RecordingHeader.Version = 1;
	RecordingHeader.FrameCount = 0;
	RecordingHeader.StartTimestamp = FPlatformTime::Seconds() * 1000.0;
	RecordingHeader.EndTimestamp = 0;
	
	RecordingFile->Write(reinterpret_cast<const uint8*>(&RecordingHeader), sizeof(FRecordingHeader));
	
	bIsRecording = true;
	FrameBuffer.Empty();
	
	UE_LOG(LogBeam, Log, TEXT("Started recording to %s"), *FilePath);
	return true;
}

void FBeamRecording::StopRecording()
{
	if (!bIsRecording)
	{
		return;
	}

	RecordingHeader.FrameCount = FrameBuffer.Num();
	RecordingHeader.EndTimestamp = FPlatformTime::Seconds() * 1000.0; // Convert to milliseconds
	
	// Seek to beginning and rewrite header
	if (RecordingFile)
	{
		RecordingFile->Seek(0);
		RecordingFile->Write(reinterpret_cast<const uint8*>(&RecordingHeader), sizeof(FRecordingHeader));
	}

	// Close file
	if (RecordingFile)
	{
		delete RecordingFile;
		RecordingFile = nullptr;
	}

	bIsRecording = false;
	FrameBuffer.Empty();
	
	UE_LOG(LogBeam, Log, TEXT("Stopped recording, saved %d frames"), RecordingHeader.FrameCount);
}

void FBeamRecording::RecordFrame(const FBeamFrame& Frame)
{
	if (!bIsRecording || !RecordingFile)
	{
		return;
	}

	// Convert frame to record format
	FFrameRecord Record;
	Record.Timestamp = Frame.SDKTimestampMs;
	Record.GazeScreen01 = Frame.Gaze.Screen01;
	Record.GazeScreenPx = Frame.Gaze.ScreenPx;
	Record.GazeConfidence = Frame.Gaze.Confidence;
	Record.HeadPosition = Frame.Head.PositionCm;
	Record.HeadRotation = Frame.Head.Rotation;
	Record.HeadConfidence = Frame.Head.Confidence;

	RecordingFile->Write(reinterpret_cast<const uint8*>(&Record), sizeof(FFrameRecord));
	
	// Store in buffer for potential in-memory access
	FrameBuffer.Add(Record);
}

bool FBeamRecording::StartPlayback(const FString& FilePath)
{
	if (bIsPlayingBack)
	{
		UE_LOG(LogBeam, Warning, TEXT("Already playing back from %s"), *FilePath);
		return false;
	}

	// Open the playback file
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	PlaybackFile = PlatformFile.OpenRead(*FilePath);
	
	if (!PlaybackFile)
	{
		UE_LOG(LogBeam, Error, TEXT("Failed to open playback file: %s"), *FilePath);
		return false;
	}

	// Read header
	if (!PlaybackFile->Read(reinterpret_cast<uint8*>(&PlaybackHeader), sizeof(FRecordingHeader)))
	{
		UE_LOG(LogBeam, Error, TEXT("Failed to read playback file header: %s"), *FilePath);
		delete PlaybackFile;
		PlaybackFile = nullptr;
		return false;
	}

	if (PlaybackHeader.Magic != 0x4245414D)
	{
		UE_LOG(LogBeam, Error, TEXT("Invalid playback file format: %s"), *FilePath);
		delete PlaybackFile;
		PlaybackFile = nullptr;
		return false;
	}

	// Read all frames into memory
	PlaybackFrames.Empty();
	PlaybackFrames.SetNum(PlaybackHeader.FrameCount);
	
			for (uint32 i = 0; i < PlaybackHeader.FrameCount; ++i)
		{
			FFrameRecord Record;
			if (!PlaybackFile->Read(reinterpret_cast<uint8*>(&Record), sizeof(FFrameRecord)))
		{
			UE_LOG(LogBeam, Error, TEXT("Failed to read frame %d from playback file"), i);
			break;
		}
		PlaybackFrames[i] = Record;
	}

	CurrentPlaybackIndex = 0;
	bIsPlayingBack = true;
	
	UE_LOG(LogBeam, Log, TEXT("Started playback from %s with %d frames"), *FilePath, PlaybackHeader.FrameCount);
	return true;
}

void FBeamRecording::StopPlayback()
{
	if (!bIsPlayingBack)
	{
		return;
	}

	// Close file
	if (PlaybackFile)
	{
		delete PlaybackFile;
		PlaybackFile = nullptr;
	}

	bIsPlayingBack = false;
	PlaybackFrames.Empty();
	CurrentPlaybackIndex = 0;
	
	UE_LOG(LogBeam, Log, TEXT("Stopped playback"));
}

bool FBeamRecording::GetNextFrame(FBeamFrame& OutFrame)
{
	if (!bIsPlayingBack || CurrentPlaybackIndex >= PlaybackFrames.Num())
	{
		return false;
	}

	const FFrameRecord& Record = PlaybackFrames[CurrentPlaybackIndex];
	
	// Convert to Beam frame format
	OutFrame.SDKTimestampMs = Record.Timestamp;
	OutFrame.FrameId = CurrentPlaybackIndex;
	
	OutFrame.Gaze.bValid = true;
	OutFrame.Gaze.Screen01 = Record.GazeScreen01;
	OutFrame.Gaze.ScreenPx = Record.GazeScreenPx;
	OutFrame.Gaze.Confidence = Record.GazeConfidence;
	OutFrame.Gaze.TimestampMs = Record.Timestamp;
	
	OutFrame.Head.PositionCm = Record.HeadPosition;
	OutFrame.Head.Rotation = Record.HeadRotation;
	OutFrame.Head.Confidence = Record.HeadConfidence;
	OutFrame.Head.TimestampMs = Record.Timestamp;
	OutFrame.Head.TrackSessionUID = 0;
	
	CurrentPlaybackIndex++;
	return true;
}

float FBeamRecording::GetPlaybackProgress() const
{
	if (!bIsPlayingBack || PlaybackFrames.Num() == 0)
	{
		return 0.0f;
	}
	
	return static_cast<float>(CurrentPlaybackIndex) / static_cast<float>(PlaybackFrames.Num());
}

bool FBeamRecording::SeekToTime(double TimestampMs)
{
	if (!bIsPlayingBack)
	{
		return false;
	}

	// Find frame closest to the target timestamp
	int32 ClosestIndex = 0;
	double ClosestTimeDiff = FMath::Abs(PlaybackFrames[0].Timestamp - TimestampMs);
	
	for (int32 i = 1; i < PlaybackFrames.Num(); ++i)
	{
		double TimeDiff = FMath::Abs(PlaybackFrames[i].Timestamp - TimestampMs);
		if (TimeDiff < ClosestTimeDiff)
		{
			ClosestTimeDiff = TimeDiff;
			ClosestIndex = i;
		}
	}

	CurrentPlaybackIndex = ClosestIndex;
	return true;
}

bool FBeamRecording::GetRecordingInfo(const FString& FilePath, FRecordingHeader& OutHeader, int32& OutFrameCount) const
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	IFileHandle* File = PlatformFile.OpenRead(*FilePath);
	
	if (!File)
	{
		return false;
	}

	// Read header
	bool bSuccess = File->Read(reinterpret_cast<uint8*>(&OutHeader), sizeof(FRecordingHeader));
	
	if (bSuccess)
	{
		OutFrameCount = OutHeader.FrameCount;
	}
	
	delete File;
	return bSuccess;
}

bool FBeamRecording::IsValidRecordingFile(const FString& FilePath) const
{
	FRecordingHeader Header;
	int32 FrameCount;
	return GetRecordingInfo(FilePath, Header, FrameCount) && Header.Magic == 0x4245414D;
}

