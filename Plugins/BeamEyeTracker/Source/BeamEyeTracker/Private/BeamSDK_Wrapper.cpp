// Low-level wrapper around Beam SDK (Windows only)

#include "BeamSDK_Wrapper.h"
#include "BeamEyeTrackerTypes.h"
#include "BeamLogging.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

// LogBeam is defined in BeamEyeTrackerComponent.cpp
#include "HAL/PlatformProcess.h"
#include "HAL/PlatformTime.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Async/Async.h"

// Platform safety check
#if PLATFORM_WINDOWS
	#include "Windows/AllowWindowsPlatformTypes.h"
	#include "Windows/WindowsHWrapper.h"
	#include "../../../ThirdParty/BeamSDK/include/eyeware/beam_eye_tracker.h"
	#include "Windows/HideWindowsPlatformTypes.h"
#else
	// Stub implementation for non-Windows platforms
	namespace eyeware {
		namespace beam_eye_tracker {
			class API {};
			class TrackingListener {};
			typedef uint64_t TRACKING_LISTENER_HANDLE;
			constexpr TRACKING_LISTENER_HANDLE INVALID_TRACKING_LISTENER_HANDLE = 0;
			using ViewportGeometry = EW_BET_ViewportGeometry;
		}
	}
#endif

// FBeamSDK_Wrapper Implementation

FBeamSDK_Wrapper::FBeamSDK_Wrapper()
	: APIInstance(nullptr)
	, ListenerHandle(eyeware::beam_eye_tracker::INVALID_TRACKING_LISTENER_HANDLE)
	, bInitialized(false)
	, ViewportWidth(1920)
	, ViewportHeight(1080)
{
	
	ViewportGeometry.point_00.x = 0.0f;
	ViewportGeometry.point_00.y = 0.0f;
	ViewportGeometry.point_11.x = static_cast<float>(ViewportWidth - 1);
	ViewportGeometry.point_11.y = static_cast<float>(ViewportHeight - 1);
}

FBeamSDK_Wrapper::~FBeamSDK_Wrapper()
{
	Shutdown();
}

bool FBeamSDK_Wrapper::InitSDK(const FString& ApplicationName, int32 InViewportWidth, int32 InViewportHeight)
{
#if PLATFORM_WINDOWS
	UE_LOG(LogBeam, Log, TEXT("BeamSDK: Initializing SDK for '%s' with viewport %dx%d"), 
		*ApplicationName, InViewportWidth, InViewportHeight);
	
	if (bInitialized)
	{
		UE_LOG(LogBeam, Warning, TEXT("BeamSDK: Already initialized, shutting down first"));
		Shutdown();
	}
	
	if (ApplicationName.IsEmpty() || InViewportWidth <= 0 || InViewportHeight <= 0)
	{
		UE_LOG(LogBeam, Error, TEXT("BeamSDK: Invalid parameters - Name: '%s', Width: %d, Height: %d"), 
			*ApplicationName, InViewportWidth, InViewportHeight);
		return false;
	}

	ViewportWidth = InViewportWidth;
	ViewportHeight = InViewportHeight;

	ViewportGeometry.point_00.x = 0.0f;
	ViewportGeometry.point_00.y = 0.0f;
	ViewportGeometry.point_11.x = static_cast<float>(ViewportWidth - 1);
	ViewportGeometry.point_11.y = static_cast<float>(ViewportHeight - 1);
	
	UE_LOG(LogBeam, Log, TEXT("BeamSDK: Creating API instance..."));

	APIInstance = new eyeware::beam_eye_tracker::API(TCHAR_TO_UTF8(*ApplicationName), ViewportGeometry);
	if (!APIInstance)
	{
		UE_LOG(LogBeam, Error, TEXT("BeamSDK: Failed to create API instance"));
		return false;
	}
	
	UE_LOG(LogBeam, Log, TEXT("BeamSDK: API instance created successfully, attempting to start Beam Eye Tracker..."));
	
	// Attempt to start the Beam Eye Tracker application
	APIInstance->attempt_starting_the_beam_eye_tracker();
	
	UE_LOG(LogBeam, Log, TEXT("BeamSDK: Beam Eye Tracker start attempt completed"));

	bool bBeamAppRunning = IsBeamAppRunning();
	if (bBeamAppRunning)
	{
		UE_LOG(LogBeam, Log, TEXT("BeamSDK: Beam Eye Tracker application is running and ready"));
	}
	else
	{
		UE_LOG(LogBeam, Warning, TEXT("BeamSDK: Beam Eye Tracker application is not running"));
		UE_LOG(LogBeam, Warning, TEXT("BeamSDK: Developers need to install and run Beam Eye Tracker from: https://beam.eyeware.tech"));
	}
	
	bInitialized = true;
	UE_LOG(LogBeam, Log, TEXT("BeamSDK: Initialized successfully for application '%s' with viewport %dx%d"), 
		*ApplicationName, ViewportWidth, ViewportHeight);
	
	return true;
#else
	UE_LOG(LogTemp, Warning, TEXT("BeamSDK: Not supported on this platform"));
	return false;
#endif
}

