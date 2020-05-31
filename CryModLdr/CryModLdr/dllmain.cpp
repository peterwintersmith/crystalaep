// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]
// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "CryModLdr.h"

VOID CALLBACK APCProc(
  __in  ULONG_PTR dwParam
)
{
	CrystalModuleLoader::Initialize(TRUE);
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	HANDLE hThread = NULL, hEvent = NULL;
	
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:

		// this load has to be delayed as calling LoadLibrary from DllMain is illegal (loader lock). Creating a thread is frowned upon too.
		
		//InitializeCriticalSection(&g_csInit);

		//hEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, L"ProcTrackerSetHookEvent");
		//if(hEvent)
		//{
			// force hook in self (and other desktop procs)
			//SetEvent(hEvent);
		//	CloseHandle(hEvent);
		//}
		
		//hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)CrystalModuleLoader::Initialize, (LPVOID)TRUE, 0, NULL);
		//if(hThread)
		//	CloseHandle(hThread);

		QueueUserAPC(APCProc, GetCurrentThread(), NULL);

		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		CrystalModuleLoader::UnInitialize();

		break;
	}
	return TRUE;
}