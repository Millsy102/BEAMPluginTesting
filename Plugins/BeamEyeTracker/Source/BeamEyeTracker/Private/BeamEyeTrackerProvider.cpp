/*=============================================================================
    BeamEyeTrackerProvider.cpp: Provider abstraction for Beam SDK.

    Implements the FBeamEyeTrackerProvider class that provides a clean
    interface to the native Beam SDK with proper resource management.

    Copyright (c) 2025 Eyeware Tech SA. All Rights Reserved.

    Eyeware® and Beam® are registered trademarks of Eyeware Tech SA.
=============================================================================*/

#include "BeamEyeTrackerProvider.h"
#include "BeamSDK_Wrapper.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

// Provider Initialization

FBeamEyeTrackerProvider::FBeamEyeTrackerProvider()
{
	
	SDKWrapper = MakeUnique<FBeamSDK_Wrapper>();
}

FBeamEyeTrackerProvider::~FBeamEyeTrackerProvider()
{
	
	SDKWrapper.Reset();
}

// IBeamDataSource Interface Implementation

bool FBeamEyeTrackerProvider::Initialize()
{
	if (!SDKWrapper.IsValid())
	{
		return false;
	}

	return SDKWrapper->InitSDK(TEXT("BeamEyeTracker"), 1920, 1080);
}

void FBeamEyeTrackerProvider::Shutdown()
{
	if (SDKWrapper.IsValid())
	{
		SDKWrapper->Shutdown();
	}
}

bool FBeamEyeTrackerProvider::IsValid() const
{
	return SDKWrapper.IsValid() && SDKWrapper->IsSDKInitialized();
}

bool FBeamEyeTrackerProvider::FetchCurrentFrame(FBeamFrame& OutFrame)
{
	if (!IsValid())
	{
		return false;
	}

	return SDKWrapper->TryGetLatest(OutFrame);
}

EBeamHealth FBeamEyeTrackerProvider::GetHealth() const
{
	if (!SDKWrapper)
	{
		return EBeamHealth::Error;
	}

	if (SDKWrapper->IsBeamAppRunning())
	{
		return EBeamHealth::Ok;
	}
	else
	{
		return EBeamHealth::AppNotRunning;
	}
}

bool FBeamEyeTrackerProvider::StartCameraRecentering()
{
	if (!IsValid())
	{
		return false;
	}

	// Start camera recentering through the SDK wrapper
	return SDKWrapper->StartCameraRecentering();
}

void FBeamEyeTrackerProvider::EndCameraRecentering()
{
	if (IsValid())
	{
		// End camera recentering through the SDK wrapper
		SDKWrapper->EndCameraRecentering();
	}
}

// Private Implementation Methods

bool FBeamEyeTrackerProvider::InitializeSDKSource()
{
	if (!SDKWrapper)
	{
		return false;
	}

	return SDKWrapper->InitSDK(TEXT("BeamEyeTracker"), 1920, 1080);
}

bool FBeamEyeTrackerProvider::InitializeFileSource()
{
	// File-based data source for playback
	// This would read from a CSV file and simulate live data
	
	return false;
}

bool FBeamEyeTrackerProvider::InitializeNetworkSource()
{
	// Network-based data source for remote tracking
	// This would connect to a remote Beam server
	
	return false;
}

// Additional Required Interface Methods

bool FBeamEyeTrackerProvider::InitSDK(const FString& AppName, int32 ViewportWidth, int32 ViewportHeight)
{
	if (!SDKWrapper)
	{
		return false;
	}
	return SDKWrapper->InitSDK(AppName, ViewportWidth, ViewportHeight);
}

bool FBeamEyeTrackerProvider::IsSDKInitialized() const
{
	return SDKWrapper && SDKWrapper->IsSDKInitialized();
}

void FBeamEyeTrackerProvider::UpdateViewportGeometry(int32 ViewportWidth, int32 ViewportHeight)
{
	if (SDKWrapper)
	{
		SDKWrapper->UpdateViewportGeometry(ViewportWidth, ViewportHeight);
	}
}

bool FBeamEyeTrackerProvider::StartCalibration(const FString& ProfileId)
{
	if (!IsValid())
	{
		return false;
	}

	// In a full implementation, this would call the actual SDK calibration methods
	return StartCameraRecentering();
}

void FBeamEyeTrackerProvider::StopCalibration()
{
	if (IsValid())
	{
			// End camera recentering as a calibration proxy
	EndCameraRecentering();
}

/*=============================================================================
    End of BeamEyeTrackerProvider.cpp
=============================================================================*/
}


