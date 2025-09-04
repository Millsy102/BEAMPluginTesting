#include "BeamCalibrationWidget.h"
#include "BeamEyeTrackerSubsystem.h"
#include "BeamAnalyticsSubsystem.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Engine/Engine.h"
#include "Blueprint/WidgetTree.h"

UBeamCalibrationWidget::UBeamCalibrationWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    bCalibrationActive = false;
    CurrentPointIndex = -1;
    PointStartTime = 0.0f;
    PointDuration = 3.0f;
    TotalCalibrationTime = 0.0f;
    SuccessfulPoints = 0;
    FailedPoints = 0;
    AverageQuality = 0.0f;
    LastCalibrationTime = 0.0f;

    CalibrationPoints = {
        FVector2D(0.1f, 0.1f), FVector2D(0.5f, 0.1f), FVector2D(0.9f, 0.1f),
        FVector2D(0.1f, 0.5f), FVector2D(0.5f, 0.5f), FVector2D(0.9f, 0.5f),
        FVector2D(0.1f, 0.9f), FVector2D(0.5f, 0.9f), FVector2D(0.9f, 0.9f)
    };

    PointQualities.SetNum(CalibrationPoints.Num());
    for (int32 i = 0; i < PointQualities.Num(); ++i)
    {
        PointQualities[i] = 0.0f;
    }
}

void UBeamCalibrationWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (UGameInstance* GameInstance = GetOwningPlayer()->GetGameInstance())
    {
        BeamSubsystem = GameInstance->GetSubsystem<UBeamEyeTrackerSubsystem>();
        AnalyticsSubsystem = GameInstance->GetSubsystem<UBeamAnalyticsSubsystem>();
    }

    if (StartCalibrationButton)
    {
        StartCalibrationButton->OnClicked.AddDynamic(this, &UBeamCalibrationWidget::StartCalibration);
    }

    if (StopCalibrationButton)
    {
        StopCalibrationButton->OnClicked.AddDynamic(this, &UBeamCalibrationWidget::StopCalibration);
    }

    if (AssessQualityButton)
    {
        AssessQualityButton->OnClicked.AddDynamic(this, &UBeamCalibrationWidget::AssessQuality);
    }

    UpdateStatusDisplay();
    UpdateInstructions();
    UpdateStatistics();
    CreatePointVisuals();
}

void UBeamCalibrationWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (bCalibrationActive && CurrentPointIndex >= 0)
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        
        if (CurrentTime - PointStartTime >= PointDuration)
        {
            CompleteCurrentPoint();
        }
        else
        {
            float Progress = (CurrentTime - PointStartTime) / PointDuration;
            OnProgressUpdated(Progress, CurrentPointIndex + 1, CalibrationPoints.Num());
        }
    }
}

void UBeamCalibrationWidget::NativeDestruct()
{
    if (bCalibrationActive)
    {
        StopCalibration();
    }

    DestroyPointVisuals();
    Super::NativeDestruct();
}

void UBeamCalibrationWidget::StartCalibration()
{
    if (bCalibrationActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("BeamCalibrationWidget: Calibration already active"));
        return;
    }

    if (!BeamSubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("BeamCalibrationWidget: No Beam subsystem available"));
        return;
    }

    // Reset calibration state
    bCalibrationActive = true;
    CurrentPointIndex = -1;
    TotalCalibrationTime = 0.0f;
    SuccessfulPoints = 0;
    FailedPoints = 0;
    AverageQuality = 0.0f;

    // Clear previous quality data
    for (int32 i = 0; i < PointQualities.Num(); ++i)
    {
        PointQualities[i] = 0.0f;
    }

    // Start with first point
    ActivateNextPoint();

    UpdateStatusDisplay();
    UpdateInstructions();

    // Trigger Blueprint event
    OnCalibrationStarted();

    UE_LOG(LogTemp, Log, TEXT("BeamCalibrationWidget: Calibration started"));
}

void UBeamCalibrationWidget::StopCalibration()
{
    if (!bCalibrationActive)
    {
        return;
    }

    bCalibrationActive = false;
    CurrentPointIndex = -1;

    UpdateStatusDisplay();
    UpdateInstructions();

    UE_LOG(LogTemp, Log, TEXT("BeamCalibrationWidget: Calibration stopped"));
}

void UBeamCalibrationWidget::AssessQuality()
{
    if (!AnalyticsSubsystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("BeamCalibrationWidget: No analytics subsystem available"));
        return;
    }

    // Trigger quality assessment
    AnalyticsSubsystem->AssessCalibration();

    FCalibrationQuality Quality = AnalyticsSubsystem->GetCalibrationQuality();

    CurrentCalibrationQuality = Quality;

    UpdateStatusDisplay();
    UpdateStatistics();
    
    // Trigger Blueprint event
    OnQualityAssessed(Quality);

    UE_LOG(LogTemp, Log, TEXT("BeamCalibrationWidget: Quality assessment complete - Score: %.1f"), Quality.OverallScore);
}

