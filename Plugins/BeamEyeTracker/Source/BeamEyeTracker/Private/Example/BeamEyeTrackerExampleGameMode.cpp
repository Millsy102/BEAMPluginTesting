#include "BeamEyeTrackerExampleGameMode.h"
#include "BeamEyeTrackerSubsystem.h"
#include "BeamEyeTrackerComponent.h"
#include "BeamEyeTrackerTypes.h"
#include "BeamEyeTrackerExampleCharacter.h"
#include "BeamEyeTrackerExampleHUD.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/GameStateBase.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Misc/App.h"
#include "BeamLogging.h"

// Constructor

ABeamEyeTrackerExampleGameMode::ABeamEyeTrackerExampleGameMode()
{
	
	UpdateInterval = 0.1f;
	bAutoStartTracking = true;
	bShowDebugHUDByDefault = true;
	bEnablePerformanceMonitoring = true;
	bGamePaused = false;
	bPerformanceMonitoringEnabled = false;
	bBeamTrackingActive = false;
	BeamHealth = EBeamHealth::Error;
	BeamFPS = 0.0f;
	BeamBufferUtilization = 0.0f;
	bBeamRecording = false;
	bBeamPlayingBack = false;

	DefaultPawnClass = ABeamEyeTrackerExampleCharacter::StaticClass();
	HUDClass = ABeamEyeTrackerExampleHUD::StaticClass();
}

// Lifecycle

void ABeamEyeTrackerExampleGameMode::BeginPlay()
{
	Super::BeginPlay();

	InitializeBeamEyeTracker();

	// Start update timer
	GetWorld()->GetTimerManager().SetTimer(UpdateTimerHandle, this, &ABeamEyeTrackerExampleGameMode::UpdateBeamEyeTracker, UpdateInterval, true);

	// Enable performance monitoring if requested
	if (bEnablePerformanceMonitoring)
	{
		EnablePerformanceMonitoring();
	}
}

void ABeamEyeTrackerExampleGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bPerformanceMonitoringEnabled)
	{
		UpdatePerformanceMetrics();
	}
}

// Game Mode Overrides

void ABeamEyeTrackerExampleGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	UE_LOG(LogBeam, Log, TEXT("BeamEyeTracker: Initializing game mode for map: %s"), *MapName);
}

void ABeamEyeTrackerExampleGameMode::InitGameState()
{
	Super::InitGameState();

	UE_LOG(LogBeam, Log, TEXT("BeamEyeTracker: Game state initialized"));
}

void ABeamEyeTrackerExampleGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (NewPlayer)
	{
		UE_LOG(LogBeam, Log, TEXT("BeamEyeTracker: Player logged in: %s"), *NewPlayer->GetName());

		// Auto-start tracking if enabled
		if (bAutoStartTracking)
		{
			StartBeamTracking();
		}
	}
}

void ABeamEyeTrackerExampleGameMode::Logout(AController* Exiting)
{
	if (Exiting)
	{
		UE_LOG(LogBeam, Log, TEXT("BeamEyeTracker: Player logged out: %s"), *Exiting->GetName());
	}

	Super::Logout(Exiting);
}

// Beam Eye Tracker Integration

void ABeamEyeTrackerExampleGameMode::InitializeBeamEyeTracker()
{
	if (UBeamEyeTrackerSubsystem* Subsystem = GetWorld()->GetGameInstance()->GetSubsystem<UBeamEyeTrackerSubsystem>())
	{
		UE_LOG(LogBeam, Log, TEXT("BeamEyeTracker: Subsystem found and initialized"));
		
		// Check initial health
		BeamHealth = Subsystem->GetBeamHealth();
		LogBeamStatus();
	}
	else
	{
		UE_LOG(LogBeam, Warning, TEXT("BeamEyeTracker: No subsystem found!"));
		HandleBeamError(TEXT("Beam Eye Tracker subsystem not found"));
	}
}

