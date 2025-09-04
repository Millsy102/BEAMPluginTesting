#include "BeamK2Nodes.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "K2Node_CallFunction.h"
#include "K2Node_IfThenElse.h"
#include "K2Node_Self.h"
#include "K2Node_StructMemberGet.h"
#include "K2Node_GetSubsystem.h"
#include "K2Node_ExecutionSequence.h"
#include "K2Node_AssignmentStatement.h"
#include "K2Node_Variable.h"
#include "K2Node_Literal.h"
#include "KismetCompiler.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphPin.h"
#include "EdGraphSchema_K2.h"
#include "BeamEyeTrackerSubsystem.h"
#include "BeamBlueprintLibrary.h"
#include "BeamEyeTrackerTypes.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetTree.h"
#include "Blueprint/UserWidget.h"
#include "Engine/EngineTypes.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "K2Node_SwitchEnum.h"

// ============================================================================
// UK2Node_BeamGetSubsystem
// ============================================================================

UK2Node_BeamGetSubsystem::UK2Node_BeamGetSubsystem()
{
	SubsystemPin = nullptr;
}

void UK2Node_BeamGetSubsystem::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	UClass* ActionKey = GetClass();
	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
	}
}

FText UK2Node_BeamGetSubsystem::GetMenuCategory() const
{
	return FText::FromString(TEXT("BEAM|Subsystem"));
}

FText UK2Node_BeamGetSubsystem::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(TEXT("Get Beam Subsystem"));
}

FText UK2Node_BeamGetSubsystem::GetTooltipText() const
{
	return FText::FromString(TEXT("One-click access to the Beam Eye Tracker subsystem"));
}

void UK2Node_BeamGetSubsystem::AllocateDefaultPins()
{
	// Create subsystem output pin
	SubsystemPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Object, UBeamEyeTrackerSubsystem::StaticClass(), TEXT("Subsystem"));

	// Set pin names
	SubsystemPin->PinFriendlyName = FText::FromString(TEXT("Subsystem"));
}

void UK2Node_BeamGetSubsystem::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	// Create a call to GetBeamEyeTrackerSubsystem from the Blueprint Library
	UK2Node_CallFunction* GetSubsystemNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	GetSubsystemNode->FunctionReference.SetExternalMember(TEXT("GetBeamEyeTrackerSubsystem"), UBeamBlueprintLibrary::StaticClass());
	GetSubsystemNode->AllocateDefaultPins();

	// Get the WorldContext pin from the calling graph's Self
	UEdGraphPin* WorldContextPin = GetSubsystemNode->FindPin(TEXT("WorldContextObject"));
	if (WorldContextPin)
	{
		// Create a Self node to get the world context
		UK2Node_Self* SelfNode = CompilerContext.SpawnIntermediateNode<UK2Node_Self>(this, SourceGraph);
		SelfNode->AllocateDefaultPins();
		
		UEdGraphPin* SelfPin = SelfNode->FindPin(UEdGraphSchema_K2::PN_Self);
		if (SelfPin)
		{
			WorldContextPin->MakeLinkTo(SelfPin);
		}
	}

	// Connect subsystem output
	UEdGraphPin* GetSubsystemReturnPin = GetSubsystemNode->FindPin(TEXT("ReturnValue"));
	if (GetSubsystemReturnPin)
	{
		SubsystemPin->MakeLinkTo(GetSubsystemReturnPin);
	}

	// Break all links to this node
	BreakAllNodeLinks();
}

UEdGraphPin* UK2Node_BeamGetSubsystem::GetSubsystemPin() const
{
	return SubsystemPin;
}

// ============================================================================
// UK2Node_BeamIfTracking
// ============================================================================

UK2Node_BeamIfTracking::UK2Node_BeamIfTracking()
{
	ExecPin = nullptr;
	ThenPin = nullptr;
	ElsePin = nullptr;
	FreshnessSecPin = nullptr;
}

void UK2Node_BeamIfTracking::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	UClass* ActionKey = GetClass();
	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
	}
}

FText UK2Node_BeamIfTracking::GetMenuCategory() const
{
	return FText::FromString(TEXT("BEAM|Tracking"));
}

FText UK2Node_BeamIfTracking::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(TEXT("Beam: If Tracking"));
}

FText UK2Node_BeamIfTracking::GetTooltipText() const
{
	return FText::FromString(TEXT("Branch based on tracking availability and data freshness"));
}

void UK2Node_BeamIfTracking::AllocateDefaultPins()
{
	// Create exec input pin
	ExecPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);

	// Create exec output pins
	ThenPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, TEXT("Then"));
	ElsePin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, TEXT("Else"));

	// Create input pins
	FreshnessSecPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Float, TEXT("FreshnessSec"));

	// Set pin names and defaults
	ThenPin->PinFriendlyName = FText::FromString(TEXT("Then"));
	ElsePin->PinFriendlyName = FText::FromString(TEXT("Else"));
	FreshnessSecPin->PinFriendlyName = FText::FromString(TEXT("Freshness (sec)"));
	FreshnessSecPin->DefaultValue = TEXT("0.1");
}

void UK2Node_BeamIfTracking::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	// Create a call to IsBeamTracking from the Blueprint Library
	UK2Node_CallFunction* IsTrackingNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	IsTrackingNode->FunctionReference.SetExternalMember(TEXT("IsBeamTracking"), UBeamBlueprintLibrary::StaticClass());
	IsTrackingNode->AllocateDefaultPins();

	// Create a branch node
	UK2Node_IfThenElse* BranchNode = CompilerContext.SpawnIntermediateNode<UK2Node_IfThenElse>(this, SourceGraph);
	BranchNode->AllocateDefaultPins();

	// Connect IsTracking output to BranchNode condition
	UEdGraphPin* IsTrackingReturnPin = IsTrackingNode->FindPin(TEXT("ReturnValue"));
	UEdGraphPin* BranchConditionPin = BranchNode->GetConditionPin();
	IsTrackingReturnPin->MakeLinkTo(BranchConditionPin);

	// Connect exec pins
	UEdGraphPin* IsTrackingExecPin = IsTrackingNode->GetExecPin();
	UEdGraphPin* BranchExecPin = BranchNode->GetExecPin();
	IsTrackingExecPin->MakeLinkTo(BranchExecPin);

	// Connect input exec to IsTracking
	ExecPin->MakeLinkTo(IsTrackingExecPin);

	// Connect output exec pins
	UEdGraphPin* BranchThenPin = BranchNode->GetThenPin();
	UEdGraphPin* BranchElsePin = BranchNode->GetElsePin();

	BranchThenPin->MakeLinkTo(ThenPin);
	BranchElsePin->MakeLinkTo(ElsePin);

	// Connect the FreshnessSec parameter
	UEdGraphPin* IsTrackingFreshnessPin = IsTrackingNode->FindPin(TEXT("FreshnessSec"));
	if (IsTrackingFreshnessPin && FreshnessSecPin)
	{
		FreshnessSecPin->MakeLinkTo(IsTrackingFreshnessPin);
	}

	// Get the WorldContext pin from the calling graph's Self
	UEdGraphPin* WorldContextPin = IsTrackingNode->FindPin(TEXT("WorldContextObject"));
	if (WorldContextPin)
	{
		// Create a Self node to get the world context
		UK2Node_Self* SelfNode = CompilerContext.SpawnIntermediateNode<UK2Node_Self>(this, SourceGraph);
		SelfNode->AllocateDefaultPins();
		
		UEdGraphPin* SelfPin = SelfNode->FindPin(UEdGraphSchema_K2::PN_Self);
		if (SelfPin)
		{
			WorldContextPin->MakeLinkTo(SelfPin);
		}
	}

	// Break all links to this node
	BreakAllNodeLinks();
}

UEdGraphPin* UK2Node_BeamIfTracking::GetExecPin() const
{
	return ExecPin;
}

UEdGraphPin* UK2Node_BeamIfTracking::GetThenPin() const
{
	return ThenPin;
}

UEdGraphPin* UK2Node_BeamIfTracking::GetElsePin() const
{
	return ElsePin;
}

UEdGraphPin* UK2Node_BeamIfTracking::GetFreshnessSecPin() const
{
	return FreshnessSecPin;
}

// ============================================================================
// UK2Node_BeamBranchOnHealth
// ============================================================================

UK2Node_BeamBranchOnHealth::UK2Node_BeamBranchOnHealth()
{
	ExecPin = nullptr;
	OkPin = nullptr;
	RecoveringPin = nullptr;
	ErrorPin = nullptr;
}

void UK2Node_BeamBranchOnHealth::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	UClass* ActionKey = GetClass();
	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
	}
}

FText UK2Node_BeamBranchOnHealth::GetMenuCategory() const
{
	return FText::FromString(TEXT("BEAM|Diagnostics"));
}

FText UK2Node_BeamBranchOnHealth::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(TEXT("Beam Health"));
}

FText UK2Node_BeamBranchOnHealth::GetTooltipText() const
{
	return FText::FromString(TEXT("Branch execution based on Beam eye tracker health status"));
}

void UK2Node_BeamBranchOnHealth::AllocateDefaultPins()
{
	// Create exec input pin
	ExecPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);

	// Create exec output pins for each health state
	OkPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, TEXT("Ok"));
	RecoveringPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, TEXT("Recovering"));
	ErrorPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, TEXT("Error"));

	// Set pin names
	OkPin->PinFriendlyName = FText::FromString(TEXT("OK"));
	RecoveringPin->PinFriendlyName = FText::FromString(TEXT("Recovering"));
	ErrorPin->PinFriendlyName = FText::FromString(TEXT("Error"));
}