void UBeamCalibrationWidget::ResetCalibration()
{
    if (bCalibrationActive)
    {
        StopCalibration();
    }

    // Reset all data
    SuccessfulPoints = 0;
    FailedPoints = 0;
    AverageQuality = 0.0f;
    LastCalibrationTime = 0.0f;

    for (int32 i = 0; i < PointQualities.Num(); ++i)
    {
        PointQualities[i] = 0.0f;
    }

    UpdateStatusDisplay();
    UpdateStatistics();
    UpdateInstructions();

    UE_LOG(LogTemp, Log, TEXT("BeamCalibrationWidget: Calibration reset"));
}

void UBeamCalibrationWidget::SetCalibrationPoints(const TArray<FVector2D>& Points)
{
    if (bCalibrationActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("BeamCalibrationWidget: Cannot change points during calibration"));
        return;
    }

    CalibrationPoints = Points;
    PointQualities.SetNum(CalibrationPoints.Num());
    
    // Recreate visual elements
    DestroyPointVisuals();
    CreatePointVisuals();

    UE_LOG(LogTemp, Log, TEXT("BeamCalibrationWidget: Calibration points updated - %d points"), Points.Num());
}

void UBeamCalibrationWidget::SetPointDuration(float Duration)
{
    PointDuration = FMath::Max(1.0f, Duration);
    UE_LOG(LogTemp, Log, TEXT("BeamCalibrationWidget: Point duration set to %.1f seconds"), PointDuration);
}

void UBeamCalibrationWidget::GetCalibrationSettings(TArray<FVector2D>& OutPoints, float& OutDuration) const
{
    OutPoints = CalibrationPoints;
    OutDuration = PointDuration;
}

FCalibrationQuality UBeamCalibrationWidget::GetCalibrationQuality() const
{
    if (AnalyticsSubsystem)
    {
        return AnalyticsSubsystem->GetCalibrationQuality();
    }
    return FCalibrationQuality();
}

float UBeamCalibrationWidget::GetCalibrationScore() const
{
    if (AnalyticsSubsystem)
    {
        return AnalyticsSubsystem->GetCalibrationScore();
    }
    return 0.0f;
}

bool UBeamCalibrationWidget::IsCalibrated() const
{
    if (BeamSubsystem)
    {
        return BeamSubsystem->IsCalibrated();
    }
    return false;
}

void UBeamCalibrationWidget::ActivateNextPoint()
{
    CurrentPointIndex++;
    
    if (CurrentPointIndex >= CalibrationPoints.Num())
    {
        // Calibration complete
        CalculateOverallQuality();
        bCalibrationActive = false;
        CurrentPointIndex = -1;

        UpdateStatusDisplay();
        UpdateInstructions();
        UpdateStatistics();
        
        // Trigger Blueprint event
        OnCalibrationCompleted();
        
        UE_LOG(LogTemp, Log, TEXT("BeamCalibrationWidget: Calibration completed successfully"));
        return;
    }

    // Activate current point
    PointStartTime = GetWorld()->GetTimeSeconds();

    UpdateCalibrationDisplay();
    UpdateInstructions();
    
    // Trigger Blueprint event
    OnPointActivated(CurrentPointIndex, CalibrationPoints[CurrentPointIndex]);
    
    UE_LOG(LogTemp, Log, TEXT("BeamCalibrationWidget: Point %d activated at (%.2f, %.2f)"), 
        CurrentPointIndex + 1, CalibrationPoints[CurrentPointIndex].X, CalibrationPoints[CurrentPointIndex].Y);
}

void UBeamCalibrationWidget::CompleteCurrentPoint()
{
    if (CurrentPointIndex < 0 || CurrentPointIndex >= CalibrationPoints.Num())
    {
        return;
    }

    FGazePoint CurrentGaze = BeamSubsystem->CurrentGaze();
    
    // Calculate quality for this point
    float Quality = CalculatePointQuality(CalibrationPoints[CurrentPointIndex], CurrentGaze.Screen01);
    PointQualities[CurrentPointIndex] = Quality;
    
    // Determine if point was successful
    if (Quality >= 0.7f) // 70% threshold
    {
        SuccessfulPoints++;
    }
    else
    {
        FailedPoints++;
    }
    
    // Trigger Blueprint event
    OnPointCompleted(CurrentPointIndex, Quality);
    
    UE_LOG(LogTemp, Log, TEXT("BeamCalibrationWidget: Point %d completed with quality %.2f"), CurrentPointIndex + 1, Quality);
    
    // Move to next point
    ActivateNextPoint();
}

void UBeamCalibrationWidget::CalculateOverallQuality()
{
    float TotalQuality = 0.0f;
    int32 ValidPoints = 0;
    
    for (int32 i = 0; i < PointQualities.Num(); ++i)
    {
        if (PointQualities[i] > 0.0f)
        {
            TotalQuality += PointQualities[i];
            ValidPoints++;
        }
    }
    
    if (ValidPoints > 0)
    {
        AverageQuality = TotalQuality / ValidPoints;
    }

    LastCalibrationTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Log, TEXT("BeamCalibrationWidget: Overall quality calculated: %.2f (%.1f%% success rate)"), 
        AverageQuality, (float)SuccessfulPoints / CalibrationPoints.Num() * 100.0f);
}

