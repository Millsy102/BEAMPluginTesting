// Implements performance tracing for Beam eye tracking

#include "BeamTrace.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "BeamLogging.h"

FBeamTrace::FBeamTrace()
	: bEnabled(false)
	, TraceLevel(ETraceCategory::Polling)
{
}

FBeamTrace::~FBeamTrace()
{
	if (bEnabled)
	{
		Shutdown();
	}
}

bool FBeamTrace::Initialize()
{
	bEnabled = true;
	ActiveEvents.Empty();
	CSVData.Empty();
	
	UE_LOG(LogBeam, Log, TEXT("Beam trace system initialized"));
	return true;
}

void FBeamTrace::Shutdown()
{
	bEnabled = false;
	ActiveEvents.Empty();
	CSVData.Empty();
	
	UE_LOG(LogBeam, Log, TEXT("Beam trace system shut down"));
}

void FBeamTrace::SetEnabled(bool bInEnabled)
{
	bEnabled = bInEnabled;
	
	if (bEnabled)
	{
		UE_LOG(LogBeam, Log, TEXT("Beam trace system enabled"));
	}
	else
	{
		UE_LOG(LogBeam, Log, TEXT("Beam trace system disabled"));
	}
}

void FBeamTrace::SetTraceLevel(ETraceCategory InTraceLevel)
{
	TraceLevel = InTraceLevel;
	UE_LOG(LogBeam, Log, TEXT("Beam trace level set to %d"), (int32)InTraceLevel);
}

void FBeamTrace::BeginEvent(ETraceCategory Category, const FString& EventName)
{
	if (!bEnabled || Category < TraceLevel)
	{
		return;
	}

	FString EventKey = FString::Printf(TEXT("%d_%s"), (int32)Category, *EventName);
	ActiveEvents.Add(EventKey, FPlatformTime::Seconds());
	
	UE_LOG(LogBeam, VeryVerbose, TEXT("Trace: Begin event %s in category %d"), *EventName, (int32)Category);
}

void FBeamTrace::EndEvent(ETraceCategory Category)
{
	if (!bEnabled || Category < TraceLevel)
	{
		return;
	}

	UE_LOG(LogBeam, VeryVerbose, TEXT("Trace: End event in category %d"), (int32)Category);
}

void FBeamTrace::InstantEvent(ETraceCategory Category, const FString& EventName)
{
	if (!bEnabled || Category < TraceLevel)
	{
		return;
	}

	UE_LOG(LogBeam, VeryVerbose, TEXT("Trace: Instant event %s in category %d"), *EventName, (int32)Category);
}

void FBeamTrace::TraceCounter(ETraceCategory Category, const FString& CounterName, double Value)
{
	if (!bEnabled || Category < TraceLevel)
	{
		return;
	}

	UE_LOG(LogBeam, VeryVerbose, TEXT("Trace: Counter %s = %.3f in category %d"), *CounterName, Value, (int32)Category);
}

void FBeamTrace::TraceFrame(const FBeamFrame& Frame)
{
	if (!bEnabled)
	{
		return;
	}

	UE_LOG(LogBeam, VeryVerbose, TEXT("Trace: Frame %lld at time %.3f"), Frame.FrameId, Frame.SDKTimestampMs);
}

void FBeamTrace::TraceHealth(EBeamHealth Health, const FString& Details)
{
	if (!bEnabled)
	{
		return;
	}

	UE_LOG(LogBeam, VeryVerbose, TEXT("Trace: Health changed to %d - %s"), (int32)Health, *Details);
}

void FBeamTrace::TraceFilterPerformance(int32 FilterType, double ProcessingTimeMs)
{
	if (!bEnabled)
	{
		return;
	}

	UE_LOG(LogBeam, VeryVerbose, TEXT("Trace: Filter %d took %.3f ms"), FilterType, ProcessingTimeMs);
}

bool FBeamTrace::ExportToCSV(const FString& FilePath, double StartTime, double EndTime)
{
	if (!bEnabled)
	{
		return false;
	}

	FString CSVContent = TEXT("Timestamp,Category,EventName,EventType,Value,Details\n");
	
	// Add data rows (simplified implementation)
	for (const FCSVRow& Row : CSVData)
	{
		if (Row.Timestamp >= StartTime && Row.Timestamp <= EndTime)
		{
			CSVContent += FString::Printf(TEXT("%.6f,%s,%s,%s,%.6f,%s\n"),
				Row.Timestamp,
				*Row.Category,
				*Row.EventName,
				*Row.EventType,
				Row.Value,
				*Row.Details);
		}
	}

	bool bSuccess = FFileHelper::SaveStringToFile(CSVContent, *FilePath);
	
	if (bSuccess)
	{
		UE_LOG(LogBeam, Log, TEXT("Trace data exported to CSV: %s"), *FilePath);
	}
	else
	{
		UE_LOG(LogBeam, Error, TEXT("Failed to export trace data to CSV: %s"), *FilePath);
	}
	
	return bSuccess;
}

