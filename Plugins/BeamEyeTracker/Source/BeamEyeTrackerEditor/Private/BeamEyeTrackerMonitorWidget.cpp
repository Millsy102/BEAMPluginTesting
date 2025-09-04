#include "BeamEyeTrackerMonitorWidget.h"
#include "BeamEyeTrackerSubsystem.h"
#include "BeamEyeTrackerComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/SpinBox.h"
#include "Components/EditableTextBox.h"
#include "Components/Slider.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "Components/GridPanel.h"

#include "TimerManager.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"

UBeamEyeTrackerMonitorWidget::UBeamEyeTrackerMonitorWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, UpdateInterval(0.1f)
	, bAutoStartTracking(false)
	, bShowDebugInfo(true)
{
	// Initialize default paths
	DefaultRecordingDirectory = TEXT("BeamRecordings");
	CurrentRecordingPath = FPaths::ProjectSavedDir() / DefaultRecordingDirectory / TEXT("recording.csv");
	CurrentPlaybackPath = FPaths::ProjectSavedDir() / DefaultRecordingDirectory / TEXT("playback.csv");
}

void UBeamEyeTrackerMonitorWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Initialize defaults and bindings
	InitializeDefaults();
	InitializeWidgetBindings();

	// Set up timer for updates
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(UpdateTimerHandle, FTimerDelegate::CreateUObject(this, &UBeamEyeTrackerMonitorWidget::UpdateMonitor), UpdateInterval, true);
	}

	// Auto-start tracking if enabled
	if (bAutoStartTracking)
	{
		StartTracking();
	}
}

void UBeamEyeTrackerMonitorWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

void UBeamEyeTrackerMonitorWidget::InitializeDefaults()
{
	// Create default recording directory if it doesn't exist
	FString FullRecordingDir = FPaths::ProjectSavedDir() / DefaultRecordingDirectory;
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*FullRecordingDir))
	{
		PlatformFile.CreateDirectoryTree(*FullRecordingDir);
	}
}

