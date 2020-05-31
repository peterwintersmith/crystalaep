// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]
// ProcTracker.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "ProcTracker_h.h"
#include "UiFeedback.h"
#include "RealtimeLog.h"
#include "EventWriterRpc.h"
#include "..\..\Shared\Utils\Utils.h"
#include "ProcessTracker.h"

DWORD RPCListen_ProcTracker_ThreadProc(LPVOID)
{
	CProcessTracker *pProcTracker = CProcessTracker::GetInstance();
	pProcTracker->StartRPCServer();
	return 0;
}

DWORD RPCListen_UiFeedback_ThreadProc(LPVOID)
{
	CUiFeedback *pUiFeedback = CUiFeedback::GetInstance();
	pUiFeedback->StartRPCServer();
	return 0;
}

DWORD RPCListen_RealtimeLog_ThreadProc(LPVOID)
{
	CRealtimeLog *pRealtimeLog = CRealtimeLog::GetInstance();
	pRealtimeLog->StartRPCServer();
	return 0;
}

DWORD RPCListen_EventWriterRpc_ThreadProc(LPVOID)
{
	CEventWriterRpc *pEventWriter = CEventWriterRpc::GetInstance();
	pEventWriter->StartRPCServer();
	return 0;
}

#define PROT_FILE_DELETE_WAIT_MS	(15 * 1000)

DWORD ProtectFileDeleteProc(LPVOID)
{
	std::deque<std::wstring> dqFiles;
	WIN32_FIND_DATA w32fd = {0};
	std::wstring wstrProcTemp = Utils::GetRegInstallPath() + L"\\Temp\\",
		wstrProcFiles = wstrProcTemp + L"Proc_*.protect";

	do
	{
		Sleep(PROT_FILE_DELETE_WAIT_MS);
		
		while(!dqFiles.empty())
		{
			// delete files in queue
			std::wstring wstrFile = wstrProcTemp + dqFiles.at(0);
			dqFiles.pop_front();
			DeleteFile(wstrFile.c_str());
		}

		HANDLE hFindFile = FindFirstFile(wstrProcFiles.c_str(), &w32fd);
		if(hFindFile != INVALID_HANDLE_VALUE)
		{
			do
			{
				dqFiles.push_back(w32fd.cFileName);
			}
			while(FindNextFile(hFindFile, &w32fd) != FALSE);

			FindClose(hFindFile);
		}
	}
	while(TRUE);

	return 0;
}

BOOL IsServiceAlreadyStarted()
{
#define PROC_TRACKER_SVC_MUTEX_NAME	L"ProcTrackerMutex"
	
	HANDLE hMutex = CreateMutex(NULL, TRUE, PROC_TRACKER_SVC_MUTEX_NAME);
	
	if(hMutex)
	{
		DWORD dwLastError = GetLastError();
		CloseHandle(hMutex);

		switch(dwLastError)
		{
		case ERROR_ALREADY_EXISTS:
			return TRUE;

		default:
			return FALSE;
		}
	}

	return FALSE;
}

#define SET_HOOK_CHECKNEWPROC_WAIT_MS	(5 * 1000)

DWORD SetHookEventProc(LPVOID)
{
	DWORD dwExitCode = 1;
	PSECURITY_DESCRIPTOR pSecDesc = NULL;
	ULONG cbSecDesc = 0;
	SECURITY_ATTRIBUTES sa = {0};
	HHOOK hHook = NULL;
	std::list<HHOOK> listHooks;
	time_t secs = 0;

	// allow Everyone access
	if(ConvertStringSecurityDescriptorToSecurityDescriptor(L"D:(A;OICI;GA;;;WD)", SDDL_REVISION_1, &pSecDesc, &cbSecDesc) == FALSE)
		goto Cleanup;
	
	sa.bInheritHandle = FALSE;
	sa.lpSecurityDescriptor = pSecDesc;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);

	HANDLE hEvent = CreateEvent(&sa, FALSE, FALSE, L"ProcTrackerSetHookEvent");
	if(!hEvent)
		goto Cleanup;

	HMODULE hmCryModLdr = LoadLibraryEx(L"crymodldr.dll", 0, DONT_RESOLVE_DLL_REFERENCES);
	if(!hmCryModLdr)
		goto Cleanup;

	FARPROC pfnHookProc = GetProcAddress(hmCryModLdr, "_Hook_DebugProc@12");
	if(!pfnHookProc)
		goto Cleanup;

	secs = time(NULL);
	
	do
	{
		DWORD dwWaitResult = WaitForSingleObject(hEvent, SET_HOOK_CHECKNEWPROC_WAIT_MS);

		if(dwWaitResult == WAIT_OBJECT_0) // both valid results for us
		{
			hHook = SetWindowsHookEx(WH_DEBUG, (HOOKPROC)pfnHookProc, hmCryModLdr, NULL);
			if(hHook)
			{
				listHooks.push_back(hHook);
			}
		}
		
		if(time(NULL) - secs > 5)
		{
			// 30-second intervals
			while(!listHooks.empty())
			{
				UnhookWindowsHookEx(listHooks.back());
				listHooks.pop_back();
			}

			secs = time(NULL);
		}
	}
	while(TRUE);

	dwExitCode = 0;