void UBeamCalibrationWidget::UpdateStatistics()
{
    if (StatisticsText)
    {
        FString Stats = FString::Printf(TEXT("Successful Points: %d\nFailed Points: %d\nAverage Quality: %.1f%%\nLast Calibration: %.1fs ago"),
            SuccessfulPoints, FailedPoints, AverageQuality * 100.0f, 
            GetWorld()->GetTimeSeconds() - LastCalibrationTime);
        StatisticsText->SetText(FText::FromString(Stats));
    }
}

void UBeamCalibrationWidget::CreatePointVisuals()
{
    if (!CalibrationCanvas || !WidgetTree)
    {
        return;
    }

    // Clear existing visuals
    DestroyPointVisuals();

    for (int32 i = 0; i < CalibrationPoints.Num(); ++i)
    {
        UImage* PointImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
        if (PointImage)
        {
            // Position the point on the canvas
            FVector2D Position = CalibrationPoints[i];
            FVector2D CanvasSize = CalibrationCanvas->GetCachedGeometry().GetLocalSize();
            
            FVector2D AbsolutePosition;
            AbsolutePosition.X = Position.X * CanvasSize.X;
            AbsolutePosition.Y = Position.Y * CanvasSize.Y;

            PointImage->SetVisibility(ESlateVisibility::Hidden); // Hidden by default
            
            // Add to canvas
            CalibrationCanvas->AddChild(PointImage);
            PointImages.Add(PointImage);
        }
    }
}

void UBeamCalibrationWidget::DestroyPointVisuals()
{
    for (UImage* PointImage : PointImages)
    {
        if (PointImage)
        {
            PointImage->RemoveFromParent();
        }
    }
    PointImages.Empty();
}

void UBeamCalibrationWidget::UpdateCalibrationDisplay()
{
    // Show current point
    for (int32 i = 0; i < PointImages.Num(); ++i)
    {
        if (PointImages[i])
        {
            if (i == CurrentPointIndex)
            {
                PointImages[i]->SetVisibility(ESlateVisibility::Visible);
            }
            else
            {
                PointImages[i]->SetVisibility(ESlateVisibility::Hidden);
            }
        }
    }
}

void UBeamCalibrationWidget::UpdateInstructions()
{
    if (!InstructionsText)
    {
        return;
    }

    FString Instructions;
    
    if (!bCalibrationActive)
    {
        if (IsCalibrated())
        {
            Instructions = TEXT("Calibration complete! Click 'Assess Quality' to check calibration quality.");
        }
        else
        {
            Instructions = TEXT("Click 'Start Calibration' to begin the calibration process.\nLook at each point as it appears.");
        }
    }
    else
    {
        Instructions = FString::Printf(TEXT("Look at point %d of %d\nKeep your gaze steady on the target"),
            CurrentPointIndex + 1, CalibrationPoints.Num());
    }
    
    InstructionsText->SetText(FText::FromString(Instructions));
}

void UBeamCalibrationWidget::UpdateStatusDisplay()
{
    if (StatusText)
    {
        FString Status;
        if (bCalibrationActive)
        {
            Status = FString::Printf(TEXT("Calibrating... Point %d/%d"), CurrentPointIndex + 1, CalibrationPoints.Num());
        }
        else if (IsCalibrated())
        {
            Status = TEXT("Calibrated");
        }
        else
        {
            Status = TEXT("Not Calibrated");
        }
        StatusText->SetText(FText::FromString(Status));
    }

    if (QualityText)
    {
        float Score = GetCalibrationScore();
        FString Quality = FString::Printf(TEXT("Quality: %.1f%%"), Score);
        QualityText->SetText(FText::FromString(Quality));
    }

    if (QualityProgressBar)
    {
        float Score = GetCalibrationScore() / 100.0f;
        QualityProgressBar->SetPercent(Score);
    }
}

float UBeamCalibrationWidget::CalculatePointQuality(const FVector2D& TargetPoint, const FVector2D& GazePoint)
{
    // Calculate distance between target and gaze
    float Distance = FVector2D::Distance(TargetPoint, GazePoint);
    
    // Convert distance to quality score (0.0 = perfect, 1.0 = poor)
    // Threshold of 0.1 (10% of screen) for minimum acceptable quality
    float Quality = FMath::Clamp(1.0f - (Distance / 0.1f), 0.0f, 1.0f);
    
    return Quality;
}

bool UBeamCalibrationWidget::IsGazeOnTarget(const FVector2D& TargetPoint, const FVector2D& GazePoint, float Threshold)
{
    float Distance = FVector2D::Distance(TargetPoint, GazePoint);
    return Distance <= Threshold;
}