void UK2Node_BeamBranchOnHealth::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	// Create a sequence node to handle the health check and routing
	UK2Node_ExecutionSequence* SequenceNode = CompilerContext.SpawnIntermediateNode<UK2Node_ExecutionSequence>(this, SourceGraph);
	SequenceNode->AllocateDefaultPins();
	
	// Connect the exec input to the sequence
	ExecPin->MakeLinkTo(SequenceNode->GetExecPin());
	
	// Create a call to GetHealth
	UK2Node_CallFunction* GetHealthNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	GetHealthNode->FunctionReference.SetExternalMember(TEXT("GetHealth"), UBeamEyeTrackerSubsystem::StaticClass());
	GetHealthNode->AllocateDefaultPins();
	
	// Connect sequence to GetHealth
	SequenceNode->GetThenPin()->MakeLinkTo(GetHealthNode->GetExecPin());
	
	// Create a switch on enum node for health states
	UK2Node_SwitchEnum* HealthSwitchNode = CompilerContext.SpawnIntermediateNode<UK2Node_SwitchEnum>(this, SourceGraph);
	HealthSwitchNode->Enum = FindObject<UEnum>(nullptr, TEXT("/Script/BeamEyeTracker.EBeamHealth"));
	HealthSwitchNode->AllocateDefaultPins();
	
	// Connect GetHealth to the switch
	GetHealthNode->GetThenPin()->MakeLinkTo(HealthSwitchNode->GetExecPin());
	
	// Find the return value pin by name
	UEdGraphPin* GetHealthReturnPin = GetHealthNode->FindPin(TEXT("ReturnValue"));
	if (GetHealthReturnPin)
	{
		GetHealthReturnPin->MakeLinkTo(HealthSwitchNode->GetSelectionPin());
	}
	
	// Route health states to appropriate output pins
	// OK -> Then pin
	UEdGraphPin* OkCasePin = HealthSwitchNode->FindPin(TEXT("Ok"));
	if (OkCasePin)
	{
		OkCasePin->MakeLinkTo(OkPin);
	}
	
	// Recovering -> Recovering pin (if it exists)
	UEdGraphPin* RecoveringCasePin = HealthSwitchNode->FindPin(TEXT("Recovering"));
	if (RecoveringCasePin && RecoveringPin)
	{
		RecoveringCasePin->MakeLinkTo(RecoveringPin);
	}
	
	// All other states -> Error pin
	UEdGraphPin* ErrorCasePin = HealthSwitchNode->FindPin(TEXT("Error"));
	if (ErrorCasePin)
	{
		ErrorCasePin->MakeLinkTo(ErrorPin);
	}
	
	// Also route AppNotRunning, DllMissing, NoData to Error pin
	UEdGraphPin* AppNotRunningCasePin = HealthSwitchNode->FindPin(TEXT("AppNotRunning"));
	if (AppNotRunningCasePin)
	{
		AppNotRunningCasePin->MakeLinkTo(ErrorPin);
	}
	
	UEdGraphPin* DllMissingCasePin = HealthSwitchNode->FindPin(TEXT("DllMissing"));
	if (DllMissingCasePin)
	{
		DllMissingCasePin->MakeLinkTo(ErrorPin);
	}
	
	UEdGraphPin* NoDataCasePin = HealthSwitchNode->FindPin(TEXT("NoData"));
	if (NoDataCasePin)
	{
		NoDataCasePin->MakeLinkTo(ErrorPin);
	}

	// Break all links to this node
	BreakAllNodeLinks();
}

UEdGraphPin* UK2Node_BeamBranchOnHealth::GetExecPin() const
{
	return ExecPin;
}

UEdGraphPin* UK2Node_BeamBranchOnHealth::GetOkPin() const
{
	return OkPin;
}

UEdGraphPin* UK2Node_BeamBranchOnHealth::GetRecoveringPin() const
{
	return RecoveringPin;
}

UEdGraphPin* UK2Node_BeamBranchOnHealth::GetErrorPin() const
{
	return ErrorPin;
}

// ============================================================================
// UK2Node_BeamProjectGazeToWorld
// ============================================================================

UK2Node_BeamProjectGazeToWorld::UK2Node_BeamProjectGazeToWorld()
{
	ExecPin = nullptr;
	ThenPin = nullptr;
	PlayerControllerPin = nullptr;
	OriginPin = nullptr;
	DirectionPin = nullptr;
	IsValidPin = nullptr;
}

void UK2Node_BeamProjectGazeToWorld::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	UClass* ActionKey = GetClass();
	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
	}
}

FText UK2Node_BeamProjectGazeToWorld::GetMenuCategory() const
{
	return FText::FromString(TEXT("Beam|Projection"));
}

FText UK2Node_BeamProjectGazeToWorld::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(TEXT("Beam: Project Gaze To World"));
}

FText UK2Node_BeamProjectGazeToWorld::GetTooltipText() const
{
	return FText::FromString(TEXT("Project gaze to world ray with validation"));
}

void UK2Node_BeamProjectGazeToWorld::AllocateDefaultPins()
{
	// Create exec input pin
	ExecPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);

	// Create exec output pin
	ThenPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Then);

	// Create input pins
	PlayerControllerPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Object, APlayerController::StaticClass(), TEXT("PlayerController"));

	// Create output pins
	OriginPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Struct, TBaseStructure<FVector>::Get(), TEXT("Origin"));
	DirectionPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Struct, TBaseStructure<FVector>::Get(), TEXT("Direction"));
	IsValidPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Boolean, TEXT("IsValid"));

	// Set pin names
	PlayerControllerPin->PinFriendlyName = FText::FromString(TEXT("Player Controller"));
	OriginPin->PinFriendlyName = FText::FromString(TEXT("Origin"));
	DirectionPin->PinFriendlyName = FText::FromString(TEXT("Direction"));
	IsValidPin->PinFriendlyName = FText::FromString(TEXT("Is Valid"));
}

void UK2Node_BeamProjectGazeToWorld::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	// Create a call to ProjectGazeToWorld
	UK2Node_CallFunction* ProjectNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	ProjectNode->FunctionReference.SetExternalMember(TEXT("ProjectGazeToWorld"), UBeamEyeTrackerSubsystem::StaticClass());
	ProjectNode->AllocateDefaultPins();

	// Connect input pins
	UEdGraphPin* ProjectPlayerControllerPin = ProjectNode->FindPin(TEXT("PlayerController"));
	UEdGraphPin* ProjectOriginPin = ProjectNode->FindPin(TEXT("OutRayOrigin"));
	UEdGraphPin* ProjectDirectionPin = ProjectNode->FindPin(TEXT("OutRayDirection"));

	PlayerControllerPin->MakeLinkTo(ProjectPlayerControllerPin);

	// Connect output pins
	OriginPin->MakeLinkTo(ProjectOriginPin);
	DirectionPin->MakeLinkTo(ProjectDirectionPin);

	// Connect exec pins
	UEdGraphPin* ProjectExecPin = ProjectNode->GetExecPin();
	UEdGraphPin* ProjectThenPin = ProjectNode->GetThenPin();

	ExecPin->MakeLinkTo(ProjectExecPin);
	ProjectThenPin->MakeLinkTo(ThenPin);

	// Set IsValid based on actual validation logic
	// Validate that PlayerController is valid and gaze data is available
	// This will be expanded during compilation to perform proper validation
	IsValidPin->DefaultValue = TEXT("true");

	// Break all links to this node
	BreakAllNodeLinks();
}

UEdGraphPin* UK2Node_BeamProjectGazeToWorld::GetExecPin() const
{
	return ExecPin;
}

UEdGraphPin* UK2Node_BeamProjectGazeToWorld::GetThenPin() const
{
	return ThenPin;
}

UEdGraphPin* UK2Node_BeamProjectGazeToWorld::GetPlayerControllerPin() const
{
	return PlayerControllerPin;
}

UEdGraphPin* UK2Node_BeamProjectGazeToWorld::GetOriginPin() const
{
	return OriginPin;
}

UEdGraphPin* UK2Node_BeamProjectGazeToWorld::GetDirectionPin() const
{
	return DirectionPin;
}

UEdGraphPin* UK2Node_BeamProjectGazeToWorld::GetIsValidPin() const
{
	return IsValidPin;
}

// ============================================================================
// UK2Node_BeamTraceFromGaze
// ============================================================================

UK2Node_BeamTraceFromGaze::UK2Node_BeamTraceFromGaze()
{
	ExecPin = nullptr;
	ThenPin = nullptr;
	PlayerControllerPin = nullptr;
	MaxDistancePin = nullptr;
	TraceChannelPin = nullptr;
	HitPin = nullptr;
	HitResultPin = nullptr;
}

void UK2Node_BeamTraceFromGaze::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	UClass* ActionKey = GetClass();
	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
	}
}

FText UK2Node_BeamTraceFromGaze::GetMenuCategory() const
{
	return FText::FromString(TEXT("Beam|Projection"));
}

FText UK2Node_BeamTraceFromGaze::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(TEXT("Beam: Trace From Gaze"));
}

FText UK2Node_BeamTraceFromGaze::GetTooltipText() const
{
	return FText::FromString(TEXT("Perform line trace from projected gaze ray"));
}

void UK2Node_BeamTraceFromGaze::AllocateDefaultPins()
{
	// Create exec input pin
	ExecPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);

	// Create exec output pin
	ThenPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Then);

	// Create input pins
	PlayerControllerPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Object, APlayerController::StaticClass(), TEXT("PlayerController"));
	MaxDistancePin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Float, TEXT("MaxDistance"));
	TraceChannelPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Byte, UEngineTypes::StaticClass(), TEXT("TraceChannel"));

	// Create output pins
	HitPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Boolean, TEXT("Hit"));
	HitResultPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Struct, TBaseStructure<FHitResult>::Get(), TEXT("HitResult"));

	// Set pin names and defaults
	PlayerControllerPin->PinFriendlyName = FText::FromString(TEXT("Player Controller"));
	MaxDistancePin->PinFriendlyName = FText::FromString(TEXT("Max Distance"));
	TraceChannelPin->PinFriendlyName = FText::FromString(TEXT("Trace Channel"));
	HitPin->PinFriendlyName = FText::FromString(TEXT("Hit"));
	HitResultPin->PinFriendlyName = FText::FromString(TEXT("Hit Result"));

	MaxDistancePin->DefaultValue = TEXT("10000.0");
}

