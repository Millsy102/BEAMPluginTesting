/*=============================================================================
    BeamEyeTrackerModule.cpp: Core plugin module entry point.

    Handles module lifecycle management including startup and shutdown
    operations for the Beam Eye Tracker plugin system.

    Copyright (c) 2025 Eyeware Tech SA. All Rights Reserved.

    Eyeware® and Beam® are registered trademarks of Eyeware Tech SA.
=============================================================================*/

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Engine/Engine.h"
#include "BeamEyeTracker.h"
#include "BeamEyeTrackerSubsystem.h"
#include "BeamEyeTrackerProvider.h"
#include "BeamLogging.h"

#include "BeamLogging.h"

#define LOCTEXT_NAMESPACE "FBeamEyeTrackerModule"

// Module Lifecycle Management

void FBeamEyeTrackerModule::StartupModule()
{
	UE_LOG(LogBeam, Log, TEXT("Beam Eye Tracker module started"));
	
	// Subsystems are automatically registered by Unreal Engine
}

void FBeamEyeTrackerModule::ShutdownModule()
{
	UE_LOG(LogBeam, Log, TEXT("Beam Eye Tracker module shutdown"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FBeamEyeTrackerModule, BeamEyeTracker)

/*=============================================================================
    End of BeamEyeTrackerModule.cpp
=============================================================================*/
