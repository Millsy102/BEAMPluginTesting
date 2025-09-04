/*=============================================================================
    BeamSDK_Wrapper.h: C++ wrapper for native Beam SDK integration.

    Provides C++ interface to the Beam SDK with full API integration
    for Windows platforms. Implements synchronous data access for
    real-time tracking with comprehensive coordinate mapping support.

    Copyright (c) 2025 Eyeware Tech SA. All Rights Reserved.

    Eyeware® and Beam® are registered trademarks of Eyeware Tech SA.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "BeamEyeTrackerTypes.h"
#include "HAL/Runnable.h"
#include "HAL/Platform.h"
#include "Math/Matrix.h"
#include "Math/Rotator.h"
#include "Math/Vector.h"
#include "Math/Vector2D.h"

// Beam SDK includes - using relative path to thirdparty directory
#include "../../../ThirdParty/BeamSDK/include/eyeware/beam_eye_tracker.h"

// Forward declarations
class FBeamFrameBuffer;
class FRunnableThread;

#if PLATFORM_WINDOWS
class FBeamTrackingListener;
#endif

/** C++ wrapper around the native Beam SDK (Windows only) */
class BEAMEYETRACKER_API FBeamSDK_Wrapper
{
public:
	FBeamSDK_Wrapper();
	~FBeamSDK_Wrapper();

	/** Initializes the SDK with application name and viewport dimensions */
	bool InitSDK(const FString& ApplicationName, int32 ViewportWidth, int32 ViewportHeight);

	/** Shuts down the SDK and cleans up resources */
	void Shutdown();

	/** Checks if SDK is properly initialized and ready */
	bool IsSDKInitialized() const;

	/** Starts tracking (synchronous mode) */
	bool Start();

	/** Checks if tracking is currently active and running */
	bool IsRunning() const;

	/** Gets the latest frame data from the SDK */
	bool TryGetLatest(FBeamFrame& OutFrame);

	/** Gets the SDK version string for compatibility checking */
	FString GetSDKVersion() const;

	/** Checks if the Beam application is currently running */
	bool IsBeamAppRunning() const;

	/** Updates viewport geometry for accurate coordinate mapping */
	void UpdateViewportGeometry(int32 ViewportWidth, int32 ViewportHeight);

	/** Starts camera recentering process using the SDK */
	bool StartCameraRecentering();

	/** Ends camera recentering process using the SDK */
	void EndCameraRecentering();

	/** Converts raw SDK data to the internal frame format */
	bool ConvertSDKDataToFrame(const eyeware::beam_eye_tracker::TrackingStateSet& TrackingStateSet, FBeamFrame& OutFrame);

private:
	/** SDK API instance */
	eyeware::beam_eye_tracker::API* APIInstance;

	/** Listener handle for tracking data */
	eyeware::beam_eye_tracker::TRACKING_LISTENER_HANDLE ListenerHandle;

	/** Initialization state */
	bool bInitialized;

	/** Current viewport dimensions */
	int32 ViewportWidth;
	int32 ViewportHeight;

	/** Current viewport geometry for the SDK */
	eyeware::beam_eye_tracker::EW_BET_ViewportGeometry ViewportGeometry;
};

/*=============================================================================
    End of BeamSDK_Wrapper.h
=============================================================================*/
