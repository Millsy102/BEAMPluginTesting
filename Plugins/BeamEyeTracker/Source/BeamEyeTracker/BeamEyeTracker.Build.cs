// Copyright (C) 2025 Eyeware

/*=============================================================================
    BeamEyeTracker.Build.cs: Build configuration for the Beam Eye Tracker plugin.
=============================================================================*/
using UnrealBuildTool;
using System.IO;

public class BeamEyeTracker : ModuleRules
{
	public BeamEyeTracker(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		// Core dependencies only - no gameplay systems
		PublicDependencyModuleNames.AddRange(new string[] {
			"Core", 
			"CoreUObject", 
			"Engine",
			"InputCore",
			"BlueprintRuntime",
			"GameplayTasks",
			"DeveloperSettings",
			"UMG",
			"Slate",
			"SlateCore",
			"ApplicationCore",
			"EnhancedInput"
		});

		// Debug overlay dependencies (only when feature is enabled)
		if (Target.Configuration == UnrealTargetConfiguration.Development)
		{
			PublicDefinitions.Add("BEAM_FEATURE_DEBUG_OVERLAY=1");
		}
		else
		{
			PublicDefinitions.Add("BEAM_FEATURE_DEBUG_OVERLAY=0");
		}

		// Lean wrapper: minimal dependencies only
		// Note: BlueprintGraph removed - not needed for runtime
		// Note: DeveloperSettings removed - not essential for core functionality
		// Note: External eye tracking bridge removed - can be added via feature flag if needed

		PrivateDependencyModuleNames.AddRange(new string[] { });

		// Get the plugin directory and resolve ThirdParty path
		string PluginDir = Path.GetFullPath(Path.Combine(ModuleDirectory, "..", ".."));
		string ThirdParty = Path.Combine(PluginDir, "ThirdParty", "BeamSDK");

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			// Windows: Use real Beam SDK
			PublicDefinitions.Add("BEAM_STUB_PLATFORM=0");
			PublicDefinitions.Add("BEAM_WINDOWS_PLATFORM=1");
			
			string IncludePath = Path.Combine(ThirdParty, "include");
			string LibPath     = Path.Combine(ThirdParty, "lib", "win64");
			string BinPath     = Path.Combine(ThirdParty, "bin", "win64");

			// Add ThirdParty include path first
			PublicIncludePaths.Add(IncludePath);
			
			// Add include paths for the Example headers
			PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
			PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public", "Example"));
			
			// Link the actual Eyeware Beam SDK library (use MT version for Unreal Engine)
			PublicAdditionalLibraries.Add(Path.Combine(LibPath, "beam_eye_tracker_client_MT.lib"));

			// Include the DLL for runtime
			PublicDelayLoadDLLs.Add("beam_eye_tracker_client_MT.dll");

			// Stage the DLL for both development and packaging
			RuntimeDependencies.Add("$(TargetOutputDir)/beam_eye_tracker_client_MT.dll", Path.Combine(BinPath, "beam_eye_tracker_client_MT.dll"));
			
			// Ensure DLL is copied for development builds - use multiple paths to ensure it works
			if (Target.Configuration == UnrealTargetConfiguration.Development || Target.Configuration == UnrealTargetConfiguration.Debug)
			{
				RuntimeDependencies.Add("$(BinaryOutputDir)/beam_eye_tracker_client_MT.dll", Path.Combine(BinPath, "beam_eye_tracker_client_MT.dll"));
				RuntimeDependencies.Add("$(EngineDir)/Binaries/Win64/beam_eye_tracker_client_MT.dll", Path.Combine(BinPath, "beam_eye_tracker_client_MT.dll"));
				RuntimeDependencies.Add("$(ProjectDir)/Binaries/Win64/beam_eye_tracker_client_MT.dll", Path.Combine(BinPath, "beam_eye_tracker_client_MT.dll"));
			}
			
			// Note: DLL staging is handled by RuntimeDependencies above
			// For development builds, the DLL should be manually copied to Binaries\Win64

			bUseRTTI = false;
			bEnableExceptions = false;
			// Removed deprecated property for UE 5.6 compatibility
			OptimizeCode = CodeOptimization.InShippingBuildsOnly;
		}
		else
		{
			// Non-Windows: compile stubs
			PublicDefinitions.Add("BEAM_STUB_PLATFORM=1");
		}
	}
}
