/*=============================================================================
    BeamEyeTrackerExampleHUD.cpp: Example HUD implementation for Beam Eye Tracker.

    Implements an example HUD that demonstrates how to integrate eye tracking
    data into a custom HUD system with real-time status panels and visualization.

    Copyright (c) 2025 Eyeware Tech SA. All Rights Reserved.

    Eyeware® and Beam® are registered trademarks of Eyeware Tech SA.
=============================================================================*/

#include "BeamEyeTrackerExampleHUD.h"
#include "BeamEyeTrackerSubsystem.h"
#include "BeamEyeTrackerComponent.h"
#include "BeamEyeTrackerTypes.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"
#include "Engine/Canvas.h"
#include "CanvasItem.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Misc/App.h"

// Constructor

ABeamEyeTrackerExampleHUD::ABeamEyeTrackerExampleHUD()
{
	
	HUDUpdateInterval = 0.1f;
	bShowStatusPanel = true;
	bShowGazeCrosshair = true;
	bShowGazeTrail = true;
	bShowPerformanceMetrics = true;
	StatusPanelPosition = FVector2D(20.0f, 20.0f);
	StatusPanelSize = FVector2D(300.0f, 200.0f);
	MaxTrailPoints = 30;
}

// Lifecycle

void ABeamEyeTrackerExampleHUD::BeginPlay()
{
	Super::BeginPlay();

	// Start HUD update timer
	GetWorld()->GetTimerManager().SetTimer(HUDUpdateTimerHandle, this, &ABeamEyeTrackerExampleHUD::UpdateHUDData, HUDUpdateInterval, true);
}

void ABeamEyeTrackerExampleHUD::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateHUDData();
}

void ABeamEyeTrackerExampleHUD::DrawHUD()
{
	Super::DrawHUD();

	// Draw status panel
	if (bShowStatusPanel)
	{
		DrawStatusPanel();
	}

	// Draw gaze crosshair
	if (bShowGazeCrosshair)
	{
		DrawGazeCrosshair();
	}

	// Draw gaze trail
	if (bShowGazeTrail)
	{
		DrawGazeTrail();
	}

	// Draw performance metrics
	if (bShowPerformanceMetrics)
	{
		DrawPerformanceMetrics();
	}
}

// HUD Update Functions

void ABeamEyeTrackerExampleHUD::UpdateHUDData()
{
	if (UBeamEyeTrackerSubsystem* Subsystem = GetWorld()->GetGameInstance()->GetSubsystem<UBeamEyeTrackerSubsystem>())
	{
		
		bIsTracking = Subsystem->IsBeamTracking();
		CurrentHealth = Subsystem->GetBeamHealth();
		CurrentFPS = Subsystem->GetTrackingFPS();
		CurrentBufferUtilization = Subsystem->GetBufferUtilization();
		CurrentGazePoint = Subsystem->CurrentGaze();
		CurrentHeadPose = Subsystem->HeadPosition();
		bIsRecording = Subsystem->IsRecording();
		bIsPlayingBack = Subsystem->IsPlayingBack();

		if (CurrentGazePoint.bValid)
		{
			// Convert normalized coordinates to screen coordinates
			FVector2D ViewportSize;
			if (GEngine && GEngine->GameViewport)
			{
				GEngine->GameViewport->GetViewportSize(ViewportSize);
			}
			else
			{
				ViewportSize = FVector2D(1920.0f, 1080.0f);
			}

			FVector2D ScreenPos = FVector2D(
				CurrentGazePoint.Screen01.X * ViewportSize.X,
				CurrentGazePoint.Screen01.Y * ViewportSize.Y
			);

			// Add to trail
			GazeTrailPoints.Add(ScreenPos);

			// Limit trail length
			if (GazeTrailPoints.Num() > MaxTrailPoints)
			{
				GazeTrailPoints.RemoveAt(0);
			}
		}
	}
}

// Drawing Functions