void UBeamEyeTrackerMonitorWidget::InitializeWidgetBindings()
{
	// Bind button events by name (allows for flexible widget naming)
	if (UButton* StartTrackingButton = Cast<UButton>(GetWidgetFromName(TEXT("StartTrackingButton"))))
	{
		StartTrackingButton->OnClicked.AddDynamic(this, &UBeamEyeTrackerMonitorWidget::OnStartTrackingClicked);
	}

	if (UButton* StopTrackingButton = Cast<UButton>(GetWidgetFromName(TEXT("StopTrackingButton"))))
	{
		StopTrackingButton->OnClicked.AddDynamic(this, &UBeamEyeTrackerMonitorWidget::OnStopTrackingClicked);
	}

	if (UButton* ResetCalibrationButton = Cast<UButton>(GetWidgetFromName(TEXT("ResetCalibrationButton"))))
	{
		ResetCalibrationButton->OnClicked.AddDynamic(this, &UBeamEyeTrackerMonitorWidget::OnResetCalibrationClicked);
	}

	if (UButton* StartRecordingButton = Cast<UButton>(GetWidgetFromName(TEXT("StartRecordingButton"))))
	{
		StartRecordingButton->OnClicked.AddDynamic(this, &UBeamEyeTrackerMonitorWidget::OnStartRecordingClicked);
	}

	if (UButton* StopRecordingButton = Cast<UButton>(GetWidgetFromName(TEXT("StopRecordingButton"))))
	{
		StopRecordingButton->OnClicked.AddDynamic(this, &UBeamEyeTrackerMonitorWidget::OnStopRecordingClicked);
	}

	if (UButton* StartPlaybackButton = Cast<UButton>(GetWidgetFromName(TEXT("StartPlaybackButton"))))
	{
		StartPlaybackButton->OnClicked.AddDynamic(this, &UBeamEyeTrackerMonitorWidget::OnStartPlaybackClicked);
	}

	if (UButton* StopPlaybackButton = Cast<UButton>(GetWidgetFromName(TEXT("StopPlaybackButton"))))
	{
		StopPlaybackButton->OnClicked.AddDynamic(this, &UBeamEyeTrackerMonitorWidget::OnStopPlaybackClicked);
	}

	// Bind checkbox events
	if (DebugHUDCheckBox)
	{
		DebugHUDCheckBox->OnCheckStateChanged.AddDynamic(this, &UBeamEyeTrackerMonitorWidget::OnDebugHUDToggled);
	}

	if (SmoothingCheckBox)
	{
		SmoothingCheckBox->OnCheckStateChanged.AddDynamic(this, &UBeamEyeTrackerMonitorWidget::OnSmoothingToggled);
	}

	// Bind spinbox events
	if (PollingRateSpinBox)
	{
		PollingRateSpinBox->OnValueChanged.AddDynamic(this, &UBeamEyeTrackerMonitorWidget::OnPollingRateChanged);
		PollingRateSpinBox->SetMinValue(1.0f);
		PollingRateSpinBox->SetMaxValue(120.0f);
		PollingRateSpinBox->SetValue(60.0f);
	}

	if (MinCutoffSpinBox)
	{
		MinCutoffSpinBox->OnValueChanged.AddDynamic(this, &UBeamEyeTrackerMonitorWidget::OnMinCutoffChanged);
		MinCutoffSpinBox->SetMinValue(0.1f);
		MinCutoffSpinBox->SetMaxValue(10.0f);
		MinCutoffSpinBox->SetValue(1.0f);
	}

	if (BetaSpinBox)
	{
		BetaSpinBox->OnValueChanged.AddDynamic(this, &UBeamEyeTrackerMonitorWidget::OnBetaChanged);
		BetaSpinBox->SetMinValue(0.0f);
		BetaSpinBox->SetMaxValue(1.0f);
		BetaSpinBox->SetValue(0.007f);
	}

	// Bind textbox events
	if (RecordingPathTextBox)
	{
		RecordingPathTextBox->OnTextChanged.AddDynamic(this, &UBeamEyeTrackerMonitorWidget::OnRecordingPathChanged);
		RecordingPathTextBox->SetText(FText::FromString(CurrentRecordingPath));
	}

	if (PlaybackPathTextBox)
	{
		PlaybackPathTextBox->OnTextChanged.AddDynamic(this, &UBeamEyeTrackerMonitorWidget::OnPlaybackPathChanged);
		PlaybackPathTextBox->SetText(FText::FromString(CurrentPlaybackPath));
	}
}

void UBeamEyeTrackerMonitorWidget::UpdateMonitor()
{
	// Update status text
	if (StatusText)
	{
		StatusText->SetText(GetTrackingStatusText());
	}

	if (HealthText)
	{
		HealthText->SetText(GetHealthStatusText());
	}

	if (FPSText)
	{
		FPSText->SetText(GetFPSText());
	}

	if (GazeDataText)
	{
		GazeDataText->SetText(GetGazePointText());
	}

	if (BufferText)
	{
		BufferText->SetText(GetBufferUtilizationText());
	}

	if (HeadDataText)
	{
		HeadDataText->SetText(GetHeadPoseText());
	}

	if (CalibrationText)
	{
		CalibrationText->SetText(GetCalibrationStatusText());
	}

	if (RecordingText)
	{
		RecordingText->SetText(GetRecordingStatusText());
	}

	if (PlaybackText)
	{
		PlaybackText->SetText(GetPlaybackStatusText());
	}
}

// Public Blueprint-callable functions
void UBeamEyeTrackerMonitorWidget::StartTracking()
{
	if (UWorld* World = GetWorld())
	{
		if (UBeamEyeTrackerSubsystem* Subsystem = World->GetGameInstance()->GetSubsystem<UBeamEyeTrackerSubsystem>())
		{
			Subsystem->StartBeamTracking();
		}
	}
}