void ABeamEyeTrackerExampleGameMode::StartBeamTracking()
{
	if (UBeamEyeTrackerSubsystem* Subsystem = GetWorld()->GetGameInstance()->GetSubsystem<UBeamEyeTrackerSubsystem>())
	{
		if (Subsystem->StartBeamTracking())
		{
			bBeamTrackingActive = true;
			UE_LOG(LogBeam, Log, TEXT("BeamEyeTracker: Tracking started successfully"));
		}
		else
		{
			UE_LOG(LogBeam, Warning, TEXT("BeamEyeTracker: Failed to start tracking"));
			HandleBeamError(TEXT("Failed to start Beam tracking"));
		}
	}
}

void ABeamEyeTrackerExampleGameMode::StopBeamTracking()
{
	if (UBeamEyeTrackerSubsystem* Subsystem = GetWorld()->GetGameInstance()->GetSubsystem<UBeamEyeTrackerSubsystem>())
	{
		Subsystem->StopBeamTracking();
		bBeamTrackingActive = false;
		UE_LOG(LogBeam, Log, TEXT("BeamEyeTracker: Tracking stopped"));
	}
}

void ABeamEyeTrackerExampleGameMode::ResetBeamCalibration()
{
	if (UBeamEyeTrackerSubsystem* Subsystem = GetWorld()->GetGameInstance()->GetSubsystem<UBeamEyeTrackerSubsystem>())
	{
		Subsystem->ResetCalibration();
		UE_LOG(LogBeam, Log, TEXT("BeamEyeTracker: Calibration reset"));
	}
}

void ABeamEyeTrackerExampleGameMode::ToggleBeamDebugHUD()
{
	// This would toggle the debug HUD visibility
	// Implementation depends on the specific HUD system
	UE_LOG(LogBeam, Log, TEXT("BeamEyeTracker: Debug HUD toggled"));
}

void ABeamEyeTrackerExampleGameMode::StartBeamRecording()
{
	if (UBeamEyeTrackerSubsystem* Subsystem = GetWorld()->GetGameInstance()->GetSubsystem<UBeamEyeTrackerSubsystem>())
	{
		FString RecordingPath = FPaths::ProjectSavedDir() / TEXT("BeamRecordings") / FString::Printf(TEXT("recording_%s.csv"), *FDateTime::Now().ToString());
		if (Subsystem->StartRecording(RecordingPath))
		{
			bBeamRecording = true;
			UE_LOG(LogBeam, Log, TEXT("BeamEyeTracker: Recording started to %s"), *RecordingPath);
		}
		else
		{
			UE_LOG(LogBeam, Warning, TEXT("BeamEyeTracker: Failed to start recording"));
		}
	}
}

void ABeamEyeTrackerExampleGameMode::StopBeamRecording()
{
	if (UBeamEyeTrackerSubsystem* Subsystem = GetWorld()->GetGameInstance()->GetSubsystem<UBeamEyeTrackerSubsystem>())
	{
		Subsystem->StopRecording();
		bBeamRecording = false;
		UE_LOG(LogBeam, Log, TEXT("BeamEyeTracker: Recording stopped"));
	}
}

void ABeamEyeTrackerExampleGameMode::StartBeamPlayback()
{
	if (UBeamEyeTrackerSubsystem* Subsystem = GetWorld()->GetGameInstance()->GetSubsystem<UBeamEyeTrackerSubsystem>())
	{
		// For demo purposes, try to play back the last recording
		FString RecordingPath = FPaths::ProjectSavedDir() / TEXT("BeamRecordings") / TEXT("recording_latest.csv");
		if (Subsystem->StartPlayback(RecordingPath))
		{
			bBeamPlayingBack = true;
			UE_LOG(LogBeam, Log, TEXT("BeamEyeTracker: Playback started from %s"), *RecordingPath);
		}
		else
		{
			UE_LOG(LogBeam, Warning, TEXT("BeamEyeTracker: Failed to start playback"));
		}
	}
}

void ABeamEyeTrackerExampleGameMode::StopBeamPlayback()
{
	if (UBeamEyeTrackerSubsystem* Subsystem = GetWorld()->GetGameInstance()->GetSubsystem<UBeamEyeTrackerSubsystem>())
	{
		Subsystem->StopPlayback();
		bBeamPlayingBack = false;
		UE_LOG(LogBeam, Log, TEXT("BeamEyeTracker: Playback stopped"));
	}
}

