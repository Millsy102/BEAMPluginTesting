/*=============================================================================
    ComponentCustomization.cpp: Component customization implementation for Beam Eye Tracker.

    Implements custom detail panel customization for the Beam Eye Tracker
    component, enhancing the editor experience with specialized controls
    and layout options.

    Copyright (c) 2025 Eyeware Tech SA. All Rights Reserved.

    Eyeware® and Beam® are registered trademarks of Eyeware Tech SA.
=============================================================================*/

#include "ComponentCustomization.h"
#include "BeamEyeTrackerComponent.h"
#include "BeamEyeTrackerSubsystem.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "PropertyHandle.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Engine/GameViewportClient.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Framework/Application/SlateApplication.h"
#include "Editor.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Styling/AppStyle.h"

TSharedRef<IDetailCustomization> FBeamEyeTrackerComponentCustomization::MakeInstance()
{
    return MakeShareable(new FBeamEyeTrackerComponentCustomization);
}

void FBeamEyeTrackerComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
    // Get the component
    UBeamEyeTrackerComponent* Component = GetComponentFromDetailBuilder(DetailBuilder);
    if (!Component)
    {
        return;
    }

    // Create clean, organized categories
    IDetailCategoryBuilder& ConnectionCategory = DetailBuilder.EditCategory("BEAM|Connection", FText::GetEmpty(), ECategoryPriority::Important);
    IDetailCategoryBuilder& FilteringCategory = DetailBuilder.EditCategory("BEAM|Filtering", FText::GetEmpty(), ECategoryPriority::Important);
    IDetailCategoryBuilder& ProjectionCategory = DetailBuilder.EditCategory("BEAM|Projection", FText::GetEmpty(), ECategoryPriority::Important);
    IDetailCategoryBuilder& DebugCategory = DetailBuilder.EditCategory("BEAM|Debug", FText::GetEmpty(), ECategoryPriority::Important);
    IDetailCategoryBuilder& StatusCategory = DetailBuilder.EditCategory("BEAM|Status", FText::GetEmpty(), ECategoryPriority::Important);

    // Connection Properties
    ConnectionCategory.AddProperty(DetailBuilder.GetProperty("bAutoStart"));
    ConnectionCategory.AddProperty(DetailBuilder.GetProperty("PollingHz"));

    // Filtering Properties
    FilteringCategory.AddProperty(DetailBuilder.GetProperty("bEnableSmoothing"));
    FilteringCategory.AddProperty(DetailBuilder.GetProperty("MinCutoff"));
    FilteringCategory.AddProperty(DetailBuilder.GetProperty("Beta"));

    // Projection Properties
    ProjectionCategory.AddProperty(DetailBuilder.GetProperty("bProjectFromOwnerCamera"));
    ProjectionCategory.AddProperty(DetailBuilder.GetProperty("TraceDistance"));

    // Debug Properties
    DebugCategory.AddProperty(DetailBuilder.GetProperty("bEnableDebugHUD"));
    DebugCategory.AddProperty(DetailBuilder.GetProperty("bShowGazeCrosshair"));
    DebugCategory.AddProperty(DetailBuilder.GetProperty("bShowGazeRay"));
    DebugCategory.AddProperty(DetailBuilder.GetProperty("bShowStatusPanel"));
    DebugCategory.AddProperty(DetailBuilder.GetProperty("bShowGazeTrail"));
    DebugCategory.AddProperty(DetailBuilder.GetProperty("GazeTrailLength"));
    DebugCategory.AddProperty(DetailBuilder.GetProperty("StatusPanelAnchorX"));
    DebugCategory.AddProperty(DetailBuilder.GetProperty("StatusPanelAnchorY"));

    // Add real-time status display (clean, no buttons)
    StatusCategory.AddCustomRow(NSLOCTEXT("Beam", "StatusRow", "Status"))
        .WholeRowContent()
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 10, 0)
            [
                SNew(STextBlock)
                .Text_Lambda([Component]()
                {
                    if (!Component) return NSLOCTEXT("Beam", "NoComponent", "No Component");
                    
                    UWorld* World = Component->GetWorld();
                    if (!World) return NSLOCTEXT("Beam", "NoWorld", "No World");
                    
                    UBeamEyeTrackerSubsystem* Subsystem = World->GetGameInstance()->GetSubsystem<UBeamEyeTrackerSubsystem>();
                    if (!Subsystem) return NSLOCTEXT("Beam", "NoSubsystem", "No Subsystem");
                    
                    if (Subsystem->IsBeamTracking())
                    {
                        // Check if we're actually getting data
                        FBeamFrame TestFrame;
                        if (Subsystem->GetLatestInterpolatedFrame(0.0, TestFrame))
                        {
                            return NSLOCTEXT("Beam", "TrackingActive", "Tracking: Active (Data Flowing)");
                        }
                        else
                        {
                            return NSLOCTEXT("Beam", "TrackingNoData", "Tracking: Active (No Data)");
                        }
                    }
                    else
                    {
                        return NSLOCTEXT("Beam", "TrackingInactive", "Tracking: Inactive");
                    }
                })
                .ToolTipText(NSLOCTEXT("Beam", "StatusTooltip", "Current tracking status. If showing 'No Data', make sure Beam Eye Tracker application is running."))
            ]
        ];
}

UBeamEyeTrackerComponent* FBeamEyeTrackerComponentCustomization::GetComponentFromDetailBuilder(IDetailLayoutBuilder& DetailBuilder)
{
    TArray<TWeakObjectPtr<UObject>> Objects;
    DetailBuilder.GetObjectsBeingCustomized(Objects);
    
    for (TWeakObjectPtr<UObject> Object : Objects)
    {
        if (UBeamEyeTrackerComponent* Component = Cast<UBeamEyeTrackerComponent>(Object.Get()))
        {
            return Component;
        }
    }
    
    	return nullptr;
}

/*=============================================================================
    End of ComponentCustomization.cpp
=============================================================================*/

