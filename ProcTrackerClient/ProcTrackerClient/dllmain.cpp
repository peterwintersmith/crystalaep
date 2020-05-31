// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]
// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "..\..\ProcTracker\ProcTracker\ProcTracker_h.h"
#include "ProcTrackerClient.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		ProcTrackerClient::Initialize();
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		ProcTrackerClient::UnInitialize();
		break;
	}
	return TRUE;
}