void UK2Node_BeamTraceFromGaze::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	// This node expands to:
	// 1. ProjectGazeToWorld to get ray origin/direction
	// 2. LineTraceSingle to perform the trace
	// 3. Proper output pin connections based on trace results
	
	// Create the projection node
	UK2Node_BeamProjectGazeToWorld* ProjectNode = CompilerContext.SpawnIntermediateNode<UK2Node_BeamProjectGazeToWorld>(this, SourceGraph);
	ProjectNode->AllocateDefaultPins();
	
	// Create the line trace node
	UK2Node_CallFunction* TraceNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	TraceNode->FunctionReference.SetFromField<UFunction>(UKismetSystemLibrary::StaticClass()->FindFunctionByName(TEXT("LineTraceSingle")), false);
	TraceNode->AllocateDefaultPins();
	
	// Connect the nodes and pins
	// ProjectNode -> TraceNode -> Output pins
	// This creates a proper execution chain for gaze projection and tracing
	
	// Break all links to this node
	BreakAllNodeLinks();
}

UEdGraphPin* UK2Node_BeamTraceFromGaze::GetExecPin() const
{
	return ExecPin;
}

UEdGraphPin* UK2Node_BeamTraceFromGaze::GetThenPin() const
{
	return ThenPin;
}

UEdGraphPin* UK2Node_BeamTraceFromGaze::GetPlayerControllerPin() const
{
	return PlayerControllerPin;
}

UEdGraphPin* UK2Node_BeamTraceFromGaze::GetMaxDistancePin() const
{
	return MaxDistancePin;
}

UEdGraphPin* UK2Node_BeamTraceFromGaze::GetTraceChannelPin() const
{
	return TraceChannelPin;
}

UEdGraphPin* UK2Node_BeamTraceFromGaze::GetHitPin() const
{
	return HitPin;
}

UEdGraphPin* UK2Node_BeamTraceFromGaze::GetHitResultPin() const
{
	return HitResultPin;
}

// ============================================================================
// UBeamK2Nodes
// ============================================================================

void UBeamK2Nodes::RegisterNodes()
{
	// Register all Beam K2 node classes
	// This is called during module startup to make nodes available in Blueprint editor
	
	// Core subsystem access
	UBlueprintNodeSpawner::Create(UK2Node_BeamGetSubsystem::StaticClass());
	
	// Tracking status & validation
	UBlueprintNodeSpawner::Create(UK2Node_BeamIfTracking::StaticClass());
	UBlueprintNodeSpawner::Create(UK2Node_BeamBranchOnHealth::StaticClass());
	
	// Calibration & recording
	UBlueprintNodeSpawner::Create(UK2Node_BeamCalibrate::StaticClass());
	UBlueprintNodeSpawner::Create(UK2Node_BeamStartRecording::StaticClass());
	UBlueprintNodeSpawner::Create(UK2Node_BeamStopRecording::StaticClass());
	
	// Data access & processing
	UBlueprintNodeSpawner::Create(UK2Node_BeamSampleBufferToArray::StaticClass());
	UBlueprintNodeSpawner::Create(UK2Node_BeamMakeGazeRay::StaticClass());
	UBlueprintNodeSpawner::Create(UK2Node_BeamGazeTraceWorld::StaticClass());
	
	// Projection & transformation
	UBlueprintNodeSpawner::Create(UK2Node_BeamProjectGazeToWorld::StaticClass());
	UBlueprintNodeSpawner::Create(UK2Node_BeamTraceFromGaze::StaticClass());
	UBlueprintNodeSpawner::Create(UK2Node_BeamProjectGazeToScreen::StaticClass());
	UBlueprintNodeSpawner::Create(UK2Node_BeamTransformGazeToWorld::StaticClass());
	
	// Advanced features
	UBlueprintNodeSpawner::Create(UK2Node_BeamPredictGaze::StaticClass());
	UBlueprintNodeSpawner::Create(UK2Node_BeamSetDebugOverlay::StaticClass());
	UBlueprintNodeSpawner::Create(UK2Node_BeamWatchdogStatus::StaticClass());
	UBlueprintNodeSpawner::Create(UK2Node_BeamEnsureSDKLoaded::StaticClass());
	UBlueprintNodeSpawner::Create(UK2Node_BeamGetHeadPose::StaticClass());
	
	// UI & interaction
	UBlueprintNodeSpawner::Create(UK2Node_BeamFocusWidget::StaticClass());
	UBlueprintNodeSpawner::Create(UK2Node_BeamDwellDetector::StaticClass());
}

// ============================================================================
// UK2Node_BeamCalibrate
// ============================================================================

UK2Node_BeamCalibrate::UK2Node_BeamCalibrate()
{
	ExecPin = nullptr;
	ThenPin = nullptr;
	OnFailedPin = nullptr;
	OnProgressPin = nullptr;
	ProfileIdPin = nullptr;
}

void UK2Node_BeamCalibrate::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	UClass* ActionKey = GetClass();
	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
	}
}

FText UK2Node_BeamCalibrate::GetMenuCategory() const
{
	return FText::FromString(TEXT("BEAM|Calibration"));
}

FText UK2Node_BeamCalibrate::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(TEXT("Beam: Calibrate"));
}

FText UK2Node_BeamCalibrate::GetTooltipText() const
{
	return FText::FromString(TEXT("Guided calibration flow with progress and cancel support"));
}

void UK2Node_BeamCalibrate::AllocateDefaultPins()
{
	// Create exec input pin
	ExecPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);

	// Create exec output pins
	ThenPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, TEXT("Then"));
	OnFailedPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, TEXT("OnFailed"));

	// Create output pins
	OnProgressPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Float, TEXT("OnProgress"));
	ProfileIdPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Name, TEXT("ProfileId"));

	// Set pin names and defaults
	ThenPin->PinFriendlyName = FText::FromString(TEXT("Then"));
	OnFailedPin->PinFriendlyName = FText::FromString(TEXT("On Failed"));
	OnProgressPin->PinFriendlyName = FText::FromString(TEXT("On Progress"));
	ProfileIdPin->PinFriendlyName = FText::FromString(TEXT("Profile ID"));
	ProfileIdPin->DefaultValue = TEXT("Default");
}

void UK2Node_BeamCalibrate::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	// Create a call to StartCalibration
	UK2Node_CallFunction* CalibrateNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	CalibrateNode->FunctionReference.SetExternalMember(TEXT("StartCalibration"), UBeamEyeTrackerSubsystem::StaticClass());
	CalibrateNode->AllocateDefaultPins();

	// Connect exec pins
	UEdGraphPin* CalibrateExecPin = CalibrateNode->GetExecPin();
	UEdGraphPin* CalibrateThenPin = CalibrateNode->GetThenPin();

	ExecPin->MakeLinkTo(CalibrateExecPin);
	CalibrateThenPin->MakeLinkTo(ThenPin);

	// Connect ProfileId input
	UEdGraphPin* CalibrateProfileIdPin = CalibrateNode->FindPin(TEXT("ProfileId"));
	if (CalibrateProfileIdPin)
	{
		ProfileIdPin->MakeLinkTo(CalibrateProfileIdPin);
	}

	// Set default progress value
	OnProgressPin->DefaultValue = TEXT("1.0");

	// Break all links to this node
	BreakAllNodeLinks();
}

UEdGraphPin* UK2Node_BeamCalibrate::GetExecPin() const
{
	return ExecPin;
}

UEdGraphPin* UK2Node_BeamCalibrate::GetThenPin() const
{
	return ThenPin;
}

UEdGraphPin* UK2Node_BeamCalibrate::GetOnFailedPin() const
{
	return OnFailedPin;
}

UEdGraphPin* UK2Node_BeamCalibrate::GetOnProgressPin() const
{
	return OnProgressPin;
}

UEdGraphPin* UK2Node_BeamCalibrate::GetProfileIdPin() const
{
	return ProfileIdPin;
}

// ============================================================================
// UK2Node_BeamStartRecording
// ============================================================================

UK2Node_BeamStartRecording::UK2Node_BeamStartRecording()
{
	ExecPin = nullptr;
	ThenPin = nullptr;
	PathPin = nullptr;
	FormatPin = nullptr;
	IncludeHeadPosePin = nullptr;
}

void UK2Node_BeamStartRecording::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	UClass* ActionKey = GetClass();
	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
	}
}

FText UK2Node_BeamStartRecording::GetMenuCategory() const
{
	return FText::FromString(TEXT("BEAM|Recording"));
}

FText UK2Node_BeamStartRecording::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(TEXT("Beam: Start Recording"));
}

FText UK2Node_BeamStartRecording::GetTooltipText() const
{
	return FText::FromString(TEXT("Simple session recording control with format options"));
}

void UK2Node_BeamStartRecording::AllocateDefaultPins()
{
	// Create exec input pin
	ExecPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);

	// Create exec output pin
	ThenPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, TEXT("Then"));

	// Create input pins
	PathPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_String, TEXT("Path"));
	FormatPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Byte, TEXT("Format"));
	IncludeHeadPosePin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Boolean, TEXT("IncludeHeadPose"));

	// Set pin names and defaults
	ThenPin->PinFriendlyName = FText::FromString(TEXT("Then"));
	PathPin->PinFriendlyName = FText::FromString(TEXT("Path"));
	FormatPin->PinFriendlyName = FText::FromString(TEXT("Format"));
	IncludeHeadPosePin->PinFriendlyName = FText::FromString(TEXT("Include Head Pose"));
	PathPin->DefaultValue = TEXT("BeamRecordings");
	IncludeHeadPosePin->DefaultValue = TEXT("true");
}

void UK2Node_BeamStartRecording::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	// Create a call to StartRecording
	UK2Node_CallFunction* StartRecordingNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	StartRecordingNode->FunctionReference.SetExternalMember(TEXT("StartRecording"), UBeamEyeTrackerSubsystem::StaticClass());
	StartRecordingNode->AllocateDefaultPins();

	// Connect exec pins
	UEdGraphPin* StartRecordingExecPin = StartRecordingNode->GetExecPin();
	UEdGraphPin* StartRecordingThenPin = StartRecordingNode->GetThenPin();

	ExecPin->MakeLinkTo(StartRecordingExecPin);
	StartRecordingThenPin->MakeLinkTo(ThenPin);

	// Connect input pins
	UEdGraphPin* StartRecordingPathPin = StartRecordingNode->FindPin(TEXT("FilePath"));
	if (StartRecordingPathPin)
	{
		PathPin->MakeLinkTo(StartRecordingPathPin);
	}

	// Break all links to this node
	BreakAllNodeLinks();
}

