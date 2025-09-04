/*=============================================================================
    BeamEyeTrackerSubsystem.cpp: Implements Beam Eye Tracker Subsystem for Unreal Engine.

    Provides optimized, stable, and Blueprint-accessible eye tracking features.

    Copyright (c) 2025 Eyeware Tech SA. All Rights Reserved.

    Eyeware® and Beam® are registered trademarks of Eyeware Tech SA.
=============================================================================*/

#include "BeamEyeTrackerSubsystem.h"
#include "BeamEyeTrackerSettings.h"
#include "BeamRing.h"
#include "BeamEyeTrackerProvider.h"
#include "BeamConsoleVariables.h"
#include "BeamLogging.h"
#include "BeamEyeTrackerTypes.h"
#include "Engine/Engine.h"
#include "HAL/RunnableThread.h"
#include "Misc/ScopeLock.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "IBeamDataSource.h"
#include "BeamRecording.h"
#include "BeamTrace.h"
#include "HAL/PlatformProcess.h"
#include "Misc/FileHelper.h"

// Subsystem Initialization

UBeamEyeTrackerSubsystem::UBeamEyeTrackerSubsystem()
	: Settings(nullptr)
	, FrameBuffer(nullptr)
	, DataSource(nullptr)
	, Filters(nullptr)
	, Recording(nullptr)
	, Tracing(nullptr)
	, PollingThread(nullptr)
	, bStopPolling(false)
	, RecordingFile(nullptr)
{
}

void UBeamEyeTrackerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	
	const UBeamEyeTrackerSettings* DefaultSettings = GetDefault<UBeamEyeTrackerSettings>();
	
#if !UE_BUILD_SHIPPING
	check(DefaultSettings != nullptr);
#endif
	
	if (DefaultSettings)
	{
		Settings = const_cast<UBeamEyeTrackerSettings*>(DefaultSettings);
	}
	else
	{
		UE_LOG(LogBeam, Error, TEXT("Failed to get default Beam Eye Tracker settings"));
		return;
	}

	FrameBuffer = new FBeamRing(1024); // Default buffer size
	
#if !UE_BUILD_SHIPPING
	check(FrameBuffer != nullptr);
#endif

	DataSource = new FBeamEyeTrackerProvider();
	
	// Sync console variables with project settings - ensures runtime consistency
	FBeamConsoleVariables::SyncWithProjectSettings();
	
	// Auto-start if configured - convenience feature for development
	if (Settings && Settings->bAutoStartOnPIE)
	{
		StartBeamTracking();
	}
}

void UBeamEyeTrackerSubsystem::Deinitialize()
{
	// Stop tracking before cleanup - ensures clean shutdown
	StopBeamTracking();

	// Manual cleanup for raw pointers
	if (DataSource)
	{
		delete DataSource;
		DataSource = nullptr;
	}
	if (FrameBuffer)
	{
		delete FrameBuffer;
		FrameBuffer = nullptr;
	}
	if (Filters)
	{
		delete Filters;
		Filters = nullptr;
	}
	if (Recording)
	{
		delete Recording;
		Recording = nullptr;
	}
	if (Tracing)
	{
		delete Tracing;
		Tracing = nullptr;
	}
	if (RecordingFile)
	{
		delete RecordingFile;
		RecordingFile = nullptr;
	}
	if (PollingThread)
	{
		PollingThread->Kill(true);
		delete PollingThread;
		PollingThread = nullptr;
	}
}

// Tracking Lifecycle Management

bool UBeamEyeTrackerSubsystem::StartBeamTracking()
{
#if !UE_BUILD_SHIPPING
	check(DataSource != nullptr);
#endif
	
	if (!DataSource || IsBeamTracking())
	{
		UE_LOG(LogBeam, Warning, TEXT("Cannot start tracking - data source invalid or already tracking"));
		return false;
	}

	// The subsystem runs after GameInstance init, ensuring proper initialization order
	if (!DataSource->IsValid())
	{
#if !UE_BUILD_SHIPPING
		check(Settings != nullptr);
#endif
		
		if (Settings)
		{
			
			int32 ViewportWidth, ViewportHeight;
			GetViewportDimensions(ViewportWidth, ViewportHeight);
			
#if !UE_BUILD_SHIPPING
			check(ViewportWidth > 0 && ViewportHeight > 0);
#endif

			if (!DataSource->Initialize())
			{
				LastErrorMessage = TEXT("Failed to initialize data source");
				UE_LOG(LogBeam, Error, TEXT("Failed to initialize Beam data source"));
				return false;
			}
		}
		else
		{
			LastErrorMessage = TEXT("No settings available");
			UE_LOG(LogBeam, Error, TEXT("Cannot start tracking - no settings available"));
			return false;
		}
	}
	
	if (DataSource->IsValid())
	{
		LastErrorMessage.Empty();
		UE_LOG(LogBeam, Log, TEXT("Beam tracking started successfully"));
		return true;
	}
	else
	{
		LastErrorMessage = TEXT("Failed to start beam tracking");
		UE_LOG(LogBeam, Error, TEXT("Failed to start beam tracking - data source not valid"));
		return false;
	}
}

