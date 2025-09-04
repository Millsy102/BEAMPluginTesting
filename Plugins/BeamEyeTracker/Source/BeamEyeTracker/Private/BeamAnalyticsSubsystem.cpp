#include "BeamAnalyticsSubsystem.h"
#include "BeamEyeTrackerSubsystem.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

UBeamAnalyticsSubsystem::UBeamAnalyticsSubsystem()
{
    bAnalyticsActive = false;
    bPerformanceMonitoringActive = false;
    SamplingRate = 60.0f;
    MinFixationDuration = 0.1f;
    MaxGapTime = 0.5f;
    LastUpdateTime = 0.0f;
}

void UBeamAnalyticsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        BeamSubsystem = GameInstance->GetSubsystem<UBeamEyeTrackerSubsystem>();
        if (BeamSubsystem)
        {
            UE_LOG(LogTemp, Log, TEXT("BeamAnalyticsSubsystem initialized successfully"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("BeamAnalyticsSubsystem: Failed to get BeamEyeTrackerSubsystem"));
        }
    }
}

void UBeamAnalyticsSubsystem::Deinitialize()
{
    if (bAnalyticsActive)
    {
        StopGazeAnalytics();
    }
    
    if (bPerformanceMonitoringActive)
    {
        StopPerformanceMonitoring();
    }
    
    Super::Deinitialize();
}

void UBeamAnalyticsSubsystem::StartGazeAnalytics()
{
    if (!BeamSubsystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("BeamAnalyticsSubsystem: Cannot start analytics - no Beam subsystem"));
        return;
    }
    
    if (bAnalyticsActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("BeamAnalyticsSubsystem: Analytics already active"));
        return;
    }
    
    bAnalyticsActive = true;
    ResetGazeAnalytics();
    OnAnalyticsStarted();
    
    UE_LOG(LogTemp, Log, TEXT("BeamAnalyticsSubsystem: Gaze analytics started"));
}

void UBeamAnalyticsSubsystem::StopGazeAnalytics()
{
    if (!bAnalyticsActive)
    {
        return;
    }
    
    bAnalyticsActive = false;
    OnAnalyticsStopped();
    
    UE_LOG(LogTemp, Log, TEXT("BeamAnalyticsSubsystem: Gaze analytics stopped"));
}

FGazeAnalytics UBeamAnalyticsSubsystem::GetGazeAnalytics() const
{
    return CurrentAnalytics;
}

void UBeamAnalyticsSubsystem::ResetGazeAnalytics()
{
    CurrentAnalytics = FGazeAnalytics();
    GazeHistory.Empty();
    GazeTimestamps.Empty();
    LastUpdateTime = 0.0f;
}

FCalibrationQuality UBeamAnalyticsSubsystem::GetCalibrationQuality() const
{
    return CurrentCalibrationQuality;
}

void UBeamAnalyticsSubsystem::AssessCalibration()
{
    if (!BeamSubsystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("BeamAnalyticsSubsystem: Cannot assess calibration - no Beam subsystem"));
        return;
    }

    bool bIsCalibrated = BeamSubsystem->IsCalibrated();
    
    if (bIsCalibrated)
    {
        // Simulate calibration quality assessment
        // In a real implementation, this would use actual calibration data from the SDK
        CurrentCalibrationQuality.OverallScore = FMath::RandRange(80.0f, 95.0f);
        CurrentCalibrationQuality.LeftEyeScore = FMath::RandRange(75.0f, 95.0f);
        CurrentCalibrationQuality.RightEyeScore = FMath::RandRange(75.0f, 95.0f);
        CurrentCalibrationQuality.LastCalibrationTime = FPlatformTime::Seconds();
        
        // Generate sample calibration points
        CurrentCalibrationQuality.CalibrationPoints.Empty();
        CurrentCalibrationQuality.PointScores.Empty();
        
        for (int32 i = 0; i < 9; ++i)
        {
            FVector2D Point;
            Point.X = FMath::RandRange(0.1f, 0.9f);
            Point.Y = FMath::RandRange(0.1f, 0.9f);
            CurrentCalibrationQuality.CalibrationPoints.Add(Point);
            CurrentCalibrationQuality.PointScores.Add(FMath::RandRange(70.0f, 95.0f));
        }
    }
    else
    {
        CurrentCalibrationQuality.OverallScore = 0.0f;
        CurrentCalibrationQuality.LeftEyeScore = 0.0f;
        CurrentCalibrationQuality.RightEyeScore = 0.0f;
        CurrentCalibrationQuality.CalibrationPoints.Empty();
        CurrentCalibrationQuality.PointScores.Empty();
    }
    
    OnCalibrationAssessed(CurrentCalibrationQuality);
    
    UE_LOG(LogTemp, Log, TEXT("BeamAnalyticsSubsystem: Calibration assessment complete - Score: %.1f"), CurrentCalibrationQuality.OverallScore);
}