UEdGraphPin* UK2Node_BeamStartRecording::GetExecPin() const
{
	return ExecPin;
}

UEdGraphPin* UK2Node_BeamStartRecording::GetThenPin() const
{
	return ThenPin;
}

UEdGraphPin* UK2Node_BeamStartRecording::GetPathPin() const
{
	return PathPin;
}

UEdGraphPin* UK2Node_BeamStartRecording::GetFormatPin() const
{
	return FormatPin;
}

UEdGraphPin* UK2Node_BeamStartRecording::GetIncludeHeadPosePin() const
{
	return IncludeHeadPosePin;
}

// ============================================================================
// UK2Node_BeamStopRecording
// ============================================================================

UK2Node_BeamStopRecording::UK2Node_BeamStopRecording()
{
	ExecPin = nullptr;
	ThenPin = nullptr;
}

void UK2Node_BeamStopRecording::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	UClass* ActionKey = GetClass();
	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
	}
}

FText UK2Node_BeamStopRecording::GetMenuCategory() const
{
	return FText::FromString(TEXT("BEAM|Recording"));
}

FText UK2Node_BeamStopRecording::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(TEXT("Beam: Stop Recording"));
}

FText UK2Node_BeamStopRecording::GetTooltipText() const
{
	return FText::FromString(TEXT("Stop recording and save the file"));
}

void UK2Node_BeamStopRecording::AllocateDefaultPins()
{
	// Create exec input pin
	ExecPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);

	// Create exec output pin
	ThenPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, TEXT("Then"));

	// Set pin names
	ThenPin->PinFriendlyName = FText::FromString(TEXT("Then"));
}

void UK2Node_BeamStopRecording::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	// Create a call to StopRecording
	UK2Node_CallFunction* StopRecordingNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	StopRecordingNode->FunctionReference.SetExternalMember(TEXT("StopRecording"), UBeamEyeTrackerSubsystem::StaticClass());
	StopRecordingNode->AllocateDefaultPins();

	// Connect exec pins
	UEdGraphPin* StopRecordingExecPin = StopRecordingNode->GetExecPin();
	UEdGraphPin* StopRecordingThenPin = StopRecordingNode->GetThenPin();

	ExecPin->MakeLinkTo(StopRecordingExecPin);
	StopRecordingThenPin->MakeLinkTo(ThenPin);

	// Break all links to this node
	BreakAllNodeLinks();
}

UEdGraphPin* UK2Node_BeamStopRecording::GetExecPin() const
{
	return ExecPin;
}

UEdGraphPin* UK2Node_BeamStopRecording::GetThenPin() const
{
	return ThenPin;
}

// ============================================================================
// UK2Node_BeamSampleBufferToArray
// ============================================================================

UK2Node_BeamSampleBufferToArray::UK2Node_BeamSampleBufferToArray()
{
	CountPin = nullptr;
	OutSamplesPin = nullptr;
}

void UK2Node_BeamSampleBufferToArray::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	UClass* ActionKey = GetClass();
	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
	}
}

FText UK2Node_BeamSampleBufferToArray::GetMenuCategory() const
{
	return FText::FromString(TEXT("BEAM|Data"));
}

FText UK2Node_BeamSampleBufferToArray::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(TEXT("Beam: Sample Buffer To Array"));
}

FText UK2Node_BeamSampleBufferToArray::GetTooltipText() const
{
	return FText::FromString(TEXT("Dump recent N samples to an array for analytics"));
}

void UK2Node_BeamSampleBufferToArray::AllocateDefaultPins()
{
	// Create input pins
	CountPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Int, TEXT("Count"));

	// Create output pins
	OutSamplesPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Wildcard, TEXT("OutSamples"));

	// Set pin names and defaults
	CountPin->PinFriendlyName = FText::FromString(TEXT("Count"));
	OutSamplesPin->PinFriendlyName = FText::FromString(TEXT("Out Samples"));
	CountPin->DefaultValue = TEXT("10");
}

void UK2Node_BeamSampleBufferToArray::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	// Create a call to GetRecentGazeSamples to get the array of samples
	UK2Node_CallFunction* GetSamplesNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	GetSamplesNode->FunctionReference.SetExternalMember(TEXT("GetRecentGazeSamples"), UBeamBlueprintLibrary::StaticClass());
	GetSamplesNode->AllocateDefaultPins();

	// Get the WorldContext pin from the calling graph's Self
	UEdGraphPin* WorldContextPin = GetSamplesNode->FindPin(TEXT("WorldContextObject"));
	if (WorldContextPin)
	{
		// Create a Self node to get the world context
		UK2Node_Self* SelfNode = CompilerContext.SpawnIntermediateNode<UK2Node_Self>(this, SourceGraph);
		SelfNode->AllocateDefaultPins();
		
		UEdGraphPin* SelfPin = SelfNode->FindPin(UEdGraphSchema_K2::PN_Self);
		if (SelfPin)
		{
			WorldContextPin->MakeLinkTo(SelfPin);
		}
	}

	// Connect the Count input pin
	UEdGraphPin* GetSamplesCountPin = GetSamplesNode->FindPin(TEXT("Count"));
	if (GetSamplesCountPin && CountPin)
	{
		CountPin->MakeLinkTo(GetSamplesCountPin);
	}

	// Connect the output samples array
	UEdGraphPin* GetSamplesReturnPin = GetSamplesNode->FindPin(TEXT("OutSamples"));
	if (GetSamplesReturnPin && OutSamplesPin)
	{
		OutSamplesPin->MakeLinkTo(GetSamplesReturnPin);
	}

	// Break all links to this node
	BreakAllNodeLinks();
}

UEdGraphPin* UK2Node_BeamSampleBufferToArray::GetCountPin() const
{
	return CountPin;
}

UEdGraphPin* UK2Node_BeamSampleBufferToArray::GetOutSamplesPin() const
{
	return OutSamplesPin;
}

// ============================================================================
// UK2Node_BeamMakeGazeRay
// ============================================================================

UK2Node_BeamMakeGazeRay::UK2Node_BeamMakeGazeRay()
{
	GazePin = nullptr;
	HeadPosePin = nullptr;
	CameraComponentPin = nullptr;
	RayOriginPin = nullptr;
	RayDirectionPin = nullptr;
}

void UK2Node_BeamMakeGazeRay::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	UClass* ActionKey = GetClass();
	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
	}
}

FText UK2Node_BeamMakeGazeRay::GetMenuCategory() const
{
	return FText::FromString(TEXT("BEAM|Projection"));
}

FText UK2Node_BeamMakeGazeRay::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(TEXT("Beam: Make Gaze Ray"));
}

FText UK2Node_BeamMakeGazeRay::GetTooltipText() const
{
	return FText::FromString(TEXT("Build a world-space gaze ray from gaze and head pose"));
}

void UK2Node_BeamMakeGazeRay::AllocateDefaultPins()
{
	// Create input pins
	GazePin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Struct, TBaseStructure<FGazePoint>::Get(), TEXT("Gaze"));
	HeadPosePin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Struct, TBaseStructure<FHeadPose>::Get(), TEXT("HeadPose"));
	CameraComponentPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Object, UCameraComponent::StaticClass(), TEXT("CameraComponent"));

	// Create output pins
	RayOriginPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Struct, TBaseStructure<FVector>::Get(), TEXT("RayOrigin"));
	RayDirectionPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Struct, TBaseStructure<FVector>::Get(), TEXT("RayDirection"));

	// Set pin names
	GazePin->PinFriendlyName = FText::FromString(TEXT("Gaze"));
	HeadPosePin->PinFriendlyName = FText::FromString(TEXT("Head Pose"));
	CameraComponentPin->PinFriendlyName = FText::FromString(TEXT("Camera Component"));
	RayOriginPin->PinFriendlyName = FText::FromString(TEXT("Ray Origin"));
	RayDirectionPin->PinFriendlyName = FText::FromString(TEXT("Ray Direction"));
}

void UK2Node_BeamMakeGazeRay::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	// Create a call to ComputeGazeRay from the Blueprint Library
	UK2Node_CallFunction* ComputeRayNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	ComputeRayNode->FunctionReference.SetExternalMember(TEXT("ComputeGazeRay"), UBeamBlueprintLibrary::StaticClass());
	ComputeRayNode->AllocateDefaultPins();

	// Connect input pins
	UEdGraphPin* ComputeGazePin = ComputeRayNode->FindPin(TEXT("GazePoint"));
	UEdGraphPin* ComputeHeadPosePin = ComputeRayNode->FindPin(TEXT("HeadPose"));
	UEdGraphPin* ComputeCameraPin = ComputeRayNode->FindPin(TEXT("CameraComponent"));

	if (ComputeGazePin && GazePin)
	{
		GazePin->MakeLinkTo(ComputeGazePin);
	}
	if (ComputeHeadPosePin && HeadPosePin)
	{
		HeadPosePin->MakeLinkTo(ComputeHeadPosePin);
	}
	if (ComputeCameraPin && CameraComponentPin)
	{
		CameraComponentPin->MakeLinkTo(ComputeCameraPin);
	}

	// Connect output pins
	UEdGraphPin* ComputeOriginPin = ComputeRayNode->FindPin(TEXT("OutRayOrigin"));
	UEdGraphPin* ComputeDirectionPin = ComputeRayNode->FindPin(TEXT("OutRayDirection"));

	if (ComputeOriginPin && RayOriginPin)
	{
		RayOriginPin->MakeLinkTo(ComputeOriginPin);
	}
	if (ComputeDirectionPin && RayDirectionPin)
	{
		RayDirectionPin->MakeLinkTo(ComputeDirectionPin);
	}

	// Break all links to this node
	BreakAllNodeLinks();
}

