/*=============================================================================
    BeamK2Nodes.h: Custom K2 nodes for Beam Eye Tracker.

    Provides custom Blueprint nodes for subsystem access, tracking validation,
    health branching, and other Beam-specific functionality to enhance
    Blueprint development experience.

    Copyright (c) 2025 Eyeware Tech SA. All Rights Reserved.

    Eyeware® and Beam® are registered trademarks of Eyeware Tech SA.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "K2Node.h"
#include "BeamK2Nodes.generated.h"

class UEdGraphPin;

// ============================================================================
// CORE SUBSYSTEM ACCESS
// ============================================================================

/** One-click access to the Beam Eye Tracker subsystem */
UCLASS()
class BEAMEYETRACKEREDITOR_API UK2Node_BeamGetSubsystem : public UK2Node
{
	GENERATED_BODY()

public:
	UK2Node_BeamGetSubsystem();

	// UK2Node interface
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual FText GetMenuCategory() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	virtual void AllocateDefaultPins() override;
	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;

	// Pin accessors
	UEdGraphPin* GetSubsystemPin() const;

private:
	/** Subsystem output pin */
	UEdGraphPin* SubsystemPin;
};

// ============================================================================
// TRACKING STATUS & VALIDATION
// ============================================================================

/** Branch based on tracking availability and data freshness */
UCLASS()
class BEAMEYETRACKEREDITOR_API UK2Node_BeamIfTracking : public UK2Node
{
	GENERATED_BODY()

public:
	UK2Node_BeamIfTracking();

	// UK2Node interface
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual FText GetMenuCategory() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	virtual void AllocateDefaultPins() override;
	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;

	// Pin accessors
	UEdGraphPin* GetExecPin() const;
	UEdGraphPin* GetThenPin() const;
	UEdGraphPin* GetElsePin() const;
	UEdGraphPin* GetFreshnessSecPin() const;

private:
	/** Exec input pin */
	UEdGraphPin* ExecPin;
	/** Exec output pin for tracking active */
	UEdGraphPin* ThenPin;
	/** Exec output pin for tracking inactive */
	UEdGraphPin* ElsePin;
	/** Freshness threshold input pin */
	UEdGraphPin* FreshnessSecPin;
};

/** Branch execution based on Beam health status */
UCLASS()
class BEAMEYETRACKEREDITOR_API UK2Node_BeamBranchOnHealth : public UK2Node
{
	GENERATED_BODY()

public:
	UK2Node_BeamBranchOnHealth();

	// UK2Node interface
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual FText GetMenuCategory() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	virtual void AllocateDefaultPins() override;
	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;

	// Pin accessors
	UEdGraphPin* GetExecPin() const;
	UEdGraphPin* GetOkPin() const;
	UEdGraphPin* GetRecoveringPin() const;
	UEdGraphPin* GetErrorPin() const;

private:
	/** Exec input pin */
	UEdGraphPin* ExecPin;
	/** Exec output pin for OK health */
	UEdGraphPin* OkPin;
	/** Exec output pin for recovering health */
	UEdGraphPin* RecoveringPin;
	/** Exec output pin for error health */
	UEdGraphPin* ErrorPin;
};

// ============================================================================
// CALIBRATION & RECORDING
// ============================================================================

/**
 * Guided calibration flow with progress and cancel support.
 * 
 * Expands to calls latent StartCalibration(ProfileId); binds progress multicast;
 * routes success/failure. Disallows use in ConstructionScript.
 */
UCLASS()
class BEAMEYETRACKEREDITOR_API UK2Node_BeamCalibrate : public UK2Node
{
	GENERATED_BODY()

public:
	UK2Node_BeamCalibrate();

	// UK2Node interface
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual FText GetMenuCategory() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	virtual void AllocateDefaultPins() override;
	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;