// Game State Management

void ABeamEyeTrackerExampleGameMode::PauseGame()
{
	if (!bGamePaused)
	{
		bGamePaused = true;
		
		// Use the correct pause API for UE5.6
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			PC->SetPause(true);
		}
		
		UE_LOG(LogBeam, Log, TEXT("BeamEyeTracker: Game paused"));
	}
}

void ABeamEyeTrackerExampleGameMode::ResumeGame()
{
	if (bGamePaused)
	{
		bGamePaused = false;
		
		// Use the correct pause API for UE5.6
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			PC->SetPause(false);
		}
		
		UE_LOG(LogBeam, Log, TEXT("BeamEyeTracker: Game resumed"));
	}
}

void ABeamEyeTrackerExampleGameMode::RestartGame()
{
	UE_LOG(LogBeam, Log, TEXT("BeamEyeTracker: Restarting game"));
	GetWorld()->ServerTravel(GetWorld()->GetMapName());
}

// Performance Monitoring

void ABeamEyeTrackerExampleGameMode::EnablePerformanceMonitoring()
{
	bPerformanceMonitoringEnabled = true;
	UE_LOG(LogBeam, Log, TEXT("BeamEyeTracker: Performance monitoring enabled"));
}

void ABeamEyeTrackerExampleGameMode::DisablePerformanceMonitoring()
{
	bPerformanceMonitoringEnabled = false;
	UE_LOG(LogBeam, Log, TEXT("BeamEyeTracker: Performance monitoring disabled"));
}

void ABeamEyeTrackerExampleGameMode::UpdateBeamEyeTracker()
{
	if (UBeamEyeTrackerSubsystem* Subsystem = GetWorld()->GetGameInstance()->GetSubsystem<UBeamEyeTrackerSubsystem>())
	{
		
		bBeamTrackingActive = Subsystem->IsBeamTracking();
		BeamHealth = Subsystem->GetBeamHealth();
		BeamFPS = Subsystem->GetTrackingFPS();
		BeamBufferUtilization = Subsystem->GetBufferUtilization();
		bBeamRecording = Subsystem->IsRecording();
		bBeamPlayingBack = Subsystem->IsPlayingBack();

		// Log status changes
		static EBeamHealth LastHealth = EBeamHealth::Error;
		if (BeamHealth != LastHealth)
		{
			LogBeamStatus();
			LastHealth = BeamHealth;
		}
	}
}

void ABeamEyeTrackerExampleGameMode::UpdatePerformanceMetrics()
{
	
	// This could include frame rate, memory usage, etc.
}

// Utility Functions

void ABeamEyeTrackerExampleGameMode::LogBeamStatus()
{
	FString HealthString;
	switch (BeamHealth)
	{
		case EBeamHealth::Ok:
			HealthString = TEXT("OK");
			break;
		case EBeamHealth::AppNotRunning:
			HealthString = TEXT("App Not Running");
			break;
		case EBeamHealth::DllMissing:
			HealthString = TEXT("DLL Missing");
			break;
		case EBeamHealth::NoData:
			HealthString = TEXT("No Data");
			break;
		case EBeamHealth::Recovering:
			HealthString = TEXT("Recovering");
			break;
		case EBeamHealth::Error:
		default:
			HealthString = TEXT("Error");
			break;
	}

	UE_LOG(LogBeam, Log, TEXT("BeamEyeTracker: Status - Health: %s, Tracking: %s, FPS: %.1f, Buffer: %.1f%%"), 
		*HealthString, 
		bBeamTrackingActive ? TEXT("Active") : TEXT("Inactive"),
		BeamFPS,
		BeamBufferUtilization * 100.0f);
}

void ABeamEyeTrackerExampleGameMode::HandleBeamError(const FString& ErrorMessage)
{
	UE_LOG(LogBeam, Error, TEXT("BeamEyeTracker Error: %s"), *ErrorMessage);
	
	// Could implement error handling logic here
	// For example, showing error messages to the player, retrying operations, etc.
}