void ABeamEyeTrackerExampleHUD::DrawStatusPanel()
{
	if (!Canvas)
	{
		return;
	}

	// Draw background panel
	FCanvasBoxItem BackgroundBox(StatusPanelPosition, StatusPanelSize);
	BackgroundBox.SetColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.7f));
	Canvas->DrawItem(BackgroundBox);

	FCanvasTextItem TextItem(FVector2D::ZeroVector, FText::GetEmpty(), GEngine->GetLargeFont(), FLinearColor::White);
	TextItem.Scale = FVector2D(0.8f, 0.8f);

	// Draw title
	TextItem.Text = FText::FromString(TEXT("Beam Eye Tracker Status"));
	TextItem.SetColor(FLinearColor::Yellow);
	Canvas->DrawItem(TextItem, StatusPanelPosition + FVector2D(10.0f, 10.0f));

	// Draw status information
	float YOffset = StatusPanelPosition.Y + 40.0f;
	const float LineHeight = 20.0f;

	// Tracking status
	FString StatusText = GetTrackingStatusString();
	TextItem.Text = FText::FromString(StatusText);
	TextItem.SetColor(GetTrackingStatusColor());
	Canvas->DrawItem(TextItem, FVector2D(StatusPanelPosition.X + 10.0f, YOffset));
	YOffset += LineHeight;

	// Health status
	FString HealthText = FString::Printf(TEXT("Health: %s"), *GetHealthStatusString(CurrentHealth));
	TextItem.Text = FText::FromString(HealthText);
	TextItem.SetColor(GetHealthColor(CurrentHealth));
	Canvas->DrawItem(TextItem, FVector2D(StatusPanelPosition.X + 10.0f, YOffset));
	YOffset += LineHeight;

	// FPS
	FString FPSText = FString::Printf(TEXT("FPS: %.1f Hz"), CurrentFPS);
	TextItem.Text = FText::FromString(FPSText);
	TextItem.SetColor(FLinearColor::White);
	Canvas->DrawItem(TextItem, FVector2D(StatusPanelPosition.X + 10.0f, YOffset));
	YOffset += LineHeight;

	// Buffer utilization
	FString BufferText = FString::Printf(TEXT("Buffer: %.1f%%"), CurrentBufferUtilization * 100.0f);
	TextItem.Text = FText::FromString(BufferText);
	TextItem.SetColor(FLinearColor::White);
	Canvas->DrawItem(TextItem, FVector2D(StatusPanelPosition.X + 10.0f, YOffset));
	YOffset += LineHeight;

	// Recording status
	if (bIsRecording)
	{
		TextItem.Text = FText::FromString(TEXT("Recording: ACTIVE"));
		TextItem.SetColor(FLinearColor::Red);
		Canvas->DrawItem(TextItem, FVector2D(StatusPanelPosition.X + 10.0f, YOffset));
		YOffset += LineHeight;
	}

	// Playback status
	if (bIsPlayingBack)
	{
		TextItem.Text = FText::FromString(TEXT("Playback: ACTIVE"));
		TextItem.SetColor(FLinearColor::Blue);
		Canvas->DrawItem(TextItem, FVector2D(StatusPanelPosition.X + 10.0f, YOffset));
		YOffset += LineHeight;
	}
}

void ABeamEyeTrackerExampleHUD::DrawGazeCrosshair()
{
	if (!Canvas || !CurrentGazePoint.bValid)
	{
		return;
	}

	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}
	else
	{
		ViewportSize = FVector2D(1920.0f, 1080.0f);
	}

	// Convert normalized coordinates to screen coordinates
	const float ScreenX = CurrentGazePoint.Screen01.X * ViewportSize.X;
	const float ScreenY = CurrentGazePoint.Screen01.Y * ViewportSize.Y;

	// Draw crosshair
	const float CrosshairSize = 20.0f;
	const float LineThickness = 2.0f;
	const FLinearColor CrosshairColor = FLinearColor::Red;

	// Draw horizontal line
	FCanvasLineItem LineItem1(FVector2D(ScreenX - CrosshairSize, ScreenY), FVector2D(ScreenX + CrosshairSize, ScreenY));
	LineItem1.SetColor(CrosshairColor);
	LineItem1.LineThickness = LineThickness;
	Canvas->DrawItem(LineItem1);

	// Draw vertical line
	FCanvasLineItem LineItem2(FVector2D(ScreenX, ScreenY - CrosshairSize), FVector2D(ScreenX, ScreenY + CrosshairSize));
	LineItem2.SetColor(CrosshairColor);
	LineItem2.LineThickness = LineThickness;
	Canvas->DrawItem(LineItem2);

	// Draw confidence circle (simplified as a box for now)
	FCanvasBoxItem CircleItem(FVector2D(ScreenX - CrosshairSize * 0.3f, ScreenY - CrosshairSize * 0.3f), 
		FVector2D(CrosshairSize * 0.6f, CrosshairSize * 0.6f));
	CircleItem.SetColor(CrosshairColor);
	Canvas->DrawItem(CircleItem);
}

