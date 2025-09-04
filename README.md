# 🧪 BEAM Plugin Testing Repository

[![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine-5.6+-blue.svg)](https://www.unrealengine.com/)
[![Platform](https://img.shields.io/badge/Platform-Windows%2064--bit-green.svg)](https://www.microsoft.com/windows)
[![Status](https://img.shields.io/badge/Status-Testing%20Release-orange.svg)](https://github.com/Millsy102/BEAMPluginTesting)
[![License](https://img.shields.io/badge/License-Eyeware%20Tech%20SA-red.svg)](LICENSE.md)

**This is a testing repository for the BEAM Plugin v1 release.** 

This repository contains the early testing version of the BEAM Plugin for Unreal Engine, which provides real-time eye and head tracking with low-latency buffering, advanced filtering, recording/playback tools, and full Blueprint integration for games and applications.

**⚠️ Important:** This is a pre-release testing version. For production use, please wait for the official release or contact Eyeware Tech SA.

## ✨ Features

### 🎯 Core Tracking
- **Real-time eye tracking** with configurable polling rates (30-144 Hz)
- **3D head pose tracking** (position & rotation) with confidence scoring
- **Low-latency buffering** with frame interpolation for smooth rendering
- **Multi-platform coordinate mapping** (screen, viewport, world space)

### 🔧 Advanced Data Processing
- **One-Euro filter** for adaptive gaze smoothing
- **EMA filter** for exponential moving average smoothing
- **Outlier detection** and removal
- **Adaptive smoothing** based on confidence levels
- **Frame interpolation** for consistent frame rates

### 🎮 Gaming Integration
- **Gaze-to-world ray projection** for 3D interaction
- **World space gaze raycasting** with collision detection
- **Camera-based coordinate transformation**
- **Distance-based interaction detection**
- **Gaze-based UI focus detection**

### 📊 Analytics & Quality
- **Gaze analytics** (fixations, saccades, scan paths)
- **Calibration quality assessment** with per-eye scoring
- **Performance metrics** (frame times, CPU usage, memory)
- **Real-time health monitoring** and recovery systems

### 🛠️ Development Tools
- **Full Blueprint support** with custom K2 nodes
- **Comprehensive debug HUD** with real-time metrics
- **Console command support** for runtime configuration
- **Async Blueprint actions** for non-blocking operations

### 📹 Recording & Playback
- **Binary .beamrec format** for efficient storage
- **Deterministic playback** for testing and development
- **Frame-by-frame data capture** with timestamps
- **CSV export** for external analysis

## 🧪 Testing Release Information

### What This Repository Is
- **Testing version** of the BEAM Plugin v1
- **Early access** for developers and researchers
- **Feedback collection** for final release improvements
- **Not intended for production use**

### Current Status
- **Version**: v1.0.0 (Testing Release)
- **Release Date**: Early 2025
- **Stability**: Beta/Testing quality
- **Support**: Community feedback and issue reporting

## 🚀 Quick Start (Testing)

### Prerequisites
- **Unreal Engine 5.6** or later
- **Windows 64-bit** operating system
- **Beam Eye Tracker** hardware and software from [Eyeware](https://beam.eyeware.tech)
- **Testing mindset** - this is pre-release software

### Installation

1. **Clone the testing repository**
   ```bash
   git clone git@github.com:Millsy102/BEAMPluginTesting.git
   ```

2. **Copy the plugin to your project**
   - Copy the `Plugins/BeamEyeTracker` folder to your project's `Plugins` directory
   - Or place it in `[UE_Install]/Engine/Plugins/` for global installation

3. **Enable the plugin**
   - Open your project in Unreal Engine
   - Go to **Edit > Plugins**
   - Find "Beam Eye Tracker" and enable it
   - Restart the editor

4. **Install Beam Eye Tracker software**
   - Download from [https://beam.eyeware.tech](https://beam.eyeware.tech)
   - Install and run the Beam Eye Tracker application

## 📖 Usage

### Basic Setup

1. **Add the component to an actor**
   ```cpp
   // In C++
   UBeamEyeTrackerComponent* EyeTracker = CreateDefaultSubobject<UBeamEyeTrackerComponent>(TEXT("EyeTracker"));
   ```

2. **Initialize in Blueprints**
   ```cpp
   // Use the Blueprint Library
   bool Success = UBeamBlueprintLibrary::InitializeEyeTracking(WorldContextObject);
   ```

3. **Access tracking data**
   ```cpp
   // Get current gaze point
   FVector2D GazePoint = UBeamBlueprintLibrary::GetGazePoint2D(WorldContextObject);
   
   // Get head position
   FVector HeadPos = UBeamBlueprintLibrary::GetHeadPosition(WorldContextObject);
   ```

### Advanced Configuration

```cpp
// Configure the component
EyeTracker->bAutoStart = true;
EyeTracker->PollingHz = 120;
EyeTracker->bEnableSmoothing = true;
EyeTracker->MinCutoff = 1.0f;
EyeTracker->Beta = 0.0f;
```

### Blueprint Integration

The plugin provides extensive Blueprint support:

- **One-click initialization** with `InitializeEyeTracking`
- **Real-time data access** with `GetGazePoint2D`, `GetHeadPosition`
- **World projection** with `ProjectGazeToWorld`
- **Quality validation** with `IsValidGazePoint`
- **Custom K2 nodes** for enhanced workflow

## 🏗️ Architecture

### Core Components

- **`UBeamEyeTrackerSubsystem`** - Global tracking management
- **`UBeamEyeTrackerComponent`** - Actor-based tracking integration
- **`FBeamSDK_Wrapper`** - Native SDK integration layer
- **`FBeamFilters`** - Data processing and smoothing
- **`UBeamAnalyticsSubsystem`** - Analytics and performance monitoring

### Data Flow

```
Beam Hardware → Beam SDK → SDK Wrapper → Filters → Subsystem → Component → Blueprint
```

### Performance Features

- **Triple buffering** system for enhanced throughput
- **Lock-free data queues** for high-performance exchange
- **SIMD-optimized processing** for frame batches
- **Adaptive polling** based on frame rate
- **Background threading** for non-blocking operations

## 🔧 Configuration

### Component Settings

| Setting | Description | Range | Default |
|---------|-------------|-------|---------|
| `PollingHz` | Tracking frequency | 30-144 Hz | 120 Hz |
| `bEnableSmoothing` | Enable data smoothing | true/false | true |
| `MinCutoff` | Smoothing strength | 0.1-5.0 | 1.0 |
| `Beta` | Smoothing responsiveness | 0.0-2.0 | 0.0 |
| `TraceDistance` | World projection distance | 100-100000 cm | 5000 cm |

### Filter Types

- **None** - Raw data without processing
- **EMA** - Exponential Moving Average
- **One-Euro** - Adaptive smoothing (recommended)

### Console Commands

```bash
# Enable debug HUD
beam.debug.hud 1

# Set polling rate
beam.polling.rate 120

# Enable smoothing
beam.smoothing.enabled 1

# Set filter parameters
beam.filter.mincutoff 1.0
beam.filter.beta 0.0
```

## 📊 Performance

### System Requirements

- **CPU**: Intel i5/AMD Ryzen 5 or better
- **RAM**: 8GB minimum, 16GB recommended
- **GPU**: DirectX 11 compatible
- **OS**: Windows 10/11 64-bit

### Performance Metrics

- **Latency**: <16ms typical
- **CPU Usage**: <5% typical
- **Memory**: <100MB typical
- **Frame Rate**: Up to 144 Hz tracking

### Optimization Tips

1. **Use appropriate polling rates** for your use case
2. **Enable frame interpolation** for smooth rendering
3. **Configure confidence thresholds** to filter low-quality data
4. **Use background threading** for heavy processing
5. **Monitor performance metrics** with the debug HUD

## 🧪 Testing & Development

### Debug Features

- **Real-time debug HUD** with tracking status
- **Performance monitoring** and metrics
- **Data visualization** (gaze crosshair, trails, rays)
- **Console logging** with configurable verbosity

### Testing Tools

- **Synthetic data generation** for development
- **Recording/playback** for reproducible testing
- **Calibration quality assessment**
- **Health monitoring** and recovery

### Example Projects

Check the `Content/Example` folder for:
- **Example Character** with eye tracking integration
- **Sample Game Mode** demonstrating basic usage
- **Test Widgets** for UI interaction testing

## 🐛 Troubleshooting

### Common Issues

**"Beam App Not Running"**
- Install and run Beam Eye Tracker from [https://beam.eyeware.tech](https://beam.eyeware.tech)
- Ensure the application is running before starting your game

**"DLL Missing"**
- Verify the plugin is properly installed
- Check that `beam_eye_tracker_client_MT.dll` is in your project's `Binaries/Win64` folder

**Low Tracking Quality**
- Ensure proper lighting conditions
- Calibrate the eye tracker
- Check confidence thresholds in component settings

**Performance Issues**
- Reduce polling rate if not needed
- Disable debug features in shipping builds
- Monitor CPU usage with debug HUD

### Debug Commands

```bash
# Check system health
beam.health.status

# Verify SDK connection
beam.sdk.version

# Test data flow
beam.debug.test
```

## 📚 API Reference

### Core Functions

| Function | Description | Returns |
|----------|-------------|---------|
| `InitializeEyeTracking` | Initialize the tracking system | bool |
| `GetGazePoint2D` | Get current gaze coordinates | FVector2D |
| `GetHeadPosition` | Get current head position | FVector |
| `ProjectGazeToWorld` | Project gaze to 3D world | bool |
| `IsEyeTrackingActive` | Check if tracking is running | bool |

### Events

| Event | Description | Parameters |
|-------|-------------|------------|
| `OnGazeUpdated` | Gaze data changed | FGazePoint |
| `OnHeadPoseUpdated` | Head pose changed | FHeadPose |
| `OnBeamHealthChanged` | System health changed | EBeamHealth |

### Data Structures

- **`FGazePoint`** - 2D gaze coordinates with confidence
- **`FHeadPose`** - 3D head position and rotation
- **`FBeamFrame`** - Complete tracking frame
- **`FGazeAnalytics`** - Gaze behavior analysis

## 🐛 Testing & Feedback

### How to Report Issues

We need your feedback to improve the final release! Please report any issues you encounter:

1. **Check existing issues** on [GitHub Issues](https://github.com/Millsy102/BEAMPluginTesting/issues)
2. **Create a new issue** with detailed information:
   - **Description** of the problem
   - **Steps to reproduce**
   - **Expected vs. actual behavior**
   - **System information** (UE version, Windows version, etc.)
   - **Logs** if available

### What We're Looking For

- **Bug reports** - crashes, errors, unexpected behavior
- **Performance issues** - lag, high CPU usage, memory leaks
- **Usability feedback** - confusing interfaces, missing features
- **Integration problems** - compatibility issues with specific setups
- **Feature requests** - what would make this plugin better?

### Testing Scenarios

Please test these common use cases:
- **Basic eye tracking** - gaze point accuracy and latency
- **Head pose tracking** - 3D position and rotation
- **Blueprint integration** - function calls and events
- **Performance** - frame rate impact and resource usage
- **Different UE versions** - compatibility across versions

## 🤝 Contributing (Testing Phase)

During this testing phase, we're primarily looking for:

- **Bug reports** and issue documentation
- **Testing feedback** and use case validation
- **Performance testing** and optimization suggestions
- **Documentation improvements** and clarity suggestions

**Note:** Code contributions are welcome but will be reviewed more carefully during testing.

## 📄 License

This plugin is licensed under the terms provided by Eyeware Tech SA. See [LICENSE.md](LICENSE.md) for details.

**Eyeware®** and **Beam®** are registered trademarks of Eyeware Tech SA.

## 🆘 Support & Feedback

### Testing Support
- **[GitHub Issues](https://github.com/Millsy102/BEAMPluginTesting/issues)** - Report bugs and provide feedback
- **[Discussions](https://github.com/Millsy102/BEAMPluginTesting/discussions)** - Ask questions and share experiences
- **Email**: Direct feedback to the repository maintainer

### Documentation
- [Plugin Documentation](https://docs.beam.eyeware.tech/integration/game_engine.html)
- [API Reference](https://docs.beam.eyeware.tech/api/)

### Community
- [Eyeware Discord](https://discord.gg/eyeware) - Official Eyeware community
- [Community Forum](https://community.eyeware.tech) - General Beam support

### Commercial Support
- [Enterprise Support](https://beam.eyeware.tech/support) - For production use
- [Professional Services](https://beam.eyeware.tech/services) - Custom development

## 🙏 Acknowledgments

- **Eyeware Tech SA** for the Beam SDK and support
- **Unreal Engine** team for the excellent plugin system
- **Testing community** for helping improve the final release

---

**🚀 This testing release brings us closer to the official BEAM Plugin v1!**

**Thank you for helping test this early release – your feedback is invaluable for making the final version the best it can be!**

---

**Made with ❤️ for the eye tracking community**

*For the latest updates and news, follow us on [Twitter](https://twitter.com/eyeware_tech) and [LinkedIn](https://linkedin.com/company/eyeware-tech-sa).*
