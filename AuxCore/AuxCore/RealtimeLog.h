// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#pragma once

#include "RealtimeLogBase.h"

#define REALTIME_LOG_MAX_QUEUED_MSGS	1000
#define QUEUE_POLL_INTERVAL_MILLIS	(2 * 1000) // two secs

class CRealtimeLog : public CRealtimeLogBase {
	friend BOOL AuxCore::Initialize();
public:
	static CRealtimeLog* GetInstance();
	
	BOOL Start();
	BOOL LogMessage(DWORD dwSeverity, LPWSTR lpwszMessage, ...);
	void Terminate();
	
private:
	CRealtimeLog() : m_bTerminate(FALSE), m_hThread(NULL) {
		InitializeCriticalSection(&m_csQueue);
		// don't create queue thread here as can't trust this pointer inside a constructor (if I recall)
	}

	static DWORD WINAPI QueueThreadProc(LPVOID lpvParam);

	static CRealtimeLog *m_Instance;
	static CRITICAL_SECTION m_csCreateInst, m_csQueue;
	
	std::list<RealtimeLogEntry> m_listQueue;
	volatile BOOL m_bTerminate;
	HANDLE m_hThread;
};