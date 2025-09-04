/*=============================================================================
    ComponentCustomization.h: Component customization for Beam Eye Tracker.

    Provides custom detail panel customization for the Beam Eye Tracker
    component, enhancing the editor experience with specialized controls
    and layout options.

    Copyright (c) 2025 Eyeware Tech SA. All Rights Reserved.

    Eyeware® and Beam® are registered trademarks of Eyeware Tech SA.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "DetailLayoutBuilder.h"

class UBeamEyeTrackerComponent;

class BEAMEYETRACKEREDITOR_API FBeamEyeTrackerComponentCustomization : public IDetailCustomization
{
public:
	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance();

	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	/** Gets the component from the detail builder */
	UBeamEyeTrackerComponent* GetComponentFromDetailBuilder(IDetailLayoutBuilder& DetailBuilder);
};

/*=============================================================================
    End of ComponentCustomization.h
=============================================================================*/