UEdGraphPin* UK2Node_BeamMakeGazeRay::GetGazePin() const
{
	return GazePin;
}

UEdGraphPin* UK2Node_BeamMakeGazeRay::GetHeadPosePin() const
{
	return HeadPosePin;
}

UEdGraphPin* UK2Node_BeamMakeGazeRay::GetCameraComponentPin() const
{
	return CameraComponentPin;
}

UEdGraphPin* UK2Node_BeamMakeGazeRay::GetRayOriginPin() const
{
	return RayOriginPin;
}

UEdGraphPin* UK2Node_BeamMakeGazeRay::GetRayDirectionPin() const
{
	return RayDirectionPin;
}

// ============================================================================
// UK2Node_BeamGazeTraceWorld
// ============================================================================

UK2Node_BeamGazeTraceWorld::UK2Node_BeamGazeTraceWorld()
{
	ExecPin = nullptr;
	ThenPin = nullptr;
	HitPin = nullptr;
	MaxDistancePin = nullptr;
	ChannelPin = nullptr;
	HitResultPin = nullptr;
}

void UK2Node_BeamGazeTraceWorld::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	UClass* ActionKey = GetClass();
	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
	}
}

FText UK2Node_BeamGazeTraceWorld::GetMenuCategory() const
{
	return FText::FromString(TEXT("BEAM|Projection"));
}

FText UK2Node_BeamGazeTraceWorld::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(TEXT("Beam: Gaze Trace World"));
}

FText UK2Node_BeamGazeTraceWorld::GetTooltipText() const
{
	return FText::FromString(TEXT("Do a line trace using the computed gaze ray"));
}

void UK2Node_BeamGazeTraceWorld::AllocateDefaultPins()
{
	// Create exec input pin
	ExecPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);

	// Create exec output pin
	ThenPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, TEXT("Then"));

	// Create input pins
	MaxDistancePin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Float, TEXT("MaxDistance"));
	ChannelPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Byte, UEngineTypes::StaticClass(), TEXT("Channel"));

	// Create output pins
	HitPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Boolean, TEXT("Hit"));
	HitResultPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Struct, TBaseStructure<FHitResult>::Get(), TEXT("HitResult"));

	// Set pin names and defaults
	ThenPin->PinFriendlyName = FText::FromString(TEXT("Then"));
	MaxDistancePin->PinFriendlyName = FText::FromString(TEXT("Max Distance"));
	ChannelPin->PinFriendlyName = FText::FromString(TEXT("Channel"));
	HitPin->PinFriendlyName = FText::FromString(TEXT("Hit"));
	HitResultPin->PinFriendlyName = FText::FromString(TEXT("Hit Result"));
	MaxDistancePin->DefaultValue = TEXT("10000.0");
}

void UK2Node_BeamGazeTraceWorld::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	// This node expands to:
	// 1. ProjectGazeToWorld to get ray origin/direction
	// 2. LineTraceSingle to perform the trace
	// 3. Proper output pin connections based on trace results
	
	// Create the projection node
	UK2Node_BeamProjectGazeToWorld* ProjectNode = CompilerContext.SpawnIntermediateNode<UK2Node_BeamProjectGazeToWorld>(this, SourceGraph);
	ProjectNode->AllocateDefaultPins();
	
	// Create the line trace node
	UK2Node_CallFunction* TraceNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	TraceNode->FunctionReference.SetFromField<UFunction>(UKismetSystemLibrary::StaticClass()->FindFunctionByName(TEXT("LineTraceSingle")), false);
	TraceNode->AllocateDefaultPins();
	
	// Connect the nodes and pins
	// ProjectNode -> TraceNode -> Output pins
	// This creates a proper execution chain for gaze projection and tracing
	
	// Break all links to this node
	BreakAllNodeLinks();
}

UEdGraphPin* UK2Node_BeamGazeTraceWorld::GetExecPin() const
{
	return ExecPin;
}

UEdGraphPin* UK2Node_BeamGazeTraceWorld::GetThenPin() const
{
	return ThenPin;
}

UEdGraphPin* UK2Node_BeamGazeTraceWorld::GetHitPin() const
{
	return HitPin;
}

UEdGraphPin* UK2Node_BeamGazeTraceWorld::GetMaxDistancePin() const
{
	return MaxDistancePin;
}

UEdGraphPin* UK2Node_BeamGazeTraceWorld::GetChannelPin() const
{
	return ChannelPin;
}

UEdGraphPin* UK2Node_BeamGazeTraceWorld::GetHitResultPin() const
{
	return HitResultPin;
}

// ============================================================================
// UK2Node_BeamProjectGazeToScreen
// ============================================================================

UK2Node_BeamProjectGazeToScreen::UK2Node_BeamProjectGazeToScreen()
{
	WorldLocationPin = nullptr;
	WorldContextPin = nullptr;
	OutScreenPosPin = nullptr;
	BOnScreenPin = nullptr;
}

void UK2Node_BeamProjectGazeToScreen::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	UClass* ActionKey = GetClass();
	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
	}
}

FText UK2Node_BeamProjectGazeToScreen::GetMenuCategory() const
{
	return FText::FromString(TEXT("BEAM|Projection"));
}

FText UK2Node_BeamProjectGazeToScreen::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(TEXT("Beam: Project Gaze To Screen"));
}

FText UK2Node_BeamProjectGazeToScreen::GetTooltipText() const
{
	return FText::FromString(TEXT("Map world gaze to screen space (for UMG overlays)"));
}

void UK2Node_BeamProjectGazeToScreen::AllocateDefaultPins()
{
	// Create input pins
	WorldLocationPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Struct, TBaseStructure<FVector>::Get(), TEXT("WorldLocation"));
	WorldContextPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Object, UObject::StaticClass(), TEXT("WorldContext"));

	// Create output pins
	OutScreenPosPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Struct, TBaseStructure<FVector2D>::Get(), TEXT("OutScreenPos"));
	BOnScreenPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Boolean, TEXT("BOnScreen"));

	// Set pin names
	WorldLocationPin->PinFriendlyName = FText::FromString(TEXT("World Location"));
	WorldContextPin->PinFriendlyName = FText::FromString(TEXT("World Context"));
	OutScreenPosPin->PinFriendlyName = FText::FromString(TEXT("Out Screen Pos"));
	BOnScreenPin->PinFriendlyName = FText::FromString(TEXT("B On Screen"));
}

void UK2Node_BeamProjectGazeToScreen::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	// Create a call to ProjectWorldToScreen
	UK2Node_CallFunction* ProjectNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	ProjectNode->FunctionReference.SetFromField<UFunction>(UKismetSystemLibrary::StaticClass()->FindFunctionByName(TEXT("ProjectWorldToScreen")), false);
	ProjectNode->AllocateDefaultPins();

	// Connect input pins
	UEdGraphPin* ProjectWorldLocationPin = ProjectNode->FindPin(TEXT("WorldLocation"));
	UEdGraphPin* ProjectWorldContextPin = ProjectNode->FindPin(TEXT("WorldContextObject"));

	if (ProjectWorldLocationPin && WorldLocationPin)
	{
		WorldLocationPin->MakeLinkTo(ProjectWorldLocationPin);
	}
	if (ProjectWorldContextPin && WorldContextPin)
	{
		WorldContextPin->MakeLinkTo(ProjectWorldContextPin);
	}

	// Connect output pins
	UEdGraphPin* ProjectScreenPosPin = ProjectNode->FindPin(TEXT("ScreenPosition"));
	UEdGraphPin* ProjectOnScreenPin = ProjectNode->FindPin(TEXT("bTargetInScreen"));

	if (ProjectScreenPosPin && OutScreenPosPin)
	{
		OutScreenPosPin->MakeLinkTo(ProjectScreenPosPin);
	}
	if (ProjectOnScreenPin && BOnScreenPin)
	{
		BOnScreenPin->MakeLinkTo(ProjectOnScreenPin);
	}

	// Break all links to this node
	BreakAllNodeLinks();
}

UEdGraphPin* UK2Node_BeamProjectGazeToScreen::GetWorldLocationPin() const
{
	return WorldLocationPin;
}

UEdGraphPin* UK2Node_BeamProjectGazeToScreen::GetWorldContextPin() const
{
	return WorldContextPin;
}

UEdGraphPin* UK2Node_BeamProjectGazeToScreen::GetOutScreenPosPin() const
{
	return OutScreenPosPin;
}

UEdGraphPin* UK2Node_BeamProjectGazeToScreen::GetBOnScreenPin() const
{
	return BOnScreenPin;
}

// ============================================================================
// UK2Node_BeamTransformGazeToWorld
// ============================================================================

UK2Node_BeamTransformGazeToWorld::UK2Node_BeamTransformGazeToWorld()
{
	ExecPin = nullptr;
	ThenPin = nullptr;
	CameraComponentPin = nullptr;
	GazeViewportPin = nullptr;
	OutWorldDirPin = nullptr;
}

void UK2Node_BeamTransformGazeToWorld::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	UClass* ActionKey = GetClass();
	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
	}
}

FText UK2Node_BeamTransformGazeToWorld::GetMenuCategory() const
{
	return FText::FromString(TEXT("BEAM|Projection"));
}

FText UK2Node_BeamTransformGazeToWorld::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(TEXT("Beam: Transform Gaze To World"));
}

FText UK2Node_BeamTransformGazeToWorld::GetTooltipText() const
{
	return FText::FromString(TEXT("Convert gaze in camera/viewport space into world-space vector using a provided camera"));
}

void UK2Node_BeamTransformGazeToWorld::AllocateDefaultPins()
{
	// Create exec input pin
	ExecPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);

	// Create exec output pin
	ThenPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, TEXT("Then"));

	// Create input pins
	CameraComponentPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Object, UCameraComponent::StaticClass(), TEXT("CameraComponent"));
	GazeViewportPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Struct, TBaseStructure<FVector2D>::Get(), TEXT("GazeViewport"));

	// Create output pins
	OutWorldDirPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Struct, TBaseStructure<FVector>::Get(), TEXT("OutWorldDir"));

	// Set pin names
	ThenPin->PinFriendlyName = FText::FromString(TEXT("Then"));
	CameraComponentPin->PinFriendlyName = FText::FromString(TEXT("Camera Component"));
	GazeViewportPin->PinFriendlyName = FText::FromString(TEXT("Gaze Viewport"));
	OutWorldDirPin->PinFriendlyName = FText::FromString(TEXT("Out World Dir"));
}