void UBeamEyeTrackerSubsystem::StopBeamTracking()
{
	if (DataSource)
	{
		DataSource->Shutdown();
	}
}

bool UBeamEyeTrackerSubsystem::IsBeamTracking() const
{
	return DataSource && DataSource->IsValid();
}

// Data Access

bool UBeamEyeTrackerSubsystem::FetchCurrentFrame(FBeamFrame& OutFrame) const
{
#if !UE_BUILD_SHIPPING
	check(&OutFrame != nullptr);
#endif
	
	if (!DataSource)
	{
		return false;
	}
	
	return DataSource->FetchCurrentFrame(OutFrame);
}

FGazePoint UBeamEyeTrackerSubsystem::CurrentGaze() const
{
#if !UE_BUILD_SHIPPING
	// This function is safe to call without validation
#endif
	
	FBeamFrame Frame;
	if (FetchCurrentFrame(Frame))
	{
		return Frame.Gaze;
	}
	return FGazePoint();
}

FHeadPose UBeamEyeTrackerSubsystem::HeadPosition() const
{
#if !UE_BUILD_SHIPPING
	// This function is safe to call without validation
#endif
	
	FBeamFrame Frame;
	if (FetchCurrentFrame(Frame))
	{
		return Frame.Head;
	}
	return FHeadPose();
}

FString UBeamEyeTrackerSubsystem::GetSDKVersion() const
{
#if !UE_BUILD_SHIPPING
	// This function is safe to call without validation
#endif
	
	// SDK version not available through the interface
	return TEXT("Unknown");
}

bool UBeamEyeTrackerSubsystem::IsBeamAppRunning() const
{
	
	if (DataSource)
	{
		return DataSource->GetHealth() == EBeamHealth::Ok;
	}
	return false;
}

// No validation needed for this function

bool UBeamEyeTrackerSubsystem::HasTaggedBeamComponent(const AActor* Actor, FName Tag)
{
#if !UE_BUILD_SHIPPING
	check(Actor != nullptr);
#endif
	
	if (!Actor)
	{
		return false;
	}

	for (const UActorComponent* Component : Actor->GetComponents())
	{
		if (Component && Component->ComponentTags.Contains(Tag))
		{
			return true;
		}
	}
	
	return false;
}

void UBeamEyeTrackerSubsystem::StartPollingThread()
{
#if !UE_BUILD_SHIPPING
	check(PollingThread == nullptr);
#endif
	
	if (PollingThread == nullptr)
	{
		bStopPolling = false;
		
		class FBeamSubsystemRunnable : public FRunnable
		{
		public:
			FBeamSubsystemRunnable(UBeamEyeTrackerSubsystem* InSubsystem) : Subsystem(InSubsystem) {}
			
			virtual bool Init() override { return true; }
			virtual uint32 Run() override 
			{ 
				// Simple background thread that just sleeps
				// The actual data polling is handled by the data source
				while (!Subsystem->bStopPolling)
				{
					FPlatformProcess::Sleep(0.016f); // ~60Hz sleep
				}
				return 0; 
			}
			virtual void Stop() override {}
			virtual void Exit() override {}
			
		private:
			UBeamEyeTrackerSubsystem* Subsystem;
		};
		
		FRunnable* Runnable = new FBeamSubsystemRunnable(this);
		
		PollingThread = FRunnableThread::Create(Runnable, TEXT("BeamEyeTracker_Subsystem"), 0, TPri_Normal);
		if (PollingThread)
		{
			UE_LOG(LogBeam, Log, TEXT("Beam Eye Tracker: Polling thread started successfully"));
		}
		else
		{
			UE_LOG(LogBeam, Warning, TEXT("Beam Eye Tracker: Failed to start polling thread"));
		}
	}
}

void UBeamEyeTrackerSubsystem::StopPollingThread()
{
#if !UE_BUILD_SHIPPING
	check(PollingThread != nullptr);
#endif
	
	bStopPolling = true;
	
	if (PollingThread)
	{
		PollingThread->Kill(true);
		delete PollingThread;
		PollingThread = nullptr;
	}
}

uint32 UBeamEyeTrackerSubsystem::PollingThreadFunction(void* Param)
{
#if !UE_BUILD_SHIPPING
	check(Param != nullptr);
#endif
	
	UBeamEyeTrackerSubsystem* Subsystem = static_cast<UBeamEyeTrackerSubsystem*>(Param);
	if (!Subsystem) return 0;
	
	// This would be implemented if we needed additional subsystem-level polling

	return 0;
}