void UBeamEyeTrackerMonitorWidget::StopTracking()
{
	if (UWorld* World = GetWorld())
	{
		if (UBeamEyeTrackerSubsystem* Subsystem = World->GetGameInstance()->GetSubsystem<UBeamEyeTrackerSubsystem>())
		{
			Subsystem->StopBeamTracking();
		}
	}
}

void UBeamEyeTrackerMonitorWidget::ResetCalibration()
{
	if (UWorld* World = GetWorld())
	{
		if (UBeamEyeTrackerSubsystem* Subsystem = World->GetGameInstance()->GetSubsystem<UBeamEyeTrackerSubsystem>())
		{
			Subsystem->ResetCalibration();
		}
	}
}

void UBeamEyeTrackerMonitorWidget::StartRecording(const FString& FilePath)
{
	FString PathToUse = FilePath.IsEmpty() ? CurrentRecordingPath : FilePath;
	
	if (UWorld* World = GetWorld())
	{
		if (UBeamEyeTrackerSubsystem* Subsystem = World->GetGameInstance()->GetSubsystem<UBeamEyeTrackerSubsystem>())
		{
			Subsystem->StartRecording(PathToUse);
		}
	}
}

void UBeamEyeTrackerMonitorWidget::StopRecording()
{
	if (UWorld* World = GetWorld())
	{
		if (UBeamEyeTrackerSubsystem* Subsystem = World->GetGameInstance()->GetSubsystem<UBeamEyeTrackerSubsystem>())
		{
			Subsystem->StopRecording();
		}
	}
}

void UBeamEyeTrackerMonitorWidget::StartPlayback(const FString& FilePath)
{
	FString PathToUse = FilePath.IsEmpty() ? CurrentPlaybackPath : FilePath;
	
	if (UWorld* World = GetWorld())
	{
		if (UBeamEyeTrackerSubsystem* Subsystem = World->GetGameInstance()->GetSubsystem<UBeamEyeTrackerSubsystem>())
		{
			Subsystem->StartPlayback(PathToUse);
		}
	}
}

void UBeamEyeTrackerMonitorWidget::StopPlayback()
{
	if (UWorld* World = GetWorld())
	{
		if (UBeamEyeTrackerSubsystem* Subsystem = World->GetGameInstance()->GetSubsystem<UBeamEyeTrackerSubsystem>())
		{
			Subsystem->StopPlayback();
		}
	}
}

// Public getter functions
bool UBeamEyeTrackerMonitorWidget::IsTracking() const
{
	if (UWorld* World = GetWorld())
	{
		if (UBeamEyeTrackerSubsystem* Subsystem = World->GetGameInstance()->GetSubsystem<UBeamEyeTrackerSubsystem>())
		{
			return Subsystem->IsBeamTracking();
		}
	}
	return false;
}

bool UBeamEyeTrackerMonitorWidget::IsRecording() const
{
	if (UWorld* World = GetWorld())
	{
		if (UBeamEyeTrackerSubsystem* Subsystem = World->GetGameInstance()->GetSubsystem<UBeamEyeTrackerSubsystem>())
		{
			return Subsystem->IsRecording();
		}
	}
	return false;
}

bool UBeamEyeTrackerMonitorWidget::IsPlayingBack() const
{
	if (UWorld* World = GetWorld())
	{
		if (UBeamEyeTrackerSubsystem* Subsystem = World->GetGameInstance()->GetSubsystem<UBeamEyeTrackerSubsystem>())
		{
			return Subsystem->IsPlayingBack();
		}
	}
	return false;
}

float UBeamEyeTrackerMonitorWidget::GetCurrentFPS() const
{
	if (UWorld* World = GetWorld())
	{
		if (UBeamEyeTrackerSubsystem* Subsystem = World->GetGameInstance()->GetSubsystem<UBeamEyeTrackerSubsystem>())
		{
			return Subsystem->GetCurrentFPS();
		}
	}
	return 0.0f;
}