	// Pin accessors
	UEdGraphPin* GetExecPin() const;
	UEdGraphPin* GetThenPin() const;
	UEdGraphPin* GetOnFailedPin() const;
	UEdGraphPin* GetOnProgressPin() const;
	UEdGraphPin* GetProfileIdPin() const;

private:
	/** Exec input pin */
	UEdGraphPin* ExecPin;
	/** Exec output pin for success */
	UEdGraphPin* ThenPin;
	/** Exec output pin for failure */
	UEdGraphPin* OnFailedPin;
	/** Progress output pin (0.0-1.0) */
	UEdGraphPin* OnProgressPin;
	/** Profile ID input pin */
	UEdGraphPin* ProfileIdPin;
};

/**
 * Simple session recording control with format options.
 * 
 * Expands to calls StartRecording(...); sets up end hook on map change optionally.
 * Validates writable path and warns if Shipping build.
 */
UCLASS()
class BEAMEYETRACKEREDITOR_API UK2Node_BeamStartRecording : public UK2Node
{
	GENERATED_BODY()

public:
	UK2Node_BeamStartRecording();

	// UK2Node interface
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual FText GetMenuCategory() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	virtual void AllocateDefaultPins() override;
	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;

	// Pin accessors
	UEdGraphPin* GetExecPin() const;
	UEdGraphPin* GetThenPin() const;
	UEdGraphPin* GetPathPin() const;
	UEdGraphPin* GetFormatPin() const;
	UEdGraphPin* GetIncludeHeadPosePin() const;

private:
	/** Exec input pin */
	UEdGraphPin* ExecPin;
	/** Exec output pin */
	UEdGraphPin* ThenPin;
	/** File path input pin */
	UEdGraphPin* PathPin;
	/** Recording format input pin */
	UEdGraphPin* FormatPin;
	/** Include head pose input pin */
	UEdGraphPin* IncludeHeadPosePin;
};

/**
 * Stop recording and save the file.
 * 
 * Expands to calls StopRecording() and optionally triggers cleanup.
 */
UCLASS()
class BEAMEYETRACKEREDITOR_API UK2Node_BeamStopRecording : public UK2Node
{
	GENERATED_BODY()

public:
	UK2Node_BeamStopRecording();

	// UK2Node interface
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual FText GetMenuCategory() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	virtual void AllocateDefaultPins() override;
	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;

	// Pin accessors
	UEdGraphPin* GetExecPin() const;
	UEdGraphPin* GetThenPin() const;

private:
	/** Exec input pin */
	UEdGraphPin* ExecPin;
	/** Exec output pin */
	UEdGraphPin* ThenPin;
};

// ============================================================================
// DATA ACCESS & PROCESSING
// ============================================================================

/**
 * Dump recent N samples to an array for analytics.
 * 
 * Expands to calls DequeueRecentSamples(Count, OutSamples).
 * Range-checks Count and emits note on truncation.
 */
UCLASS()
class BEAMEYETRACKEREDITOR_API UK2Node_BeamSampleBufferToArray : public UK2Node
{
	GENERATED_BODY()

public:
	UK2Node_BeamSampleBufferToArray();

	// UK2Node interface
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual FText GetMenuCategory() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	virtual void AllocateDefaultPins() override;
	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;

	// Pin accessors
	UEdGraphPin* GetCountPin() const;
	UEdGraphPin* GetOutSamplesPin() const;

private:
	/** Sample count input pin */
	UEdGraphPin* CountPin;
	/** Output samples array pin */
	UEdGraphPin* OutSamplesPin;
};

/**
 * Build a world-space gaze ray from gaze and head pose.
 * 
 * Expands to calls math helper; normalizes direction.
 * Ensures inputs are valid/fresh; errors on zero-length direction.
 */
UCLASS()
class BEAMEYETRACKEREDITOR_API UK2Node_BeamMakeGazeRay : public UK2Node
{
	GENERATED_BODY()

public:
	UK2Node_BeamMakeGazeRay();

	// UK2Node interface
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual FText GetMenuCategory() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	virtual void AllocateDefaultPins() override;
	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;

