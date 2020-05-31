// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]
// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "DiDCore.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		DiDCore::Initialize();
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		DiDCore::UnInitialize();
		break;
	}
	return TRUE;
}

