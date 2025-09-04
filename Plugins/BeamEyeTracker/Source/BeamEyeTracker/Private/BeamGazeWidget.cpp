#include "BeamGazeWidget.h"
#include "BeamEyeTrackerSubsystem.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/VerticalBox.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "BeamLogging.h"

UBeamGazeWidget::UBeamGazeWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    CurrentDisplayMode = EDisplayMode::Detailed;
    UpdateFrequency = 30.0f;
    LastUpdateTime = 0.0f;
    CurrentTrackingStatus = EBeamWidgetStatus::NotTracking;
    CurrentGazePoint = FVector2D::ZeroVector;
    CurrentConfidence = 0.0f;
    CurrentFPS = 0.0f;
    LastFPSUpdateTime = 0.0f;
    FrameCount = 0;
}

void UBeamGazeWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            BeamSubsystem = GameInstance->GetSubsystem<UBeamEyeTrackerSubsystem>();
        }
        
        if (BeamSubsystem)
        {
            UE_LOG(LogBeam, Log, TEXT("BeamGazeWidget: Successfully connected to eye tracking subsystem"));
        }
        else
        {
            UE_LOG(LogBeam, Warning, TEXT("BeamGazeWidget: Could not find eye tracking subsystem"));
        }
    }
    
    UpdateWidget();
}

void UBeamGazeWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    
    if (ShouldUpdate())
    {
        UpdateWidget();
        LastUpdateTime = GetWorld()->GetTimeSeconds();
    }
    
    CalculateFPS();
}

void UBeamGazeWidget::NativeDestruct()
{
    // Cleanup
    BeamSubsystem = nullptr;
    
    Super::NativeDestruct();
}

