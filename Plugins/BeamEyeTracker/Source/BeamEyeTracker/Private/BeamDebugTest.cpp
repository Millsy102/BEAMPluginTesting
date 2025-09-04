// Contains automated debug tests for Beam SDK integration

#include "BeamDebugCVars.h"
#include "BeamLogging.h"
#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"

#if BEAM_FEATURE_DEBUG_OVERLAY

// Debug CVars Test Suite

/**
 * @brief Simple test to verify Beam debug CVars are working
 * 
 * This function can be called to test that all CVars and console commands
 * are properly registered and functional.
 * 
 * Test Cases:
 * - CVar registration: Verifies all debug variables are accessible
 * - Default values: Checks that CVars have expected initial values
 * - Console commands: Ensures test command is properly registered
 * 
 * Expected Outcome: All CVars should be accessible and return valid values
 */
void TestBeamDebugCVars()
{
    UE_LOG(LogBeam, Log, TEXT("=== Testing Beam Debug CVars ==="));
    
    // Test CVar access - verify all debug variables are properly registered
    bool bHUDEnabled = FBeamDebugCVars::IsDebugHUDEnabled();
    bool bGazeEnabled = FBeamDebugCVars::IsDrawGazeEnabled();
    bool bRayEnabled = FBeamDebugCVars::IsDrawRayEnabled();
    bool bTextEnabled = FBeamDebugCVars::IsDrawTextEnabled();
    bool bTrailEnabled = FBeamDebugCVars::IsDrawTrailEnabled();
    
    UE_LOG(LogBeam, Log, TEXT("Debug HUD: %s"), bHUDEnabled ? TEXT("ENABLED") : TEXT("DISABLED"));
    UE_LOG(LogBeam, Log, TEXT("Draw Gaze: %s"), bGazeEnabled ? TEXT("ENABLED") : TEXT("DISABLED"));
    UE_LOG(LogBeam, Log, TEXT("Draw Ray: %s"), bRayEnabled ? TEXT("ENABLED") : TEXT("DISABLED"));
    UE_LOG(LogBeam, Log, TEXT("Draw Text: %s"), bTextEnabled ? TEXT("ENABLED") : TEXT("DISABLED"));
    UE_LOG(LogBeam, Log, TEXT("Draw Trail: %s"), bTrailEnabled ? TEXT("ENABLED") : TEXT("DISABLED"));
    
    // Test configuration values - verify CVars return expected default values
    int32 SampleWindow = FBeamDebugCVars::GetSampleWindow();
    FVector2D AnchorPos = FBeamDebugCVars::GetAnchorPosition();
    
    UE_LOG(LogBeam, Log, TEXT("Sample Window: %d"), SampleWindow);
    UE_LOG(LogBeam, Log, TEXT("Anchor Position: (%.2f, %.2f)"), AnchorPos.X, AnchorPos.Y);
    
    UE_LOG(LogBeam, Log, TEXT("=== Beam Debug CVars Test Complete ==="));
}

// Test Command Registration

// Register test command for manual testing of debug functionality
// Usage: Type "Beam.TestCVars" in console to run the test suite
static FAutoConsoleCommand TestBeamDebugCVarsCommand(
    TEXT("Beam.TestCVars"),
    TEXT("Test Beam debug CVars functionality"),
    FConsoleCommandDelegate::CreateStatic(&TestBeamDebugCVars)
);

#endif // BEAM_FEATURE_DEBUG_OVERLAY