	// Pin accessors
	UEdGraphPin* GetGazePin() const;
	UEdGraphPin* GetHeadPosePin() const;
	UEdGraphPin* GetCameraComponentPin() const;
	UEdGraphPin* GetRayOriginPin() const;
	UEdGraphPin* GetRayDirectionPin() const;

private:
	/** Gaze input pin */
	UEdGraphPin* GazePin;
	/** Head pose input pin */
	UEdGraphPin* HeadPosePin;
	/** Camera component input pin */
	UEdGraphPin* CameraComponentPin;
	/** Ray origin output pin */
	UEdGraphPin* RayOriginPin;
	/** Ray direction output pin */
	UEdGraphPin* RayDirectionPin;
};

/**
 * Do a line trace using the computed gaze ray.
 * 
 * Expands to MakeGazeRay → LineTraceSingle → Branch.
 * Clamps MaxDistance and validates trace channel.
 */
UCLASS()
class BEAMEYETRACKEREDITOR_API UK2Node_BeamGazeTraceWorld : public UK2Node
{
	GENERATED_BODY()

public:
	UK2Node_BeamGazeTraceWorld();

	// UK2Node interface
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual FText GetMenuCategory() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	virtual void AllocateDefaultPins() override;
	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;

	// Pin accessors
	UEdGraphPin* GetExecPin() const;
	UEdGraphPin* GetThenPin() const;
	UEdGraphPin* GetHitPin() const;
	UEdGraphPin* GetMaxDistancePin() const;
	UEdGraphPin* GetChannelPin() const;
	UEdGraphPin* GetHitResultPin() const;

private:
	/** Exec input pin */
	UEdGraphPin* ExecPin;
	/** Exec output pin */
	UEdGraphPin* ThenPin;
	/** Hit boolean output pin */
	UEdGraphPin* HitPin;
	/** Max distance input pin */
	UEdGraphPin* MaxDistancePin;
	/** Trace channel input pin */
	UEdGraphPin* ChannelPin;
	/** Hit result output pin */
	UEdGraphPin* HitResultPin;
};

// ============================================================================
// PROJECTION & TRANSFORMATION
// ============================================================================

/**
 * Project gaze to world ray with validation.
 * 
 * Provides a clean interface for gaze-to-world projection.
 * Expands to call ProjectGazeToWorld() → output ray origin/direction.
 */
UCLASS()
class BEAMEYETRACKEREDITOR_API UK2Node_BeamProjectGazeToWorld : public UK2Node
{
	GENERATED_BODY()

public:
	UK2Node_BeamProjectGazeToWorld();

	// UK2Node interface
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual FText GetMenuCategory() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	virtual void AllocateDefaultPins() override;
	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;

	// Pin accessors
	UEdGraphPin* GetExecPin() const;
	UEdGraphPin* GetThenPin() const;
	UEdGraphPin* GetPlayerControllerPin() const;
	UEdGraphPin* GetOriginPin() const;
	UEdGraphPin* GetDirectionPin() const;
	UEdGraphPin* GetIsValidPin() const;

private:
	/** Exec input pin */
	UEdGraphPin* ExecPin;
	/** Exec output pin */
	UEdGraphPin* ThenPin;
	/** Player controller input pin */
	UEdGraphPin* PlayerControllerPin;
	/** Ray origin output pin */
	UEdGraphPin* OriginPin;
	/** Ray direction output pin */
	UEdGraphPin* DirectionPin;
	/** Validation output pin */
	UEdGraphPin* IsValidPin;
};

/**
 * Perform line trace from projected gaze ray.
 * 
 * Provides a clean interface for gaze-based raycasting.
 * Expands to ProjectGazeToWorld() → LineTraceSingle().
 */
UCLASS()
class BEAMEYETRACKEREDITOR_API UK2Node_BeamTraceFromGaze : public UK2Node
{
	GENERATED_BODY()

public:
	UK2Node_BeamTraceFromGaze();

