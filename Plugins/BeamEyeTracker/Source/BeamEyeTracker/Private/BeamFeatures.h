/*=============================================================================
    BeamFeatures.h: Feature gates and configuration for Beam SDK.

    Defines comprehensive feature gates for core functionality, P0 must-have
    features, P1 optional features, and P2 tooling features. Configured
    for full UE 5.6 testing with all features enabled.

    Copyright (c) 2025 Eyeware Tech SA. All Rights Reserved.

    Eyeware® and Beam® are registered trademarks of Eyeware Tech SA.
=============================================================================*/

#pragma once

// BEAM EYE TRACKER FEATURE GATES
// 
// FULL BUILD FOR UE 5.6 TESTING
// All features enabled for comprehensive testing

// Core functionality (always enabled)
#define BEAM_FEATURE_CORE_SUBSYSTEM      1   /** Enables UBeamEyeTrackerSubsystem for global tracking access */
#define BEAM_FEATURE_CORE_COMPONENT      1   /** Enables UBeamEyeTrackerComponent for actor-based tracking */
#define BEAM_FEATURE_CORE_SETTINGS       1   /** Enables UBeamEyeTrackerSettings for project configuration */
#define BEAM_FEATURE_CORE_TYPES          1   /** Enables FGazePoint, FHeadPose, FBeamFrame, and enums */
#define BEAM_FEATURE_CORE_WRAPPER        1   /** Enables BeamSDK_Wrapper for native SDK communication */
#define BEAM_FEATURE_CORE_RING           1   /** Enables BeamRing (SPSC ring buffer) for data storage */
#define BEAM_FEATURE_CORE_FILTERS        1   /** Enables BeamFilters (One-Euro + EMA optional) */

// P0 - Must-Have for SDK Wrapper
#define BEAM_FEATURE_WATCHDOG_RECOVERY   1   /** Enables watchdog recovery with exponential backoff and health transitions */
#define BEAM_FEATURE_DLL_SAFETY          1   /** Enables DLL path verification, lazy-load, and symbol checks */
#define BEAM_FEATURE_WORLD_MAPPING       1   /** Enables ProjectGazeToWorld helpers for 3D interaction */
#define BEAM_FEATURE_RECORDED_DATA       1   /** Enables tiny .beamrec recorder/playback (development only) */

// P1 - Optional (Behind Flags; ON for testing)
#define BEAM_FEATURE_EYETRACKER_BRIDGE   1   /** Enables IEyeTracker adapter and conditional registration */
#define BEAM_FEATURE_DEBUG_OVERLAY       1   /** Enables code-only debug overlay via UDebugDrawService/AHUD */
#define BEAM_FEATURE_UNREAL_INSIGHTS     1   /** Enables trace poll duration, queue depth, and frame age metrics */

// P2 - Tooling & Advanced (ON for testing)
#define BEAM_FEATURE_BLUEPRINTS          1   /** Enables minimal Blueprint nodes (start/stop/getters/project) */
#define BEAM_FEATURE_K2NODES             1   /** Enables custom K2 nodes for enhanced UX (Editor only) */
#define BEAM_FEATURE_CONSOLE_COMMANDS    1   /** Enables CVars: Beam.Start, Beam.Stop, Beam.Dump */
#define BEAM_FEATURE_PROFILES            1   /** Enables UDeveloperSettings profiles for configuration presets */
#define BEAM_FEATURE_SYNTHETIC_DATA      1   /** Enables synthetic data source for CI/testing without hardware */

// FULL BUILD FOR UE 5.6 TESTING
// For Testing: All features enabled for comprehensive functionality testing

/*=============================================================================
    End of BeamFeatures.h
=============================================================================*/

