// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]
// ProcTrackerClient.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "..\..\ProcTracker\ProcTracker\ProcTracker_h.h"
#include "ProcTrackerClient.h"

extern "C" void __RPC_FAR * __RPC_USER midl_user_allocate(size_t len)
{
	return(malloc(len));
}

extern "C" void __RPC_USER midl_user_free(void __RPC_FAR * ptr)
{
	free(ptr);
}

extern "C" DWORD DllGetEnabledFeaturesProcess(
	DWORD dwProcessID)
{
	DWORD dwResult = 0;

	RPC_WSTR pwszBinding = NULL;
	RPC_STATUS rpcStatus = RpcStringBindingCompose(NULL, (RPC_WSTR)L"ncalrpc", NULL, (RPC_WSTR)L"RealtimeLog", NULL, &pwszBinding);
	if(rpcStatus != RPC_S_OK)
		goto Done_RPC;
	
	rpcStatus = RpcBindingFromStringBinding(pwszBinding, &::IRealtimeLog_v1_0_c_ifspec);
	if(rpcStatus != RPC_S_OK)
		goto Done_RPC;
	
	RpcTryExcept
	{
		dwResult = GetEnabledFeaturesProcess(::IRealtimeLog_v1_0_c_ifspec, dwProcessID);
	}
	RpcExcept(1)
	{
	}
	RpcEndExcept
	
	rpcStatus = RpcStringFree(&pwszBinding);
	if(rpcStatus != RPC_S_OK)
		goto Done_RPC;
	
	rpcStatus = RpcBindingFree(&IRealtimeLog_v1_0_c_ifspec);
	if(rpcStatus != RPC_S_OK)
		goto Done_RPC;
	
Done_RPC:
	return dwResult;
}

extern "C" BOOL DllAddEntryProcess(
	DWORD dwProcessID,
	RealtimeLogEntry *pRtLogEnt)
{
	BOOL bResult = FALSE;

	RPC_WSTR pwszBinding = NULL;
	RPC_STATUS rpcStatus = RpcStringBindingCompose(NULL, (RPC_WSTR)L"ncalrpc", NULL, (RPC_WSTR)L"RealtimeLog", NULL, &pwszBinding);
	if(rpcStatus != RPC_S_OK)
		goto Done_RPC;
	
	rpcStatus = RpcBindingFromStringBinding(pwszBinding, &::IRealtimeLog_v1_0_c_ifspec);
	if(rpcStatus != RPC_S_OK)
		goto Done_RPC;
	
	RpcTryExcept
	{
		bResult = AddEntryProcess(::IRealtimeLog_v1_0_c_ifspec, dwProcessID, pRtLogEnt);
	}
	RpcExcept(1)
	{
	}
	RpcEndExcept
	
	rpcStatus = RpcStringFree(&pwszBinding);
	if(rpcStatus != RPC_S_OK)
		goto Done_RPC;
	
	rpcStatus = RpcBindingFree(&IRealtimeLog_v1_0_c_ifspec);
	if(rpcStatus != RPC_S_OK)
		goto Done_RPC;
	
Done_RPC:
	return bResult;
}

extern "C" DWORD DllGetProcessLogs(
	DWORD dwProcessID,
	DWORD cMaxLogEntries,
	RealtimeLogEntry rg_RtLogEnts[  ])
{
	DWORD dwResult = 0;

	RPC_WSTR pwszBinding = NULL;
	RPC_STATUS rpcStatus = RpcStringBindingCompose(NULL, (RPC_WSTR)L"ncalrpc", NULL, (RPC_WSTR)L"RealtimeLog", NULL, &pwszBinding);
	if(rpcStatus != RPC_S_OK)
		goto Done_RPC;
	
	rpcStatus = RpcBindingFromStringBinding(pwszBinding, &::IRealtimeLog_v1_0_c_ifspec);
	if(rpcStatus != RPC_S_OK)
		goto Done_RPC;
	
	RpcTryExcept
	{
		dwResult = GetProcessLogs(::IRealtimeLog_v1_0_c_ifspec, dwProcessID, cMaxLogEntries, rg_RtLogEnts);
	}
	RpcExcept(1)
	{
	}
	RpcEndExcept
	
	rpcStatus = RpcStringFree(&pwszBinding);
	if(rpcStatus != RPC_S_OK)
		goto Done_RPC;
	
	rpcStatus = RpcBindingFree(&IRealtimeLog_v1_0_c_ifspec);
	if(rpcStatus != RPC_S_OK)
		goto Done_RPC;
	
Done_RPC:
	return dwResult;
}

extern "C" DWORD DllGetProcessIDs( 
    DWORD cUniqueProcIDs,
    char rg_pszUniqueProcID[  ][ 512 ])
{
	DWORD dwResult = 0;

	RPC_WSTR pwszBinding = NULL;
	RPC_STATUS rpcStatus = RpcStringBindingCompose(NULL, (RPC_WSTR)L"ncalrpc", NULL, (RPC_WSTR)L"ProcTracker", NULL, &pwszBinding);
	if(rpcStatus != RPC_S_OK)
		goto Done_RPC;
	
	rpcStatus = RpcBindingFromStringBinding(pwszBinding, &::IProcTracker_v1_0_c_ifspec);
	if(rpcStatus != RPC_S_OK)
		goto Done_RPC;
	
	RpcTryExcept
	{
		dwResult = GetProcessIDs(::IProcTracker_v1_0_c_ifspec, cUniqueProcIDs, (unsigned char (*)[512])rg_pszUniqueProcID);
	}
	RpcExcept(1)
	{
	}
	RpcEndExcept
	
	rpcStatus = RpcStringFree(&pwszBinding);
	if(rpcStatus != RPC_S_OK)
		goto Done_RPC;
	
	rpcStatus = RpcBindingFree(&IProcTracker_v1_0_c_ifspec);
	if(rpcStatus != RPC_S_OK)
		goto Done_RPC;
	
Done_RPC:
	return dwResult;
}

BOOL ProcTrackerClient::Initialize()
{
	return TRUE;
}

BOOL ProcTrackerClient::UnInitialize()
{
	return TRUE;
}
