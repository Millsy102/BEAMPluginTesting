/*=============================================================================
    BeamEyeTrackerInputDevice.cpp: Input device implementation for Beam Eye Tracker.

    Implements the FBeamEyeTrackerInputDevice class that provides input
    mapping and event handling for gaze and head pose data.

    Copyright (c) 2025 Eyeware Tech SA. All Rights Reserved.

    Eyeware® and Beam® are registered trademarks of Eyeware Tech SA.
=============================================================================*/

#include "BeamEyeTrackerInputDevice.h"
#include "BeamEyeTrackerSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GenericPlatform/GenericApplicationMessageHandler.h"

FBeamEyeTrackerInputDevice::FBeamEyeTrackerInputDevice(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler)
	: MessageHandler(InMessageHandler)
	, BeamSubsystem(nullptr)
	, bIsConnected(false)
	, bIsCalibrated(false)
	, ConnectionQuality(0.0f)
{
	InitializeInputMappings();
}

FBeamEyeTrackerInputDevice::~FBeamEyeTrackerInputDevice()
{
}

void FBeamEyeTrackerInputDevice::Tick(float DeltaTime)
{
	if (BeamSubsystem)
	{
		bIsConnected = BeamSubsystem->IsBeamTracking();
		bIsCalibrated = BeamSubsystem->IsCalibrated();
		ConnectionQuality = BeamSubsystem->IsBeamTracking() ? 1.0f : 0.0f;
	}
}

void FBeamEyeTrackerInputDevice::SendControllerEvents()
{
	// No controller events needed for eye tracking
}

void FBeamEyeTrackerInputDevice::SetMessageHandler(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler)
{
	MessageHandler = InMessageHandler;
}

bool FBeamEyeTrackerInputDevice::Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar)
{
	// Handle custom commands if needed
	return false;
}

void FBeamEyeTrackerInputDevice::SetChannelValue(int32 ControllerId, FForceFeedbackChannelType ChannelType, float Value)
{
	// No force feedback for eye tracking
}

void FBeamEyeTrackerInputDevice::SetChannelValues(int32 ControllerId, const FForceFeedbackValues& Values)
{
	// No force feedback for eye tracking
}

void FBeamEyeTrackerInputDevice::SendGazeInput(const FGazePoint& GazePoint)
{
	if (!GazePoint.bValid)
		return;

	float NormalizedX = (GazePoint.Screen01.X) * 2.0f - 1.0f;
	float NormalizedY = (GazePoint.Screen01.Y) * 2.0f - 1.0f;
	
	MessageHandler->OnControllerAnalog(InputMapping.GazeXAxis, FPlatformUserId(), FInputDeviceId(), NormalizedX);
	MessageHandler->OnControllerAnalog(InputMapping.GazeYAxis, FPlatformUserId(), FInputDeviceId(), NormalizedY);
}

void FBeamEyeTrackerInputDevice::SendHeadPoseInput(const FHeadPose& HeadPose)
{
	if (HeadPose.Confidence <= 0.0)
		return;

	FVector EulerAngles = HeadPose.Rotation.Euler();
	
	float NormalizedPitch = FMath::Clamp(EulerAngles.X / 180.0f, -1.0f, 1.0f);
	float NormalizedYaw = FMath::Clamp(EulerAngles.Y / 180.0f, -1.0f, 1.0f);
	float NormalizedRoll = FMath::Clamp(EulerAngles.Z / 180.0f, -1.0f, 1.0f);
	
	MessageHandler->OnControllerAnalog(InputMapping.HeadPitchAxis, FPlatformUserId(), FInputDeviceId(), NormalizedPitch);
	MessageHandler->OnControllerAnalog(InputMapping.HeadYawAxis, FPlatformUserId(), FInputDeviceId(), NormalizedYaw);
	MessageHandler->OnControllerAnalog(InputMapping.HeadRollAxis, FPlatformUserId(), FInputDeviceId(), NormalizedRoll);
}

void FBeamEyeTrackerInputDevice::InitializeInputMappings()
{
	
	InputMapping.GazeXAxis = TEXT("BeamGazeX");
	InputMapping.GazeYAxis = TEXT("BeamGazeY");
	InputMapping.HeadPitchAxis = TEXT("BeamHeadPitch");
	InputMapping.HeadYawAxis = TEXT("BeamHeadYaw");
	InputMapping.HeadRollAxis = TEXT("BeamHeadRoll");
	InputMapping.LeftEyeOpennessAxis = TEXT("BeamLeftEyeOpenness");
	InputMapping.RightEyeOpennessAxis = TEXT("BeamRightEyeOpenness");
	InputMapping.CalibrationAction = TEXT("BeamCalibration");
}

void FBeamEyeTrackerInputDevice::SendInputEvent(const FName& AxisName, float Value, int32 ControllerId)
{
	// Send axis input event to Unreal Engine using UE 5.6 compatible API
	// Clamp value to valid range and send
	float ClampedValue = FMath::Clamp(Value, -1.0f, 1.0f);
	MessageHandler->OnControllerAnalog(AxisName, FPlatformUserId(), FInputDeviceId(), ClampedValue);
}

void FBeamEyeTrackerInputDevice::SendInputAction(const FName& ActionName, bool bPressed, int32 ControllerId)
{
	// Send action input event to Unreal Engine using UE 5.6 compatible API
	if (bPressed)
	{
		MessageHandler->OnControllerButtonPressed(ActionName, FPlatformUserId(), FInputDeviceId(), false);
	}
	else
	{
		MessageHandler->OnControllerButtonReleased(ActionName, FPlatformUserId(), FInputDeviceId(), false);
	}
}

/*=============================================================================
    End of BeamEyeTrackerInputDevice.cpp
=============================================================================*/

