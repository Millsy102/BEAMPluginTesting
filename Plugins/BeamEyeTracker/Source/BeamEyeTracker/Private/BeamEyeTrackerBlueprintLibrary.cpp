// Copyright (C) 2025 Eyeware

/*=============================================================================
    BeamEyeTrackerBlueprintLibrary.cpp: Implements Blueprint Function Library for Beam Eye Tracker.
=============================================================================*/

#include "BeamEyeTrackerBlueprintLibrary.h"
#include "BeamEyeTrackerSubsystem.h"
#include "BeamEyeTrackerComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "BeamLogging.h"

UBeamEyeTrackerSubsystem* UBeamEyeTrackerBlueprintLibrary::GetBeamEyeTrackerSubsystem(const UObject* WorldContextObject)
{
	if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			return GameInstance->GetSubsystem<UBeamEyeTrackerSubsystem>();
		}
	}
	return nullptr;
}

bool UBeamEyeTrackerBlueprintLibrary::IsBeamEyeTrackingAvailable(const UObject* WorldContextObject)
{
	UBeamEyeTrackerSubsystem* Subsystem = GetBeamEyeTrackerSubsystem(WorldContextObject);
	return Subsystem != nullptr && Subsystem->IsBeamTracking();
}

bool UBeamEyeTrackerBlueprintLibrary::StartBeamEyeTracking(const UObject* WorldContextObject)
{
	UBeamEyeTrackerSubsystem* Subsystem = GetBeamEyeTrackerSubsystem(WorldContextObject);
	if (Subsystem)
	{
		return Subsystem->StartBeamTracking();
	}
	return false;
}

void UBeamEyeTrackerBlueprintLibrary::StopBeamEyeTracking(const UObject* WorldContextObject)
{
	UBeamEyeTrackerSubsystem* Subsystem = GetBeamEyeTrackerSubsystem(WorldContextObject);
	if (Subsystem)
	{
		Subsystem->StopBeamTracking();
	}
}

FGazePoint UBeamEyeTrackerBlueprintLibrary::CurrentGaze(const UObject* WorldContextObject)
{
	UBeamEyeTrackerSubsystem* Subsystem = GetBeamEyeTrackerSubsystem(WorldContextObject);
	if (Subsystem)
	{
		
		return Subsystem->CurrentGaze();
	}
	return FGazePoint();
}

FGazePoint UBeamEyeTrackerBlueprintLibrary::GetGazePointAtTime(const UObject* WorldContextObject, float TimeSeconds)
{
	UBeamEyeTrackerSubsystem* Subsystem = GetBeamEyeTrackerSubsystem(WorldContextObject);
	if (Subsystem)
	{
		
		FBeamFrame Frame;
		if (Subsystem->GetFrameAt(TimeSeconds * 1000.0, Frame))
		{
			return Frame.Gaze;
		}
	}
	return FGazePoint();
}

bool UBeamEyeTrackerBlueprintLibrary::IsGazePointValid(const UObject* WorldContextObject)
{
	UBeamEyeTrackerSubsystem* Subsystem = GetBeamEyeTrackerSubsystem(WorldContextObject);
	if (Subsystem)
	{
		
		FGazePoint GazePoint = Subsystem->CurrentGaze();
		return GazePoint.bValid && GazePoint.Confidence > 0.0;
	}
	return false;
}

float UBeamEyeTrackerBlueprintLibrary::GetGazeConfidence(const UObject* WorldContextObject)
{
	UBeamEyeTrackerSubsystem* Subsystem = GetBeamEyeTrackerSubsystem(WorldContextObject);
	if (Subsystem)
	{
		
		FGazePoint GazePoint = Subsystem->CurrentGaze();
		return static_cast<float>(GazePoint.Confidence);
	}
	return 0.0f;
}

FHeadPose UBeamEyeTrackerBlueprintLibrary::HeadPosition(const UObject* WorldContextObject)
{
	UBeamEyeTrackerSubsystem* Subsystem = GetBeamEyeTrackerSubsystem(WorldContextObject);
	if (Subsystem)
	{
		
		return Subsystem->HeadPosition();
	}
	return FHeadPose();
}

FHeadPose UBeamEyeTrackerBlueprintLibrary::GetHeadPoseAtTime(const UObject* WorldContextObject, float TimeSeconds)
{
	UBeamEyeTrackerSubsystem* Subsystem = GetBeamEyeTrackerSubsystem(WorldContextObject);
	if (Subsystem)
	{
		
		FBeamFrame Frame;
		if (Subsystem->GetFrameAt(TimeSeconds * 1000.0, Frame))
		{
			return Frame.Head;
		}
	}
	return FHeadPose();
}