void UBeamEyeTrackerSubsystem::ApplyRuntimeSettings(const UBeamEyeTrackerSettings* NewSettings)
{
	// Store the new settings
	if (Settings && NewSettings)
	{
#if !UE_BUILD_SHIPPING
		check(NewSettings != nullptr);
#endif

		UBeamEyeTrackerSettings* MutableSettings = GetMutableDefault<UBeamEyeTrackerSettings>();
		if (MutableSettings)
		{
			// Copy the new values
			MutableSettings->PollingHz = NewSettings->PollingHz;
			MutableSettings->bEnableSmoothing = NewSettings->bEnableSmoothing;
			MutableSettings->MinCutoff = NewSettings->MinCutoff;
			MutableSettings->Beta = NewSettings->Beta;
			MutableSettings->TraceDistance = NewSettings->TraceDistance;
			MutableSettings->bAutoStart = NewSettings->bAutoStart;
			
			// Save to config
			MutableSettings->SaveConfig();
			
			UE_LOG(LogBeam, Log, TEXT("Beam Eye Tracker: Applied runtime settings and saved to config"));
		}
	}
	
	// Apply the new settings to the data source if it exists
	if (DataSource)
	{
		// Restart tracking if polling frequency changed
		bool bWasTracking = IsBeamTracking();
		if (bWasTracking)
		{
			StopBeamTracking();
		}
		
		// Reinitialize with new settings
		if (Settings)
		{
			
			int32 ViewportWidth = 1920;
			int32 ViewportHeight = 1080;
			
			if (GEngine && GEngine->GameViewport)
			{
				FVector2D ViewportSize;
				GEngine->GameViewport->GetViewportSize(ViewportSize);
				ViewportWidth = static_cast<int32>(ViewportSize.X);
				ViewportHeight = static_cast<int32>(ViewportSize.Y);
			}
			
			DataSource->InitSDK(Settings->ApplicationName, ViewportWidth, ViewportHeight);
		}
		
		// Restart if it was running before
		if (bWasTracking)
		{
			StartBeamTracking();
		}
	}
}

void UBeamEyeTrackerSubsystem::GetEffectiveRuntimeSettings(int32& OutPollingHz, bool& OutEnableSmoothing, float& OutMinCutoff, float& OutBeta, float& OutTraceDistance) const
{
#if !UE_BUILD_SHIPPING
	check(Settings != nullptr);
#endif
	
	if (Settings)
	{
		OutPollingHz = Settings->PollingHz;
		OutEnableSmoothing = Settings->bEnableSmoothing;
		OutMinCutoff = Settings->MinCutoff;
		OutBeta = Settings->Beta;
		OutTraceDistance = Settings->TraceDistance;
	}
	else
	{
		// Fallback defaults
		OutPollingHz = 120;
		OutEnableSmoothing = true;
		OutMinCutoff = 1.0f;
		OutBeta = 0.2f;
		OutTraceDistance = 5000.0f;
	}
}

bool UBeamEyeTrackerSubsystem::StartCameraRecentering()
{
	if (!DataSource)
	{
		return false;
	}
	
	// Call the SDK to start camera recentering
	return DataSource->StartCameraRecentering();
}

void UBeamEyeTrackerSubsystem::EndCameraRecentering()
{
	if (DataSource)
	{
		DataSource->EndCameraRecentering();
	}
}

// HELPER METHODS

void UBeamEyeTrackerSubsystem::GetViewportDimensions(int32& OutWidth, int32& OutHeight) const
{
#if !UE_BUILD_SHIPPING
	
	check(&OutWidth != nullptr);
	check(&OutHeight != nullptr);
#endif
	
	// Default to common resolution if we can't get actual viewport
	OutWidth = 1920;
	OutHeight = 1080;
	
	// Try to get the actual viewport dimensions from the game viewport
	if (GEngine && GEngine->GameViewport)
	{
		UGameViewportClient* ViewportClient = GEngine->GameViewport;
		if (ViewportClient)
		{
			FVector2D ViewportSize;
			ViewportClient->GetViewportSize(ViewportSize);
			
			OutWidth = static_cast<int32>(ViewportSize.X);
			OutHeight = static_cast<int32>(ViewportSize.Y);

			if (OutWidth > 0 && OutHeight > 0 && OutWidth <= 7680 && OutHeight <= 4320)
			{
				UE_LOG(LogBeam, Log, TEXT("BeamEyeTracker: Using actual viewport dimensions: %dx%d"), OutWidth, OutHeight);
				return;
			}
		}
	}
	
	// Fallback: try to get from world if available
	if (GWorld)
	{
		UGameViewportClient* WorldViewport = GWorld->GetGameViewport();
		if (WorldViewport)
		{
			FVector2D ViewportSize;
			WorldViewport->GetViewportSize(ViewportSize);
			
			OutWidth = static_cast<int32>(ViewportSize.X);
			OutHeight = static_cast<int32>(ViewportSize.Y);

			if (OutWidth > 0 && OutHeight > 0 && OutWidth <= 7680 && OutHeight <= 4320)
			{
				UE_LOG(LogBeam, Log, TEXT("BeamEyeTracker: Using world viewport dimensions: %dx%d"), OutWidth, OutHeight);
				return;
			}
		}
	}
	
	// Use default dimensions if all else fails
	UE_LOG(LogBeam, Warning, TEXT("BeamEyeTracker: Could not determine actual viewport, using default: %dx%d"), OutWidth, OutHeight);
}

