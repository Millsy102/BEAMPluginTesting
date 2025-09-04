// Registers Beam console variables (CVars)

#include "BeamConsoleVariables.h"
#include "BeamEyeTrackerSubsystem.h"
#include "BeamEyeTrackerSettings.h"
#include "BeamLogging.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"

// Console Variables

// Polling frequency for eye tracking data collection
// Controls how often gaze data is sampled from the SDK
// Default: 120 Hz, Safe range: 15-240 Hz
static TAutoConsoleVariable<int32> CVarBeamPollHz(
	TEXT("beam.PollHz"),
	120,
	TEXT("Beam Eye Tracker polling frequency in Hz (15-240)"),
	ECVF_Default
);

// Enable/disable One-Euro smoothing filter for gaze data
// Reduces jitter in gaze coordinates for smoother tracking
// Default: true, Safe range: true/false
static TAutoConsoleVariable<bool> CVarBeamFilterEnable(
	TEXT("beam.Filter.Enable"),
	true,
	TEXT("Enable One-Euro smoothing filter for gaze data"),
	ECVF_Default
);

// One-Euro filter minimum cutoff frequency
// Controls smoothing aggressiveness - lower values = more smoothing
// Default: 1.0 Hz, Safe range: 0.1-5.0 Hz
static TAutoConsoleVariable<float> CVarBeamFilterMinCutoff(
	TEXT("beam.Filter.MinCutoff"),
	1.0f,
	TEXT("One-Euro filter minimum cutoff frequency (0.1-5.0)"),
	ECVF_Default
);

// One-Euro filter beta parameter
// Controls speed adaptation - higher values = faster adaptation to changes
// Default: 0.2, Safe range: 0.0-2.0
static TAutoConsoleVariable<float> CVarBeamFilterBeta(
	TEXT("beam.Filter.Beta"),
	0.2f,
	TEXT("One-Euro filter beta parameter (0.0-2.0)"),
	ECVF_Default
);

// Maximum distance for gaze line traces in world space
// Used for determining what objects the user is looking at
// Default: 5000 cm (50m), Safe range: 100-100000 cm
static TAutoConsoleVariable<float> CVarBeamTraceDistance(
	TEXT("beam.TraceDistance"),
	5000.0f,
	TEXT("Default max distance for gaze line traces in cm (100-100000)"),
	ECVF_Default
);

// Automatically start eye tracking when PIE begins
// Convenience feature for development and testing
// Default: false, Safe range: true/false
static TAutoConsoleVariable<bool> CVarBeamAutoStart(
	TEXT("beam.AutoStart"),
	false,
	TEXT("Automatically start tracking when PIE begins"),
	ECVF_Default
);

// Console Commands

// Apply current console variable values to runtime settings
// Useful for testing different configurations without restarting
static FAutoConsoleCommand CmdBeamApplySettings(
	TEXT("Beam.ApplySettings"),
	TEXT("Apply current console variable values to runtime settings"),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		
		int32 PollingHz = CVarBeamPollHz.GetValueOnGameThread();
		bool bEnableSmoothing = CVarBeamFilterEnable.GetValueOnGameThread();
		float MinCutoff = CVarBeamFilterMinCutoff.GetValueOnGameThread();
		float Beta = CVarBeamFilterBeta.GetValueOnGameThread();
		float TraceDistance = CVarBeamTraceDistance.GetValueOnGameThread();
		bool bAutoStart = CVarBeamAutoStart.GetValueOnGameThread();

		UBeamEyeTrackerSettings TempSettings;
		TempSettings.PollingHz = PollingHz;
		TempSettings.bEnableSmoothing = bEnableSmoothing;
		TempSettings.MinCutoff = MinCutoff;
		TempSettings.Beta = Beta;
		TempSettings.TraceDistance = TraceDistance;
		TempSettings.bAutoStart = bAutoStart;

		// Apply to the subsystem if available
		if (GEngine)
		{
			if (UWorld* World = GEngine->GetWorld())
			{
				if (UGameInstance* GameInstance = World->GetGameInstance())
				{
					if (UBeamEyeTrackerSubsystem* Subsystem = GameInstance->GetSubsystem<UBeamEyeTrackerSubsystem>())
					{
						Subsystem->ApplyRuntimeSettings(&TempSettings);
						UE_LOG(LogBeam, Log, TEXT("Beam: Applied console variable settings to runtime"));
						return;
					}
				}
			}
		}

		UE_LOG(LogBeam, Warning, TEXT("Beam: Could not find subsystem to apply console settings"));
	})
);