	// UK2Node interface
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual FText GetMenuCategory() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	virtual void AllocateDefaultPins() override;
	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;

	// Pin accessors
	UEdGraphPin* GetExecPin() const;
	UEdGraphPin* GetThenPin() const;
	UEdGraphPin* GetPlayerControllerPin() const;
	UEdGraphPin* GetMaxDistancePin() const;
	UEdGraphPin* GetTraceChannelPin() const;
	UEdGraphPin* GetHitPin() const;
	UEdGraphPin* GetHitResultPin() const;

private:
	/** Exec input pin */
	UEdGraphPin* ExecPin;
	/** Exec output pin */
	UEdGraphPin* ThenPin;
	/** Player controller input pin */
	UEdGraphPin* PlayerControllerPin;
	/** Maximum trace distance input pin */
	UEdGraphPin* MaxDistancePin;
	/** Trace channel input pin */
	UEdGraphPin* TraceChannelPin;
	/** Hit result boolean output pin */
	UEdGraphPin* HitPin;
	/** Hit result struct output pin */
	UEdGraphPin* HitResultPin;
};

/**
 * Map world gaze to screen space (for UMG overlays).
 * 
 * Expands to ProjectWorldToScreen using PlayerController from context.
 * Requires local player; errors if not found.
 */
UCLASS()
class BEAMEYETRACKEREDITOR_API UK2Node_BeamProjectGazeToScreen : public UK2Node
{
	GENERATED_BODY()

public:
	UK2Node_BeamProjectGazeToScreen();

	// UK2Node interface
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual FText GetMenuCategory() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	virtual void AllocateDefaultPins() override;
	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;

	// Pin accessors
	UEdGraphPin* GetWorldLocationPin() const;
	UEdGraphPin* GetWorldContextPin() const;
	UEdGraphPin* GetOutScreenPosPin() const;
	UEdGraphPin* GetBOnScreenPin() const;

private:
	/** World location input pin */
	UEdGraphPin* WorldLocationPin;
	/** World context input pin */
	UEdGraphPin* WorldContextPin;
	/** Screen position output pin */
	UEdGraphPin* OutScreenPosPin;
	/** On screen boolean output pin */
	UEdGraphPin* BOnScreenPin;
};

/**
 * Convert gaze in camera/viewport space into world-space vector using a provided camera.
 * 
 * Expands to deproject screen to world; normalizes.
 * Camera component is required.
 */
UCLASS()
class BEAMEYETRACKEREDITOR_API UK2Node_BeamTransformGazeToWorld : public UK2Node
{
	GENERATED_BODY()

public:
	UK2Node_BeamTransformGazeToWorld();

	// UK2Node interface
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual FText GetMenuCategory() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	virtual void AllocateDefaultPins() override;
	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;

	// Pin accessors
	UEdGraphPin* GetExecPin() const;
	UEdGraphPin* GetThenPin() const;
	UEdGraphPin* GetCameraComponentPin() const;
	UEdGraphPin* GetGazeViewportPin() const;
	UEdGraphPin* GetOutWorldDirPin() const;

private:
	/** Exec input pin */
	UEdGraphPin* ExecPin;
	/** Exec output pin */
	UEdGraphPin* ThenPin;
	/** Camera component input pin */
	UEdGraphPin* CameraComponentPin;
	/** Gaze viewport input pin */
	UEdGraphPin* GazeViewportPin;
	/** World direction output pin */
	UEdGraphPin* OutWorldDirPin;
};

// ============================================================================
// ADVANCED FEATURES
// ============================================================================

/**
 * Predict near-future gaze using filter settings.
 * 
 * Expands to calls filter/predictor helper (One-Euro, EMA).
 * Bounds-checks Horizon and warns if filters disabled.
 */
UCLASS()
class BEAMEYETRACKEREDITOR_API UK2Node_BeamPredictGaze : public UK2Node
{
	GENERATED_BODY()

public:
	UK2Node_BeamPredictGaze();

