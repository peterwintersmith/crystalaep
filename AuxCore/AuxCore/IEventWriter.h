// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#pragma once

// CEventWriter Interface Class

struct IEventWriter;

#define CRYSTAL_LATE_BINDING

#ifdef CRYSTAL_LATE_BINDING
#ifdef DEBUG_BUILD
extern "C" static IEventWriter *(*GetEventWriter)() = (IEventWriter *(*)())GetProcAddress(
		GetModuleHandle(L"C:\\Work\\crystal\\BP\\Impl\\AuxCore\\Debug\\auxcore.dll") ?
			GetModuleHandle(L"C:\\Work\\crystal\\BP\\Impl\\AuxCore\\Debug\\auxcore.dll") :
			LoadLibrary(L"C:\\Work\\crystal\\BP\\Impl\\AuxCore\\Debug\\auxcore.dll"), 
		"GetEventWriter"
	);
#else
extern "C" static IEventWriter *(*GetEventWriter)() = (IEventWriter *(*)())GetProcAddress(
		GetModuleHandle(L"auxcore.dll") ?
			GetModuleHandle(L"auxcore.dll") :
			LoadLibrary(L"auxcore.dll"), 
		"GetEventWriter"
	);
#endif
#else
extern "C" __declspec(dllimport) IEventWriter *GetEventWriter();
#endif

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

struct IEventWriter {

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