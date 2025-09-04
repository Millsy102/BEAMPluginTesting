/*=============================================================================
    BeamAsyncActions.h: Asynchronous Blueprint actions for Beam SDK.

    Provides async Blueprint nodes for non-blocking eye tracking operations.
    Each async action automatically handles cleanup and can be configured
    with confidence thresholds and throttling for optimal performance.

    Copyright (c) 2025 Eyeware Tech SA. All Rights Reserved.

    Eyeware® and Beam® are registered trademarks of Eyeware Tech SA.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "BeamEyeTrackerTypes.h"
#include "BeamAsyncActions.generated.h"

class UBeamEyeTrackerSubsystem;

/** Provides async Blueprint nodes for eye tracking operations */
UCLASS()
class BEAMEYETRACKER_API UBeamWaitForValidGaze : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGazeReceived, const FGazePoint&, GazePoint);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTimeout);

	/** Event fired when valid gaze is received */
	UPROPERTY(BlueprintAssignable, Category = "BEAM|Events")
	FOnGazeReceived OnGazeReceived;

	/** Event fired when timeout is reached */
	UPROPERTY(BlueprintAssignable, Category = "BEAM|Events")
	FOnTimeout OnTimeout;

	/** Start waiting for valid gaze data with confidence threshold and throttling */
	UFUNCTION(BlueprintCallable, Category = "BEAM|Async", meta = (BlueprintInternalUseOnly = "true"))
	static UBeamWaitForValidGaze* WaitForValidGaze(const UObject* WorldContextObject, float MinConfidence = 0.5f, int32 ThrottleMs = 16);

	// UBlueprintAsyncActionBase interface
	virtual void Activate() override;

private:
	/** Minimum confidence threshold for valid gaze */
	float MinimumConfidence;
	
	/** Throttle interval in milliseconds */
	int32 ThrottleIntervalMs;
	
	/** Last callback time to implement throttling */
	double LastCallbackTime;
	
	/** Timer handle for polling */
	FTimerHandle PollingTimerHandle;
	
	/** World context for getting subsystem */
	UObject* WorldContext;
	
	/** Polling function called by timer */
	void PollForValidGaze();
	
	/** Clean up timer and mark as completed */
	void CompleteAction();
};

/*=============================================================================
    End of BeamAsyncActions.h
=============================================================================*/
