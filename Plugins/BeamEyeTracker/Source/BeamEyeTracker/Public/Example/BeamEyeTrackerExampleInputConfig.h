/*=============================================================================
    BeamEyeTrackerExampleInputConfig.h: Example input configuration for Beam SDK.

    Provides input mapping context and actions for eye tracking integration
    including movement, look controls, and eye tracking toggle actions
    for comprehensive demonstration and testing purposes.

    Copyright (c) 2025 Eyeware Tech SA. All Rights Reserved.

    Eyeware® and Beam® are registered trademarks of Eyeware Tech SA.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "BeamEyeTrackerExampleInputConfig.generated.h"

UCLASS()
class BEAMEYETRACKER_API UBeamEyeTrackerExampleInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* InputMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* ToggleEyeTrackingAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* StopEyeTrackingAction;
};

/*=============================================================================
    End of BeamEyeTrackerExampleInputConfig.h
=============================================================================*/