void ABeamEyeTrackerExampleHUD::DrawGazeTrail()
{
	if (!Canvas || GazeTrailPoints.Num() < 2)
	{
		return;
	}

	// Draw trail segments
	for (int32 i = 1; i < GazeTrailPoints.Num(); ++i)
	{
		const float Alpha = 1.0f - (float)i / GazeTrailPoints.Num();
		const FVector2D Start = GazeTrailPoints[i - 1];
		const FVector2D End = GazeTrailPoints[i];
		
		FCanvasLineItem LineItem(Start, End);
		LineItem.SetColor(FLinearColor(1.0f, 1.0f, 1.0f, Alpha));
		LineItem.LineThickness = 2.0f * Alpha;
		Canvas->DrawItem(LineItem);
	}
}

void ABeamEyeTrackerExampleHUD::DrawPerformanceMetrics()
{
	if (!Canvas)
	{
		return;
	}

	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}
	else
	{
		ViewportSize = FVector2D(1920.0f, 1080.0f);
	}

	// Draw performance metrics in top-right corner
	FCanvasTextItem TextItem(FVector2D::ZeroVector, FText::GetEmpty(), GEngine->GetMediumFont(), FLinearColor::White);
	TextItem.Scale = FVector2D(0.6f, 0.6f);

	FVector2D MetricsPosition = FVector2D(ViewportSize.X - 200.0f, 20.0f);
	float YOffset = MetricsPosition.Y;
	const float LineHeight = 16.0f;

	// Frame time
	float FrameTime = 1.0f / FMath::Max(CurrentFPS, 1.0f);
	FString FrameTimeText = FString::Printf(TEXT("Frame: %.2f ms"), FrameTime * 1000.0f);
	TextItem.Text = FText::FromString(FrameTimeText);
	TextItem.SetColor(FLinearColor::White);
	Canvas->DrawItem(TextItem, FVector2D(MetricsPosition.X, YOffset));
	YOffset += LineHeight;

	// Memory usage (placeholder)
	FString MemoryText = TEXT("Memory: N/A");
	TextItem.Text = FText::FromString(MemoryText);
	TextItem.SetColor(FLinearColor::White);
	Canvas->DrawItem(TextItem, FVector2D(MetricsPosition.X, YOffset));
	YOffset += LineHeight;

	FString UpdateRateText = FString::Printf(TEXT("Update: %.1f Hz"), 1.0f / HUDUpdateInterval);
	TextItem.Text = FText::FromString(UpdateRateText);
	TextItem.SetColor(FLinearColor::White);
	Canvas->DrawItem(TextItem, FVector2D(MetricsPosition.X, YOffset));
}

// Utility Functions

FString ABeamEyeTrackerExampleHUD::GetHealthStatusString(EBeamHealth Health) const
{
	switch (Health)
	{
		case EBeamHealth::Ok:
			return TEXT("OK");
		case EBeamHealth::AppNotRunning:
			return TEXT("App Not Running");
		case EBeamHealth::DllMissing:
			return TEXT("DLL Missing");
		case EBeamHealth::NoData:
			return TEXT("No Data");
		case EBeamHealth::Recovering:
			return TEXT("Recovering");
		case EBeamHealth::Error:
		default:
			return TEXT("Error");
	}
}

FLinearColor ABeamEyeTrackerExampleHUD::GetHealthColor(EBeamHealth Health) const
{
	switch (Health)
	{
		case EBeamHealth::Ok:
			return FLinearColor::Green;
		case EBeamHealth::AppNotRunning:
			return FLinearColor::Red;
		case EBeamHealth::DllMissing:
			return FLinearColor::Red;
		case EBeamHealth::NoData:
			return FLinearColor::Yellow;
		case EBeamHealth::Recovering:
			return FLinearColor(1.0f, 0.5f, 0.0f, 1.0f); // Orange
		case EBeamHealth::Error:
		default:
			return FLinearColor::Red;
	}
}

FString ABeamEyeTrackerExampleHUD::GetTrackingStatusString() const
{
	return bIsTracking ? TEXT("Tracking: ACTIVE") : TEXT("Tracking: INACTIVE");
}

FLinearColor ABeamEyeTrackerExampleHUD::GetTrackingStatusColor() const
{
	return bIsTracking ? FLinearColor::Green : FLinearColor::Red;
}

/*=============================================================================
    End of BeamEyeTrackerExampleHUD.cpp
=============================================================================*/


