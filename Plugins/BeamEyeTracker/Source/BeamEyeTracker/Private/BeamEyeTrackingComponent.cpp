#include "BeamEyeTrackingComponent.h"
#include "BeamEyeTrackerSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "BeamLogging.h"

UBeamEyeTrackingComponent::UBeamEyeTrackingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    bTrackingActive = false;
    LastGazePoint2D = FVector2D::ZeroVector;
    LastGazePoint3D = FVector::ZeroVector;
    LastHeadPose = FTransform::Identity;
    LastConfidence = 0.0f;
    LastUpdateTime = 0.0f;
    FrameCount = 0;
    CurrentFPS = 0.0f;
    
    UpdateFrequency = ValidateUpdateFrequency(UpdateFrequency);
}

void UBeamEyeTrackingComponent::BeginPlay()
{
    Super::BeginPlay();

    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            BeamSubsystem = GameInstance->GetSubsystem<UBeamEyeTrackerSubsystem>();
        }
        
        if (BeamSubsystem)
        {
            // Auto-initialize if enabled
            if (bAutoInitialize)
            {
                InitializeEyeTracking();
            }
        }
        else
        {
            UE_LOG(LogBeam, Warning, TEXT("BeamEyeTrackingComponent: Could not find BeamEyeTrackerSubsystem"));
        }
    }
}

void UBeamEyeTrackingComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Stop tracking and cleanup
    if (bTrackingActive)
    {
        StopEyeTracking();
    }
    
    Super::EndPlay(EndPlayReason);
}

void UBeamEyeTrackingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bAutoUpdate && bTrackingActive && ShouldUpdate())
    {
        UpdateGazeData();
    }
}

bool UBeamEyeTrackingComponent::InitializeEyeTracking()
{
    if (!BeamSubsystem)
    {
        UE_LOG(LogBeam, Warning, TEXT("BeamEyeTrackingComponent: No subsystem available"));
        return false;
    }
    
    if (bTrackingActive)
    {
        UE_LOG(LogBeam, Log, TEXT("BeamEyeTrackingComponent: Eye tracking already active"));
        return true;
    }
    
    // Start tracking using the subsystem
    if (BeamSubsystem->StartBeamTracking())
    {
        bTrackingActive = true;
        LastUpdateTime = GetWorld()->GetTimeSeconds();
        FrameCount = 0;
        
        UE_LOG(LogBeam, Log, TEXT("BeamEyeTrackingComponent: Eye tracking initialized successfully"));
        
        // Call Blueprint event
        OnEyeTrackingInitialized();
        
        return true;
    }
    else
    {
        UE_LOG(LogBeam, Error, TEXT("BeamEyeTrackingComponent: Failed to initialize eye tracking"));
        return false;
    }
}

void UBeamEyeTrackingComponent::StopEyeTracking()
{
    if (!bTrackingActive)
    {
        return;
    }
    
    if (BeamSubsystem)
    {
        BeamSubsystem->StopBeamTracking();
    }
    
    bTrackingActive = false;
    
    UE_LOG(LogBeam, Log, TEXT("BeamEyeTrackingComponent: Eye tracking stopped"));
    
    // Call Blueprint event
    OnEyeTrackingStopped();
}

bool UBeamEyeTrackingComponent::IsEyeTrackingActive() const
{
    return bTrackingActive && BeamSubsystem && BeamSubsystem->IsBeamTracking();
}

FVector2D UBeamEyeTrackingComponent::GetGazePoint2D() const
{
    if (!bTrackingActive || !BeamSubsystem)
    {
        return FVector2D::ZeroVector;
    }

    FGazePoint CurrentGaze = BeamSubsystem->CurrentGaze();
    return CurrentGaze.Screen01;
}

FVector UBeamEyeTrackingComponent::GetGazePoint3D() const
{
    if (!bTrackingActive || !BeamSubsystem)
    {
        return FVector::ZeroVector;
    }

    // This could be enhanced with proper screen-to-world projection
    FVector2D Gaze2D = GetGazePoint2D();
    FVector CameraLocation = FVector::ZeroVector;
    FVector CameraForward = FVector::ForwardVector;
    
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        if (APawn* Pawn = PC->GetPawn())
        {
            CameraLocation = Pawn->GetActorLocation();
            CameraForward = Pawn->GetActorForwardVector();
        }
    }
    
    // Project gaze point into world space (simplified)
    float Distance = 1000.0f; // Default distance
    FVector GazeDirection = FVector(
        (Gaze2D.X - 0.5f) * 2.0f,          (Gaze2D.Y - 0.5f) * 2.0f,          1.0f                        // Forward direction
    );
    
    return CameraLocation + (GazeDirection * Distance);
}