	// UK2Node interface
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual FText GetMenuCategory() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	virtual void AllocateDefaultPins() override;
	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;

	// Pin accessors
	UEdGraphPin* GetExecPin() const;
	UEdGraphPin* GetThenPin() const;
	UEdGraphPin* GetInSamplePin() const;
	UEdGraphPin* GetHorizonMsPin() const;
	UEdGraphPin* GetOutSamplePin() const;

private:
	/** Exec input pin */
	UEdGraphPin* ExecPin;
	/** Exec output pin */
	UEdGraphPin* ThenPin;
	/** Input sample pin */
	UEdGraphPin* InSamplePin;
	/** Prediction horizon input pin */
	UEdGraphPin* HorizonMsPin;
	/** Output sample pin */
	UEdGraphPin* OutSamplePin;
};

/**
 * Toggle overlay visuals from Blueprints.
 * 
 * Expands to subsystem setter; triggers redraw.
 * No-op on Shipping builds.
 */
UCLASS()
class BEAMEYETRACKEREDITOR_API UK2Node_BeamSetDebugOverlay : public UK2Node
{
	GENERATED_BODY()

public:
	UK2Node_BeamSetDebugOverlay();

	// UK2Node interface
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual FText GetMenuCategory() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	virtual void AllocateDefaultPins() override;
	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;

	// Pin accessors
	UEdGraphPin* GetExecPin() const;
	UEdGraphPin* GetThenPin() const;
	UEdGraphPin* GetFlagsPin() const;
	UEdGraphPin* GetBEnabledPin() const;

private:
	/** Exec input pin */
	UEdGraphPin* ExecPin;
	/** Exec output pin */
	UEdGraphPin* ThenPin;
	/** Overlay flags input pin */
	UEdGraphPin* FlagsPin;
	/** Enable/disable input pin */
	UEdGraphPin* BEnabledPin;
};

/**
 * Query SDK watchdog/health status.
 * 
 * Expands to calls subsystem GetHealthStatus(...).
 * Warns if on stub platform.
 */
UCLASS()
class BEAMEYETRACKEREDITOR_API UK2Node_BeamWatchdogStatus : public UK2Node
{
	GENERATED_BODY()

public:
	UK2Node_BeamWatchdogStatus();

	// UK2Node interface
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual FText GetMenuCategory() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	virtual void AllocateDefaultPins() override;
	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;

	// Pin accessors
	UEdGraphPin* GetExecPin() const;
	UEdGraphPin* GetThenPin() const;
	UEdGraphPin* GetOutStatusPin() const;
	UEdGraphPin* GetOutRetryDelayPin() const;

private:
	/** Exec input pin */
	UEdGraphPin* ExecPin;
	/** Exec output pin */
	UEdGraphPin* ThenPin;
	/** Status output pin */
	UEdGraphPin* OutStatusPin;
	/** Retry delay output pin */
	UEdGraphPin* OutRetryDelayPin;
};

/**
 * Ensure DLL load + symbol check occurred; emit compile-time error if missing.
 * 
 * Expands to calls wrapper EnsureLoaded(); emits Kismet compiler error on hard failure.
 * Enforces Windows-only usage.
 */
UCLASS()
class BEAMEYETRACKEREDITOR_API UK2Node_BeamEnsureSDKLoaded : public UK2Node
{
	GENERATED_BODY()

public:
	UK2Node_BeamEnsureSDKLoaded();

	// UK2Node interface
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual FText GetMenuCategory() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	virtual void AllocateDefaultPins() override;
	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;

	// Pin accessors
	UEdGraphPin* GetExecPin() const;
	UEdGraphPin* GetThenPin() const;
	UEdGraphPin* GetBAvailablePin() const;

private:
	/** Exec input pin */
	UEdGraphPin* ExecPin;
	/** Exec output pin */
	UEdGraphPin* ThenPin;
	/** Available boolean output pin */
	UEdGraphPin* BAvailablePin;
};