int32 UBeamEyeTrackerMonitorWidget::GetBufferUtilization() const
{
	if (UWorld* World = GetWorld())
	{
		if (UBeamEyeTrackerSubsystem* Subsystem = World->GetGameInstance()->GetSubsystem<UBeamEyeTrackerSubsystem>())
		{
			return Subsystem->GetBufferUtilization();
		}
	}
	return 0;
}

FVector2D UBeamEyeTrackerMonitorWidget::GetCurrentGazePoint() const
{
	if (UWorld* World = GetWorld())
	{
		if (UBeamEyeTrackerSubsystem* Subsystem = World->GetGameInstance()->GetSubsystem<UBeamEyeTrackerSubsystem>())
		{
			FBeamFrame Frame;
			if (Subsystem->GetLatestInterpolatedFrame(0.0, Frame))
			{
				if (Frame.Gaze.bValid)
				{
					return Frame.Gaze.Screen01;
				}
			}
		}
	}
	return FVector2D::ZeroVector;
}

FVector UBeamEyeTrackerMonitorWidget::GetCurrentHeadPosition() const
{
	if (UWorld* World = GetWorld())
	{
		if (UBeamEyeTrackerSubsystem* Subsystem = World->GetGameInstance()->GetSubsystem<UBeamEyeTrackerSubsystem>())
		{
			FBeamFrame Frame;
			if (Subsystem->GetLatestInterpolatedFrame(0.0, Frame))
			{
				if (Frame.Head.Confidence > 0.0f)
				{
					return Frame.Head.PositionCm;
				}
			}
		}
	}
	return FVector::ZeroVector;
}

// Event handlers
void UBeamEyeTrackerMonitorWidget::OnStartTrackingClicked()
{
	StartTracking();
}

void UBeamEyeTrackerMonitorWidget::OnStopTrackingClicked()
{
	StopTracking();
}

void UBeamEyeTrackerMonitorWidget::OnResetCalibrationClicked()
{
	ResetCalibration();
}

void UBeamEyeTrackerMonitorWidget::OnStartRecordingClicked()
{
	StartRecording();
}

void UBeamEyeTrackerMonitorWidget::OnStopRecordingClicked()
{
	StopRecording();
}

void UBeamEyeTrackerMonitorWidget::OnStartPlaybackClicked()
{
	StartPlayback();
}

void UBeamEyeTrackerMonitorWidget::OnStopPlaybackClicked()
{
	StopPlayback();
}

void UBeamEyeTrackerMonitorWidget::OnDebugHUDToggled(bool bIsChecked)
{
	if (UWorld* World = GetWorld())
	{
		if (UBeamEyeTrackerSubsystem* Subsystem = World->GetGameInstance()->GetSubsystem<UBeamEyeTrackerSubsystem>())
		{
			Subsystem->SetDebugHUDEnabled(bIsChecked);
		}
	}
}

void UBeamEyeTrackerMonitorWidget::OnPollingRateChanged(float NewValue)
{
	if (UWorld* World = GetWorld())
	{
		if (UBeamEyeTrackerSubsystem* Subsystem = World->GetGameInstance()->GetSubsystem<UBeamEyeTrackerSubsystem>())
		{
			Subsystem->SetPollingRate(NewValue);
		}
	}
}

void UBeamEyeTrackerMonitorWidget::OnSmoothingToggled(bool bIsChecked)
{
	if (UWorld* World = GetWorld())
	{
		if (UBeamEyeTrackerSubsystem* Subsystem = World->GetGameInstance()->GetSubsystem<UBeamEyeTrackerSubsystem>())
		{
			Subsystem->SetSmoothingEnabled(bIsChecked);
		}
	}
}

void UBeamEyeTrackerMonitorWidget::OnMinCutoffChanged(float NewValue)
{
	if (UWorld* World = GetWorld())
	{
		if (UBeamEyeTrackerSubsystem* Subsystem = World->GetGameInstance()->GetSubsystem<UBeamEyeTrackerSubsystem>())
		{
			Subsystem->SetMinCutoff(NewValue);
		}
	}
}

