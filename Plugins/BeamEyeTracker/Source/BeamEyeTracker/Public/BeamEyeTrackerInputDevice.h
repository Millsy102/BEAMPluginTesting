/*=============================================================================
    BeamEyeTrackerInputDevice.h: Custom input device for Beam eye tracking.

    Integrates seamlessly with Unreal Engine's input system, providing
    eye-tracking data as standard input events. Includes input mapping
    for gaze, head pose, and eye openness data.

    Copyright (c) 2025 Eyeware Tech SA. All Rights Reserved.

    Eyeware® and Beam® are registered trademarks of Eyeware Tech SA.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "GenericPlatform/GenericApplicationMessageHandler.h"
#include "BeamEyeTrackerTypes.h"
#include "BeamEyeTrackerSubsystem.h"

/** Custom Input Device for Beam Eye Tracker */
class BEAMEYETRACKER_API FBeamEyeTrackerInputDevice
{
public:
	FBeamEyeTrackerInputDevice(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler);
	virtual ~FBeamEyeTrackerInputDevice();

	// Input device interface methods
	virtual void Tick(float DeltaTime);
	virtual void SendControllerEvents();
	virtual void SetMessageHandler(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler);
	virtual bool Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar);
	virtual void SetChannelValue(int32 ControllerId, FForceFeedbackChannelType ChannelType, float Value);
	virtual void SetChannelValues(int32 ControllerId, const FForceFeedbackValues& Values);

	// Beam-specific input methods
	void SendGazeInput(const FGazePoint& GazePoint);
	void SendHeadPoseInput(const FHeadPose& HeadPose);

	// Input device state
	bool IsConnected() const { return bIsConnected; }
	bool IsCalibrated() const { return bIsCalibrated; }
	float GetConnectionQuality() const { return ConnectionQuality; }

private:
	/** Message handler for input events */
	TSharedRef<FGenericApplicationMessageHandler> MessageHandler;

	/** Reference to the Beam subsystem */
	UBeamEyeTrackerSubsystem* BeamSubsystem;

	/** Input device state */
	bool bIsConnected;
	bool bIsCalibrated;
	float ConnectionQuality;

	/** Input mapping for eye-tracking data */
	struct FBeamInputMapping
	{
		FName GazeXAxis;
		FName GazeYAxis;
		FName HeadPitchAxis;
		FName HeadYawAxis;
		FName HeadRollAxis;
		FName LeftEyeOpennessAxis;
		FName RightEyeOpennessAxis;
		FName CalibrationAction;
	};

	FBeamInputMapping InputMapping;

	/** Initialize input mappings */
	void InitializeInputMappings();

	/** Send input events to Unreal Engine */
	void SendInputEvent(const FName& AxisName, float Value, int32 ControllerId = 0);
	void SendInputAction(const FName& ActionName, bool bPressed, int32 ControllerId = 0);
};

/*=============================================================================
    End of BeamEyeTrackerInputDevice.h
=============================================================================*/
