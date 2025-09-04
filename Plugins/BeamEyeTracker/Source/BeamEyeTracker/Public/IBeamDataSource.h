/*=============================================================================
    IBeamDataSource.h: Data source interface for Beam Eye Tracker.

    Essential interface for Beam data sources in UE integration.
    Provides unified access to live hardware, recorded files, and synthetic data.

    Copyright (c) 2025 Eyeware Tech SA. All Rights Reserved.

    Eyeware® and Beam® are registered trademarks of Eyeware Tech SA.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "BeamEyeTrackerTypes.h"

/**
 * Essential interface for Beam data sources in UE integration.
 * 
 * Simplified interface focused on core UE integration needs.
 * Provides unified access to live hardware, recorded files, and synthetic data.
 */
class BEAMEYETRACKER_API IBeamDataSource
{
public:
	virtual ~IBeamDataSource() = default;

	/** Core lifecycle management */
	virtual bool Initialize() = 0;
	virtual void Shutdown() = 0;
	virtual bool IsValid() const = 0;

	/** Essential data access */
	virtual bool FetchCurrentFrame(FBeamFrame& OutFrame) = 0;
	virtual EBeamHealth GetHealth() const = 0;

	/** Camera recentering (core SDK functionality) */
	virtual bool StartCameraRecentering() = 0;
	virtual void EndCameraRecentering() = 0;

	/** SDK initialization and viewport management */
	virtual bool InitSDK(const FString& AppName, int32 ViewportWidth, int32 ViewportHeight) = 0;
	virtual bool IsSDKInitialized() const = 0;
	virtual void UpdateViewportGeometry(int32 ViewportWidth, int32 ViewportHeight) = 0;

	/** Calibration support */
	virtual bool StartCalibration(const FString& ProfileId) = 0;
	virtual void StopCalibration() = 0;
};

/*=============================================================================
    End of IBeamDataSource.h
=============================================================================*/