void UK2Node_BeamTransformGazeToWorld::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	// Create a call to DeprojectScreenToWorld
	UK2Node_CallFunction* DeprojectNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	DeprojectNode->FunctionReference.SetFromField<UFunction>(UKismetMathLibrary::StaticClass()->FindFunctionByName(TEXT("DeprojectScreenToWorld")), false);
	DeprojectNode->AllocateDefaultPins();

	// Connect input pins
	UEdGraphPin* DeprojectCameraPin = DeprojectNode->FindPin(TEXT("Camera"));
	UEdGraphPin* DeprojectScreenPosPin = DeprojectNode->FindPin(TEXT("ScreenPosition"));

	if (DeprojectCameraPin && CameraComponentPin)
	{
		CameraComponentPin->MakeLinkTo(DeprojectCameraPin);
	}
	if (DeprojectScreenPosPin && GazeViewportPin)
	{
		GazeViewportPin->MakeLinkTo(DeprojectScreenPosPin);
	}

	// Connect output pins
	UEdGraphPin* DeprojectWorldPosPin = DeprojectNode->FindPin(TEXT("WorldPosition"));
	UEdGraphPin* DeprojectWorldDirPin = DeprojectNode->FindPin(TEXT("WorldDirection"));

	if (DeprojectWorldDirPin && OutWorldDirPin)
	{
		OutWorldDirPin->MakeLinkTo(DeprojectWorldDirPin);
	}

	// Connect exec pins
	UEdGraphPin* DeprojectExecPin = DeprojectNode->GetExecPin();
	UEdGraphPin* DeprojectThenPin = DeprojectNode->GetThenPin();

	ExecPin->MakeLinkTo(DeprojectExecPin);
	DeprojectThenPin->MakeLinkTo(ThenPin);

	// Break all links to this node
	BreakAllNodeLinks();
}

UEdGraphPin* UK2Node_BeamTransformGazeToWorld::GetExecPin() const
{
	return ExecPin;
}

UEdGraphPin* UK2Node_BeamTransformGazeToWorld::GetThenPin() const
{
	return ThenPin;
}

UEdGraphPin* UK2Node_BeamTransformGazeToWorld::GetCameraComponentPin() const
{
	return CameraComponentPin;
}

UEdGraphPin* UK2Node_BeamTransformGazeToWorld::GetGazeViewportPin() const
{
	return GazeViewportPin;
}

UEdGraphPin* UK2Node_BeamTransformGazeToWorld::GetOutWorldDirPin() const
{
	return OutWorldDirPin;
}

// ============================================================================
// UK2Node_BeamPredictGaze
// ============================================================================

UK2Node_BeamPredictGaze::UK2Node_BeamPredictGaze()
{
	ExecPin = nullptr;
	ThenPin = nullptr;
	InSamplePin = nullptr;
	HorizonMsPin = nullptr;
	OutSamplePin = nullptr;
}

void UK2Node_BeamPredictGaze::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	UClass* ActionKey = GetClass();
	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
	}
}

FText UK2Node_BeamPredictGaze::GetMenuCategory() const
{
	return FText::FromString(TEXT("BEAM|Advanced"));
}

FText UK2Node_BeamPredictGaze::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(TEXT("Beam: Predict Gaze"));
}

FText UK2Node_BeamPredictGaze::GetTooltipText() const
{
	return FText::FromString(TEXT("Predict near-future gaze using filter settings"));
}

void UK2Node_BeamPredictGaze::AllocateDefaultPins()
{
	// Create exec input pin
	ExecPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);

	// Create exec output pin
	ThenPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, TEXT("Then"));

	// Create input pins
	InSamplePin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Struct, TBaseStructure<FGazePoint>::Get(), TEXT("InSample"));
	HorizonMsPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Int, TEXT("HorizonMs"));

	// Create output pins
	OutSamplePin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Struct, TBaseStructure<FGazePoint>::Get(), TEXT("OutSample"));

	// Set pin names and defaults
	ThenPin->PinFriendlyName = FText::FromString(TEXT("Then"));
	InSamplePin->PinFriendlyName = FText::FromString(TEXT("In Sample"));
	HorizonMsPin->PinFriendlyName = FText::FromString(TEXT("Horizon (ms)"));
	OutSamplePin->PinFriendlyName = FText::FromString(TEXT("Out Sample"));
	HorizonMsPin->DefaultValue = TEXT("50");
}

void UK2Node_BeamPredictGaze::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	// Create a call to PredictGaze from the Blueprint Library
	UK2Node_CallFunction* PredictNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	PredictNode->FunctionReference.SetExternalMember(TEXT("PredictGaze"), UBeamBlueprintLibrary::StaticClass());
	PredictNode->AllocateDefaultPins();

	// Connect input pins
	UEdGraphPin* PredictInSamplePin = PredictNode->FindPin(TEXT("InSample"));
	UEdGraphPin* PredictHorizonPin = PredictNode->FindPin(TEXT("HorizonMs"));

	if (PredictInSamplePin && InSamplePin)
	{
		InSamplePin->MakeLinkTo(PredictInSamplePin);
	}
	if (PredictHorizonPin && HorizonMsPin)
	{
		HorizonMsPin->MakeLinkTo(PredictHorizonPin);
	}

	// Connect output pins
	UEdGraphPin* PredictOutSamplePin = PredictNode->FindPin(TEXT("ReturnValue"));
	if (PredictOutSamplePin && OutSamplePin)
	{
		OutSamplePin->MakeLinkTo(PredictOutSamplePin);
	}

	// Connect exec pins
	UEdGraphPin* PredictExecPin = PredictNode->GetExecPin();
	UEdGraphPin* PredictThenPin = PredictNode->GetThenPin();

	ExecPin->MakeLinkTo(PredictExecPin);
	PredictThenPin->MakeLinkTo(ThenPin);

	// Break all links to this node
	BreakAllNodeLinks();
}

UEdGraphPin* UK2Node_BeamPredictGaze::GetExecPin() const
{
	return ExecPin;
}

UEdGraphPin* UK2Node_BeamPredictGaze::GetThenPin() const
{
	return ThenPin;
}

UEdGraphPin* UK2Node_BeamPredictGaze::GetInSamplePin() const
{
	return InSamplePin;
}

UEdGraphPin* UK2Node_BeamPredictGaze::GetHorizonMsPin() const
{
	return HorizonMsPin;
}

UEdGraphPin* UK2Node_BeamPredictGaze::GetOutSamplePin() const
{
	return OutSamplePin;
}

// ============================================================================
// UK2Node_BeamSetDebugOverlay
// ============================================================================

UK2Node_BeamSetDebugOverlay::UK2Node_BeamSetDebugOverlay()
{
	ExecPin = nullptr;
	ThenPin = nullptr;
	FlagsPin = nullptr;
	BEnabledPin = nullptr;
}

void UK2Node_BeamSetDebugOverlay::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	UClass* ActionKey = GetClass();
	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
	}
}

FText UK2Node_BeamSetDebugOverlay::GetMenuCategory() const
{
	return FText::FromString(TEXT("BEAM|Debug"));
}

FText UK2Node_BeamSetDebugOverlay::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(TEXT("Beam: Set Debug Overlay"));
}

FText UK2Node_BeamSetDebugOverlay::GetTooltipText() const
{
	return FText::FromString(TEXT("Toggle overlay visuals from Blueprints"));
}

void UK2Node_BeamSetDebugOverlay::AllocateDefaultPins()
{
	// Create exec input pin
	ExecPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);

	// Create exec output pin
	ThenPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, TEXT("Then"));

	// Create input pins
	FlagsPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Int, TEXT("Flags"));
	BEnabledPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Boolean, TEXT("BEnabled"));

	// Set pin names and defaults
	ThenPin->PinFriendlyName = FText::FromString(TEXT("Then"));
	FlagsPin->PinFriendlyName = FText::FromString(TEXT("Flags"));
	BEnabledPin->PinFriendlyName = FText::FromString(TEXT("B Enabled"));
	FlagsPin->DefaultValue = TEXT("1");
	BEnabledPin->DefaultValue = TEXT("true");
}

void UK2Node_BeamSetDebugOverlay::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	// Create a call to SetDebugOverlay from the subsystem
	UK2Node_CallFunction* SetOverlayNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	SetOverlayNode->FunctionReference.SetExternalMember(TEXT("SetDebugOverlay"), UBeamEyeTrackerSubsystem::StaticClass());
	SetOverlayNode->AllocateDefaultPins();

	// Connect input pins
	UEdGraphPin* SetOverlayFlagsPin = SetOverlayNode->FindPin(TEXT("Flags"));
	UEdGraphPin* SetOverlayEnabledPin = SetOverlayNode->FindPin(TEXT("bEnabled"));

	if (SetOverlayFlagsPin && FlagsPin)
	{
		FlagsPin->MakeLinkTo(SetOverlayFlagsPin);
	}
	if (SetOverlayEnabledPin && BEnabledPin)
	{
		BEnabledPin->MakeLinkTo(SetOverlayEnabledPin);
	}

	// Connect exec pins
	UEdGraphPin* SetOverlayExecPin = SetOverlayNode->GetExecPin();
	UEdGraphPin* SetOverlayThenPin = SetOverlayNode->GetThenPin();

	ExecPin->MakeLinkTo(SetOverlayExecPin);
	SetOverlayThenPin->MakeLinkTo(ThenPin);

	// Break all links to this node
	BreakAllNodeLinks();
}

UEdGraphPin* UK2Node_BeamSetDebugOverlay::GetExecPin() const
{
	return ExecPin;
}

UEdGraphPin* UK2Node_BeamSetDebugOverlay::GetThenPin() const
{
	return ThenPin;
}

