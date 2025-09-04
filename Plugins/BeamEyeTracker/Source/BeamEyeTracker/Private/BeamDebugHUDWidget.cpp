#include "BeamDebugHUDWidget.h"
#include "BeamEyeTrackerSubsystem.h"
#include "BeamEyeTrackerComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "TimerManager.h"

UBeamDebugHUDWidget::UBeamDebugHUDWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bDebugHUDVisible(true)
	, bShowGazeCrosshair(true)
	, bShowHeadPoseIndicator(true)
	, bShowPerformanceMetrics(true)
	, bShowConnectionStatus(true)
	, UpdateInterval(0.016f)
	, GazeCrosshairSize(20.0f)
	, HeadPoseIndicatorSize(15.0f)
	, bGazeValid(false)
	, bHeadValid(false)
	, CurrentFPS(0.0f)
	, CurrentBufferUtilization(0)
	, CurrentLatency(0.0)
	, bIsConnected(false)
	, bIsTracking(false)
{
	
	CurrentGazeScreenPos = FVector2D::ZeroVector;
	CurrentGazeScreen01 = FVector2D::ZeroVector;
	CurrentGazeConfidence = 0.0f;
	CurrentHeadPosition = FVector::ZeroVector;
	CurrentHeadRotation = FRotator::ZeroRotator;
	CurrentHeadConfidence = 0.0f;
	ConnectionStatus = TEXT("Disconnected");
}

void UBeamDebugHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	InitializeWidgetBindings();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(UpdateTimerHandle, FTimerDelegate::CreateUObject(this, &UBeamDebugHUDWidget::UpdateDebugDisplay), UpdateInterval, true);
	}
}

void UBeamDebugHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

void UBeamDebugHUDWidget::InitializeWidgetBindings()
{
	// Widget bindings are handled by the meta = (BindWidget) system
	// No manual binding needed for basic functionality
}

void UBeamDebugHUDWidget::UpdateDebugDisplay()
{
	
	UpdateGazeCrosshair();
	UpdateHeadPoseIndicator();
	UpdatePerformanceMetrics();
	UpdateConnectionStatus();
}

void UBeamDebugHUDWidget::UpdateGazeCrosshair()
{
	if (!bShowGazeCrosshair || !GazeCrosshair)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (UBeamEyeTrackerSubsystem* Subsystem = World->GetGameInstance()->GetSubsystem<UBeamEyeTrackerSubsystem>())
		{
			FBeamFrame Frame;
			if (Subsystem->FetchCurrentFrame(Frame))
			{
				bGazeValid = Frame.Gaze.bValid;
				if (bGazeValid)
				{
					CurrentGazeScreen01 = Frame.Gaze.Screen01;
					CurrentGazeScreenPos = Frame.Gaze.ScreenPx;
					CurrentGazeConfidence = Frame.Gaze.Confidence;

					if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(GazeCrosshair->Slot))
					{
						CanvasSlot->SetPosition(CurrentGazeScreenPos);
						GazeCrosshair->SetVisibility(ESlateVisibility::Visible);
					}
				}
				else
				{
					GazeCrosshair->SetVisibility(ESlateVisibility::Hidden);
				}
			}
		}
	}
}

void UBeamDebugHUDWidget::UpdateHeadPoseIndicator()
{
	if (!bShowHeadPoseIndicator || !HeadPoseIndicator)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (UBeamEyeTrackerSubsystem* Subsystem = World->GetGameInstance()->GetSubsystem<UBeamEyeTrackerSubsystem>())
		{
			FBeamFrame Frame;
			if (Subsystem->FetchCurrentFrame(Frame))
			{
				bHeadValid = Frame.Head.Confidence > 0.0f;
				if (bHeadValid)
				{
					CurrentHeadPosition = Frame.Head.PositionCm;
					CurrentHeadRotation = Frame.Head.Rotation;
					CurrentHeadConfidence = Frame.Head.Confidence;

					HeadPoseIndicator->SetVisibility(ESlateVisibility::Visible);
				}
				else
				{
					HeadPoseIndicator->SetVisibility(ESlateVisibility::Hidden);
				}
			}
		}
	}
}

