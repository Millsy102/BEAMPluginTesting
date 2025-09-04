/*=============================================================================
    BeamEyeTrackerExampleWidget.h: Example widget for Beam SDK demonstration.

    Provides a comprehensive example widget showing how to integrate
    eye tracking data into UMG interfaces with real-time updates
    for status, gaze position, head pose, and confidence metrics.

    Copyright (c) 2025 Eyeware Tech SA. All Rights Reserved.

    Eyeware® and Beam® are registered trademarks of Eyeware Tech SA.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BeamEyeTrackerTypes.h"
#include "BeamEyeTrackerExampleWidget.generated.h"

class ABeamEyeTrackerExampleCharacter;
class UTextBlock;
class UProgressBar;
class UImage;
class UBorder;

UCLASS(BlueprintType, Blueprintable)
class BEAMEYETRACKER_API UBeamEyeTrackerExampleWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UBeamEyeTrackerExampleWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// Character reference
	UPROPERTY()
	ABeamEyeTrackerExampleCharacter* CharacterRef;

	// UI Elements
	UPROPERTY(meta = (BindWidget))
	UTextBlock* StatusText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* GazePositionText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* HeadPoseText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ConfidenceText;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* ConfidenceBar;

	UPROPERTY(meta = (BindWidget))
	UBorder* StatusBorder;

	UPROPERTY(meta = (BindWidget))
	UImage* GazeIndicator;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* FrameInfoText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* HealthText;

public:
	
	UFUNCTION(BlueprintCallable, Category = "Widget")
	void SetCharacterReference(ABeamEyeTrackerExampleCharacter* Character);

	UFUNCTION(BlueprintCallable, Category = "Widget")
	void UpdateWidgetDisplay();

protected:
	
	void UpdateStatusDisplay();

	void UpdateGazeDisplay();

	void UpdateHeadPoseDisplay();

	void UpdateConfidenceDisplay();

	void UpdateFrameInfo();

	void UpdateHealthStatus();

	FLinearColor GetStatusColor(EBeamHealth Health) const;

	FText GetStatusText(EBeamHealth Health) const;
};

/*=============================================================================
    End of BeamEyeTrackerExampleWidget.h
=============================================================================*/

