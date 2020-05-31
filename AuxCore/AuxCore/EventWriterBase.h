// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#pragma once

// CEventWriter virtual base class (identical to interface)

enum EEventWriteStatus {
	StatusSuccess = 0,
	StatusFailure
};

enum EEventRepository {
	AntiSpray,
	ApiFirewall,
	AudioValidator,
	ConnectionMonitor,
	DfnsInDepth,
	HeapMonitor,
	ImageValidator,
	StackMonitor,
	VideoValidator,
	WebAppMonitor
};

enum EThreatRiskRating {
	Information = 0,
	LowRisk,
	MediumRisk,
	HighRisk
};

struct CEventWriterBase {

	virtual EEventWriteStatus WriteEvent(
			EEventRepository repos,
			EThreatRiskRating risk,
			LPWSTR pwszTitle,
			LPWSTR pwszBodyFormat,
			...
		) = 0;
	
	virtual EEventWriteStatus WriteEventProcess(
			HANDLE hProcess,
			LPWSTR pwszProcessName,
			EEventRepository repos,
			EThreatRiskRating risk,
			LPWSTR pwszTitle,
			LPWSTR pwszBodyFormat,
			...
		) = 0;
};