void FBeamSDK_Wrapper::Shutdown()
{
#if PLATFORM_WINDOWS
	if (APIInstance)
	{
		delete APIInstance;
		APIInstance = nullptr;
	}
#endif
	
	bInitialized = false;
	ListenerHandle = eyeware::beam_eye_tracker::INVALID_TRACKING_LISTENER_HANDLE;
}

bool FBeamSDK_Wrapper::IsSDKInitialized() const
{
	return bInitialized;
}

bool FBeamSDK_Wrapper::Start()
{
#if PLATFORM_WINDOWS
	if (!bInitialized || !APIInstance)
	{
		return false;
	}

	// In a full implementation, you'd implement the TrackingListener interface
	// and use start_receiving_tracking_data_on_listener
	return true;
#else
	return false;
#endif
}

bool FBeamSDK_Wrapper::IsRunning() const
{
	return bInitialized && APIInstance != nullptr;
}

bool FBeamSDK_Wrapper::TryGetLatest(FBeamFrame& OutFrame)
{
#if PLATFORM_WINDOWS
	if (!bInitialized || !APIInstance)
	{
		return false;
	}

	auto TrackingStateSet = APIInstance->get_latest_tracking_state_set();
	
	// Convert to our frame format
	return ConvertSDKDataToFrame(TrackingStateSet, OutFrame);
#else
	return false;
#endif
}

FString FBeamSDK_Wrapper::GetSDKVersion() const
{
#if PLATFORM_WINDOWS
	if (!bInitialized || !APIInstance)
	{
		return TEXT("Not Initialized");
	}
	
	auto Version = APIInstance->get_version();
	return FString::Printf(TEXT("%d.%d.%d.%d"), Version.major, Version.minor, Version.patch, Version.build);
#else
	return TEXT("Not Supported");
#endif
}

bool FBeamSDK_Wrapper::IsBeamAppRunning() const
{
#if PLATFORM_WINDOWS
	if (!bInitialized || !APIInstance)
	{
		return false;
	}
	
	auto Status = APIInstance->get_tracking_data_reception_status();
	return Status == eyeware::beam_eye_tracker::TrackingDataReceptionStatus::RECEIVING_TRACKING_DATA;
#else
	return false;
#endif
}

void FBeamSDK_Wrapper::UpdateViewportGeometry(int32 InViewportWidth, int32 InViewportHeight)
{
#if PLATFORM_WINDOWS
	if (InViewportWidth > 0 && InViewportHeight > 0 && APIInstance)
	{
		ViewportWidth = InViewportWidth;
		ViewportHeight = InViewportHeight;

		ViewportGeometry.point_00.x = 0.0f;
		ViewportGeometry.point_00.y = 0.0f;
		ViewportGeometry.point_11.x = static_cast<float>(ViewportWidth - 1);
		ViewportGeometry.point_11.y = static_cast<float>(ViewportHeight - 1);
		
		APIInstance->update_viewport_geometry(ViewportGeometry);
		UE_LOG(LogBeam, Log, TEXT("BeamSDK: Viewport updated to %dx%d"), ViewportWidth, ViewportHeight);
	}
	else
	{
		UE_LOG(LogBeam, Warning, TEXT("BeamSDK: Invalid viewport dimensions: %dx%d"), InViewportWidth, InViewportHeight);
	}
#endif
}

bool FBeamSDK_Wrapper::StartCameraRecentering()
{
#if PLATFORM_WINDOWS
	if (!bInitialized || !APIInstance)
	{
		return false;
	}
	
	// Camera recentering is not directly supported by the Beam SDK
	// This would need to be implemented at the application level
	UE_LOG(LogBeam, Log, TEXT("BeamSDK: Camera recentering requested (not directly supported by SDK)"));
	return true;
#else
	return false;
#endif
}

void FBeamSDK_Wrapper::EndCameraRecentering()
{
#if PLATFORM_WINDOWS
	if (bInitialized && APIInstance)
	{
		UE_LOG(LogBeam, Log, TEXT("BeamSDK: Camera recentering ended"));
	}
#endif
}