float UBeamAnalyticsSubsystem::GetCalibrationScore() const
{
    return CurrentCalibrationQuality.OverallScore;
}

void UBeamAnalyticsSubsystem::StartPerformanceMonitoring()
{
    if (bPerformanceMonitoringActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("BeamAnalyticsSubsystem: Performance monitoring already active"));
        return;
    }
    
    bPerformanceMonitoringActive = true;
    CurrentPerformanceMetrics = FBeamPerformanceMetrics();
    OnPerformanceMonitoringStarted();
    
    UE_LOG(LogTemp, Log, TEXT("BeamAnalyticsSubsystem: Performance monitoring started"));
}

void UBeamAnalyticsSubsystem::StopPerformanceMonitoring()
{
    if (!bPerformanceMonitoringActive)
    {
        return;
    }
    
    bPerformanceMonitoringActive = false;
    OnPerformanceMonitoringStopped();
    
    UE_LOG(LogTemp, Log, TEXT("BeamAnalyticsSubsystem: Performance monitoring stopped"));
}

FBeamPerformanceMetrics UBeamAnalyticsSubsystem::GetPerformanceMetrics() const
{
    return CurrentPerformanceMetrics;
}

float UBeamAnalyticsSubsystem::GetPerformanceScore() const
{
    if (CurrentPerformanceMetrics.AverageFrameTime <= 0.0f)
    {
        return 0.0f;
    }
    
    // Calculate performance score based on frame time
    // Target: 16.67ms (60 FPS), Excellent: < 16.67ms, Good: < 33.33ms, Poor: > 33.33ms
    float TargetFrameTime = 16.67f;
    float Score = FMath::Clamp((TargetFrameTime / CurrentPerformanceMetrics.AverageFrameTime) * 100.0f, 0.0f, 100.0f);
    
    return Score;
}

bool UBeamAnalyticsSubsystem::ExportAnalyticsData(const FString& FilePath)
{
    if (!bAnalyticsActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("BeamAnalyticsSubsystem: Cannot export - analytics not active"));
        return false;
    }
    
    FString CSVData = TEXT("Timestamp,AverageFixationDuration,SaccadeVelocity,FixationCount,ScanPathLength\n");
    
    // Add current analytics data
    FString Row = FString::Printf(TEXT("%.3f,%.3f,%.3f,%d,%.3f\n"),
        CurrentAnalytics.TimeStamp,
        CurrentAnalytics.AverageFixationDuration,
        CurrentAnalytics.SaccadeVelocity,
        CurrentAnalytics.FixationCount,
        CurrentAnalytics.ScanPathLength);
    
    CSVData += Row;

    bool bSuccess = FFileHelper::SaveStringToFile(CSVData, *FilePath);
    
    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("BeamAnalyticsSubsystem: Analytics data exported to %s"), *FilePath);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("BeamAnalyticsSubsystem: Failed to export analytics data to %s"), *FilePath);
    }
    
    return bSuccess;
}

bool UBeamAnalyticsSubsystem::ExportPerformanceData(const FString& FilePath)
{
    if (!bPerformanceMonitoringActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("BeamAnalyticsSubsystem: Cannot export - performance monitoring not active"));
        return false;
    }
    
    FString CSVData = TEXT("Timestamp,AverageFrameTime,MinFrameTime,MaxFrameTime,CPUUsage,MemoryUsage,DroppedFrames\n");
    
    // Add current performance data
    FString Row = FString::Printf(TEXT("%.3f,%.3f,%.3f,%.3f,%.2f,%.2f,%d\n"),
        CurrentPerformanceMetrics.TimeStamp,
        CurrentPerformanceMetrics.AverageFrameTime,
        CurrentPerformanceMetrics.MinFrameTime,
        CurrentPerformanceMetrics.MaxFrameTime,
        CurrentPerformanceMetrics.CPUUsage,
        CurrentPerformanceMetrics.MemoryUsage,
        CurrentPerformanceMetrics.DroppedFrames);
    
    CSVData += Row;

    bool bSuccess = FFileHelper::SaveStringToFile(CSVData, *FilePath);
    
    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("BeamAnalyticsSubsystem: Performance data exported to %s"), *FilePath);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("BeamAnalyticsSubsystem: Failed to export performance data to %s"), *FilePath);
    }
    
    return bSuccess;
}

