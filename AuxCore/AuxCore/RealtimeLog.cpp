// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]
#include "stdafx.h"
#include "AuxCore.h"
#include "..\..\ProcTracker\ProcTracker\ProcTracker_h.h"
#include "ProcTrackerServices.h"
#include "RealtimeLog.h"

CRITICAL_SECTION CRealtimeLog::m_csCreateInst = {0}, CRealtimeLog::m_csQueue = {0};
CRealtimeLog *CRealtimeLog::m_Instance = NULL;

CRealtimeLog* CRealtimeLog::GetInstance()
{
	if(m_Instance == NULL)
	{
		EnterCriticalSection(&m_csCreateInst);

		if(m_Instance == NULL)
		{
			m_Instance = new CRealtimeLog();
		}

		LeaveCriticalSection(&m_csCreateInst);
	}

	return m_Instance;
}

BOOL CRealtimeLog::LogMessage(DWORD dwSeverity, LPWSTR lpwszMessage, ...)
{
	BOOL bSuccess = FALSE;
	RealtimeLogEntry rt = {0, L""};
	WCHAR wszFormatted[256];

	if(m_hThread == NULL)
		return TRUE;

	// format
	{
		va_list ap;
		va_start(ap, lpwszMessage);
		_vsnwprintf(wszFormatted, 255, lpwszMessage, ap);
		// no need to call va_end?
		// va_end(ap);
		wszFormatted[255] = '\0';

		for(DWORD i=0; wszFormatted[i] != '\0'; i++)
		{
			if(wszFormatted[i] == '\r' || wszFormatted[i] == '\n')
			{
				wszFormatted[i] = ' ';
			}
		}
	}

	rt.dwSeverity = dwSeverity;
	wcsncpy(rt.wszMessage, wszFormatted, 255);
	rt.wszMessage[255] = '\0';

	EnterCriticalSection(&m_csQueue);

	// let's only permit n queued messages per process at any one time
	if(m_listQueue.size() > REALTIME_LOG_MAX_QUEUED_MSGS)
	{
		m_listQueue.pop_back();
	}

	m_listQueue.push_front(rt);

//Cleanup:
	LeaveCriticalSection(&m_csQueue);

	return bSuccess;
}

BOOL CRealtimeLog::Start()
{
	BOOL bSuccess = FALSE;
	DWORD dwTID = 0;

	EnterCriticalSection(&m_csQueue);

	if(m_hThread)
	{
		// started already
		bSuccess = TRUE;
		goto Cleanup;
	}

	m_hThread = CreateThread(NULL, 0, QueueThreadProc, (LPVOID)'AUXR', 0, &dwTID);
	bSuccess = (m_hThread != NULL);
	
Cleanup:
	LeaveCriticalSection(&m_csQueue);

	return bSuccess;
}

void CRealtimeLog::Terminate()
{
	if(!m_hThread || !this)
		return;

	m_bTerminate = TRUE;
	
	TerminateThread(m_hThread, 0);
	/*
	DWORD dwWaitStatus = WaitForSingleObject(m_hThread, 2 * QUEUE_POLL_INTERVAL_MILLIS);

	switch(dwWaitStatus)
	{
	case WAIT_OBJECT_0:
	case WAIT_TIMEOUT:
		CloseHandle(m_hThread);
		m_hThread = NULL;
		break;
	default:
		break;
	}
	*/
}

DWORD WINAPI CRealtimeLog::QueueThreadProc(LPVOID lpvParam)
{
	CRealtimeLog *pRealtimeLog = CRealtimeLog::GetInstance(); //reinterpret_cast<CRealtimeLog *>(lpvParam);
	CProcTrackerServices *pProcTracker = CProcTrackerServices::GetInstance();
	
	if(!pRealtimeLog || !pProcTracker)
		return 0;

	while(!pRealtimeLog->m_bTerminate)
	{
		// poll queue
		EnterCriticalSection(&pRealtimeLog->m_csQueue);

		// submit new messages
		
		while(pRealtimeLog->m_listQueue.size() != 0)
		{
			RealtimeLogEntry rt = pRealtimeLog->m_listQueue.back();
			pRealtimeLog->m_listQueue.pop_back();
			pProcTracker->AddEntryProcess(GetCurrentProcessId(), &rt);
		}

		LeaveCriticalSection(&pRealtimeLog->m_csQueue);

		// wait; repeat
		Sleep(QUEUE_POLL_INTERVAL_MILLIS);
	}

	return 0;
}

extern "C" __declspec(dllexport) CRealtimeLogBase *GetRealtimeLog()
{
	return reinterpret_cast<CRealtimeLogBase *>(CRealtimeLog::GetInstance());
}