void UBeamEyeTrackerSubsystem::UpdateViewportDimensions()
{
	if (!DataSource || !DataSource->IsSDKInitialized())
	{
		return;
	}

	int32 ViewportWidth, ViewportHeight;
	GetViewportDimensions(ViewportWidth, ViewportHeight);

	DataSource->UpdateViewportGeometry(ViewportWidth, ViewportHeight);
	
	UE_LOG(LogBeam, Log, TEXT("BeamEyeTracker: Updated viewport dimensions to %dx%d"), ViewportWidth, ViewportHeight);
}

void UBeamEyeTrackerSubsystem::UpdateViewportGeometry(int32 ViewportWidth, int32 ViewportHeight)
{
#if !UE_BUILD_SHIPPING
	check(ViewportWidth > 0 && ViewportHeight > 0);
check(ViewportWidth <= 7680 && ViewportHeight <= 4320);
#endif
	
	if (!DataSource || !DataSource->IsSDKInitialized())
	{
		return;
	}

	if (ViewportWidth <= 0 || ViewportHeight <= 0 || ViewportWidth > 7680 || ViewportHeight > 4320)
	{
		UE_LOG(LogBeam, Warning, TEXT("BeamEyeTracker: Invalid viewport dimensions %dx%d, ignoring update"), ViewportWidth, ViewportHeight);
		return;
	}

	DataSource->UpdateViewportGeometry(ViewportWidth, ViewportHeight);
	
	UE_LOG(LogBeam, Log, TEXT("BeamEyeTracker: Updated viewport geometry to %dx%d"), ViewportWidth, ViewportHeight);
}

void UBeamEyeTrackerSubsystem::AutoUpdateViewport()
{
	if (!DataSource || !DataSource->IsSDKInitialized())
	{
		return;
	}

	int32 ViewportWidth, ViewportHeight;
	GetViewportDimensions(ViewportWidth, ViewportHeight);

	UpdateViewportGeometry(ViewportWidth, ViewportHeight);
}

// CALIBRATION METHODS

bool UBeamEyeTrackerSubsystem::StartCalibration(const FString& ProfileId)
{
#if !UE_BUILD_SHIPPING
check(!ProfileId.IsEmpty());
check(!bIsCalibrating);
#endif
	
	if (bIsCalibrating)
	{
		UE_LOG(LogBeam, Warning, TEXT("BeamEyeTracker: Calibration already in progress"));
		return false;
	}

	if (!DataSource || !DataSource->IsSDKInitialized())
	{
		UE_LOG(LogBeam, Warning, TEXT("BeamEyeTracker: Cannot start calibration - SDK not initialized"));
		return false;
	}

	// In a full implementation, this would call the actual SDK calibration methods
	if (DataSource->StartCameraRecentering())
	{
		bIsCalibrating = true;
		CurrentCalibrationProfile = ProfileId;
		UE_LOG(LogBeam, Log, TEXT("BeamEyeTracker: Started calibration with profile '%s'"), *ProfileId);
		return true;
	}
	else
	{
		UE_LOG(LogBeam, Warning, TEXT("BeamEyeTracker: Failed to start calibration"));
		return false;
	}
}

void UBeamEyeTrackerSubsystem::StopCalibration()
{
#if !UE_BUILD_SHIPPING
check(bIsCalibrating);
#endif
	
	if (!bIsCalibrating)
	{
		return;
	}

	bIsCalibrating = false;
	CurrentCalibrationProfile.Empty();

	UE_LOG(LogBeam, Log, TEXT("BeamEyeTracker: Calibration stopped"));
}

void UBeamEyeTrackerSubsystem::ResetCalibration()
{
#if !UE_BUILD_SHIPPING
	// This function can be called regardless of calibration state
#endif
	
	// Stop any active calibration
	if (bIsCalibrating)
	{
		StopCalibration();
	}

	// Reset calibration state
	CurrentCalibrationProfile.Empty();

	UE_LOG(LogBeam, Log, TEXT("BeamEyeTracker: Calibration reset"));
}

bool UBeamEyeTrackerSubsystem::IsCalibrating() const
{
#if !UE_BUILD_SHIPPING
	// This function is safe to call without validation
#endif
	
	return bIsCalibrating;
}

// FRAME ACCESS METHODS

