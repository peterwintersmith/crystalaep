// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]
// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "StdChecks.h"

namespace AllPolicies
{
	static char *rg_szConnectExports[1] = {
		"CheckInvalidUrlEncoding"
	};

	static char *rg_szResponseExports[6] = {
		"CheckPngFileValidity",
		"CheckJpegJFIFValidity",
		"CheckGifFileValidity",
		"CheckFlvFileValidityChunked",
		"CheckCrossSiteScriptingUrlParams",
		"CheckCrossSiteScriptingPostBody"
	};
}

extern "C" __declspec(dllexport) BOOL QueryFilters(
		char ***pppszConnectExports, int *pnConnectExports,
		char ***pppszRequestExports, int *pnRequestExports,
		char ***pppszResponseExports, int *pnResponseExports
	)
{
	*pppszConnectExports = AllPolicies::rg_szConnectExports;
	*pnConnectExports = 1;

	*pppszRequestExports = NULL;
	*pnRequestExports = 0;

	*pppszResponseExports = AllPolicies::rg_szResponseExports;
	*pnResponseExports = 6;

	return TRUE;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		StdChecks::Initialize();
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		StdChecks::UnInitialize();
		break;
	}
	return TRUE;
}

