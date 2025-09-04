// Copyright (C) 2025 Eyeware

// Implements Blueprint utility functions for Beam

#include "BeamBlueprintLibrary.h"
#include "BeamEyeTrackerSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Camera/CameraComponent.h"
#include "BeamLogging.h"

bool UBeamBlueprintLibrary::InitializeEyeTracking(const UObject* WorldContextObject)
{
    UBeamEyeTrackerSubsystem* Subsystem = GetSubsystemSafe(WorldContextObject);
    if (!Subsystem)
    {
        UE_LOG(LogBeam, Warning, TEXT("BeamBlueprintLibrary: Could not get subsystem for initialization"));
        return false;
    }
    
    return Subsystem->StartBeamTracking();
}

bool UBeamBlueprintLibrary::IsEyeTrackingAvailable(const UObject* WorldContextObject)
{
    UBeamEyeTrackerSubsystem* Subsystem = GetSubsystemSafe(WorldContextObject);
    if (!Subsystem)
    {
        return false;
    }

    return Subsystem->IsBeamTracking() || Subsystem->StartBeamTracking();
}

UBeamEyeTrackerSubsystem* UBeamBlueprintLibrary::GetEyeTrackingSubsystem(const UObject* WorldContextObject)
{
    return GetSubsystemSafe(WorldContextObject);
}
// Basic Data Access
FVector2D UBeamBlueprintLibrary::GetGazePoint2D(const UObject* WorldContextObject)
{
    UBeamEyeTrackerSubsystem* Subsystem = GetSubsystemSafe(WorldContextObject);
    if (!Subsystem || !Subsystem->IsBeamTracking())
    {
        return FVector2D::ZeroVector;
    }
    
    FGazePoint GazePoint = Subsystem->CurrentGaze();
    return GazePoint.Screen01;
}

FVector2D UBeamBlueprintLibrary::GetGazePointPixels(const UObject* WorldContextObject)
{
    UBeamEyeTrackerSubsystem* Subsystem = GetSubsystemSafe(WorldContextObject);
    if (!Subsystem || !Subsystem->IsBeamTracking())
    {
        return FVector2D(-1.0f, -1.0f); // Invalid value
    }
    
    FGazePoint GazePoint = Subsystem->CurrentGaze();
    
    // Convert normalized coordinates to pixels
    if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            int32 ViewportSizeX, ViewportSizeY;
            PC->GetViewportSize(ViewportSizeX, ViewportSizeY);
            
            if (ViewportSizeX > 0 && ViewportSizeY > 0)
            {
                            return FVector2D(
                GazePoint.Screen01.X * ViewportSizeX,
                GazePoint.Screen01.Y * ViewportSizeY
            );
            }
        }
    }
    
    return FVector2D(-1.0f, -1.0f); // Invalid value
}

FVector UBeamBlueprintLibrary::GetHeadPosition(const UObject* WorldContextObject)
{
    UBeamEyeTrackerSubsystem* Subsystem = GetSubsystemSafe(WorldContextObject);
    if (!Subsystem || !Subsystem->IsBeamTracking())
    {
        return FVector::ZeroVector;
    }
    
    FHeadPose HeadPose = Subsystem->HeadPosition();
    return HeadPose.PositionCm * 0.01f; // Convert cm to meters
}

FRotator UBeamBlueprintLibrary::GetHeadRotation(const UObject* WorldContextObject)
{
    UBeamEyeTrackerSubsystem* Subsystem = GetSubsystemSafe(WorldContextObject);
    if (!Subsystem || !Subsystem->IsBeamTracking())
    {
        return FRotator::ZeroRotator;
    }
    
    FHeadPose HeadPose = Subsystem->HeadPosition();
    return HeadPose.Rotation;
}