FTransform UBeamEyeTrackingComponent::GetHeadPose() const
{
    if (!bTrackingActive || !BeamSubsystem)
    {
        return FTransform::Identity;
    }

    FHeadPose CurrentHeadPose = BeamSubsystem->HeadPosition();
    
    FTransform HeadTransform;
    HeadTransform.SetLocation(CurrentHeadPose.PositionCm * 0.01f); // Convert cm to meters
    HeadTransform.SetRotation(CurrentHeadPose.Rotation.Quaternion());
    
    return HeadTransform;
}

float UBeamEyeTrackingComponent::GetTrackingConfidence() const
{
    if (!bTrackingActive || !BeamSubsystem)
    {
        return 0.0f;
    }

    // This could be enhanced with actual confidence data from the SDK
    return BeamSubsystem->IsBeamTracking() ? 0.8f : 0.0f;
}

float UBeamEyeTrackingComponent::GetTrackingFPS() const
{
    return CurrentFPS;
}

void UBeamEyeTrackingComponent::SetUpdateFrequency(float NewFrequency)
{
    float ValidatedFrequency = ValidateUpdateFrequency(NewFrequency);
    
    if (ValidatedFrequency != UpdateFrequency)
    {
        UpdateFrequency = ValidatedFrequency;
        UE_LOG(LogBeam, Log, TEXT("BeamEyeTrackingComponent: Update frequency changed to %.1f Hz"), UpdateFrequency);
        
        // Apply performance optimizations if needed
        if (bPerformanceMode)
        {
            ApplyPerformanceOptimizations();
        }
    }
}

void UBeamEyeTrackingComponent::SetPerformanceMode(bool bEnable)
{
    if (bPerformanceMode != bEnable)
    {
        bPerformanceMode = bEnable;
        
        if (bPerformanceMode)
        {
            ApplyPerformanceOptimizations();
            UE_LOG(LogBeam, Log, TEXT("BeamEyeTrackingComponent: Performance mode enabled"));
        }
        else
        {
            UE_LOG(LogBeam, Log, TEXT("BeamEyeTrackingComponent: Performance mode disabled"));
        }
    }
}

void UBeamEyeTrackingComponent::UpdateGazeData()
{
    if (!BeamSubsystem || !bTrackingActive)
    {
        return;
    }

    FGazePoint CurrentGaze = BeamSubsystem->CurrentGaze();
    FHeadPose CurrentHeadPose = BeamSubsystem->HeadPosition();
    
    // Store previous values for comparison
    FVector2D PreviousGaze2D = LastGazePoint2D;
    float PreviousConfidence = LastConfidence;

    LastGazePoint2D = CurrentGaze.Screen01;
    LastGazePoint3D = GetGazePoint3D(); // This will calculate 3D position
    LastHeadPose = GetHeadPose();        // This will calculate head transform
    LastConfidence = GetTrackingConfidence();

    UpdateFPS();
    
    // Call Blueprint events
    OnGazeDataUpdated(LastGazePoint2D, LastGazePoint3D, LastConfidence);
    
    // Check for significant confidence changes
    if (FMath::Abs(LastConfidence - PreviousConfidence) > 0.1f)
    {
        OnConfidenceChanged(LastConfidence, PreviousConfidence);
    }
    
    // Debug visualization
    if (bDebugVisualization)
    {
        // Draw debug sphere at gaze point (if in editor)
        #if WITH_EDITOR
        if (UWorld* World = GetWorld())
        {
            DrawDebugSphere(World, LastGazePoint3D, 5.0f, 8, FColor::Green, false, 0.1f);
        }
        #endif
    }
}

void UBeamEyeTrackingComponent::UpdateFPS()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float DeltaTime = CurrentTime - LastUpdateTime;
    
    if (DeltaTime > 0.0f)
    {
        FrameCount++;
        
        // Calculate FPS over a rolling window
        if (FrameCount >= 60) 
        {
            CurrentFPS = FrameCount / DeltaTime;
            FrameCount = 0;
            LastUpdateTime = CurrentTime;
        }
    }
}

bool UBeamEyeTrackingComponent::ShouldUpdate() const
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

void UBeamEyeTrackingComponent::ApplyPerformanceOptimizations()
{
    if (!bPerformanceMode)
    {
        return;
    }
    
    // Limit update frequency in performance mode
    if (UpdateFrequency > MaxPerformanceFrequency)
    {
        UpdateFrequency = MaxPerformanceFrequency;
        UE_LOG(LogBeam, Log, TEXT("BeamEyeTrackingComponent: Update frequency limited to %.1f Hz for performance mode"), UpdateFrequency);
    }
    
    // Additional performance optimizations could be added here
    // Such as reducing debug visualization, optimizing data structures, etc.
}

float UBeamEyeTrackingComponent::ValidateUpdateFrequency(float Frequency) const
{
    return FMath::Clamp(Frequency, 1.0f, 1000.0f);
}