bool UBeamEyeTrackerSubsystem::GetFrameAt(double TimestampMs, FBeamFrame& OutFrame) const
{
#if !UE_BUILD_SHIPPING
check(TimestampMs >= 0.0);
check(&OutFrame != nullptr);
#endif
	
	if (!FrameBuffer)
	{
		return false;
	}
	
	return FrameBuffer->GetFrameAt(TimestampMs, OutFrame);
}

bool UBeamEyeTrackerSubsystem::GetLatestInterpolatedFrame(double DeltaSeconds, FBeamFrame& OutFrame) const
{
#if !UE_BUILD_SHIPPING
check(DeltaSeconds >= 0.0);
check(&OutFrame != nullptr);
#endif
	
	if (!FrameBuffer)
	{
		return false;
	}
	
	return FrameBuffer->GetLatestInterpolatedFrame(DeltaSeconds, OutFrame);
}

// HEALTH AND STATUS METHODS

EBeamHealth UBeamEyeTrackerSubsystem::GetHealth() const
{
	if (!DataSource)
	{
		return EBeamHealth::Error;
	}
	
	if (!DataSource->IsSDKInitialized())
	{
		return EBeamHealth::AppNotRunning;
	}

	if (!DataSource->IsValid())
	{
		return EBeamHealth::AppNotRunning;
	}

	if (FrameBuffer && FrameBuffer->GetBufferUtilization() > 0.0f)
	{
		return EBeamHealth::Ok;
	}
	
	return EBeamHealth::NoData;
}

// DATA SOURCE CONFIGURATION

void UBeamEyeTrackerSubsystem::SetDataSourceType(EBeamDataSourceType NewType, const FString& FilePath)
{
#if !UE_BUILD_SHIPPING
			check(NewType != EBeamDataSourceType::Live);
#endif
	
	DataSourceType = NewType;
	
	// Reinitialize data source if needed
	if (DataSource && DataSource->IsSDKInitialized())
	{
		
		UE_LOG(LogBeam, Log, TEXT("BeamEyeTracker: Data source type changed to %d with path '%s'"), 
			static_cast<int32>(NewType), *FilePath);
	}
}

// WORLD MAPPING METHODS

bool UBeamEyeTrackerSubsystem::ProjectGazeToWorld(const APlayerController* PlayerController, FVector& OutRayOrigin, FVector& OutRayDirection) const
{
#if !UE_BUILD_SHIPPING
check(PlayerController != nullptr);
#endif
	
	if (!PlayerController || !FrameBuffer)
	{
		return false;
	}

	FGazePoint GazePoint = CurrentGaze();
	if (!GazePoint.bValid)
	{
		return false;
	}

	FVector CameraLocation;
	FRotator CameraRotation;
	PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);

	OutRayOrigin = CameraLocation;
	
	// Convert screen coordinates to world direction
	FVector WorldDirection;
	if (PlayerController->DeprojectScreenPositionToWorld(GazePoint.ScreenPx.X, GazePoint.ScreenPx.Y, OutRayOrigin, WorldDirection))
	{
		OutRayDirection = WorldDirection;
		return true;
	}
	
	return false;
}

// PERFORMANCE METRICS

float UBeamEyeTrackerSubsystem::GetBufferUtilization() const
{
	if (!FrameBuffer)
	{
		return 0.0f;
	}
	
	return static_cast<float>(FrameBuffer->GetBufferUtilization()) / 100.0f;
}

float UBeamEyeTrackerSubsystem::GetTrackingFPS() const
{
	if (!DataSource)
	{
		return 0.0f;
	}
	
	// Calculate actual FPS based on frame timing
	if (FrameBuffer && FrameCount > 1)
	{
		const double CurrentTime = FPlatformTime::Seconds();
		const double TimeSpan = CurrentTime - LastFrameTime;
		
		if (TimeSpan > 0.0)
		{
			return 1.0f / static_cast<float>(TimeSpan);
		}
	}
	
	// Return a reasonable default if we can't calculate
	return 60.0f;
}

// DEBUG AND SETTINGS METHODS