/**
 * Retrieve latest head pose with freshness validation.
 * 
 * Expands to calls GetLatestHeadPose(...).
 * Same freshness validation as gaze data.
 */
UCLASS()
class BEAMEYETRACKEREDITOR_API UK2Node_BeamGetHeadPose : public UK2Node
{
	GENERATED_BODY()

public:
	UK2Node_BeamGetHeadPose();

	// UK2Node interface
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual FText GetMenuCategory() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	virtual void AllocateDefaultPins() override;
	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;

	// Pin accessors
	UEdGraphPin* GetExecPin() const;
	UEdGraphPin* GetThenPin() const;
	UEdGraphPin* GetOutPosePin() const;
	UEdGraphPin* GetBFreshPin() const;
	UEdGraphPin* GetFreshnessSecPin() const;

private:
	/** Exec input pin */
	UEdGraphPin* ExecPin;
	/** Exec output pin */
	UEdGraphPin* ThenPin;
	/** Head pose output pin */
	UEdGraphPin* OutPosePin;
	/** Fresh boolean output pin */
	UEdGraphPin* BFreshPin;
	/** Freshness threshold input pin */
	UEdGraphPin* FreshnessSecPin;
};

// ============================================================================
// UI & INTERACTION
// ============================================================================

/**
 * Get the UMG widget currently under gaze (for gaze UI interactions).
 * 
 * Expands to project gaze to screen → Slate hit test → return top widget.
 * Requires local player + viewport; Editor-only warning for non-PIE graphs.
 */
UCLASS()
class BEAMEYETRACKEREDITOR_API UK2Node_BeamFocusWidget : public UK2Node
{
	GENERATED_BODY()

public:
	UK2Node_BeamFocusWidget();

	// UK2Node interface
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual FText GetMenuCategory() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	virtual void AllocateDefaultPins() override;
	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;

	// Pin accessors
	UEdGraphPin* GetExecPin() const;
	UEdGraphPin* GetThenPin() const;
	UEdGraphPin* GetOutWidgetPin() const;
	UEdGraphPin* GetOutLocalPosPin() const;

private:
	/** Exec input pin */
	UEdGraphPin* ExecPin;
	/** Exec output pin */
	UEdGraphPin* ThenPin;
	/** Output widget pin */
	UEdGraphPin* OutWidgetPin;
	/** Local position output pin */
	UEdGraphPin* OutLocalPosPin;
};

/** High-level UX node that fires after a dwell time on a target */
UCLASS()
class BEAMEYETRACKEREDITOR_API UK2Node_BeamDwellDetector : public UK2Node
{
	GENERATED_BODY()

public:
	UK2Node_BeamDwellDetector();

	// UK2Node interface
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual FText GetMenuCategory() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetTooltipText() const override;
	virtual void AllocateDefaultPins() override;
	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;

	// Pin accessors
	UEdGraphPin* GetExecPin() const;
	UEdGraphPin* GetThenPin() const;
	UEdGraphPin* GetOnDwellPin() const;
	UEdGraphPin* GetTargetPin() const;
	UEdGraphPin* GetDwellTimePin() const;

private:
	/** Exec input pin */
	UEdGraphPin* ExecPin;
	/** Exec output pin */
	UEdGraphPin* ThenPin;
	/** Exec output pin for dwell event */
	UEdGraphPin* OnDwellPin;
	/** Target actor/widget input pin */
	UEdGraphPin* TargetPin;
	/** Dwell time input pin */
	UEdGraphPin* DwellTimePin;
};

// ============================================================================
// NODE REGISTRATION
// ============================================================================

/** Utility class for registering all Beam K2 nodes */
UCLASS()
class BEAMEYETRACKEREDITOR_API UBeamK2Nodes : public UObject
{
	GENERATED_BODY()

public:
	/** Register all Beam K2 node classes */
	static void RegisterNodes();
	
	/** Unregister all Beam K2 node classes */
	static void UnregisterNodes();
};

/*=============================================================================
    End of BeamK2Nodes.h
=============================================================================*/