void UBeamEyeTrackerMonitorWidget::OnBetaChanged(float NewValue)
{
	if (UWorld* World = GetWorld())
	{
		if (UBeamEyeTrackerSubsystem* Subsystem = World->GetGameInstance()->GetSubsystem<UBeamEyeTrackerSubsystem>())
		{
			Subsystem->SetBeta(NewValue);
		}
	}
}

void UBeamEyeTrackerMonitorWidget::OnRecordingPathChanged(const FText& NewText)
{
	CurrentRecordingPath = NewText.ToString();
}

void UBeamEyeTrackerMonitorWidget::OnPlaybackPathChanged(const FText& NewText)
{
	CurrentPlaybackPath = NewText.ToString();
}

// Status text getters
FText UBeamEyeTrackerMonitorWidget::GetTrackingStatusText() const
{
	if (UWorld* World = GetWorld())
	{
		if (UBeamEyeTrackerSubsystem* Subsystem = World->GetGameInstance()->GetSubsystem<UBeamEyeTrackerSubsystem>())
		{
			if (Subsystem->IsBeamTracking())
			{
				FBeamFrame TestFrame;
				if (Subsystem->GetLatestInterpolatedFrame(0.0, TestFrame))
				{
					return NSLOCTEXT("Beam", "TrackingActive", "Tracking: Active (Data Flowing)");
				}
				else
				{
					return NSLOCTEXT("Beam", "TrackingNoData", "Tracking: Active (No Data)");
				}
			}
			else
			{
				return NSLOCTEXT("Beam", "TrackingInactive", "Tracking: Inactive");
			}
		}
	}
	return NSLOCTEXT("Beam", "StatusUnknown", "Unknown");
}

FText UBeamEyeTrackerMonitorWidget::GetHealthStatusText() const
{
	if (UWorld* World = GetWorld())
	{
		if (UBeamEyeTrackerSubsystem* Subsystem = World->GetGameInstance()->GetSubsystem<UBeamEyeTrackerSubsystem>())
		{
			if (Subsystem->IsBeamTracking())
			{
				return NSLOCTEXT("Beam", "HealthGood", "Health: Good");
			}
			else
			{
				return NSLOCTEXT("Beam", "HealthInactive", "Health: Inactive");
			}
		}
	}
	return NSLOCTEXT("Beam", "HealthUnknown", "Unknown");
}

FText UBeamEyeTrackerMonitorWidget::GetFPSText() const
{
	if (UWorld* World = GetWorld())
	{
		if (UBeamEyeTrackerSubsystem* Subsystem = World->GetGameInstance()->GetSubsystem<UBeamEyeTrackerSubsystem>())
		{
			float FPS = Subsystem->GetCurrentFPS();
			return FText::Format(NSLOCTEXT("Beam", "FPSFormat", "FPS: {0}"), FText::AsNumber(FPS));
		}
	}
	return NSLOCTEXT("Beam", "FPSUnknown", "FPS: Unknown");
}

FText UBeamEyeTrackerMonitorWidget::GetGazePointText() const
{
	if (UWorld* World = GetWorld())
	{
		if (UBeamEyeTrackerSubsystem* Subsystem = World->GetGameInstance()->GetSubsystem<UBeamEyeTrackerSubsystem>())
		{
			FBeamFrame Frame;
			if (Subsystem->GetLatestInterpolatedFrame(0.0, Frame))
			{
				if (Frame.Gaze.bValid)
				{
					return FText::Format(NSLOCTEXT("Beam", "GazeFormat", "Gaze: ({0}, {1})"), 
						FText::AsNumber(Frame.Gaze.Screen01.X), 
						FText::AsNumber(Frame.Gaze.Screen01.Y));
				}
				else
				{
					return NSLOCTEXT("Beam", "GazeInvalid", "Gaze: Invalid");
				}
			}
		}
	}
	return NSLOCTEXT("Beam", "GazeUnknown", "Gaze: Unknown");
}

