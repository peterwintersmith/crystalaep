// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#include "stdafx.h"
#include "ProcTracker_h.h"
#include "RealtimeLog.h"

CRITICAL_SECTION CRealtimeLog::m_csCreateInst = {0}, CRealtimeLog::m_csLog = {0};
CRealtimeLog *CRealtimeLog::m_Instance = NULL;

CRealtimeLog *CRealtimeLog::GetInstance()
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

RPC_STATUS CRealtimeLog::StartRPCServer()
{
	RPC_STATUS rpcStatus;
	PSECURITY_DESCRIPTOR pSecDesc = NULL;
	ULONG cbSecDesc = 0;
	
	// allow Everyone access
	if(ConvertStringSecurityDescriptorToSecurityDescriptor(L"D:(A;OICI;GA;;;WD)", SDDL_REVISION_1, &pSecDesc, &cbSecDesc) == FALSE)
	{
		rpcStatus = RPC_S_INVALID_SECURITY_DESC;
		goto Cleanup;
	}
	
	rpcStatus = RpcServerUseProtseqEp(
			(RPC_WSTR)L"ncalrpc",
			RPC_C_LISTEN_MAX_CALLS_DEFAULT,
			(RPC_WSTR)L"RealtimeLog",
			pSecDesc
		);
	if(rpcStatus != RPC_S_OK)
		goto Cleanup;

	rpcStatus = RpcServerRegisterIf(IRealtimeLog_v1_0_s_ifspec, NULL, NULL);
	if(rpcStatus != RPC_S_OK)
		goto Cleanup;

	rpcStatus = RpcServerListen(1, RPC_C_LISTEN_MAX_CALLS_DEFAULT, FALSE);
	
	// server will be ongoing

Cleanup:
	if(pSecDesc)
		LocalFree(pSecDesc);

	return rpcStatus;
}

RPC_STATUS CRealtimeLog::StopRPCServer()
{
	RPC_STATUS rpcStatus;
	
	rpcStatus = RpcMgmtStopServerListening(IRealtimeLog_v1_0_s_ifspec);
	if(rpcStatus != RPC_S_OK)
		goto Cleanup;

	rpcStatus = RpcServerUnregisterIf(IRealtimeLog_v1_0_s_ifspec, NULL, TRUE);

Cleanup:
	return rpcStatus;
}

RealtimeLogEntry *CRealtimeLog::CopyLogEntry(RealtimeLogEntry *p)
{
	RealtimeLogEntry *pRtCopy = new RealtimeLogEntry;

	pRtCopy->dwSeverity = p->dwSeverity;
	wcsncpy(pRtCopy->wszMessage, p->wszMessage, 255);
	pRtCopy->wszMessage[255] = '\0';

	return pRtCopy;
}

void CRealtimeLog::DeleteLogEntry(RealtimeLogEntry *p)
{
	delete p;
}

BOOL CRealtimeLog::RemoveProcess(DWORD dwProcessID)
{
	BOOL bSuccess = FALSE;
	
	std::list<RealtimeLogEntry *> *p_listLog = NULL;

	EnterCriticalSection(&m_csLog);

	if(m_mapLog.find(dwProcessID) == m_mapLog.end())
		goto Cleanup;
	
	p_listLog = m_mapLog[dwProcessID];

	while(p_listLog->size() != 0)
	{
		RealtimeLogEntry *pRtLogEnt = p_listLog->back();
		if(pRtLogEnt)
			DeleteLogEntry(pRtLogEnt);

		p_listLog->pop_back();
	}

	delete m_mapLog[dwProcessID];
	m_mapLog.erase(dwProcessID);

	m_mapEnabledFeatures.erase(dwProcessID);

	bSuccess = TRUE;
Cleanup:
	LeaveCriticalSection(&m_csLog);

	return bSuccess;
}

BOOL CRealtimeLog::SetEnabledFeaturesProcess(DWORD dwProcessID, DWORD dwFeaturesMask)
{
	BOOL bSuccess = FALSE;
	
	// lock for sync with RemoveProcess()
	EnterCriticalSection(&m_csLog);

	// this can be overwritten as necessary to reflect enabling/disabling of a feature
	m_mapEnabledFeatures[dwProcessID] = dwFeaturesMask;

	bSuccess = TRUE;
Cleanup:
	LeaveCriticalSection(&m_csLog);

	return bSuccess;
}

