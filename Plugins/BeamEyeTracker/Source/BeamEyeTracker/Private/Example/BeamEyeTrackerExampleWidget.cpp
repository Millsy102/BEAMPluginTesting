#include "BeamEyeTrackerExampleWidget.h"
#include "BeamEyeTrackerExampleCharacter.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"

UBeamEyeTrackerExampleWidget::UBeamEyeTrackerExampleWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CharacterRef = nullptr;
}

void UBeamEyeTrackerExampleWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UBeamEyeTrackerExampleWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	UpdateWidgetDisplay();
}

void UBeamEyeTrackerExampleWidget::SetCharacterReference(ABeamEyeTrackerExampleCharacter* Character)
{
	CharacterRef = Character;
}

void UBeamEyeTrackerExampleWidget::UpdateWidgetDisplay()
{
	if (!CharacterRef)
	{
		return;
	}

	UpdateStatusDisplay();
	UpdateGazeDisplay();
	UpdateHeadPoseDisplay();
	UpdateConfidenceDisplay();
	UpdateFrameInfo();
	UpdateHealthStatus();
}

void UBeamEyeTrackerExampleWidget::UpdateStatusDisplay()
{
	if (!StatusText || !StatusBorder)
	{
		return;
	}

	EBeamHealth Health = CharacterRef->GetTrackingHealth();
	StatusText->SetText(GetStatusText(Health));
	StatusBorder->SetBrushColor(GetStatusColor(Health));
}

void UBeamEyeTrackerExampleWidget::UpdateGazeDisplay()
{
	if (!GazePositionText || !GazeIndicator)
	{
		return;
	}

	FGazePoint GazePoint = CharacterRef->GetCurrentGazePoint();
	
	if (GazePoint.bValid)
	{
		
		FString GazeText = FString::Printf(TEXT("Gaze: (%.2f, %.2f) | Pixels: (%.0f, %.0f)"), 
			GazePoint.Screen01.X, GazePoint.Screen01.Y,
			GazePoint.ScreenPx.X, GazePoint.ScreenPx.Y);
		GazePositionText->SetText(FText::FromString(GazeText));

		// The actual positioning would be handled in the Blueprint widget
	}
	else
	{
		GazePositionText->SetText(FText::FromString(TEXT("Gaze: No Data")));
	}
}

void UBeamEyeTrackerExampleWidget::UpdateHeadPoseDisplay()
{
	if (!HeadPoseText)
	{
		return;
	}

	FHeadPose HeadPose = CharacterRef->GetCurrentHeadPose();
	
	if (HeadPose.Confidence > 0.0f)
	{
		FString HeadText = FString::Printf(TEXT("Head: Pos(%.1f, %.1f, %.1f) | Rot(%.1f, %.1f, %.1f)"), 
			HeadPose.PositionCm.X, HeadPose.PositionCm.Y, HeadPose.PositionCm.Z,
			HeadPose.Rotation.Pitch, HeadPose.Rotation.Yaw, HeadPose.Rotation.Roll);
		HeadPoseText->SetText(FText::FromString(HeadText));
	}
	else
	{
		HeadPoseText->SetText(FText::FromString(TEXT("Head: No Data")));
	}
}

void UBeamEyeTrackerExampleWidget::UpdateConfidenceDisplay()
{
	if (!ConfidenceText || !ConfidenceBar)
	{
		return;
	}

	FGazePoint GazePoint = CharacterRef->GetCurrentGazePoint();
	FHeadPose HeadPose = CharacterRef->GetCurrentHeadPose();
	
	// Use the higher confidence between gaze and head pose
	float Confidence = FMath::Max(GazePoint.Confidence, HeadPose.Confidence);

	FString ConfidenceString = FString::Printf(TEXT("Confidence: %.1f%%"), Confidence * 100.0f);
	ConfidenceText->SetText(FText::FromString(ConfidenceString));

	ConfidenceBar->SetPercent(Confidence);

	FLinearColor BarColor;
	if (Confidence > 0.7f)
	{
		BarColor = FLinearColor::Green;
	}
	else if (Confidence > 0.4f)
	{
		BarColor = FLinearColor::Yellow;
	}
	else
	{
		BarColor = FLinearColor::Red;
	}
	
	ConfidenceBar->SetFillColorAndOpacity(BarColor);
}

void UBeamEyeTrackerExampleWidget::UpdateFrameInfo()
{
	if (!FrameInfoText)
	{
		return;
	}

	FBeamFrame Frame = CharacterRef->GetCurrentFrame();
	
	if (Frame.FrameId > 0)
	{
		FString FrameText = FString::Printf(TEXT("Frame: %lld | SDK: %.1fms | UE: %.3fs | Delta: %.3fs"), 
			Frame.FrameId, Frame.SDKTimestampMs, Frame.UETimestampSeconds, Frame.DeltaTimeSeconds);
		FrameInfoText->SetText(FText::FromString(FrameText));
	}
	else
	{
		FrameInfoText->SetText(FText::FromString(TEXT("Frame: No Data")));
	}
}

void UBeamEyeTrackerExampleWidget::UpdateHealthStatus()
{
	if (!HealthText)
	{
		return;
	}

	EBeamHealth Health = CharacterRef->GetTrackingHealth();
	bool bActive = CharacterRef->IsEyeTrackingActive();
	
	FString HealthString;
	if (bActive)
	{
		HealthString = FString::Printf(TEXT("Tracking: ACTIVE | Health: %s"), 
			*GetStatusText(Health).ToString());
	}
	else
	{
		HealthString = TEXT("Tracking: INACTIVE");
	}
	
	HealthText->SetText(FText::FromString(HealthString));
}

FLinearColor UBeamEyeTrackerExampleWidget::GetStatusColor(EBeamHealth Health) const
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
		return FLinearColor(1.0f, 0.5f, 0.0f, 1.0f); // Custom orange color
	case EBeamHealth::Error:
	default:
		return FLinearColor::Red;
	}
}

FText UBeamEyeTrackerExampleWidget::GetStatusText(EBeamHealth Health) const
{
	switch (Health)
	{
	case EBeamHealth::Ok:
		return FText::FromString(TEXT("OK"));
	case EBeamHealth::AppNotRunning:
		return FText::FromString(TEXT("Beam App Not Running"));
	case EBeamHealth::DllMissing:
		return FText::FromString(TEXT("DLL Missing"));
	case EBeamHealth::NoData:
		return FText::FromString(TEXT("No Data"));
	case EBeamHealth::Recovering:
		return FText::FromString(TEXT("Recovering"));
	case EBeamHealth::Error:
	default:
		return FText::FromString(TEXT("Error"));
	}
}

