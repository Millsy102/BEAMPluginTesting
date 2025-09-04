/*=============================================================================
    BeamTrace.h: Gaze ray tracing and performance analysis for Beam SDK.

    Performs gaze ray tracing into the 3D world with Unreal Insights/CSV
    helpers for comprehensive performance analysis including poll duration,
    queue depth, frame age, and system health monitoring.

    Copyright (c) 2025 Eyeware Tech SA. All Rights Reserved.

    Eyeware® and Beam® are registered trademarks of Eyeware Tech SA.
=============================================================================*/

#pragma once

#include "CoreMinimal.h"
#include "BeamEyeTrackerTypes.h"

/** Performs gaze ray tracing into the 3D world */
class BEAMEYETRACKER_API FBeamTrace
{
public:
	/** Trace categories for organizing performance data */
	enum class ETraceCategory : uint8
	{
		Polling,        // SDK polling performance
		QueueDepth,     // Frame buffer utilization
		FrameAge,       // Frame latency analysis
		Health,         // System health transitions
		Filters         // Filter performance
	};

	/** Trace event types for different measurement approaches */
	enum class ETraceEvent : uint8
	{
		Begin,          // Start of operation
		End,            // End of operation
		Instant,        // Instantaneous event
		Counter         // Counter value
	};

public:
	FBeamTrace();
	~FBeamTrace();

	/** Initializes the tracing system and prepares for data collection */
	bool Initialize();
	
	/** Shuts down the tracing system and cleans up resources */
	void Shutdown();
	
	/** Checks if tracing is currently enabled and active */
	bool IsEnabled() const { return bEnabled; }
	
	/** Enables or disables tracing system at runtime */
	void SetEnabled(bool bInEnabled);
	
	/** Sets the trace level to control which categories are recorded */
	void SetTraceLevel(ETraceCategory InTraceLevel);
	
	/** Begins a trace event for timing measurements */
	void BeginEvent(ETraceCategory Category, const FString& EventName);
	
	/** Ends a trace event and records the duration */
	void EndEvent(ETraceCategory Category);
	
	/** Records an instantaneous trace event without duration */
	void InstantEvent(ETraceCategory Category, const FString& EventName);
	
	/** Traces a counter value for performance metrics */
	void TraceCounter(ETraceCategory Category, const FString& CounterName, double Value);
	
	/** Traces frame data for analysis and debugging */
	void TraceFrame(const FBeamFrame& Frame);
	
	/** Traces health status changes for system monitoring */
	void TraceHealth(EBeamHealth Health, const FString& Details = TEXT(""));
	
	/** Traces filter performance for optimization analysis */
	void TraceFilterPerformance(int32 FilterType, double ProcessingTimeMs);
	
	/** Exports trace data to CSV format for external analysis */
	bool ExportToCSV(const FString& FilePath, double StartTime, double EndTime);

private:
	/** Tracing system state and configuration */
	bool bEnabled = false;
	ETraceCategory TraceLevel = ETraceCategory::Polling;
	
	/** Active events for timing measurements */
	TMap<FString, double> ActiveEvents;
	
	/** CSV export data structure for data analysis */
	struct FCSVRow
	{
		double Timestamp;
		FString Category;
		FString EventName;
		FString EventType;
		double Value;
		FString Details;
	};
	
	TArray<FCSVRow> CSVData;
	
	/** Gets current timestamp in seconds for event timing */
	double GetCurrentTimestamp() const;
	
	/** Formats trace data row for CSV export */
	FString FormatCSVRow(const FCSVRow& Row) const;
	
	/** Writes CSV header with column names */
	void WriteCSVHeader(IFileHandle* File) const;
};

/**
 * RAII wrapper for trace events to ensure proper cleanup.
 * 
 * Automatically begins and ends trace events when created and destroyed,
 * ensuring that all trace events are properly closed even if exceptions occur.
 */
class BEAMEYETRACKER_API FBeamTraceEvent
{
public:
	FBeamTraceEvent(FBeamTrace* InTracer, FBeamTrace::ETraceCategory InCategory, const FString& InEventName);
	~FBeamTraceEvent();

private:
	FBeamTrace* Tracer;
	FBeamTrace::ETraceCategory Category;
	FString EventName;
};

/**
 * Trace macros for easy usage throughout the codebase.
 * 
 * These macros provide convenient access to tracing functionality
 * and automatically handle feature flag checking for Unreal Insights.
 */
#if BEAM_FEATURE_UNREAL_INSIGHTS

#define BEAM_TRACE_BEGIN(Category, EventName) \
	FBeamTraceEvent BEAM_TRACE_EVENT_##__LINE__(GBeamTracer, Category, EventName)

#define BEAM_TRACE_END(Category) \
	/* End event handled by RAII */

#define BEAM_TRACE_INSTANT(Category, EventName) \
	GBeamTracer->InstantEvent(Category, EventName)

#define BEAM_TRACE_COUNTER(Category, CounterName, Value) \
	GBeamTracer->TraceCounter(Category, CounterName, Value)

#else

#define BEAM_TRACE_BEGIN(Category, EventName)
#define BEAM_TRACE_END(Category)
#define BEAM_TRACE_INSTANT(Category, EventName)
#define BEAM_TRACE_COUNTER(Category, CounterName, Value)

#endif

/** Global tracer instance for easy access throughout the system */
extern BEAMEYETRACKER_API FBeamTrace* GBeamTracer;

/*=============================================================================
    End of BeamTrace.h
=============================================================================*/
