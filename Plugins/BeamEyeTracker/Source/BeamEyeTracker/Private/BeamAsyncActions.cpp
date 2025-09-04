// Implements async Blueprint nodes for Beam (async execution of SDK calls)

#include "BeamAsyncActions.h"
#include "BeamEyeTrackerSubsystem.h"
#include "BeamLogging.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"

// UBeamWaitForValidGaze

UBeamWaitForValidGaze* UBeamWaitForValidGaze::WaitForValidGaze(const UObject* WorldContextObject, float MinConfidence, int32 ThrottleMs)
{
	UBeamWaitForValidGaze* Action = NewObject<UBeamWaitForValidGaze>();
	
#if !UE_BUILD_SHIPPING
	check(WorldContextObject != nullptr);
#endif
	
	Action->WorldContext = const_cast<UObject*>(WorldContextObject);
	
	Action->MinimumConfidence = FMath::Clamp(MinConfidence, 0.0f, 1.0f);
	Action->ThrottleIntervalMs = FMath::Max(ThrottleMs, 1);
	Action->LastCallbackTime = 0.0;
	
	return Action;
}

void UBeamWaitForValidGaze::Activate()
{
	// Log start of async operation for debugging
	UE_LOG(LogBeam, Log, TEXT("BeamAsyncActions: Starting WaitForValidGaze operation"));
	
	if (!WorldContext)
	{
		UE_LOG(LogBeam, Warning, TEXT("BeamAsyncActions: No WorldContext provided, completing action"));
		CompleteAction();
		return;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		UE_LOG(LogBeam, Warning, TEXT("BeamAsyncActions: Failed to get World from context, completing action"));
		CompleteAction();
		return;
	}

	// Start polling timer for gaze data
	World->GetTimerManager().SetTimer(
		PollingTimerHandle,
		this,
		&UBeamWaitForValidGaze::PollForValidGaze,
		ThrottleIntervalMs / 1000.0f, // Convert to seconds
		true // Repeat
	);
}

void UBeamWaitForValidGaze::PollForValidGaze()
{
	if (!WorldContext)
	{
		CompleteAction();
		return;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		CompleteAction();
		return;
	}

	UBeamEyeTrackerSubsystem* Subsystem = nullptr;
	if (UGameInstance* GameInstance = World->GetGameInstance())
	{
		Subsystem = GameInstance->GetSubsystem<UBeamEyeTrackerSubsystem>();
	}
	
	if (!Subsystem)
	{
		CompleteAction();
		return;
	}

	FGazePoint GazePoint = Subsystem->CurrentGaze();

	if (GazePoint.bValid && GazePoint.Confidence >= MinimumConfidence)
	{
		// Throttle callbacks to avoid excessive firing
		double CurrentTime = World->GetTimeSeconds();
		if (CurrentTime - LastCallbackTime >= (ThrottleIntervalMs / 1000.0f))
		{
			// Broadcast OnGazeReceived delegate when valid gaze is found
			// This triggers the Blueprint OnSuccess execution path
			OnGazeReceived.Broadcast(GazePoint);
			LastCallbackTime = CurrentTime;
		}
	}
}

void UBeamWaitForValidGaze::CompleteAction()
{
	
	if (WorldContext)
	{
		if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull))
		{
			World->GetTimerManager().ClearTimer(PollingTimerHandle);
		}
	}
	
	// Log completion of async operation for debugging
	UE_LOG(LogBeam, Log, TEXT("BeamAsyncActions: Completed WaitForValidGaze operation"));
	
	// Mark as completed
	SetReadyToDestroy();
}