// Save current console variable values to DefaultEngine.ini
// Persists configuration changes across editor sessions
static FAutoConsoleCommand CmdBeamSaveConfig(
	TEXT("Beam.SaveConfig"),
	TEXT("Save current console variable values to DefaultEngine.ini"),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		
		int32 PollingHz = CVarBeamPollHz.GetValueOnGameThread();
		bool bEnableSmoothing = CVarBeamFilterEnable.GetValueOnGameThread();
		float MinCutoff = CVarBeamFilterMinCutoff.GetValueOnGameThread();
		float Beta = CVarBeamFilterBeta.GetValueOnGameThread();
		float TraceDistance = CVarBeamTraceDistance.GetValueOnGameThread();
		bool bAutoStart = CVarBeamAutoStart.GetValueOnGameThread();

		if (UBeamEyeTrackerSettings* MutableSettings = GetMutableDefault<UBeamEyeTrackerSettings>())
		{
			MutableSettings->PollingHz = PollingHz;
			MutableSettings->bEnableSmoothing = bEnableSmoothing;
			MutableSettings->MinCutoff = MinCutoff;
			MutableSettings->Beta = Beta;
			MutableSettings->TraceDistance = TraceDistance;
			MutableSettings->bAutoStart = bAutoStart;

			// Save to config
			MutableSettings->SaveConfig();
			UE_LOG(LogBeam, Log, TEXT("Beam: Saved console variable values to DefaultEngine.ini"));
		}
		else
		{
			UE_LOG(LogBeam, Warning, TEXT("Beam: Could not access settings to save config"));
		}
	})
);

static FAutoConsoleCommand CmdBeamDumpSettings(
	TEXT("Beam.DumpSettings"),
	TEXT("Dump current runtime settings to log"),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		UE_LOG(LogBeam, Log, TEXT("=== Beam Eye Tracker Current Settings ==="));
		UE_LOG(LogBeam, Log, TEXT("Console Variables:"));
		UE_LOG(LogBeam, Log, TEXT("  beam.PollHz: %d"), CVarBeamPollHz.GetValueOnGameThread());
		UE_LOG(LogBeam, Log, TEXT("  beam.Filter.Enable: %s"), CVarBeamFilterEnable.GetValueOnGameThread() ? TEXT("True") : TEXT("False"));
		UE_LOG(LogBeam, Log, TEXT("  beam.Filter.MinCutoff: %.2f"), CVarBeamFilterMinCutoff.GetValueOnGameThread());
		UE_LOG(LogBeam, Log, TEXT("  beam.Filter.Beta: %.2f"), CVarBeamFilterBeta.GetValueOnGameThread());
		UE_LOG(LogBeam, Log, TEXT("  beam.TraceDistance: %.0f"), CVarBeamTraceDistance.GetValueOnGameThread());
		UE_LOG(LogBeam, Log, TEXT("  beam.AutoStart: %s"), CVarBeamAutoStart.GetValueOnGameThread() ? TEXT("True") : TEXT("False"));

		// Also dump the current project settings
		if (const UBeamEyeTrackerSettings* Settings = GetDefault<UBeamEyeTrackerSettings>())
		{
					UE_LOG(LogBeam, Log, TEXT("Project Settings:"));
		UE_LOG(LogBeam, Log, TEXT("  PollingHz: %d"), Settings->PollingHz);
		UE_LOG(LogBeam, Log, TEXT("  bEnableSmoothing: %s"), Settings->bEnableSmoothing ? TEXT("True") : TEXT("False"));
		UE_LOG(LogBeam, Log, TEXT("  MinCutoff: %.2f"), Settings->MinCutoff);
		UE_LOG(LogBeam, Log, TEXT("  Beta: %.2f"), Settings->Beta);
		UE_LOG(LogBeam, Log, TEXT("  TraceDistance: %.0f"), Settings->TraceDistance);
		UE_LOG(LogBeam, Log, TEXT("  bAutoStart: %s"), Settings->bAutoStart ? TEXT("True") : TEXT("False"));
		}

		// Dump subsystem status if available
		if (GEngine)
		{
			if (UWorld* World = GEngine->GetWorld())
			{
				if (UGameInstance* GameInstance = World->GetGameInstance())
				{
					if (UBeamEyeTrackerSubsystem* Subsystem = GameInstance->GetSubsystem<UBeamEyeTrackerSubsystem>())
					{
						UE_LOG(LogBeam, Log, TEXT("Subsystem Status:"));
						UE_LOG(LogBeam, Log, TEXT("  IsTracking: %s"), Subsystem->IsBeamTracking() ? TEXT("True") : TEXT("False"));
						UE_LOG(LogBeam, Log, TEXT("  IsBeamAppRunning: %s"), Subsystem->IsBeamAppRunning() ? TEXT("True") : TEXT("False"));
						UE_LOG(LogBeam, Log, TEXT("  SDKVersion: %s"), *Subsystem->GetSDKVersion());

						int32 ViewportWidth, ViewportHeight;
						Subsystem->GetViewportDimensions(ViewportWidth, ViewportHeight);
						UE_LOG(LogBeam, Log, TEXT("  CurrentViewport: %dx%d"), ViewportWidth, ViewportHeight);
					}
				}
			}
		}
		UE_LOG(LogBeam, Log, TEXT("========================================"));
	})
);