void UBeamAnalyticsSubsystem::SetAnalyticsSettings(float InSamplingRate, float InMinFixationDuration, float InMaxGapTime)
{
    SamplingRate = FMath::Max(1.0f, InSamplingRate);
    MinFixationDuration = FMath::Max(0.01f, InMinFixationDuration);
    MaxGapTime = FMath::Max(0.1f, InMaxGapTime);
    
    UE_LOG(LogTemp, Log, TEXT("BeamAnalyticsSubsystem: Settings updated - Rate: %.1f, MinFix: %.3f, MaxGap: %.3f"),
        SamplingRate, MinFixationDuration, MaxGapTime);
}

void UBeamAnalyticsSubsystem::GetAnalyticsSettings(float& OutSamplingRate, float& OutMinFixationDuration, float& OutMaxGapTime) const
{
    OutSamplingRate = SamplingRate;
    OutMinFixationDuration = MinFixationDuration;
    OutMaxGapTime = MaxGapTime;
}

void UBeamAnalyticsSubsystem::UpdateGazeAnalytics()
{
    if (!bAnalyticsActive || !BeamSubsystem)
    {
        return;
    }
    
    float CurrentTime = FPlatformTime::Seconds();

    if (CurrentTime - LastUpdateTime < (1.0f / SamplingRate))
    {
        return;
    }

    FGazePoint CurrentGaze = BeamSubsystem->CurrentGaze();
    
    if (CurrentGaze.Confidence > 0.5f)
    {
        // Add to history
        GazeHistory.Add(CurrentGaze.Screen01);
        GazeTimestamps.Add(CurrentTime);

        CleanupOldData();
        
        // Calculate analytics
        CalculateFixations();
        CalculateSaccades();

        CurrentAnalytics.TimeStamp = CurrentTime;
        
        // Trigger Blueprint event
        OnAnalyticsUpdated(CurrentAnalytics);
    }
    
    LastUpdateTime = CurrentTime;
}

void UBeamAnalyticsSubsystem::UpdatePerformanceMetrics()
{
    if (!bPerformanceMonitoringActive)
    {
        return;
    }
    
    float CurrentTime = FPlatformTime::Seconds();

    if (GEngine)
    {
        float DeltaTime = 1.0f / 60.0f; // Use fixed delta time for performance monitoring
        
        if (DeltaTime > 0.0f)
        {
            
            if (CurrentPerformanceMetrics.AverageFrameTime <= 0.0f)
            {
                CurrentPerformanceMetrics.AverageFrameTime = DeltaTime;
                CurrentPerformanceMetrics.MinFrameTime = DeltaTime;
                CurrentPerformanceMetrics.MaxFrameTime = DeltaTime;
            }
            else
            {
                // Rolling average
                CurrentPerformanceMetrics.AverageFrameTime = 
                    (CurrentPerformanceMetrics.AverageFrameTime * 0.9f) + (DeltaTime * 0.1f);
                
                CurrentPerformanceMetrics.MinFrameTime = FMath::Min(CurrentPerformanceMetrics.MinFrameTime, DeltaTime);
                CurrentPerformanceMetrics.MaxFrameTime = FMath::Max(CurrentPerformanceMetrics.MaxFrameTime, DeltaTime);
            }
            
            // Check for dropped frames (frame time > 33.33ms = < 30 FPS)
            if (DeltaTime > 0.033f)
            {
                CurrentPerformanceMetrics.DroppedFrames++;
            }
        }
    }
    
    // Simulate CPU and memory usage (in real implementation, get from platform)
    CurrentPerformanceMetrics.CPUUsage = FMath::RandRange(5.0f, 25.0f);
    CurrentPerformanceMetrics.MemoryUsage = FMath::RandRange(100.0f, 500.0f);
    
    CurrentPerformanceMetrics.TimeStamp = CurrentTime;
    
    // Trigger Blueprint event
    OnPerformanceUpdated(CurrentPerformanceMetrics);
}

