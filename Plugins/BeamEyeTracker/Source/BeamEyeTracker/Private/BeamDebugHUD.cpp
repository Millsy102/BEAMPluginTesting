// Implements debug HUD widget for Beam Eye Tracker

#include "BeamDebugHUD.h"
#include "BeamEyeTrackerSubsystem.h"
#include "BeamEyeTrackerComponent.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Blueprint/UserWidget.h"
#include "BeamLogging.h"

UBeamDebugHUD::UBeamDebugHUD(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, BeamSubsystem(nullptr)
{
}

void UBeamDebugHUD::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			BeamSubsystem = GameInstance->GetSubsystem<UBeamEyeTrackerSubsystem>();
		}
	}
	
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(UpdateTimerHandle, this, &UBeamDebugHUD::UpdateStatusDisplay, 0.1f, true);
	}
	
	UE_LOG(LogBeam, Log, TEXT("Beam Debug HUD constructed"));
}

void UBeamDebugHUD::NativeDestruct()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(UpdateTimerHandle);
	}
	
	Super::NativeDestruct();
}

void UBeamDebugHUD::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (GEngine && GEngine->IsEditor())
	{
		
		static auto CVar = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("beam.debug.enabled"));
		if (CVar && CVar->GetValueOnGameThread() != 0)
		{
			UpdateStatusDisplay();
		}
	}
	else
	{
		// Always update in standalone/game
		UpdateStatusDisplay();
	}
}

void UBeamDebugHUD::UpdateStatusDisplay()
{
	// This would update the actual UI elements
	
	if (BeamSubsystem)
	{
		UE_LOG(LogBeam, VeryVerbose, TEXT("Debug HUD: Updating status display"));
	}
}

void UBeamDebugHUD::UpdateGazeDisplay()
{
	if (BeamSubsystem)
	{
		UE_LOG(LogBeam, VeryVerbose, TEXT("Debug HUD: Updating gaze display"));
	}
}

void UBeamDebugHUD::UpdatePerformanceDisplay()
{
	if (BeamSubsystem)
	{
		UE_LOG(LogBeam, VeryVerbose, TEXT("Debug HUD: Updating performance display"));
	}
}

FText UBeamDebugHUD::GetTrackingStatusText() const
{
	if (BeamSubsystem && BeamSubsystem->IsBeamTracking())
	{
		return FText::FromString(TEXT("Tracking: Active"));
	}
	return FText::FromString(TEXT("Tracking: Inactive"));
}

FText UBeamDebugHUD::GetHealthStatusText() const
{
	if (BeamSubsystem)
	{
		EBeamHealth Health = BeamSubsystem->GetBeamHealth();
		switch (Health)
		{
		case EBeamHealth::Ok:
			return FText::FromString(TEXT("Health: Good"));
		case EBeamHealth::Warning:
			return FText::FromString(TEXT("Health: Warning"));
		case EBeamHealth::Error:
			return FText::FromString(TEXT("Health: Error"));
		case EBeamHealth::AppNotRunning:
			return FText::FromString(TEXT("Health: App Not Running"));
		default:
			return FText::FromString(TEXT("Health: Unknown"));
		}
	}
	return FText::FromString(TEXT("Health: Unknown"));
}

FText UBeamDebugHUD::GetFPSText() const
{
	if (GEngine)
	{
		// Use a simple FPS calculation since GetAverageFPS is not available in UE 5.6
		static double LastTime = FPlatformTime::Seconds();
		static int32 FrameCount = 0;
		static float CalculatedFPS = 0.0f;
		
		double CurrentTime = FPlatformTime::Seconds();
		FrameCount++;
		
		if (CurrentTime - LastTime >= 1.0)
		{
			CalculatedFPS = FrameCount / (CurrentTime - LastTime);
			LastTime = CurrentTime;
			FrameCount = 0;
		}
		
		return FText::FromString(FString::Printf(TEXT("FPS: %.1f"), CalculatedFPS));
	}
	return FText::FromString(TEXT("FPS: Unknown"));
}

FText UBeamDebugHUD::GetBufferUtilizationText() const
{
	if (BeamSubsystem)
	{
		
		float Utilization = BeamSubsystem->GetBufferUtilization();
		return FText::FromString(FString::Printf(TEXT("Buffer: %.1f%%"), Utilization * 100.0f));
	}
	return FText::FromString(TEXT("Buffer: Unknown"));
}

FText UBeamDebugHUD::GetGazePositionText() const
{
	if (BeamSubsystem)
	{
		FGazePoint GazePoint = BeamSubsystem->CurrentGaze();
		if (GazePoint.bValid)
		{
			return FText::FromString(FString::Printf(TEXT("Gaze: (%.1f, %.1f)"), GazePoint.Screen01.X, GazePoint.Screen01.Y));
		}
		return FText::FromString(TEXT("Gaze: Invalid"));
	}
	return FText::FromString(TEXT("Gaze: Unknown"));
}