bool UBeamEyeTrackerBlueprintLibrary::IsHeadPoseValid(const UObject* WorldContextObject)
{
	UBeamEyeTrackerSubsystem* Subsystem = GetBeamEyeTrackerSubsystem(WorldContextObject);
	if (Subsystem)
	{
		
		FHeadPose HeadPose = Subsystem->HeadPosition();
		return HeadPose.Confidence > 0.0;
	}
	return false;
}

float UBeamEyeTrackerBlueprintLibrary::GetHeadPoseConfidence(const UObject* WorldContextObject)
{
	UBeamEyeTrackerSubsystem* Subsystem = GetBeamEyeTrackerSubsystem(WorldContextObject);
	if (Subsystem)
	{
		
		FHeadPose HeadPose = Subsystem->HeadPosition();
		return static_cast<float>(HeadPose.Confidence);
	}
	return 0.0f;
}

bool UBeamEyeTrackerBlueprintLibrary::StartCalibration(const UObject* WorldContextObject)
{
	UBeamEyeTrackerSubsystem* Subsystem = GetBeamEyeTrackerSubsystem(WorldContextObject);
	if (Subsystem)
	{
		// Start calibration using the subsystem
		return Subsystem->StartCalibration(TEXT("Default"));
	}
	return false;
}

void UBeamEyeTrackerBlueprintLibrary::StopCalibration(const UObject* WorldContextObject)
{
	UBeamEyeTrackerSubsystem* Subsystem = GetBeamEyeTrackerSubsystem(WorldContextObject);
	if (Subsystem)
	{
		// Stop calibration using the subsystem
		Subsystem->StopCalibration();
	}
}

bool UBeamEyeTrackerBlueprintLibrary::IsCalibrating(const UObject* WorldContextObject)
{
	UBeamEyeTrackerSubsystem* Subsystem = GetBeamEyeTrackerSubsystem(WorldContextObject);
	if (Subsystem)
	{
		
		return Subsystem->IsCalibrating();
	}
	return false;
}

bool UBeamEyeTrackerBlueprintLibrary::IsCalibrated(const UObject* WorldContextObject)
{
	UBeamEyeTrackerSubsystem* Subsystem = GetBeamEyeTrackerSubsystem(WorldContextObject);
	if (Subsystem)
	{
		
		// This indicates the system is calibrated and working
		FGazePoint GazePoint = Subsystem->CurrentGaze();
		FHeadPose HeadPose = Subsystem->HeadPosition();
		
		// Consider calibrated if we have valid data with medium or high confidence
		bool bGazeCalibrated = GazePoint.bValid && GazePoint.Confidence >= 0.5f;
		bool bHeadCalibrated = HeadPose.Confidence >= 0.5f;
		
		return bGazeCalibrated && bHeadCalibrated;
	}
	return false;
}

float UBeamEyeTrackerBlueprintLibrary::GetCalibrationQuality(const UObject* WorldContextObject)
{
	UBeamEyeTrackerSubsystem* Subsystem = GetBeamEyeTrackerSubsystem(WorldContextObject);
	if (Subsystem)
	{
		// Calculate calibration quality based on tracking confidence
		FGazePoint GazePoint = Subsystem->CurrentGaze();
		FHeadPose HeadPose = Subsystem->HeadPosition();
		
		// If no data available, quality is 0
		if (!GazePoint.bValid || HeadPose.Confidence <= 0.0)
		{
			return 0.0f;
		}
		
		// Calculate average confidence as quality metric
		float GazeQuality = static_cast<float>(GazePoint.Confidence);
		float HeadQuality = static_cast<float>(HeadPose.Confidence);
		
		// Weight gaze tracking more heavily (70%) than head tracking (30%)
		float OverallQuality = (GazeQuality * 0.7f) + (HeadQuality * 0.3f);
		
		return FMath::Clamp(OverallQuality, 0.0f, 1.0f);
	}
	return 0.0f;
}

bool UBeamEyeTrackerBlueprintLibrary::StartRecording(const UObject* WorldContextObject, const FString& FilePath)
{
	UBeamEyeTrackerSubsystem* Subsystem = GetBeamEyeTrackerSubsystem(WorldContextObject);
	if (Subsystem)
	{
		// Start recording using the subsystem
		return Subsystem->StartRecording(FilePath);
	}
	return false;
}

void UBeamEyeTrackerBlueprintLibrary::StopRecording(const UObject* WorldContextObject)
{
	UBeamEyeTrackerSubsystem* Subsystem = GetBeamEyeTrackerSubsystem(WorldContextObject);
	if (Subsystem)
	{
		Subsystem->StopRecording();
		UE_LOG(LogBeam, Log, TEXT("Blueprint Library: Recording stopped"));
	}
}

