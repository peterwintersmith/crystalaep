// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]
#include "stdafx.h"
#include "..\..\ProcTracker\ProcTracker\ProcTracker_h.h"
#include "ProcTrackerServices.h"

CRITICAL_SECTION CProcTrackerServices::m_csCreateInst = {0};
CProcTrackerServices *CProcTrackerServices::m_Instance = NULL;

extern "C" void __RPC_FAR * __RPC_USER midl_user_allocate(size_t len)
{
	return(malloc(len));
}

extern "C" void __RPC_USER midl_user_free(void __RPC_FAR * ptr)
{
	free(ptr);
}

CProcTrackerServices* CProcTrackerServices::GetInstance()
{
	if(m_Instance == NULL)
	{
		EnterCriticalSection(&m_csCreateInst);

		if(m_Instance == NULL)
		{
			m_Instance = new CProcTrackerServices();
		}

		LeaveCriticalSection(&m_csCreateInst);
	}

	return m_Instance;
}

// IProcTracker
BOOL CProcTrackerServices::AddProcess(DWORD dwProcessID)
{
	BOOL bResult = 0;

	RPC_WSTR pwszBinding = NULL;
	RPC_STATUS rpcStatus = RpcStringBindingCompose(NULL, (RPC_WSTR)L"ncalrpc", NULL, (RPC_WSTR)L"ProcTracker", NULL, &pwszBinding);
	if(rpcStatus != RPC_S_OK)
		goto Done_RPC;
	
	rpcStatus = RpcBindingFromStringBinding(pwszBinding, &::IProcTracker_v1_0_c_ifspec);
	if(rpcStatus != RPC_S_OK)
		goto Done_RPC;
	
	RpcTryExcept
	{
		bResult = ::AddProcess(::IProcTracker_v1_0_c_ifspec, dwProcessID);
	}
	RpcExcept(1)
	{
	}
	RpcEndExcept
	
	rpcStatus = RpcStringFree(&pwszBinding);
	if(rpcStatus != RPC_S_OK)
		goto Done_RPC;
	
	rpcStatus = RpcBindingFree(&::IProcTracker_v1_0_c_ifspec);
	if(rpcStatus != RPC_S_OK)
		goto Done_RPC;
	
Done_RPC:
	return bResult;
}

// IUiFeedback
BOOL CProcTrackerServices::UserBoolFeedback(UINT style, WCHAR *pwszTitle, WCHAR *pwszCaption)
{
	BOOL bResult = FALSE;

	RPC_WSTR pwszBinding = NULL;
	RPC_STATUS rpcStatus = RpcStringBindingCompose(NULL, (RPC_WSTR)L"ncalrpc", NULL, (RPC_WSTR)L"UiFeedback", NULL, &pwszBinding);
	if(rpcStatus != RPC_S_OK)
		goto Done_RPC;
	
	rpcStatus = RpcBindingFromStringBinding(pwszBinding, &::IUiFeedback_v1_0_c_ifspec);
	if(rpcStatus != RPC_S_OK)
		goto Done_RPC;
	
	RpcTryExcept
	{
		bResult = ::UserBoolFeedback(::IUiFeedback_v1_0_c_ifspec, style, pwszTitle, pwszCaption);
	}
	RpcExcept(1)
	{
	}
	RpcEndExcept
	
	rpcStatus = RpcStringFree(&pwszBinding);
	if(rpcStatus != RPC_S_OK)
		goto Done_RPC;
	
	rpcStatus = RpcBindingFree(&::IUiFeedback_v1_0_c_ifspec);
	if(rpcStatus != RPC_S_OK)
		goto Done_RPC;
	
Done_RPC:
	return bResult;
}

// IRealtimeLog
BOOL CProcTrackerServices::SetEnabledFeaturesProcess(DWORD dwProcessID, DWORD dwFeaturesMask)
{
	BOOL bResult = 0;

	RPC_WSTR pwszBinding = NULL;
	RPC_STATUS rpcStatus = RpcStringBindingCompose(NULL, (RPC_WSTR)L"ncalrpc", NULL, (RPC_WSTR)L"RealtimeLog", NULL, &pwszBinding);
	if(rpcStatus != RPC_S_OK)
		goto Done_RPC;
	
	rpcStatus = RpcBindingFromStringBinding(pwszBinding, &::IRealtimeLog_v1_0_c_ifspec);
	if(rpcStatus != RPC_S_OK)
		goto Done_RPC;
	
	RpcTryExcept
	{
		bResult = ::SetEnabledFeaturesProcess(::IRealtimeLog_v1_0_c_ifspec, dwProcessID, dwFeaturesMask);
	}
	RpcExcept(1)
	{
	}
	RpcEndExcept
	
	rpcStatus = RpcStringFree(&pwszBinding);
	if(rpcStatus != RPC_S_OK)
		goto Done_RPC;
	
	rpcStatus = RpcBindingFree(&::IRealtimeLog_v1_0_c_ifspec);
	if(rpcStatus != RPC_S_OK)
		goto Done_RPC;
	
Done_RPC:
	return bResult;
}

DWORD CProcTrackerServices::GetEnabledFeaturesProcess(DWORD dwProcessID)
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
		dwResult = ::GetEnabledFeaturesProcess(::IRealtimeLog_v1_0_c_ifspec, dwProcessID);
	}
	RpcExcept(1)
	{
	}
	RpcEndExcept
	
	rpcStatus = RpcStringFree(&pwszBinding);
	if(rpcStatus != RPC_S_OK)
		goto Done_RPC;
	
	rpcStatus = RpcBindingFree(&::IRealtimeLog_v1_0_c_ifspec);
	if(rpcStatus != RPC_S_OK)
		goto Done_RPC;
	
Done_RPC:
	return dwResult;
}

BOOL CProcTrackerServices::AddEntryProcess(DWORD dwProcessID, RealtimeLogEntry *pRtLogEnt)
{
	BOOL bResult = 0;

	RPC_WSTR pwszBinding = NULL;
	RPC_STATUS rpcStatus = RpcStringBindingCompose(NULL, (RPC_WSTR)L"ncalrpc", NULL, (RPC_WSTR)L"RealtimeLog", NULL, &pwszBinding);
	if(rpcStatus != RPC_S_OK)
		goto Done_RPC;
	
	rpcStatus = RpcBindingFromStringBinding(pwszBinding, &::IRealtimeLog_v1_0_c_ifspec);
	if(rpcStatus != RPC_S_OK)
		goto Done_RPC;
	
	RpcTryExcept
	{
		bResult = ::AddEntryProcess(::IRealtimeLog_v1_0_c_ifspec, dwProcessID, pRtLogEnt);
	}
	RpcExcept(1)
	{
	}
	RpcEndExcept
	
	rpcStatus = RpcStringFree(&pwszBinding);
	if(rpcStatus != RPC_S_OK)
		goto Done_RPC;
	
	rpcStatus = RpcBindingFree(&::IRealtimeLog_v1_0_c_ifspec);
	if(rpcStatus != RPC_S_OK)
		goto Done_RPC;
	
Done_RPC:
	return bResult;
}

extern "C" __declspec(dllexport) CProcTrackerServicesBase *GetProcTrackerServices()
{
	return reinterpret_cast<CProcTrackerServicesBase *>(CProcTrackerServices::GetInstance());
}