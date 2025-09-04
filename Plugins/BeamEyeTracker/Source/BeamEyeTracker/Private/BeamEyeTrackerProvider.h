#pragma once

#include "CoreMinimal.h"
#include "IBeamDataSource.h"
#include "BeamEyeTrackerTypes.h"

class FBeamSDK_Wrapper;

/** Internal provider abstraction that connects to Beam SDK */
class BEAMEYETRACKER_API FBeamEyeTrackerProvider : public IBeamDataSource
{
public:
	FBeamEyeTrackerProvider();
	virtual ~FBeamEyeTrackerProvider();

	// IBeamDataSource interface
	virtual bool Initialize() override;
	virtual void Shutdown() override;
	virtual bool IsValid() const override;
	virtual bool FetchCurrentFrame(FBeamFrame& OutFrame) override;
	virtual EBeamHealth GetHealth() const override;
	virtual bool StartCameraRecentering() override;
	virtual void EndCameraRecentering() override;
	
	// Additional required interface methods
	virtual bool InitSDK(const FString& AppName, int32 ViewportWidth, int32 ViewportHeight) override;
	virtual bool IsSDKInitialized() const override;
	virtual void UpdateViewportGeometry(int32 ViewportWidth, int32 ViewportHeight) override;
	virtual bool StartCalibration(const FString& ProfileId) override;
	virtual void StopCalibration() override;

private:
	/** SDK wrapper instance */
	TUniquePtr<FBeamSDK_Wrapper> SDKWrapper;

	/** Data source type */
	EBeamDataSourceType DataSourceType = EBeamDataSourceType::Live;

	/** File path for file-based data source */
	FString FilePath;

	/** Initialize SDK-based data source */
	bool InitializeSDKSource();

	/** Initialize file-based data source */
	bool InitializeFileSource();

	/** Initialize network-based data source */
	bool InitializeNetworkSource();
};