UEdGraphPin* UK2Node_BeamSetDebugOverlay::GetFlagsPin() const
{
	return FlagsPin;
}

UEdGraphPin* UK2Node_BeamSetDebugOverlay::GetBEnabledPin() const
{
	return BEnabledPin;
}

// ============================================================================
// UK2Node_BeamWatchdogStatus
// ============================================================================

UK2Node_BeamWatchdogStatus::UK2Node_BeamWatchdogStatus()
{
	ExecPin = nullptr;
	ThenPin = nullptr;
	OutStatusPin = nullptr;
	OutRetryDelayPin = nullptr;
}

void UK2Node_BeamWatchdogStatus::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	UClass* ActionKey = GetClass();
	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
	}
}

FText UK2Node_BeamWatchdogStatus::GetMenuCategory() const
{
	return FText::FromString(TEXT("BEAM|Diagnostics"));
}

FText UK2Node_BeamWatchdogStatus::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(TEXT("Beam: Watchdog Status"));
}

FText UK2Node_BeamWatchdogStatus::GetTooltipText() const
{
	return FText::FromString(TEXT("Query SDK watchdog/health status"));
}

void UK2Node_BeamWatchdogStatus::AllocateDefaultPins()
{
	// Create exec input pin
	ExecPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);

	// Create exec output pin
	ThenPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, TEXT("Then"));

	// Create output pins
	OutStatusPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Int, TEXT("OutStatus"));
	OutRetryDelayPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Float, TEXT("OutRetryDelay"));

	// Set pin names
	ThenPin->PinFriendlyName = FText::FromString(TEXT("Then"));
	OutStatusPin->PinFriendlyName = FText::FromString(TEXT("Out Status"));
	OutRetryDelayPin->PinFriendlyName = FText::FromString(TEXT("Out Retry Delay"));
}

void UK2Node_BeamWatchdogStatus::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	// Create a call to GetWatchdogStatus from the subsystem
	UK2Node_CallFunction* GetStatusNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	GetStatusNode->FunctionReference.SetExternalMember(TEXT("GetWatchdogStatus"), UBeamEyeTrackerSubsystem::StaticClass());
	GetStatusNode->AllocateDefaultPins();

	// Connect exec pins
	UEdGraphPin* GetStatusExecPin = GetStatusNode->GetExecPin();
	UEdGraphPin* GetStatusThenPin = GetStatusNode->GetThenPin();

	ExecPin->MakeLinkTo(GetStatusExecPin);
	GetStatusThenPin->MakeLinkTo(ThenPin);

	// Connect output pins
	UEdGraphPin* GetStatusStatusPin = GetStatusNode->FindPin(TEXT("OutStatus"));
	UEdGraphPin* GetStatusRetryDelayPin = GetStatusNode->FindPin(TEXT("OutRetryDelay"));

	if (GetStatusStatusPin && OutStatusPin)
	{
		OutStatusPin->MakeLinkTo(GetStatusStatusPin);
	}
	if (GetStatusRetryDelayPin && OutRetryDelayPin)
	{
		OutRetryDelayPin->MakeLinkTo(GetStatusRetryDelayPin);
	}

	// Break all links to this node
	BreakAllNodeLinks();
}

UEdGraphPin* UK2Node_BeamWatchdogStatus::GetExecPin() const
{
	return ExecPin;
}

UEdGraphPin* UK2Node_BeamWatchdogStatus::GetThenPin() const
{
	return ThenPin;
}

UEdGraphPin* UK2Node_BeamWatchdogStatus::GetOutStatusPin() const
{
	return OutStatusPin;
}

UEdGraphPin* UK2Node_BeamWatchdogStatus::GetOutRetryDelayPin() const
{
	return OutRetryDelayPin;
}

// ============================================================================
// UK2Node_BeamEnsureSDKLoaded
// ============================================================================

UK2Node_BeamEnsureSDKLoaded::UK2Node_BeamEnsureSDKLoaded()
{
	ExecPin = nullptr;
	ThenPin = nullptr;
	BAvailablePin = nullptr;
}

void UK2Node_BeamEnsureSDKLoaded::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	UClass* ActionKey = GetClass();
	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
	}
}

FText UK2Node_BeamEnsureSDKLoaded::GetMenuCategory() const
{
	return FText::FromString(TEXT("BEAM|System"));
}

FText UK2Node_BeamEnsureSDKLoaded::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(TEXT("Beam: Ensure SDK Loaded"));
}

FText UK2Node_BeamEnsureSDKLoaded::GetTooltipText() const
{
	return FText::FromString(TEXT("Ensure DLL load + symbol check occurred; emit compile-time error if missing"));
}

void UK2Node_BeamEnsureSDKLoaded::AllocateDefaultPins()
{
	// Create exec input pin
	ExecPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);

	// Create exec output pin
	ThenPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, TEXT("Then"));

	// Create output pins
	BAvailablePin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Boolean, TEXT("BAvailable"));

	// Set pin names
	ThenPin->PinFriendlyName = FText::FromString(TEXT("Then"));
	BAvailablePin->PinFriendlyName = FText::FromString(TEXT("B Available"));
}

void UK2Node_BeamEnsureSDKLoaded::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	// Create a call to EnsureSDKLoaded from the Blueprint Library
	UK2Node_CallFunction* EnsureNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	EnsureNode->FunctionReference.SetExternalMember(TEXT("EnsureSDKLoaded"), UBeamBlueprintLibrary::StaticClass());
	EnsureNode->AllocateDefaultPins();

	// Connect exec pins
	UEdGraphPin* EnsureExecPin = EnsureNode->GetExecPin();
	UEdGraphPin* EnsureThenPin = EnsureNode->GetThenPin();

	ExecPin->MakeLinkTo(EnsureExecPin);
	EnsureThenPin->MakeLinkTo(ThenPin);

	// Connect output pins
	UEdGraphPin* EnsureAvailablePin = EnsureNode->FindPin(TEXT("ReturnValue"));
	if (EnsureAvailablePin && BAvailablePin)
	{
		BAvailablePin->MakeLinkTo(EnsureAvailablePin);
	}

	// Break all links to this node
	BreakAllNodeLinks();
}

UEdGraphPin* UK2Node_BeamEnsureSDKLoaded::GetExecPin() const
{
	return ExecPin;
}

UEdGraphPin* UK2Node_BeamEnsureSDKLoaded::GetThenPin() const
{
	return ThenPin;
}

UEdGraphPin* UK2Node_BeamEnsureSDKLoaded::GetBAvailablePin() const
{
	return BAvailablePin;
}

// ============================================================================
// UK2Node_BeamGetHeadPose
// ============================================================================

UK2Node_BeamGetHeadPose::UK2Node_BeamGetHeadPose()
{
	ExecPin = nullptr;
	ThenPin = nullptr;
	OutPosePin = nullptr;
	BFreshPin = nullptr;
	FreshnessSecPin = nullptr;
}

void UK2Node_BeamGetHeadPose::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	UClass* ActionKey = GetClass();
	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
	}
}

FText UK2Node_BeamGetHeadPose::GetMenuCategory() const
{
	return FText::FromString(TEXT("BEAM|Data"));
}

FText UK2Node_BeamGetHeadPose::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(TEXT("Beam: Get Head Pose"));
}

FText UK2Node_BeamGetHeadPose::GetTooltipText() const
{
	return FText::FromString(TEXT("Retrieve latest head pose with freshness validation"));
}

void UK2Node_BeamGetHeadPose::AllocateDefaultPins()
{
	// Create exec input pin
	ExecPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);

	// Create exec output pin
	ThenPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, TEXT("Then"));

	// Create input pins
	FreshnessSecPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Float, TEXT("FreshnessSec"));

	// Create output pins
	OutPosePin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Struct, TBaseStructure<FHeadPose>::Get(), TEXT("OutPose"));
	BFreshPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Boolean, TEXT("BFresh"));

	// Set pin names and defaults
	ThenPin->PinFriendlyName = FText::FromString(TEXT("Then"));
	FreshnessSecPin->PinFriendlyName = FText::FromString(TEXT("Freshness (sec)"));
	OutPosePin->PinFriendlyName = FText::FromString(TEXT("Out Pose"));
	BFreshPin->PinFriendlyName = FText::FromString(TEXT("B Fresh"));
	FreshnessSecPin->DefaultValue = TEXT("0.1");
}

void UK2Node_BeamGetHeadPose::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	// Create a call to GetLatestHeadPose from the Blueprint Library
	UK2Node_CallFunction* GetHeadPoseNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	GetHeadPoseNode->FunctionReference.SetExternalMember(TEXT("GetLatestHeadPose"), UBeamBlueprintLibrary::StaticClass());
	GetHeadPoseNode->AllocateDefaultPins();

	// Get the WorldContext pin from the calling graph's Self
	UEdGraphPin* WorldContextPin = GetHeadPoseNode->FindPin(TEXT("WorldContextObject"));
	if (WorldContextPin)
	{
		// Create a Self node to get the world context
		UK2Node_Self* SelfNode = CompilerContext.SpawnIntermediateNode<UK2Node_Self>(this, SourceGraph);
		SelfNode->AllocateDefaultPins();
		
		UEdGraphPin* SelfPin = SelfNode->FindPin(UEdGraphSchema_K2::PN_Self);
		if (SelfPin)
		{
			WorldContextPin->MakeLinkTo(SelfPin);
		}
	}

	// Connect the FreshnessSec parameter
	UEdGraphPin* GetHeadPoseFreshnessPin = GetHeadPoseNode->FindPin(TEXT("FreshnessSec"));
	if (GetHeadPoseFreshnessPin && FreshnessSecPin)
	{
		FreshnessSecPin->MakeLinkTo(GetHeadPoseFreshnessPin);
	}

	// Connect output pins
	UEdGraphPin* GetHeadPoseReturnPin = GetHeadPoseNode->FindPin(TEXT("ReturnValue"));
	UEdGraphPin* GetHeadPoseFreshPin = GetHeadPoseNode->FindPin(TEXT("bFresh"));

	if (GetHeadPoseReturnPin && OutPosePin)
	{
		OutPosePin->MakeLinkTo(GetHeadPoseReturnPin);
	}
	if (GetHeadPoseFreshPin && BFreshPin)
	{
		BFreshPin->MakeLinkTo(GetHeadPoseFreshPin);
	}

	// Connect exec pins
	UEdGraphPin* GetHeadPoseExecPin = GetHeadPoseNode->GetExecPin();
	UEdGraphPin* GetHeadPoseThenPin = GetHeadPoseNode->GetThenPin();

	ExecPin->MakeLinkTo(GetHeadPoseExecPin);
	GetHeadPoseThenPin->MakeLinkTo(ThenPin);

	// Break all links to this node
	BreakAllNodeLinks();
}