FTransform UBeamBlueprintLibrary::GetHeadPoseTransform(const UObject* WorldContextObject)
{
    FVector Position = GetHeadPosition(WorldContextObject);
    FRotator Rotation = GetHeadRotation(WorldContextObject);
    
    FTransform Transform;
    Transform.SetLocation(Position);
    Transform.SetRotation(Rotation.Quaternion());
    
    return Transform;
}
//~ End Basic Data Access

//~ Begin Advanced Data Access
float UBeamBlueprintLibrary::GetTrackingConfidence(const UObject* WorldContextObject)
{
    UBeamEyeTrackerSubsystem* Subsystem = GetSubsystemSafe(WorldContextObject);
    if (!Subsystem || !Subsystem->IsBeamTracking())
    {
        return 0.0f;
    }

    // This could be enhanced with actual confidence data from the SDK
    return 0.8f; // Default confidence when tracking is active
}

float UBeamBlueprintLibrary::GetTrackingFPS(const UObject* WorldContextObject)
{
    UBeamEyeTrackerSubsystem* Subsystem = GetSubsystemSafe(WorldContextObject);
    if (!Subsystem)
    {
        return 0.0f;
    }
    
    return Subsystem->GetCurrentFPS();
}

bool UBeamBlueprintLibrary::IsLookingAtActor(const UObject* WorldContextObject, AActor* TargetActor, float MaxDistance)
{
    if (!TargetActor)
    {
        return false;
    }
    
    FVector GazeWorldPos = ScreenToWorldPosition(GetGazePoint2D(WorldContextObject));
    FVector ActorLocation = TargetActor->GetActorLocation();
    
    float Distance = GetDistance3D(GazeWorldPos, ActorLocation);
    return Distance <= MaxDistance;
}

AActor* UBeamBlueprintLibrary::GetClosestActorToGaze(const UObject* WorldContextObject, TArray<AActor*> ActorList, float MaxDistance)
{
    if (ActorList.Num() == 0)
    {
        return nullptr;
    }
    
    FVector GazeWorldPos = ScreenToWorldPosition(GetGazePoint2D(WorldContextObject));
    AActor* ClosestActor = nullptr;
    float ClosestDistance = MaxDistance;
    
    for (AActor* Actor : ActorList)
    {
        if (Actor)
        {
            float Distance = GetDistance3D(GazeWorldPos, Actor->GetActorLocation());
            if (Distance < ClosestDistance)
            {
                ClosestDistance = Distance;
                ClosestActor = Actor;
            }
        }
    }
    
    return ClosestActor;
}
//~ End Advanced Data Access

//~ Begin Control Functions
bool UBeamBlueprintLibrary::StartEyeTracking(const UObject* WorldContextObject)
{
    return InitializeEyeTracking(WorldContextObject);
}

void UBeamBlueprintLibrary::StopEyeTracking(const UObject* WorldContextObject)
{
    UBeamEyeTrackerSubsystem* Subsystem = GetSubsystemSafe(WorldContextObject);
    if (Subsystem)
    {
        Subsystem->StopBeamTracking();
    }
}

bool UBeamBlueprintLibrary::IsEyeTrackingActive(const UObject* WorldContextObject)
{
    UBeamEyeTrackerSubsystem* Subsystem = GetSubsystemSafe(WorldContextObject);
    return Subsystem && Subsystem->IsBeamTracking();
}
//~ End Control Functions

//~ Begin Utility Functions
FVector UBeamBlueprintLibrary::ScreenToWorldPosition(const FVector2D& ScreenPosition, float Distance)
{
    // Simple screen-to-world projection
    // This could be enhanced with proper camera projection
    FVector WorldPos;
    WorldPos.X = (ScreenPosition.X - 0.5f) * 2.0f * Distance;
    WorldPos.Y = (ScreenPosition.Y - 0.5f) * 2.0f * Distance;
    WorldPos.Z = Distance;
    
    return WorldPos;
}