Cleanup:
	if(pSecDesc)
		LocalFree(pSecDesc);

	return dwExitCode;
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	if(IsServiceAlreadyStarted())
		return 0;

	InitializeCriticalSection(&CProcessTracker::m_csCreateInst);
	InitializeCriticalSection(&CUiFeedback::m_csCreateInst);
	InitializeCriticalSection(&CRealtimeLog::m_csCreateInst);
	InitializeCriticalSection(&CEventWriterRpc::m_csCreateInst);

#ifndef DEBUG_BUILD
	std::wstring wstrInstallPath = Utils::GetRegInstallPath();
#else
	std::wstring wstrInstallPath = L"<path>";
#endif

	if(!wstrInstallPath.empty())
	{

		// move current directory into PATH variable so that it is not excluded from DLL search path
		std::wstring wstrPathVar, wstrCurDir;

		if(Utils::GetEnvVar(L"PATH", wstrPathVar))
		{
			if(Utils::GetCurDir(wstrCurDir))
			{
				if(wstrPathVar.size() < wstrCurDir.size() || wcsnicmp(wstrPathVar.c_str(), wstrCurDir.c_str(), wstrCurDir.size()))
				{
					Utils::SetEnvVar(L"PATH", wstrCurDir + L";" + wstrPathVar);
				}
			}
		}

		SetDllDirectory(wstrInstallPath.c_str());
	}

	CProcessTracker *pProcTracker = CProcessTracker::GetInstance();
	CUiFeedback *pUiFeedback = CUiFeedback::GetInstance();
	CRealtimeLog *pRealtimeLog = CRealtimeLog::GetInstance();
	CEventWriterRpc *pEventWriterRpc = CEventWriterRpc::GetInstance();

	DWORD dwExitCode = 0;
	
	HANDLE rghThreads[4] = {0}, hDelProtThd = NULL, hSetHookThd = NULL;

	//hSetHookThd = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SetHookEventProc, NULL, 0, NULL);
	hDelProtThd = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ProtectFileDeleteProc, NULL, 0, NULL);
	rghThreads[0] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RPCListen_ProcTracker_ThreadProc, NULL, 0, NULL);
	rghThreads[1] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RPCListen_UiFeedback_ThreadProc, NULL, 0, NULL);
	rghThreads[2] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RPCListen_RealtimeLog_ThreadProc, NULL, 0, NULL);
	rghThreads[3] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RPCListen_EventWriterRpc_ThreadProc, NULL, 0, NULL);

	DWORD dwWaitResult = WaitForMultipleObjects(4, rghThreads, TRUE, INFINITE); // waitforall as one will return rpc-server-already-listening

	switch(dwWaitResult)
	{
	case WAIT_OBJECT_0:
		break;
	default:
		pProcTracker->StopRPCServer();
		pUiFeedback->StopRPCServer();
		pRealtimeLog->StopRPCServer();
		pEventWriterRpc->StopRPCServer();

		dwExitCode = 1;
		break;
	}

	CloseHandle(rghThreads[0]);
	CloseHandle(rghThreads[1]);
	CloseHandle(rghThreads[2]);
	CloseHandle(rghThreads[3]);
	CloseHandle(hDelProtThd);
	//CloseHandle(hSetHookThd);
	
	return dwExitCode;
}