static FAutoConsoleCommand CmdBeamUpdateViewport(
	TEXT("Beam.UpdateViewport"),
	TEXT("Update viewport dimensions for proper coordinate mapping"),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		if (GEngine)
		{
			if (UWorld* World = GEngine->GetWorld())
			{
				if (UGameInstance* GameInstance = World->GetGameInstance())
				{
					if (UBeamEyeTrackerSubsystem* Subsystem = GameInstance->GetSubsystem<UBeamEyeTrackerSubsystem>())
					{
						Subsystem->AutoUpdateViewport();
						UE_LOG(LogBeam, Log, TEXT("Beam: Updated viewport dimensions"));
					}
					else
					{
						UE_LOG(LogBeam, Warning, TEXT("Beam: Subsystem not available"));
					}
				}
				else
				{
					UE_LOG(LogBeam, Warning, TEXT("Beam: GameInstance not available"));
				}
			}
			else
			{
				UE_LOG(LogBeam, Warning, TEXT("Beam: World not available"));
			}
		}
		else
		{
			UE_LOG(LogBeam, Warning, TEXT("Beam: Engine not available"));
		}
	})
);

// Manually set viewport dimensions for testing
static FAutoConsoleCommand CmdBeamSetViewport(
	TEXT("Beam.SetViewport"),
	TEXT("Manually set viewport dimensions (width height)"),
	FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
	{
		if (Args.Num() != 2)
		{
			UE_LOG(LogBeam, Warning, TEXT("Beam.SetViewport: Usage: Beam.SetViewport <width> <height>"));
			return;
		}

		int32 Width = FCString::Atoi(*Args[0]);
		int32 Height = FCString::Atoi(*Args[1]);

		if (Width <= 0 || Height <= 0 || Width > 7680 || Height > 4320)
		{
			UE_LOG(LogBeam, Warning, TEXT("Beam.SetViewport: Invalid dimensions %dx%d. Must be 1-7680 x 1-4320"), Width, Height);
			return;
		}

		if (GEngine)
		{
			if (UWorld* World = GEngine->GetWorld())
			{
				if (UGameInstance* GameInstance = World->GetGameInstance())
				{
					if (UBeamEyeTrackerSubsystem* Subsystem = GameInstance->GetSubsystem<UBeamEyeTrackerSubsystem>())
					{
						Subsystem->UpdateViewportGeometry(Width, Height);
						UE_LOG(LogBeam, Log, TEXT("Beam: Manually set viewport dimensions to %dx%d"), Width, Height);
					}
					else
					{
						UE_LOG(LogBeam, Warning, TEXT("Beam: Subsystem not available"));
					}
				}
				else
				{
					UE_LOG(LogBeam, Warning, TEXT("Beam: GameInstance not available"));
				}
			}
			else
			{
				UE_LOG(LogBeam, Warning, TEXT("Beam: World not available"));
			}
		}
		else
		{
			UE_LOG(LogBeam, Warning, TEXT("Beam: Engine not available"));
		}
	})
);

// Function to sync console variables with project settings on startup
void FBeamConsoleVariables::SyncWithProjectSettings()
{
	if (const UBeamEyeTrackerSettings* Settings = GetDefault<UBeamEyeTrackerSettings>())
	{
		
		CVarBeamPollHz->Set(Settings->PollingHz, ECVF_SetByProjectSetting);
		CVarBeamFilterEnable->Set(Settings->bEnableSmoothing, ECVF_SetByProjectSetting);
		CVarBeamFilterMinCutoff->Set(Settings->MinCutoff, ECVF_SetByProjectSetting);
		CVarBeamFilterBeta->Set(Settings->Beta, ECVF_SetByProjectSetting);
		CVarBeamTraceDistance->Set(Settings->TraceDistance, ECVF_SetByProjectSetting);
		CVarBeamAutoStart->Set(Settings->bAutoStart, ECVF_SetByProjectSetting);

		UE_LOG(LogBeam, Log, TEXT("Beam: Synced console variables with project settings"));
	}
}