FText UBeamDebugHUD::GetHeadPoseText() const
{
	if (BeamSubsystem)
	{
		FHeadPose HeadPose = BeamSubsystem->HeadPosition();
		if (HeadPose.Confidence > 0.0)
		{
			FVector EulerAngles = HeadPose.Rotation.Euler();
			return FText::FromString(FString::Printf(TEXT("Head: (%.1f°, %.1f°, %.1f°)"), EulerAngles.X, EulerAngles.Y, EulerAngles.Z));
		}
		return FText::FromString(TEXT("Head: Invalid"));
	}
	return FText::FromString(TEXT("Head: Unknown"));
}

FText UBeamDebugHUD::GetCalibrationStatusText() const
{
	if (BeamSubsystem)
	{
		if (BeamSubsystem->IsCalibrating())
		{
			return FText::FromString(TEXT("Calibration: In Progress"));
		}
		else
		{
			return FText::FromString(TEXT("Calibration: Available"));
		}
	}
	return FText::FromString(TEXT("Calibration: Unknown"));
}

FText UBeamDebugHUD::GetRecordingStatusText() const
{
	if (BeamSubsystem)
	{
		if (BeamSubsystem->IsRecording())
		{
			return FText::FromString(TEXT("Recording: Active"));
		}
		return FText::FromString(TEXT("Recording: Stopped"));
	}
	return FText::FromString(TEXT("Recording: Unknown"));
}

FText UBeamDebugHUD::GetPlaybackStatusText() const
{
	if (BeamSubsystem)
	{
		if (BeamSubsystem->IsPlayingBack())
		{
			return FText::FromString(TEXT("Playback: Active"));
		}
		return FText::FromString(TEXT("Playback: Stopped"));
	}
	return FText::FromString(TEXT("Playback: Unknown"));
}

void UBeamDebugHUD::OnToggleTracking()
{
	if (BeamSubsystem)
	{
		if (BeamSubsystem->IsBeamTracking())
		{
			BeamSubsystem->StopBeamTracking();
		}
		else
		{
			BeamSubsystem->StartBeamTracking();
		}
	}
}

void UBeamDebugHUD::OnResetCalibration()
{
	if (BeamSubsystem)
	{
		BeamSubsystem->StopCalibration();
		UE_LOG(LogBeam, Log, TEXT("Debug HUD: Calibration reset completed"));
	}
}

void UBeamDebugHUD::OnToggleRecording()
{
	if (BeamSubsystem)
	{
		if (BeamSubsystem->IsRecording())
		{
			BeamSubsystem->StopRecording();
			UE_LOG(LogBeam, Log, TEXT("Debug HUD: Recording stopped"));
		}
		else
		{
			BeamSubsystem->StartRecording(TEXT("debug_recording"));
			UE_LOG(LogBeam, Log, TEXT("Debug HUD: Recording started"));
		}
	}
}

void UBeamDebugHUD::OnTogglePlayback()
{
	if (BeamSubsystem)
	{
		if (BeamSubsystem->IsPlayingBack())
		{
			BeamSubsystem->StopPlayback();
			UE_LOG(LogBeam, Log, TEXT("Debug HUD: Playback stopped"));
		}
		else
		{
			BeamSubsystem->StartPlayback(TEXT("debug_playback"));
			UE_LOG(LogBeam, Log, TEXT("Debug HUD: Playback started"));
		}
	}
}

void UBeamDebugHUD::OnToggleDebugHUD()
{
	SetVisibility(GetVisibility() == ESlateVisibility::Visible ? ESlateVisibility::Hidden : ESlateVisibility::Visible);
	UE_LOG(LogBeam, Log, TEXT("Debug HUD: Debug HUD visibility toggled"));
}

void UBeamDebugHUD::OnToggleGazeCrosshair()
{
	bShowGazeCrosshair = !bShowGazeCrosshair;
	UE_LOG(LogBeam, Log, TEXT("Debug HUD: Gaze crosshair %s"), bShowGazeCrosshair ? TEXT("enabled") : TEXT("disabled"));
}

void UBeamDebugHUD::OnToggleGazeTrail()
{
	bShowGazeTrail = !bShowGazeTrail;
	UE_LOG(LogBeam, Log, TEXT("Debug HUD: Gaze trail %s"), bShowGazeTrail ? TEXT("enabled") : TEXT("disabled"));
}

void UBeamDebugHUD::OnToggleGazeRay()
{
	bShowGazeRay = !bShowGazeRay;
	UE_LOG(LogBeam, Log, TEXT("Debug HUD: Gaze ray %s"), bShowGazeRay ? TEXT("enabled") : TEXT("disabled"));
}