void UBeamAnalyticsSubsystem::CalculateFixations()
{
    if (GazeHistory.Num() < 2)
    {
        return;
    }
    
    TArray<float> FixationDurations;
    TArray<FVector2D> FixationCenters;
    
    int32 StartIndex = 0;
    FVector2D StartPoint = GazeHistory[0];
    
    for (int32 i = 1; i < GazeHistory.Num(); ++i)
    {
        float Distance = CalculateDistance(StartPoint, GazeHistory[i]);
        
        // If gaze moved significantly, end current fixation
        if (Distance > 0.05f) // 5% of screen size threshold
        {
            float Duration = GazeTimestamps[i - 1] - GazeTimestamps[StartIndex];
            
            if (Duration >= MinFixationDuration)
            {
                FixationDurations.Add(Duration);
                
                // Calculate fixation center
                FVector2D Center = FVector2D::ZeroVector;
                for (int32 j = StartIndex; j < i; ++j)
                {
                    Center += GazeHistory[j];
                }
                Center /= (i - StartIndex);
                FixationCenters.Add(Center);
            }
            
            StartIndex = i;
            StartPoint = GazeHistory[i];
        }
    }
    
    // Handle last fixation
    if (StartIndex < GazeHistory.Num() - 1)
    {
        float Duration = GazeTimestamps.Last() - GazeTimestamps[StartIndex];
        if (Duration >= MinFixationDuration)
        {
            FixationDurations.Add(Duration);
            
            FVector2D Center = FVector2D::ZeroVector;
            for (int32 j = StartIndex; j < GazeHistory.Num(); ++j)
            {
                Center += GazeHistory[j];
            }
            Center /= (GazeHistory.Num() - StartIndex);
            FixationCenters.Add(Center);
        }
    }

    CurrentAnalytics.FixationCount = FixationCenters.Num();
    CurrentAnalytics.FixationPoints = FixationCenters;
    
    if (FixationDurations.Num() > 0)
    {
        float TotalDuration = 0.0f;
        for (float Duration : FixationDurations)
        {
            TotalDuration += Duration;
        }
        CurrentAnalytics.AverageFixationDuration = TotalDuration / FixationDurations.Num();
    }
}

void UBeamAnalyticsSubsystem::CalculateSaccades()
{
    if (GazeHistory.Num() < 2)
    {
        return;
    }
    
    float TotalVelocity = 0.0f;
    int32 SaccadeCount = 0;
    
    for (int32 i = 1; i < GazeHistory.Num(); ++i)
    {
        float Distance = CalculateDistance(GazeHistory[i - 1], GazeHistory[i]);
        float TimeDelta = GazeTimestamps[i] - GazeTimestamps[i - 1];
        
        if (TimeDelta > 0.0f && Distance > 0.01f) // Significant movement threshold
        {
            float Velocity = Distance / TimeDelta;
            TotalVelocity += Velocity;
            SaccadeCount++;
        }
    }
    
    if (SaccadeCount > 0)
    {
        CurrentAnalytics.SaccadeVelocity = TotalVelocity / SaccadeCount;
    }
    
    // Calculate scan path length
    float ScanLength = 0.0f;
    for (int32 i = 1; i < GazeHistory.Num(); ++i)
    {
        ScanLength += CalculateDistance(GazeHistory[i - 1], GazeHistory[i]);
    }
    CurrentAnalytics.ScanPathLength = ScanLength;
}

float UBeamAnalyticsSubsystem::CalculateDistance(const FVector2D& Point1, const FVector2D& Point2)
{
    return FVector2D::Distance(Point1, Point2);
}

void UBeamAnalyticsSubsystem::CleanupOldData()
{
    float CurrentTime = FPlatformTime::Seconds();
    float MaxAge = 10.0f; // Keep 10 seconds of data
    
    // Remove old gaze points
    while (GazeHistory.Num() > 0 && GazeTimestamps.Num() > 0)
    {
        if (CurrentTime - GazeTimestamps[0] > MaxAge)
        {
            GazeHistory.RemoveAt(0);
            GazeTimestamps.RemoveAt(0);
        }
        else
        {
            break;
        }
    }
}


