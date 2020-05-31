// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#pragma once

// CProcTrackerServices Interface Class

struct IProcTrackerServices;

#define CRYSTAL_LATE_BINDING

#ifdef CRYSTAL_LATE_BINDING
#ifdef DEBUG_BUILD
extern "C" static IProcTrackerServices *(*GetProcTrackerServices)() = (IProcTrackerServices *(*)())GetProcAddress(
		GetModuleHandle(L"C:\\Work\\crystal\\BP\\Impl\\AuxCore\\Debug\\auxcore.dll") ?
			GetModuleHandle(L"C:\\Work\\crystal\\BP\\Impl\\AuxCore\\Debug\\auxcore.dll") :
			LoadLibrary(L"C:\\Work\\crystal\\BP\\Impl\\AuxCore\\Debug\\auxcore.dll"), 
		"GetProcTrackerServices"
	);
#else
extern "C" static IProcTrackerServices *(*GetProcTrackerServices)() = (IProcTrackerServices *(*)())GetProcAddress(
		GetModuleHandle(L"auxcore.dll") ?
			GetModuleHandle(L"auxcore.dll") :
			LoadLibrary(L"auxcore.dll"), 
		"GetProcTrackerServices"
	);
#endif
#else
extern "C" __declspec(dllimport) IProcTrackerServices *GetProcTrackerServices();
#endif

struct IProcTrackerServices {
	// IProcTracker
	virtual BOOL AddProcess(DWORD dwProcessID) = 0;

	// IUiFeedback
	virtual BOOL UserBoolFeedback(UINT style, WCHAR *pwszTitle, WCHAR *pwszCaption) = 0;

	// IRealtimeLog
	virtual BOOL SetEnabledFeaturesProcess(DWORD dwProcessID, DWORD dwFeaturesMask) = 0;
	virtual DWORD GetEnabledFeaturesProcess(DWORD dwProcessID) = 0;
	virtual BOOL AddEntryProcess(DWORD dwProcessID, PVOID pRtLogEnt) = 0;
};