FText UBeamEyeTrackerMonitorWidget::GetBufferUtilizationText() const
{
	if (UWorld* World = GetWorld())
	{
		if (UBeamEyeTrackerSubsystem* Subsystem = World->GetGameInstance()->GetSubsystem<UBeamEyeTrackerSubsystem>())
		{
			int32 Utilization = Subsystem->GetBufferUtilization();
			return FText::Format(NSLOCTEXT("Beam", "BufferFormat", "Buffer: {0}%"), FText::AsNumber(Utilization));
		}
	}
	return NSLOCTEXT("Beam", "BufferUnknown", "Buffer: Unknown");
}

FText UBeamEyeTrackerMonitorWidget::GetHeadPoseText() const
{
	if (UWorld* World = GetWorld())
	{
		if (UBeamEyeTrackerSubsystem* Subsystem = World->GetGameInstance()->GetSubsystem<UBeamEyeTrackerSubsystem>())
		{
			FBeamFrame Frame;
			if (Subsystem->GetLatestInterpolatedFrame(0.0, Frame))
			{
				if (Frame.Head.Confidence > 0.0f)
				{
					return FText::Format(NSLOCTEXT("Beam", "HeadPoseFormat", "Head: ({0}, {1}, {2})"), 
						FText::AsNumber(Frame.Head.PositionCm.X), 
						FText::AsNumber(Frame.Head.PositionCm.Y),
						FText::AsNumber(Frame.Head.PositionCm.Z));
				}
				else
				{
					return NSLOCTEXT("Beam", "HeadPoseInvalid", "Head: Invalid");
				}
			}
		}
	}
	return NSLOCTEXT("Beam", "HeadPoseUnknown", "Head: Unknown");
}

FText UBeamEyeTrackerMonitorWidget::GetCalibrationStatusText() const
{
	if (UWorld* World = GetWorld())
	{
		if (UBeamEyeTrackerSubsystem* Subsystem = World->GetGameInstance()->GetSubsystem<UBeamEyeTrackerSubsystem>())
		{
			if (Subsystem->IsCalibrated())
			{
				return NSLOCTEXT("Beam", "CalibrationGood", "Calibration: Good");
			}
			else
			{
				return NSLOCTEXT("Beam", "CalibrationNeeded", "Calibration: Needed");
			}
		}
	}
	return NSLOCTEXT("Beam", "CalibrationUnknown", "Calibration: Unknown");
}

FText UBeamEyeTrackerMonitorWidget::GetRecordingStatusText() const
{
	if (UWorld* World = GetWorld())
	{
		if (UBeamEyeTrackerSubsystem* Subsystem = World->GetGameInstance()->GetSubsystem<UBeamEyeTrackerSubsystem>())
		{
			if (Subsystem->IsRecording())
			{
				return NSLOCTEXT("Beam", "RecordingActive", "Recording: Active");
			}
			else
			{
				return NSLOCTEXT("Beam", "RecordingInactive", "Recording: Inactive");
			}
		}
	}
	return NSLOCTEXT("Beam", "RecordingUnknown", "Recording: Unknown");
}

FText UBeamEyeTrackerMonitorWidget::GetPlaybackStatusText() const
{
	if (UWorld* World = GetWorld())
	{
		if (UBeamEyeTrackerSubsystem* Subsystem = World->GetGameInstance()->GetSubsystem<UBeamEyeTrackerSubsystem>())
		{
			if (Subsystem->IsPlayingBack())
			{
				return NSLOCTEXT("Beam", "PlaybackActive", "Playback: Active");
			}
			else
			{
				return NSLOCTEXT("Beam", "PlaybackInactive", "Playback: Inactive");
			}
		}
	}
	return NSLOCTEXT("Beam", "PlaybackUnknown", "Playback: Unknown");
}
