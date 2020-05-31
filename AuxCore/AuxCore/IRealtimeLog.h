// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#pragma once

// CProcTrackerServices Interface Class

struct IRealtimeLog;

#define CRYSTAL_LATE_BINDING

#ifdef CRYSTAL_LATE_BINDING
#ifdef DEBUG_BUILD
extern "C" static IRealtimeLog *(*GetRealtimeLog)() = (IRealtimeLog *(*)())GetProcAddress(
		GetModuleHandle(L"C:\\Work\\crystal\\BP\\Impl\\AuxCore\\Debug\\auxcore.dll") ?
			GetModuleHandle(L"C:\\Work\\crystal\\BP\\Impl\\AuxCore\\Debug\\auxcore.dll") :
			LoadLibrary(L"C:\\Work\\crystal\\BP\\Impl\\AuxCore\\Debug\\auxcore.dll"), 
		"GetRealtimeLog"
	);
#else
extern "C" static IRealtimeLog *(*GetRealtimeLog)() = (IRealtimeLog *(*)())GetProcAddress(
		GetModuleHandle(L"auxcore.dll") ?
			GetModuleHandle(L"auxcore.dll") :
			LoadLibrary(L"auxcore.dll"), 
		"GetRealtimeLog"
	);
#endif
#else
extern "C" __declspec(dllimport) IRealtimeLog *GetRealtimeLog();
#endif

struct IRealtimeLog {
	virtual BOOL Start() = 0;
	virtual BOOL LogMessage(DWORD dwSeverity, LPWSTR lpwszMessage, ...) = 0;
	virtual void Terminate() = 0;
};