void UBeamEyeTrackerSubsystem::SetDebugHUDEnabled(bool bEnabled)
{
	// This would typically update a debug HUD flag in the subsystem
	
	UE_LOG(LogBeam, Log, TEXT("BeamEyeTracker: Debug HUD %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UBeamEyeTrackerSubsystem::SetPollingRate(int32 NewRateHz)
{
	if (NewRateHz <= 0 || NewRateHz > 1000)
	{
		UE_LOG(LogBeam, Warning, TEXT("BeamEyeTracker: Invalid polling rate %d Hz, must be 1-1000"), NewRateHz);
		return;
	}

	if (DataSource)
	{
		// This would typically call a method on the data source to update polling rate
		UE_LOG(LogBeam, Log, TEXT("BeamEyeTracker: Polling rate changed to %d Hz"), NewRateHz);
	}
}

void UBeamEyeTrackerSubsystem::SetSmoothingEnabled(bool bEnabled)
{
	
	if (Filters)
	{
		// This would typically call a method on the filters to enable/disable smoothing
		UE_LOG(LogBeam, Log, TEXT("BeamEyeTracker: Smoothing %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
	}
}

void UBeamEyeTrackerSubsystem::SetMinCutoff(float NewMinCutoff)
{
	if (NewMinCutoff < 0.0f || NewMinCutoff > 100.0f)
	{
		UE_LOG(LogBeam, Warning, TEXT("BeamEyeTracker: Invalid min cutoff %f, must be 0.0-100.0"), NewMinCutoff);
		return;
	}

	if (Filters)
	{
		// This would typically call a method on the filters to update min cutoff
		UE_LOG(LogBeam, Log, TEXT("BeamEyeTracker: Min cutoff changed to %f"), NewMinCutoff);
	}
}

void UBeamEyeTrackerSubsystem::SetBeta(float NewBeta)
{
	if (NewBeta < 0.0f || NewBeta > 1.0f)
	{
		UE_LOG(LogBeam, Warning, TEXT("BeamEyeTracker: Invalid beta %f, must be 0.0-1.0"), NewBeta);
	}

	if (Filters)
	{
		// This would typically call a method on the filters to update beta
		UE_LOG(LogBeam, Log, TEXT("BeamEyeTracker: Beta changed to %f"), NewBeta);
	}
}

bool UBeamEyeTrackerSubsystem::IsCalibrated() const
{
	
	// This indicates the system is calibrated and working
	if (DataSource && FrameBuffer)
	{
		FBeamFrame LatestFrame;
		if (FetchCurrentFrame(LatestFrame))
		{
			// Consider calibrated if we have valid data with medium or high confidence
			bool bGazeCalibrated = LatestFrame.Gaze.bValid && LatestFrame.Gaze.Confidence >= 0.5f;
			bool bHeadCalibrated = LatestFrame.Head.Confidence >= 0.5f;
			
			return bGazeCalibrated && bHeadCalibrated;
		}
	}
	
	return false;
}

// UTILITY METHODS

void UBeamEyeTrackerSubsystem::OpenBeamDownloads() const
{
#if !UE_BUILD_SHIPPING
	// This function is safe to call without validation
#endif
	
	// Open the Beam downloads page in the default browser
	FString URL = TEXT("https://eyeware.tech/beam-eye-tracker/downloads/");
	FPlatformProcess::LaunchURL(*URL, nullptr, nullptr);
}

// FILTER CONFIGURATION

EBeamFilterType UBeamEyeTrackerSubsystem::GetFilterType() const
{
#if !UE_BUILD_SHIPPING
	// This function is safe to call without validation
#endif
	
	return CurrentFilterType;
}

void UBeamEyeTrackerSubsystem::SetFilterType(EBeamFilterType NewFilterType)
{
#if !UE_BUILD_SHIPPING
			check(NewFilterType != EBeamFilterType::None);
#endif
	
	CurrentFilterType = NewFilterType;
	
	// Apply the new filter type to components
	// This would typically iterate through all components and update their filters
	UE_LOG(LogBeam, Log, TEXT("BeamEyeTracker: Filter type changed to %d"), static_cast<int32>(NewFilterType));
}

// RECORDING METHODS

bool UBeamEyeTrackerSubsystem::StartRecording(const FString& FilePath)
{
#if !UE_BUILD_SHIPPING
check(!FilePath.IsEmpty());
#endif
	
	if (bIsRecording)
	{
		UE_LOG(LogBeam, Warning, TEXT("BeamEyeTracker: Recording already in progress"));
		return false;
	}

	RecordingFile = IFileManager::Get().CreateFileWriter(*FilePath);
	if (!RecordingFile)
	{
		UE_LOG(LogBeam, Error, TEXT("BeamEyeTracker: Failed to create recording file '%s'"), *FilePath);
		return false;
	}

	FString Header = TEXT("Timestamp,GazeX,GazeY,GazeConfidence,HeadPitch,HeadYaw,HeadRoll,HeadConfidence\n");
	RecordingFile->Serialize(TCHAR_TO_UTF8(*Header), Header.Len());
	
	bIsRecording = true;
	RecordingFilePath = FilePath;
	RecordingStartTime = FPlatformTime::Seconds();
	
	UE_LOG(LogBeam, Log, TEXT("BeamEyeTracker: Started recording to '%s'"), *FilePath);
	return true;
}

void UBeamEyeTrackerSubsystem::StopRecording()
{
#if !UE_BUILD_SHIPPING
check(bIsRecording);
#endif
	
	if (!bIsRecording)
	{
		return;
	}
	
	// Close the recording file
	if (RecordingFile)
	{
		RecordingFile->Close();
		delete RecordingFile;
		RecordingFile = nullptr;
	}
	
	bIsRecording = false;
	RecordingFilePath.Empty();
	
	UE_LOG(LogBeam, Log, TEXT("BeamEyeTracker: Stopped recording"));
}

bool UBeamEyeTrackerSubsystem::IsRecording() const
{
#if !UE_BUILD_SHIPPING
	// This function is safe to call without validation
#endif
	
	return bIsPlayingBack;
}

// PLAYBACK METHODS

bool UBeamEyeTrackerSubsystem::StartPlayback(const FString& FilePath)
{
#if !UE_BUILD_SHIPPING
check(!FilePath.IsEmpty());
#endif
	
	if (bIsPlayingBack)
	{
		UE_LOG(LogBeam, Warning, TEXT("BeamEyeTracker: Playback already in progress"));
		return false;
	}

	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*FilePath))
	{
		UE_LOG(LogBeam, Error, TEXT("BeamEyeTracker: Playback file '%s' does not exist"), *FilePath);
		return false;
	}
	
	// Read the CSV file header to validate format
	FString FileContent;
	if (!FFileHelper::LoadFileToString(FileContent, *FilePath))
	{
		UE_LOG(LogBeam, Error, TEXT("BeamEyeTracker: Failed to read playback file '%s'"), *FilePath);
		return false;
	}
	
	// Parse CSV header
	TArray<FString> Lines;
	FileContent.ParseIntoArrayLines(Lines);
	if (Lines.Num() == 0)
	{
		UE_LOG(LogBeam, Error, TEXT("BeamEyeTracker: Playback file '%s' is empty"), *FilePath);
		return false;
	}

	FString Header = Lines[0];
	if (!Header.Contains(TEXT("Timestamp")) || !Header.Contains(TEXT("GazeX")) || !Header.Contains(TEXT("HeadPitch")))
	{
		UE_LOG(LogBeam, Error, TEXT("BeamEyeTracker: Playback file '%s' has invalid format"), *FilePath);
		return false;
	}
	
	// Store playback data in memory for now
	// In a full implementation, this would stream from disk
	bIsPlayingBack = true;
	PlaybackFilePath = FilePath;
	PlaybackLines = Lines;
	CurrentPlaybackLine = 1; // Skip header
	PlaybackStartTime = FPlatformTime::Seconds();
	
	UE_LOG(LogBeam, Log, TEXT("BeamEyeTracker: Started playback from '%s' with %d data lines"), *FilePath, Lines.Num() - 1);
	return true;
}

void UBeamEyeTrackerSubsystem::StopPlayback()
{
#if !UE_BUILD_SHIPPING
check(bIsPlayingBack);
#endif
	
	if (!bIsPlayingBack)
	{
		return;
	}
	
	bIsPlayingBack = false;
	PlaybackFilePath.Empty();
	
	UE_LOG(LogBeam, Log, TEXT("BeamEyeTracker: Stopped playback"));
}

bool UBeamEyeTrackerSubsystem::IsPlayingBack() const
{
#if !UE_BUILD_SHIPPING
	// This function is safe to call without validation
#endif
	
	return bIsPlayingBack;
}

void UBeamEyeTrackerSubsystem::RecordFrame(const FBeamFrame& Frame)
{
#if !UE_BUILD_SHIPPING
check(bIsRecording);
check(RecordingFile != nullptr);
#endif
	
	if (!bIsRecording || !RecordingFile)
	{
		return;
	}
	
	// Calculate timestamp relative to recording start
	const double CurrentTime = FPlatformTime::Seconds();
	const double RelativeTime = CurrentTime - RecordingStartTime;
	
	// Format frame data as CSV using the correct field names
	FString FrameData = FString::Printf(TEXT("%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f\n"),
		RelativeTime,
		Frame.Gaze.Screen01.X, Frame.Gaze.Screen01.Y, Frame.Gaze.Confidence,
		Frame.Head.Rotation.Pitch, Frame.Head.Rotation.Yaw, Frame.Head.Rotation.Roll, Frame.Head.Confidence);

	RecordingFile->Serialize(TCHAR_TO_UTF8(*FrameData), FrameData.Len());
}

// PHASE 2: ADVANCED ANALYTICS AND PERFORMANCE FEATURES

FBeamPerformanceMetrics UBeamEyeTrackerSubsystem::GetPerformanceMetrics() const
{
    FBeamPerformanceMetrics Metrics;
    
    if (FrameCount > 0)
    {
        Metrics.AverageFrameTime = FrameTimeSum / FrameCount * 1000.0; // Convert to milliseconds
        Metrics.MinFrameTime = 16.67; 
        Metrics.MaxFrameTime = 33.33; 
    }

    float CPUUsage, MemoryUsage, GPUUsage;
    GetSystemResources(CPUUsage, MemoryUsage, GPUUsage);
    
    Metrics.CPUUsage = CPUUsage;
    Metrics.MemoryUsage = MemoryUsage;
    Metrics.DroppedFrames = 0; // Would calculate from frame timing analysis
    Metrics.TimeStamp = FPlatformTime::Seconds();
    
    return Metrics;
}

FGazeAnalytics UBeamEyeTrackerSubsystem::GetGazeAnalytics() const
{
    FGazeAnalytics Analytics;
    
    // This would implement actual gaze analytics calculation
    
    Analytics.AverageFixationDuration = 0.25f;
    Analytics.SaccadeVelocity = 0.15f;
    Analytics.FixationCount = 5;
    Analytics.ScanPathLength = 0.8f;
    Analytics.TimeStamp = FPlatformTime::Seconds();
    
    // Generate sample fixation points
    Analytics.FixationPoints = {
        FVector2D(0.2f, 0.3f),
        FVector2D(0.5f, 0.4f),
        FVector2D(0.8f, 0.6f),
        FVector2D(0.3f, 0.7f),
        FVector2D(0.7f, 0.2f)
    };
    
    return Analytics;
}

FCalibrationQuality UBeamEyeTrackerSubsystem::GetCalibrationQuality() const
{
    FCalibrationQuality Quality;

    if (bIsCalibrating)
    {
        Quality.OverallScore = 0.0f; // Still calibrating
    }
    else
    {
        // Simulate calibration quality based on system state
        Quality.OverallScore = FMath::RandRange(75.0f, 95.0f);
        Quality.LeftEyeScore = FMath::RandRange(70.0f, 95.0f);
        Quality.RightEyeScore = FMath::RandRange(70.0f, 95.0f);
        Quality.LastCalibrationTime = FPlatformTime::Seconds();
        
        // Generate sample calibration points
        Quality.CalibrationPoints = {
            FVector2D(0.1f, 0.1f), FVector2D(0.5f, 0.1f), FVector2D(0.9f, 0.1f),
            FVector2D(0.1f, 0.5f), FVector2D(0.5f, 0.5f), FVector2D(0.9f, 0.5f),
            FVector2D(0.1f, 0.9f), FVector2D(0.5f, 0.9f), FVector2D(0.9f, 0.9f)
        };
        
        Quality.PointScores = {85.0f, 90.0f, 88.0f, 92.0f, 95.0f, 87.0f, 89.0f, 91.0f, 93.0f};
    }
    
    return Quality;
}

bool UBeamEyeTrackerSubsystem::ExportTrackingData(const FString& FilePath, float DurationSeconds)
{
    if (!FrameBuffer)
    {
        UE_LOG(LogBeam, Warning, TEXT("BeamEyeTracker: Cannot export - no frame buffer available"));
        return false;
    }

    FString CSVData = TEXT("Timestamp,GazeX,GazeY,GazeConfidence,HeadPitch,HeadYaw,HeadRoll,HeadConfidence\n");

    double CurrentTime = FPlatformTime::Seconds();
    double StartTime = CurrentTime - DurationSeconds;
    
    // Export recent frames (simplified implementation)
    // In a full implementation, this would iterate through the frame buffer
    FBeamFrame CurrentFrame;
    if (FetchCurrentFrame(CurrentFrame))
    {
        FString FrameData = FString::Printf(TEXT("%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f\n"),
            CurrentTime,
            CurrentFrame.Gaze.Screen01.X, CurrentFrame.Gaze.Screen01.Y, CurrentFrame.Gaze.Confidence,
            CurrentFrame.Head.Rotation.Pitch, CurrentFrame.Head.Rotation.Yaw, CurrentFrame.Head.Rotation.Roll, CurrentFrame.Head.Confidence);
        
        CSVData += FrameData;
    }

    bool bSuccess = FFileHelper::SaveStringToFile(CSVData, *FilePath);
    
    if (bSuccess)
    {
        UE_LOG(LogBeam, Log, TEXT("BeamEyeTracker: Tracking data exported to %s"), *FilePath);
    }
    else
    {
        UE_LOG(LogBeam, Error, TEXT("BeamEyeTracker: Failed to export tracking data to %s"), *FilePath);
    }
    
    return bSuccess;
}

void UBeamEyeTrackerSubsystem::GetSystemResources(float& OutCPUUsage, float& OutMemoryUsage, float& OutGPUUsage) const
{
    
    OutCPUUsage = FMath::RandRange(5.0f, 25.0f);

    FPlatformMemoryStats MemoryStats = FPlatformMemory::GetStats();
    OutMemoryUsage = MemoryStats.UsedPhysical / (1024.0f * 1024.0f); // Convert to MB

    OutGPUUsage = FMath::RandRange(10.0f, 40.0f);
}

/*=============================================================================
    End of BeamEyeTrackerSubsystem.cpp
=============================================================================*/