FVector2D UBeamBlueprintLibrary::WorldToScreenPosition(const FVector& WorldPosition)
{
    // Simple world-to-screen projection
    // This could be enhanced with proper camera projection
    FVector2D ScreenPos;
    ScreenPos.X = (WorldPosition.X / (2.0f * WorldPosition.Z)) + 0.5f;
    ScreenPos.Y = (WorldPosition.Y / (2.0f * WorldPosition.Z)) + 0.5f;
    
    return ScreenPos;
}

float UBeamBlueprintLibrary::GetDistance3D(const FVector& PointA, const FVector& PointB)
{
    return FVector::Dist(PointA, PointB);
}

float UBeamBlueprintLibrary::GetDistance2D(const FVector2D& PointA, const FVector2D& PointB)
{
    return FVector2D::Distance(PointA, PointB);
}
//~ End Utility Functions

//~ Begin Validation & Error Handling
bool UBeamBlueprintLibrary::IsValidGazePoint(const FVector2D& GazePoint)
{
    
    return GazePoint.X >= 0.0f && GazePoint.X <= 1.0f &&
           GazePoint.Y >= 0.0f && GazePoint.Y <= 1.0f;
}

FString UBeamBlueprintLibrary::GetErrorMessage(const UObject* WorldContextObject)
{
    UBeamEyeTrackerSubsystem* Subsystem = GetSubsystemSafe(WorldContextObject);
    
    if (!Subsystem)
    {
        return TEXT("Eye tracking subsystem not available. Make sure the plugin is enabled.");
    }
    
    if (!Subsystem->IsBeamTracking())
    {
        return TEXT("Eye tracking is not active. Try starting tracking first.");
    }
    
    // Check for specific error conditions
    if (Subsystem->GetCurrentFPS() <= 0.0f)
    {
        return TEXT("No eye tracking data received. Check hardware connection and calibration.");
    }
    
    return TEXT("Eye tracking is working normally.");
}

bool UBeamBlueprintLibrary::NeedsCalibration(const UObject* WorldContextObject)
{
    UBeamEyeTrackerSubsystem* Subsystem = GetSubsystemSafe(WorldContextObject);
    if (!Subsystem)
    {
        return true; // Assume calibration needed if no subsystem
    }

    // This could be enhanced with actual calibration status from the SDK
    return !Subsystem->IsBeamTracking();
}
//~ End Validation & Error Handling

//~ Begin Private Helper Functions
UBeamEyeTrackerSubsystem* UBeamBlueprintLibrary::GetSubsystemSafe(const UObject* WorldContextObject)
{
    if (!WorldContextObject)
    {
        UE_LOG(LogBeam, Warning, TEXT("BeamBlueprintLibrary: WorldContextObject is null"));
        return nullptr;
    }
    
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        UE_LOG(LogBeam, Warning, TEXT("BeamBlueprintLibrary: Could not get World from context"));
        return nullptr;
    }
    
    if (UGameInstance* GameInstance = World->GetGameInstance())
    {
        return GameInstance->GetSubsystem<UBeamEyeTrackerSubsystem>();
    }
    return nullptr;
}

bool UBeamBlueprintLibrary::ValidateWorldContext(const UObject* WorldContextObject, FString& OutErrorMessage)
{
    if (!WorldContextObject)
    {
        OutErrorMessage = TEXT("World context object is null");
        return false;
    }
    
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        OutErrorMessage = TEXT("Could not get world from context");
        return false;
    }
    
    return true;
}

bool UBeamBlueprintLibrary::GetPlayerCameraInfo(const UObject* WorldContextObject, FVector& OutLocation, FRotator& OutRotation)
{
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        return false;
    }
    
    if (APlayerController* PC = World->GetFirstPlayerController())
    {
        if (APawn* Pawn = PC->GetPawn())
        {
            OutLocation = Pawn->GetActorLocation();
            OutRotation = Pawn->GetActorRotation();
            return true;
        }
    }
    
    return false;
}
//~ End Private Helper Functions