bool UBeamEyeTrackerBlueprintLibrary::IsRecording(const UObject* WorldContextObject)
{
	UBeamEyeTrackerSubsystem* Subsystem = GetBeamEyeTrackerSubsystem(WorldContextObject);
	if (Subsystem)
	{
		return Subsystem->IsRecording();
	}
	return false;
}

bool UBeamEyeTrackerBlueprintLibrary::StartPlayback(const UObject* WorldContextObject, const FString& FilePath)
{
	UBeamEyeTrackerSubsystem* Subsystem = GetBeamEyeTrackerSubsystem(WorldContextObject);
	if (Subsystem)
	{
		return Subsystem->StartPlayback(FilePath);
	}
	return false;
}

void UBeamEyeTrackerBlueprintLibrary::StopPlayback(const UObject* WorldContextObject)
{
	UBeamEyeTrackerSubsystem* Subsystem = GetBeamEyeTrackerSubsystem(WorldContextObject);
	if (Subsystem)
	{
		// Stop playback using the subsystem
		Subsystem->StopPlayback();
	}
}

bool UBeamEyeTrackerBlueprintLibrary::IsPlayingBack(const UObject* WorldContextObject)
{
	UBeamEyeTrackerSubsystem* Subsystem = GetBeamEyeTrackerSubsystem(WorldContextObject);
	if (Subsystem)
	{
		return Subsystem->IsPlayingBack();
	}
	return false;
}

FVector UBeamEyeTrackerBlueprintLibrary::ProjectGazeToWorld(const UObject* WorldContextObject, const FVector2D& ScreenPosition, float Distance)
{
	UBeamEyeTrackerSubsystem* Subsystem = GetBeamEyeTrackerSubsystem(WorldContextObject);
	if (Subsystem)
	{
		
		if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
		{
			
			if (APlayerController* PC = World->GetFirstPlayerController())
			{
				// Project screen position to world using player controller
				FVector WorldLocation, WorldDirection;
				if (PC->DeprojectScreenPositionToWorld(ScreenPosition.X, ScreenPosition.Y, WorldLocation, WorldDirection))
				{
					// Return the world position at the specified distance
					return WorldLocation + (WorldDirection * Distance);
				}
			}
		}
	}
	return FVector::ZeroVector;
}

FString UBeamEyeTrackerBlueprintLibrary::GetSDKVersion(const UObject* WorldContextObject)
{
	UBeamEyeTrackerSubsystem* Subsystem = GetBeamEyeTrackerSubsystem(WorldContextObject);
	if (Subsystem)
	{
		
		return Subsystem->GetSDKVersion();
	}
	return TEXT("Unknown");
}

float UBeamEyeTrackerBlueprintLibrary::GetConnectionQuality(const UObject* WorldContextObject)
{
	UBeamEyeTrackerSubsystem* Subsystem = GetBeamEyeTrackerSubsystem(WorldContextObject);
	if (Subsystem)
	{
		
		if (Subsystem->IsBeamAppRunning())
		{
			// Return high quality if we're receiving data
			return 1.0f;
		}
		else if (Subsystem->GetBeamHealth() == EBeamHealth::AppNotRunning)
		{
			// Return low quality if app not running
			return 0.1f;
		}
		else if (Subsystem->GetBeamHealth() == EBeamHealth::Error)
		{
			// Return zero quality if there's an error
			return 0.0f;
		}
		// Return medium quality for other states
		return 0.5f;
	}
	return 0.0f;
}

bool UBeamEyeTrackerBlueprintLibrary::IsCameraConnected(const UObject* WorldContextObject)
{
	UBeamEyeTrackerSubsystem* Subsystem = GetBeamEyeTrackerSubsystem(WorldContextObject);
	if (Subsystem)
	{
		
		return Subsystem->IsBeamAppRunning() && 
			   Subsystem->GetBeamHealth() == EBeamHealth::Ok;
	}
	return false;
}

FBeamFrame UBeamEyeTrackerBlueprintLibrary::GetLatestRawFrame(const UObject* WorldContextObject)
{
	UBeamEyeTrackerSubsystem* Subsystem = GetBeamEyeTrackerSubsystem(WorldContextObject);
	if (Subsystem)
	{
		
		FBeamFrame Frame;
		if (Subsystem->FetchCurrentFrame(Frame))
		{
			return Frame;
		}
	}
	return FBeamFrame();
}

FBeamFrame UBeamEyeTrackerBlueprintLibrary::GetRawFrameAtTime(const UObject* WorldContextObject, float TimeSeconds)
{
	UBeamEyeTrackerSubsystem* Subsystem = GetBeamEyeTrackerSubsystem(WorldContextObject);
	if (Subsystem)
	{
		
		FBeamFrame Frame;
		if (Subsystem->GetFrameAt(TimeSeconds * 1000.0, Frame))
		{
			return Frame;
		}
	}
	return FBeamFrame();
}


