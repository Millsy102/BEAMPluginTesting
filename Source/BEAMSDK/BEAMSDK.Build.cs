/*=============================================================================
    BEAMSDK.Build.cs: Build configuration for BEAM SDK project.

    Configures dependencies and module rules for the main project
    including the Beam Eye Tracker plugin integration.

    Copyright (c) 2025 Eyeware Tech SA. All Rights Reserved.

    Eyeware® and Beam® are registered trademarks of Eyeware Tech SA.
=============================================================================*/

using UnrealBuildTool;

public class BEAMSDK : ModuleRules
{
	public BEAMSDK(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		// Enable verbose logging and detailed build output
		CppCompileWarningSettings.UndefinedIdentifierWarningLevel = WarningLevel.Warning;
		
		// Enable all warnings for better error detection
		PublicDefinitions.Add("UE_BUILD_DEVELOPMENT=1");
		// Note: UE_BUILD_DEBUG is already defined by the engine, don't redefine
		
		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore",
			"BeamEyeTracker" // Add the plugin dependency
		});

		PrivateDependencyModuleNames.AddRange(new string[] {  });
	}
}