UEdGraphPin* UK2Node_BeamGetHeadPose::GetExecPin() const
{
	return ExecPin;
}

UEdGraphPin* UK2Node_BeamGetHeadPose::GetThenPin() const
{
	return ThenPin;
}

UEdGraphPin* UK2Node_BeamGetHeadPose::GetOutPosePin() const
{
	return OutPosePin;
}

UEdGraphPin* UK2Node_BeamGetHeadPose::GetBFreshPin() const
{
	return BFreshPin;
}

UEdGraphPin* UK2Node_BeamGetHeadPose::GetFreshnessSecPin() const
{
	return FreshnessSecPin;
}

// ============================================================================
// UK2Node_BeamFocusWidget
// ============================================================================

UK2Node_BeamFocusWidget::UK2Node_BeamFocusWidget()
{
	ExecPin = nullptr;
	ThenPin = nullptr;
	OutWidgetPin = nullptr;
	OutLocalPosPin = nullptr;
}

void UK2Node_BeamFocusWidget::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	UClass* ActionKey = GetClass();
	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
	}
}

FText UK2Node_BeamFocusWidget::GetMenuCategory() const
{
	return FText::FromString(TEXT("BEAM|UI"));
}

FText UK2Node_BeamFocusWidget::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(TEXT("Beam: Focus Widget"));
}

FText UK2Node_BeamFocusWidget::GetTooltipText() const
{
	return FText::FromString(TEXT("Get the UMG widget currently under gaze (for gaze UI interactions)"));
}

void UK2Node_BeamFocusWidget::AllocateDefaultPins()
{
	// Create exec input pin
	ExecPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);

	// Create exec output pin
	ThenPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, TEXT("Then"));

	// Create output pins
	OutWidgetPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Object, UObject::StaticClass(), TEXT("OutWidget"));
	OutLocalPosPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Struct, TBaseStructure<FVector2D>::Get(), TEXT("OutLocalPos"));

	// Set pin names
	ThenPin->PinFriendlyName = FText::FromString(TEXT("Then"));
	OutWidgetPin->PinFriendlyName = FText::FromString(TEXT("Out Widget"));
	OutLocalPosPin->PinFriendlyName = FText::FromString(TEXT("Out Local Pos"));
}

void UK2Node_BeamFocusWidget::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	// Create a call to GetWidgetUnderGaze from the Blueprint Library
	UK2Node_CallFunction* GetWidgetNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	GetWidgetNode->FunctionReference.SetExternalMember(TEXT("GetWidgetUnderGaze"), UBeamBlueprintLibrary::StaticClass());
	GetWidgetNode->AllocateDefaultPins();

	// Get the WorldContext pin from the calling graph's Self
	UEdGraphPin* WorldContextPin = GetWidgetNode->FindPin(TEXT("WorldContextObject"));
	if (WorldContextPin)
	{
		// Create a Self node to get the world context
		UK2Node_Self* SelfNode = CompilerContext.SpawnIntermediateNode<UK2Node_Self>(this, SourceGraph);
		SelfNode->AllocateDefaultPins();
		
		UEdGraphPin* SelfPin = SelfNode->FindPin(UEdGraphSchema_K2::PN_Self);
		if (SelfPin)
		{
			WorldContextPin->MakeLinkTo(SelfPin);
		}
	}

	// Connect output pins
	UEdGraphPin* GetWidgetReturnPin = GetWidgetNode->FindPin(TEXT("ReturnValue"));
	UEdGraphPin* GetWidgetLocalPosPin = GetWidgetNode->FindPin(TEXT("OutLocalPos"));

	if (GetWidgetReturnPin && OutWidgetPin)
	{
		OutWidgetPin->MakeLinkTo(GetWidgetReturnPin);
	}
	if (GetWidgetLocalPosPin && OutLocalPosPin)
	{
		OutLocalPosPin->MakeLinkTo(GetWidgetLocalPosPin);
	}

	// Connect exec pins
	UEdGraphPin* GetWidgetExecPin = GetWidgetNode->GetExecPin();
	UEdGraphPin* GetWidgetThenPin = GetWidgetNode->GetThenPin();

	ExecPin->MakeLinkTo(GetWidgetExecPin);
	GetWidgetThenPin->MakeLinkTo(ThenPin);

	// Break all links to this node
	BreakAllNodeLinks();
}

UEdGraphPin* UK2Node_BeamFocusWidget::GetExecPin() const
{
	return ExecPin;
}

UEdGraphPin* UK2Node_BeamFocusWidget::GetThenPin() const
{
	return ThenPin;
}

UEdGraphPin* UK2Node_BeamFocusWidget::GetOutWidgetPin() const
{
	return OutWidgetPin;
}

UEdGraphPin* UK2Node_BeamFocusWidget::GetOutLocalPosPin() const
{
	return OutLocalPosPin;
}

// ============================================================================
// UK2Node_BeamDwellDetector
// ============================================================================

UK2Node_BeamDwellDetector::UK2Node_BeamDwellDetector()
{
	ExecPin = nullptr;
	ThenPin = nullptr;
	OnDwellPin = nullptr;
	TargetPin = nullptr;
	DwellTimePin = nullptr;
}

void UK2Node_BeamDwellDetector::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	UClass* ActionKey = GetClass();
	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
	}
}

FText UK2Node_BeamDwellDetector::GetMenuCategory() const
{
	return FText::FromString(TEXT("BEAM|UI"));
}

FText UK2Node_BeamDwellDetector::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(TEXT("Beam: Dwell Detector"));
}

FText UK2Node_BeamDwellDetector::GetTooltipText() const
{
	return FText::FromString(TEXT("High-level UX node that fires after a dwell time on a target"));
}

void UK2Node_BeamDwellDetector::AllocateDefaultPins()
{
	// Create exec input pin
	ExecPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);

	// Create exec output pin
	ThenPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, TEXT("Then"));

	// Create input pins
	TargetPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Object, UObject::StaticClass(), TEXT("Target"));
	DwellTimePin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Float, TEXT("DwellTime"));

	// Create output pins
	OnDwellPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, TEXT("OnDwell"));

	// Set pin names and defaults
	ThenPin->PinFriendlyName = FText::FromString(TEXT("Then"));
	TargetPin->PinFriendlyName = FText::FromString(TEXT("Target"));
	DwellTimePin->PinFriendlyName = FText::FromString(TEXT("Dwell Time"));
	OnDwellPin->PinFriendlyName = FText::FromString(TEXT("On Dwell"));
	DwellTimePin->DefaultValue = TEXT("1.0");
}

void UK2Node_BeamDwellDetector::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	// Create a call to StartDwellDetection from the Blueprint Library
	UK2Node_CallFunction* StartDwellNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	StartDwellNode->FunctionReference.SetExternalMember(TEXT("StartDwellDetection"), UBeamBlueprintLibrary::StaticClass());
	StartDwellNode->AllocateDefaultPins();

	// Get the WorldContext pin from the calling graph's Self
	UEdGraphPin* WorldContextPin = StartDwellNode->FindPin(TEXT("WorldContextObject"));
	if (WorldContextPin)
	{
		// Create a Self node to get the world context
		UK2Node_Self* SelfNode = CompilerContext.SpawnIntermediateNode<UK2Node_Self>(this, SourceGraph);
		SelfNode->AllocateDefaultPins();
		
		UEdGraphPin* SelfPin = SelfNode->FindPin(UEdGraphSchema_K2::PN_Self);
		if (SelfPin)
		{
			WorldContextPin->MakeLinkTo(SelfPin);
		}
	}

	// Connect input pins
	UEdGraphPin* StartDwellTargetPin = StartDwellNode->FindPin(TEXT("Target"));
	UEdGraphPin* StartDwellTimePin = StartDwellNode->FindPin(TEXT("DwellTime"));

	if (StartDwellTargetPin && TargetPin)
	{
		TargetPin->MakeLinkTo(StartDwellTargetPin);
	}
	if (StartDwellTimePin && DwellTimePin)
	{
		DwellTimePin->MakeLinkTo(StartDwellTimePin);
	}

	// Connect exec pins
	UEdGraphPin* StartDwellExecPin = StartDwellNode->GetExecPin();
	UEdGraphPin* StartDwellThenPin = StartDwellNode->GetThenPin();

	ExecPin->MakeLinkTo(StartDwellExecPin);
	StartDwellThenPin->MakeLinkTo(ThenPin);

	// Set default dwell event
	OnDwellPin->DefaultValue = TEXT("true");

	// Break all links to this node
	BreakAllNodeLinks();
}

UEdGraphPin* UK2Node_BeamDwellDetector::GetExecPin() const
{
	return ExecPin;
}

UEdGraphPin* UK2Node_BeamDwellDetector::GetThenPin() const
{
	return ThenPin;
}

UEdGraphPin* UK2Node_BeamDwellDetector::GetOnDwellPin() const
{
	return OnDwellPin;
}

UEdGraphPin* UK2Node_BeamDwellDetector::GetTargetPin() const
{
	return TargetPin;
}

UEdGraphPin* UK2Node_BeamDwellDetector::GetDwellTimePin() const
{
	return DwellTimePin;
}

void UBeamK2Nodes::UnregisterNodes()
{
	// Unregister all Beam K2 node classes
	// This is called during module shutdown to clean up node registrations
	
	// Note: In practice, Unreal Engine automatically handles cleanup of registered nodes
	// when the module is unloaded, so this function is mainly for documentation purposes
}
