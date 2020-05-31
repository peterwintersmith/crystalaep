// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#pragma once

/*

typedef struct
{
	DWORD dwSeverity;
	[string] WCHAR wszMessage[256];
} RealtimeLogEntry;

enum EEnabledFeatures
{
	InvalidFeature = 0,
	ContentFilterFeature = 1,
	APIMonitorFeature = 2,
	ROPMonitorFeature = 4,
	AntiSprayFeature  = 8,
	StackMonitorFeature = 16,
	HeapMonitorFeature  = 32
};

*/

#define REALTIME_LOG_MAX_QUEUED_MSGS	1000

class CRealtimeLog {
	friend int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow);
public:
	static CRealtimeLog *GetInstance();

	RPC_STATUS StartRPCServer();
	RPC_STATUS StopRPCServer();

	BOOL SetEnabledFeaturesProcess(DWORD dwProcessID, DWORD dwFeaturesMask);
	DWORD GetEnabledFeaturesProcess(DWORD dwProcessID);

	BOOL AddEntryProcess(DWORD dwProcessID, RealtimeLogEntry *pRtLogEnt);
	DWORD GetProcessLogs(DWORD dwProcessID, DWORD cMaxLogEntries, RealtimeLogEntry rg_RtLogEnts[]);
	
	BOOL RemoveProcess(DWORD dwProcessID);

private:
	CRealtimeLog() {
		InitializeCriticalSection(&m_csLog);
	}

	RealtimeLogEntry *CopyLogEntry(RealtimeLogEntry *p);
	void DeleteLogEntry(RealtimeLogEntry *p);

	static CRITICAL_SECTION m_csCreateInst, m_csLog;
	static CRealtimeLog *m_Instance;

	std::map<DWORD, std::list<RealtimeLogEntry *> *> m_mapLog;
	std::map<DWORD, DWORD> m_mapEnabledFeatures;
};