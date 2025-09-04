/*=============================================================================
    BEAMSDK.cpp: Main module implementation for BEAM SDK project.

    Provides console commands and module initialization for testing
    the Beam Eye Tracker plugin functionality.

    Copyright (c) 2025 Eyeware Tech SA. All Rights Reserved.

    Eyeware® and Beam® are registered trademarks of Eyeware Tech SA.
=============================================================================*/

#include "BEAMSDK.h"
#include "Modules/ModuleManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

IMPLEMENT_PRIMARY_GAME_MODULE(FDefaultGameModuleImpl, BEAMSDK, "BEAMSDK");

// Console command to switch to our Beam SDK GameMode
static void SwitchToBeamSDKGameMode(const TArray<FString>& Args)
{
    if (GEngine && GEngine->GetWorldContexts().Num() > 0)
    {
        UWorld* World = GEngine->GetWorldContexts()[0].World();
        if (World)
        {
            World->ServerTravel(TEXT("?game=BEAMSDKGameModeBase"));
            UE_LOG(LogTemp, Log, TEXT("Switched to Beam SDK GameMode"));
        }
    }
}

static FAutoConsoleCommand SwitchToBeamSDKGameModeCommand(
    TEXT("beam.switchgamemode"),
    TEXT("Switch to Beam SDK GameMode for testing"),
    FConsoleCommandWithArgsDelegate::CreateStatic(&SwitchToBeamSDKGameMode)
);

// Console command to spawn the example actor
static void SpawnExampleActor(const TArray<FString>& Args)
{
    if (GEngine && GEngine->GetWorldContexts().Num() > 0)
    {
        UWorld* World = GEngine->GetWorldContexts()[0].World();
        if (World)
        {
            FActorSpawnParameters SpawnParams;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            
            // We need to include the header for this to work
            // ABeamEyeTrackerExampleActor* SpawnedActor = World->SpawnActor<ABeamEyeTrackerExampleActor>(FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
            // if (SpawnedActor)
            // {
            //     UE_LOG(LogTemp, Log, TEXT("Spawned BeamEyeTrackerExampleActor"));
            // }
            UE_LOG(LogTemp, Log, TEXT("Example actor spawn command - implement in Blueprint or use GameMode"));
        }
    }
}

static FAutoConsoleCommand SpawnExampleActorCommand(
	TEXT("beam.spawnactor"),
	TEXT("Spawn the Beam Eye Tracker example actor"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&SpawnExampleActor)
);

/*=============================================================================
    End of BEAMSDK.cpp
=============================================================================*/