DWORD CRealtimeLog::GetEnabledFeaturesProcess(DWORD dwProcessID)
{
	DWORD dwFeatures = (DWORD)EEnabledFeatures::InvalidFeature;

	if(m_mapEnabledFeatures.find(dwProcessID) == m_mapEnabledFeatures.end())
	{
		goto Cleanup;
	}

	dwFeatures = m_mapEnabledFeatures[dwProcessID];

Cleanup:
	return dwFeatures;
}

BOOL CRealtimeLog::AddEntryProcess(DWORD dwProcessID, RealtimeLogEntry *pRtLogEnt)
{
	BOOL bSuccess = FALSE;
	
	EnterCriticalSection(&m_csLog);

	if(m_mapLog.find(dwProcessID) == m_mapLog.end())
	{
		m_mapLog[dwProcessID] = new std::list<RealtimeLogEntry *>();
	}

	std::list<RealtimeLogEntry *> *p_listLog = m_mapLog[dwProcessID];

	if(p_listLog->size() > REALTIME_LOG_MAX_QUEUED_MSGS)
	{
		RealtimeLogEntry *pRtBack = p_listLog->back();
		if(pRtBack)
			DeleteLogEntry(pRtBack);
		
		p_listLog->pop_back();
	}

	RealtimeLogEntry *pRtCopy = CopyLogEntry(pRtLogEnt);
	if(!pRtCopy)
		goto Cleanup;

	p_listLog->push_front(pRtCopy);

	bSuccess = TRUE;
Cleanup:
	LeaveCriticalSection(&m_csLog);

	return bSuccess;
}

DWORD CRealtimeLog::GetProcessLogs(DWORD dwProcessID, DWORD cMaxLogEntries, RealtimeLogEntry rg_RtLogEnts[])
{
	DWORD dwCount = 0;
	
	std::list<RealtimeLogEntry *> *p_listLog = NULL;

	EnterCriticalSection(&m_csLog);

	if(m_mapLog.find(dwProcessID) == m_mapLog.end())
		goto Cleanup;
	
	p_listLog = m_mapLog[dwProcessID];

	for(DWORD i=0; i < cMaxLogEntries && p_listLog->size() != 0; i++)
	{
		rg_RtLogEnts[i] = *p_listLog->back();
		p_listLog->pop_back();
		
		dwCount++;
	}

Cleanup:
	LeaveCriticalSection(&m_csLog);

	return dwCount;
}

BOOL SetEnabledFeaturesProcess(
	/* [in] */ handle_t IDL_handle,
	DWORD dwProcessID,
	DWORD dwFeaturesMask)
{
	CRealtimeLog *pRealtimeLog = CRealtimeLog::GetInstance();
	return pRealtimeLog->SetEnabledFeaturesProcess(dwProcessID, dwFeaturesMask);
}

DWORD GetEnabledFeaturesProcess(
	/* [in] */ handle_t IDL_handle,
	DWORD dwProcessID)
{
	CRealtimeLog *pRealtimeLog = CRealtimeLog::GetInstance();
	return pRealtimeLog->GetEnabledFeaturesProcess(dwProcessID);
}

BOOL AddEntryProcess(
	/* [in] */ handle_t IDL_handle,
	DWORD dwProcessID,
	RealtimeLogEntry *pRtLogEnt)
{
	CRealtimeLog *pRealtimeLog = CRealtimeLog::GetInstance();
	return pRealtimeLog->AddEntryProcess(dwProcessID, pRtLogEnt);
}

DWORD GetProcessLogs(
	/* [in] */ handle_t IDL_handle,
	DWORD dwProcessID,
	DWORD cMaxLogEntries,
	RealtimeLogEntry rg_RtLogEnts[])
{
	CRealtimeLog *pRealtimeLog = CRealtimeLog::GetInstance();
	return pRealtimeLog->GetProcessLogs(dwProcessID, cMaxLogEntries, rg_RtLogEnts);
}