void UBeamGazeWidget::SetGazeVisualization(bool bEnable)
{
    if (GazeIndicator)
    {
        GazeIndicator->SetVisibility(bEnable ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
    }
}

void UBeamGazeWidget::SetDisplayMode(EDisplayMode Mode)
{
    CurrentDisplayMode = Mode;

    if (HeadPoseInfo)
    {
        HeadPoseInfo->SetVisibility(Mode == EDisplayMode::Expert ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
    }
    
    if (PerformanceMetrics)
    {
        PerformanceMetrics->SetVisibility(Mode == EDisplayMode::Expert ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
    }
    
    if (GazeCoordinatesText)
    {
        GazeCoordinatesText->SetVisibility(Mode == EDisplayMode::Detailed || Mode == EDisplayMode::Expert ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
    }
}

void UBeamGazeWidget::SetUpdateFrequency(float Frequency)
{
    UpdateFrequency = FMath::Clamp(Frequency, 1.0f, 120.0f);
}

EBeamWidgetStatus UBeamGazeWidget::GetTrackingStatus() const
{
    return CurrentTrackingStatus;
}

FVector2D UBeamGazeWidget::GetCurrentGaze() const
{
    return CurrentGazePoint;
}

float UBeamGazeWidget::GetCurrentConfidence() const
{
    return CurrentConfidence;
}

void UBeamGazeWidget::UpdateWidget()
{
    if (!BeamSubsystem)
    {
        return;
    }

    bool bIsTracking = BeamSubsystem->IsBeamTracking();
    FGazePoint GazePoint = BeamSubsystem->CurrentGaze();
    FHeadPose HeadPose = BeamSubsystem->HeadPosition();
    float FPS = BeamSubsystem->GetCurrentFPS();

    EBeamWidgetStatus PreviousStatus = CurrentTrackingStatus;
    float PreviousConfidence = CurrentConfidence;
    
    CurrentGazePoint = GazePoint.Screen01;
    CurrentFPS = FPS;
    
    // Determine tracking status
    if (!bIsTracking)
    {
        CurrentTrackingStatus = EBeamWidgetStatus::NotTracking;
        CurrentConfidence = 0.0f;
    }
    else if (FPS <= 0.0f)
    {
        CurrentTrackingStatus = EBeamWidgetStatus::Initializing;
        CurrentConfidence = 0.5f;
    }
    else if (GazePoint.Screen01.X < 0.0f || GazePoint.Screen01.Y < 0.0f)
    {
        CurrentTrackingStatus = EBeamWidgetStatus::LowConfidence;
        CurrentConfidence = 0.3f;
    }
    else
    {
        CurrentTrackingStatus = EBeamWidgetStatus::Tracking;
        CurrentConfidence = 0.8f;
    }

    UpdateGazeIndicator();
    UpdateStatusText();
    UpdateCalibrationQuality();
    UpdateFPSDisplay();
    UpdateConfidenceDisplay();
    UpdateHeadPoseInfo();
    UpdateGazeCoordinates();
    UpdatePerformanceMetrics();
    UpdateErrorDisplay();
    
    // Call Blueprint events
    if (PreviousStatus != CurrentTrackingStatus)
    {
        OnTrackingStatusChanged(CurrentTrackingStatus, PreviousStatus);
    }
    
    OnGazeDataUpdated(CurrentGazePoint, CurrentConfidence);
    
    if (FMath::Abs(CurrentConfidence - PreviousConfidence) > 0.1f)
    {
        OnConfidenceChanged(CurrentConfidence, PreviousConfidence);
    }
}

void UBeamGazeWidget::UpdateGazeIndicator()
{
    if (!GazeIndicator)
    {
        return;
    }
    
    // Position gaze indicator based on current gaze point
    if (CurrentTrackingStatus == EBeamWidgetStatus::Tracking)
    {
        // Convert normalized coordinates to screen position
        FVector2D ScreenPosition = CurrentGazePoint;

        GazeIndicator->SetVisibility(ESlateVisibility::Visible);
        
        // You could also animate the indicator or change its appearance based on confidence
        if (CurrentConfidence > 0.7f)
        {
            // High confidence - solid indicator
            GazeIndicator->SetColorAndOpacity(FLinearColor::Green);
        }
        else if (CurrentConfidence > 0.4f)
        {
            // Medium confidence - yellow indicator
            GazeIndicator->SetColorAndOpacity(FLinearColor::Yellow);
        }
        else
        {
            // Low confidence - red indicator
            GazeIndicator->SetColorAndOpacity(FLinearColor::Red);
        }
    }
    else
    {
        GazeIndicator->SetVisibility(ESlateVisibility::Hidden);
    }
}

void UBeamGazeWidget::UpdateStatusText()
{
    if (!StatusText)
    {
        return;
    }
    
    FString StatusString;
    FLinearColor StatusColor = GetStatusColor(CurrentTrackingStatus);
    
    switch (CurrentTrackingStatus)
    {
        case EBeamWidgetStatus::NotTracking:
            StatusString = TEXT("Not Tracking");
            break;
        case EBeamWidgetStatus::Initializing:
            StatusString = TEXT("Initializing...");
            break;
        case EBeamWidgetStatus::Tracking:
            StatusString = TEXT("Tracking Active");
            break;
        case EBeamWidgetStatus::LowConfidence:
            StatusString = TEXT("Low Confidence");
            break;
        case EBeamWidgetStatus::Error:
            StatusString = TEXT("Error");
            break;
        default:
            StatusString = TEXT("Unknown");
            break;
    }
    
    StatusText->SetText(FText::FromString(StatusString));
    StatusText->SetColorAndOpacity(StatusColor);
}

void UBeamGazeWidget::UpdateCalibrationQuality()
{
    if (!CalibrationQuality)
    {
        return;
    }

    CalibrationQuality->SetPercent(CurrentConfidence);

    FLinearColor BarColor = GetConfidenceColor(CurrentConfidence);
    CalibrationQuality->SetFillColorAndOpacity(BarColor);
}

void UBeamGazeWidget::UpdateFPSDisplay()
{
    if (!FPSText)
    {
        return;
    }
    
    FString FPSString = FString::Printf(TEXT("FPS: %.1f"), CurrentFPS);
    FPSText->SetText(FText::FromString(FPSString));
}

void UBeamGazeWidget::UpdateConfidenceDisplay()
{
    if (!ConfidenceText)
    {
        return;
    }
    
    FString ConfidenceString = FString::Printf(TEXT("Confidence: %.1f%%"), CurrentConfidence * 100.0f);
    ConfidenceText->SetText(FText::FromString(ConfidenceString));
    ConfidenceText->SetColorAndOpacity(GetConfidenceColor(CurrentConfidence));
}

void UBeamGazeWidget::UpdateHeadPoseInfo()
{
    if (!HeadPoseInfo || CurrentDisplayMode != EDisplayMode::Expert)
    {
        return;
    }
    
    // This would update detailed head pose information
    
    HeadPoseInfo->SetVisibility(ESlateVisibility::Visible);
}

void UBeamGazeWidget::UpdateGazeCoordinates()
{
    if (!GazeCoordinatesText || (CurrentDisplayMode != EDisplayMode::Detailed && CurrentDisplayMode != EDisplayMode::Expert))
    {
        return;
    }
    
    FString CoordinatesString = FString::Printf(TEXT("Gaze: (%.3f, %.3f)"), CurrentGazePoint.X, CurrentGazePoint.Y);
    GazeCoordinatesText->SetText(FText::FromString(CoordinatesString));
}

void UBeamGazeWidget::UpdatePerformanceMetrics()
{
    if (!PerformanceMetrics || CurrentDisplayMode != EDisplayMode::Expert)
    {
        return;
    }
    
    // This would update detailed performance metrics
    
    PerformanceMetrics->SetVisibility(ESlateVisibility::Visible);
}

void UBeamGazeWidget::UpdateErrorDisplay()
{
    if (!ErrorText)
    {
        return;
    }
    
    if (CurrentTrackingStatus == EBeamWidgetStatus::Error)
    {
        ErrorText->SetText(FText::FromString(TEXT("Eye tracking error detected. Check hardware connection.")));
        ErrorText->SetVisibility(ESlateVisibility::Visible);
    }
    else
    {
        ErrorText->SetVisibility(ESlateVisibility::Hidden);
    }
}

bool UBeamGazeWidget::ShouldUpdate() const
{
    if (UpdateFrequency <= 0.0f)
    {
        return false;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float TimeSinceLastUpdate = CurrentTime - LastUpdateTime;
    float UpdateInterval = 1.0f / UpdateFrequency;
    
    return TimeSinceLastUpdate >= UpdateInterval;
}

void UBeamGazeWidget::CalculateFPS()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    FrameCount++;
    
    if (CurrentTime - LastFPSUpdateTime >= 1.0f) 
    {
        CurrentFPS = FrameCount / (CurrentTime - LastFPSUpdateTime);
        FrameCount = 0;
        LastFPSUpdateTime = CurrentTime;
    }
}

FLinearColor UBeamGazeWidget::GetStatusColor(EBeamWidgetStatus Status) const
{
    switch (Status)
    {
        case EBeamWidgetStatus::NotTracking:
            return FLinearColor::Red;
        case EBeamWidgetStatus::Initializing:
            return FLinearColor::Yellow;
        case EBeamWidgetStatus::Tracking:
            return FLinearColor::Green;
        case EBeamWidgetStatus::LowConfidence:
            return FLinearColor(1.0f, 0.5f, 0.0f, 1.0f); // Orange
        case EBeamWidgetStatus::Error:
            return FLinearColor::Red;
        default:
            return FLinearColor::White;
    }
}

FLinearColor UBeamGazeWidget::GetConfidenceColor(float Confidence) const
{
    if (Confidence >= 0.8f)
    {
        return FLinearColor::Green;
    }
    else if (Confidence >= 0.6f)
    {
        return FLinearColor::Yellow;
    }
    else if (Confidence >= 0.4f)
    {
        return FLinearColor(1.0f, 0.5f, 0.0f, 1.0f); // Orange
    }
    else
    {
        return FLinearColor::Red;
    }
}