void UBeamDebugHUDWidget::UpdatePerformanceMetrics()
{
	if (!bShowPerformanceMetrics)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (UBeamEyeTrackerSubsystem* Subsystem = World->GetGameInstance()->GetSubsystem<UBeamEyeTrackerSubsystem>())
		{
			CurrentFPS = Subsystem->GetCurrentFPS();
			CurrentBufferUtilization = Subsystem->GetBufferUtilization();
			
		}
	}

	if (PerformanceText)
	{
		FText PerformanceInfo = FText::Format(
			NSLOCTEXT("Beam", "PerformanceFormat", "FPS: {0} | Buffer: {1}%"),
			FText::AsNumber(CurrentFPS),
			FText::AsNumber(CurrentBufferUtilization)
		);
		PerformanceText->SetText(PerformanceInfo);
	}
}

void UBeamDebugHUDWidget::UpdateConnectionStatus()
{
	if (!bShowConnectionStatus)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (UBeamEyeTrackerSubsystem* Subsystem = World->GetGameInstance()->GetSubsystem<UBeamEyeTrackerSubsystem>())
		{
			bIsConnected = Subsystem->IsBeamTracking();
			bIsTracking = Subsystem->IsBeamTracking();

			if (bIsConnected)
			{
				ConnectionStatus = TEXT("Connected");
			}
			else
			{
				ConnectionStatus = TEXT("Disconnected");
			}
		}
	}

	if (ConnectionText)
	{
		FText ConnectionInfo = FText::Format(
			NSLOCTEXT("Beam", "ConnectionFormat", "Status: {0} | Tracking: {1}"),
			FText::FromString(ConnectionStatus),
			bIsTracking ? NSLOCTEXT("Beam", "TrackingActive", "Active") : NSLOCTEXT("Beam", "TrackingInactive", "Inactive")
		);
		ConnectionText->SetText(ConnectionInfo);
	}
}

// Public Blueprint-callable functions
void UBeamDebugHUDWidget::ShowDebugHUD()
{
	bDebugHUDVisible = true;
	SetVisibility(ESlateVisibility::Visible);
}

void UBeamDebugHUDWidget::HideDebugHUD()
{
	bDebugHUDVisible = false;
	SetVisibility(ESlateVisibility::Hidden);
}

void UBeamDebugHUDWidget::ToggleDebugHUD()
{
	if (bDebugHUDVisible)
	{
		HideDebugHUD();
	}
	else
	{
		ShowDebugHUD();
	}
}

void UBeamDebugHUDWidget::SetGazeCrosshairVisible(bool bVisible)
{
	bShowGazeCrosshair = bVisible;
	if (GazeCrosshair)
	{
		GazeCrosshair->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

void UBeamDebugHUDWidget::SetHeadPoseIndicatorVisible(bool bVisible)
{
	bShowHeadPoseIndicator = bVisible;
	if (HeadPoseIndicator)
	{
		HeadPoseIndicator->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

void UBeamDebugHUDWidget::SetPerformanceMetricsVisible(bool bVisible)
{
	bShowPerformanceMetrics = bVisible;
	if (PerformanceText)
	{
		PerformanceText->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

// Public getter functions
bool UBeamDebugHUDWidget::IsDebugHUDVisible() const
{
	return bDebugHUDVisible;
}

FVector2D UBeamDebugHUDWidget::GetCurrentGazeScreenPosition() const
{
	return CurrentGazeScreenPos;
}

FVector UBeamDebugHUDWidget::GetCurrentHeadPosition() const
{
	return CurrentHeadPosition;
}

float UBeamDebugHUDWidget::GetCurrentGazeConfidence() const
{
	return CurrentGazeConfidence;
}

float UBeamDebugHUDWidget::GetCurrentHeadConfidence() const
{
	return CurrentHeadConfidence;
}