bool FBeamSDK_Wrapper::ConvertSDKDataToFrame(const eyeware::beam_eye_tracker::TrackingStateSet& TrackingStateSet, FBeamFrame& OutFrame)
{
#if PLATFORM_WINDOWS
	
	const auto& UserState = TrackingStateSet.user_state();

	if (UserState.timestamp_in_seconds == 0.0)
	{
		return false;
	}

	const auto& UnifiedScreenGaze = UserState.unified_screen_gaze;
	if (UnifiedScreenGaze.confidence == static_cast<EW_BET_TrackingConfidence>(eyeware::beam_eye_tracker::TrackingConfidence::LOST_TRACKING))
	{
		return false;
	}
	
	// Convert gaze data
	OutFrame.Gaze.bValid = true;
	OutFrame.Gaze.Screen01.X = UnifiedScreenGaze.point_of_regard.x;
	OutFrame.Gaze.Screen01.Y = UnifiedScreenGaze.point_of_regard.y;
	OutFrame.Gaze.Confidence = static_cast<float>(UnifiedScreenGaze.confidence) / 3.0f; // Convert 0-3 scale to 0-1
	OutFrame.Gaze.TimestampMs = FPlatformTime::Seconds() * 1000.0; // Convert to milliseconds

	const auto& HeadPose = UserState.head_pose;
	if (HeadPose.confidence > static_cast<EW_BET_TrackingConfidence>(eyeware::beam_eye_tracker::TrackingConfidence::LOST_TRACKING))
	{
		OutFrame.Head.PositionCm = FVector(
			HeadPose.translation_from_hcs_to_wcs.x * 100.0f, // Convert meters to cm
			HeadPose.translation_from_hcs_to_wcs.y * 100.0f,
			HeadPose.translation_from_hcs_to_wcs.z * 100.0f
		);
		
		// Convert rotation matrix to Unreal rotator
		// Extract Yaw and Pitch from the rotation matrix properly
		FMatrix RotationMatrix;
		RotationMatrix.M[0][0] = HeadPose.rotation_from_hcs_to_wcs[0][0];
		RotationMatrix.M[0][1] = HeadPose.rotation_from_hcs_to_wcs[0][1];
		RotationMatrix.M[0][2] = HeadPose.rotation_from_hcs_to_wcs[0][2];
		RotationMatrix.M[0][3] = 0.0f;
		RotationMatrix.M[1][0] = HeadPose.rotation_from_hcs_to_wcs[1][0];
		RotationMatrix.M[1][1] = HeadPose.rotation_from_hcs_to_wcs[1][1];
		RotationMatrix.M[1][2] = HeadPose.rotation_from_hcs_to_wcs[1][2];
		RotationMatrix.M[1][3] = 0.0f;
		RotationMatrix.M[2][0] = HeadPose.rotation_from_hcs_to_wcs[2][0];
		RotationMatrix.M[2][1] = HeadPose.rotation_from_hcs_to_wcs[2][1];
		RotationMatrix.M[2][2] = HeadPose.rotation_from_hcs_to_wcs[2][2];
		RotationMatrix.M[2][3] = 0.0f;
		RotationMatrix.M[3][0] = 0.0f;
		RotationMatrix.M[3][1] = 0.0f;
		RotationMatrix.M[3][2] = 0.0f;
		RotationMatrix.M[3][3] = 1.0f;
		
		// Convert to rotator and extract Yaw and Pitch for head tracking
		FRotator MatrixRotator = RotationMatrix.Rotator();
		
		// For head tracking, we want Yaw (left/right) and Pitch (up/down)
		// The Beam SDK provides head rotation relative to the camera
		OutFrame.Head.Rotation = FRotator(
			MatrixRotator.Pitch,  // Up/down head movement
			MatrixRotator.Yaw,    // Left/right head movement  
			MatrixRotator.Roll    // Head tilt (usually minimal)
		);
		OutFrame.Head.Confidence = static_cast<double>(HeadPose.confidence) / 3.0; // Convert 0-3 scale to 0-1
		OutFrame.Head.TimestampMs = FPlatformTime::Seconds() * 1000.0;
		OutFrame.Head.TrackSessionUID = HeadPose.track_session_uid;
	}
	else
	{
		OutFrame.Head.PositionCm = FVector::ZeroVector;
		OutFrame.Head.Rotation = FRotator::ZeroRotator;
		OutFrame.Head.Confidence = 0.0;
		OutFrame.Head.TimestampMs = 0.0;
		OutFrame.Head.TrackSessionUID = 0;
	}

	OutFrame.SDKTimestampMs = UserState.timestamp_in_seconds * 1000.0; // Convert seconds to milliseconds
	OutFrame.UETimestampSeconds = FPlatformTime::Seconds();
	
	return true;
#else
	return false;
#endif
}

