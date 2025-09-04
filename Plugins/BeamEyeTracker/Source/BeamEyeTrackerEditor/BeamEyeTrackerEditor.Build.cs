/*=============================================================================
    BeamEyeTrackerEditor.Build.cs: Build configuration for Beam Eye Tracker Editor.

    Configures editor-specific dependencies and module rules for the
    Beam Eye Tracker editor integration including K2 nodes and UI tools.

    Copyright (c) 2025 Eyeware Tech SA. All Rights Reserved.

    Eyeware® and Beam® are registered trademarks of Eyeware Tech SA.
=============================================================================*/

using UnrealBuildTool;

public class BeamEyeTrackerEditor : ModuleRules
{
	public BeamEyeTrackerEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		// Editor-only dependencies for comprehensive K2 node suite and new editor features
		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"PropertyEditor",
			"UnrealEd",
			"Kismet",
			"KismetCompiler",
			"BlueprintGraph",
			"GraphEditor",
			"ToolMenus",
			"LevelEditor",
			"Slate",
			"SlateCore",
			"Projects",
			"Settings",
			"UMG"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
			"BeamEyeTracker" // Our runtime module
		});

		// Editor-only module - use the correct property for UE 5.6
		// Note: In UE 5.6, we don't need to set Type for editor modules
	}
}
