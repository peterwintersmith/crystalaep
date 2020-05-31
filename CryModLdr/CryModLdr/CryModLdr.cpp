// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]
// CryModLdr.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "MinUtils.h"
#include "..\..\AuxCore\AuxCore\IConfigReader.h"
#include "..\..\AuxCore\AuxCore\ILoadedModules.h"
#include "..\..\ProcTracker\ProcTracker\ProcTracker_h.h"
#include "CryModLdr.h"

extern "C" void __RPC_FAR * __RPC_USER midl_user_allocate(size_t len)
{
	return(malloc(len));
}

extern "C" void __RPC_USER midl_user_free(void __RPC_FAR * ptr)
{
	free(ptr);
}

BOOL CheckProtectFileProcess(DWORD dwPID)
{
	BOOL bSuccess = FALSE;
	
	RPC_WSTR pwszBinding = NULL;
	RPC_STATUS rpcStatus = RpcStringBindingCompose(NULL, (RPC_WSTR)L"ncalrpc", NULL, (RPC_WSTR)L"ProcTracker", NULL, &pwszBinding);
	if(rpcStatus != RPC_S_OK)
		goto Done_RPC;
	
	rpcStatus = RpcBindingFromStringBinding(pwszBinding, &::IProcTracker_v1_0_c_ifspec);
	if(rpcStatus != RPC_S_OK)
		goto Done_RPC;
	
	RpcTryExcept
	{
		bSuccess = ::CheckProtectFileProcess(IProcTracker_v1_0_c_ifspec, dwPID);
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
	return bSuccess;
}

BOOL CrystalModuleLoader::Initialize(BOOL bUnloadSelf)
{
	BOOL bResult = FALSE, bCanUnloadAuxCore = FALSE, bProtect = FALSE;
	HMODULE hmAuxCore = NULL;
	
	std::wstring wstrInstallPath, wstrConfigPath, wstrProcFile, wstrPathVar, wstrCurDir;

#ifdef DEBUG_BUILD
	wstrInstallPath = L"C:\Work\crystal\BP\Impl\BpCore\Debug";
#else
	wstrInstallPath = MinUtils::GetRegInstallPath();
#endif
	
	//MessageBoxW(0,wstrInstallPath.c_str(),L"Install Path",0);
	
	if(wstrInstallPath.empty())
	{
		goto Cleanup;
	}

	// move current directory into PATH variable so that it is not excluded from DLL search path
	
	if(MinUtils::GetEnvVar(L"PATH", wstrPathVar))
	{
		if(MinUtils::GetCurDir(wstrCurDir))
		{
			if(wstrPathVar.size() < wstrCurDir.size() || wcsnicmp(wstrPathVar.c_str(), wstrCurDir.c_str(), wstrCurDir.size()))
			{
				MinUtils::SetEnvVar(L"PATH", wstrCurDir + L";" + wstrPathVar);
			}
		}
	}
	
	if(!SetDllDirectory(wstrInstallPath.c_str()))
	{
		// cannot set BpCore directory as DLL directory - other modules probably won't load correctly
		goto Cleanup;
	}
	
	// now check whether the application in which this binary is executing is contained within the
	// (protected) processList config list.
	ILoadedModules *pLoadedMods = NULL;

	if(!bProtect)
	{
		hmAuxCore = LoadLibrary(L"AuxCore.dll"); // DONT_RESOLVE_DLL_REFERENCES);
		if(!hmAuxCore)
			goto Cleanup;

		BOOL bSuccess = FALSE;

		IConfigReader *pConfigReader = NULL;
		IConfigReader *(*pfnGetConfigReader)() = (IConfigReader *(*)())GetProcAddress(hmAuxCore, "GetConfigReader");
	
		if(pfnGetConfigReader)
		{
			pConfigReader = pfnGetConfigReader();
			if(pConfigReader)
			{
				bSuccess = TRUE;
			}
		}

		//ILoadedModules *pLoadedMods = NULL;
		ILoadedModules *(*pfnGetLoadedModules)() = (ILoadedModules *(*)())GetProcAddress(hmAuxCore, "GetLoadedModules");
	
		if(pfnGetLoadedModules)
		{
			pLoadedMods = pfnGetLoadedModules();

			//if(pLoadedMods)
			//	pLoadedMods->Start();

			if(pLoadedMods)// && pLoadedMods->RemoveModuleHooks())
			{
				// only safe to free if module hooks were removed or a detour will be corrupted
				bCanUnloadAuxCore = TRUE;
			}
		}
	
		if(!bSuccess)
		{
			// unable to determine whether process should be protected
			goto Cleanup;
		}

		wstrConfigPath = wstrInstallPath + L"\\config.xml";
		if(!pConfigReader->LoadFromXmlFile((WCHAR *)wstrConfigPath.c_str()))
		{
			// error loading config.xml
			goto Cleanup;
		}

		WCHAR *pwszProtProcesses = pConfigReader->GetConfigElement(L"protectedProcessList", L"processList");
		if(!pwszProtProcesses)
		{
			// no processes to protect
			goto Cleanup;
		}

		pwszProtProcesses = wcsdup(pwszProtProcesses);
		if(!pwszProtProcesses)
		{
			goto Cleanup;
		}

		WCHAR wszCurrentProcName[512] = {0};

		DWORD dwFileNameLen = GetModuleFileName(NULL, wszCurrentProcName, 511);
		wszCurrentProcName[dwFileNameLen] = '\0';

		WCHAR *pwszPathChr = wcsrchr(wszCurrentProcName, '\\');
		if(pwszPathChr)
		{
			// move the process name to the start of the buffer (+1 for NULL)
		
			DWORD cch = wcslen(pwszPathChr + 1);
			memmove(wszCurrentProcName, pwszPathChr + 1, sizeof(WCHAR) * (cch + 1));
		}

		WCHAR *pwszToken = wcstok(pwszProtProcesses, L"|");
		while(pwszToken)
		{
			// do something
			if(wcsicmp(pwszToken, wszCurrentProcName) == 0)
			{
				// should protect this process!
				bProtect = TRUE;
				break;
			}

			pwszToken = wcstok(NULL, L"|");
		}

		free(pwszProtProcesses);
		pwszProtProcesses = NULL;
	}

	if(!bProtect)
	{
		if(CheckProtectFileProcess(GetCurrentProcessId()))
		{
			bProtect = TRUE;
		}
	}

	if(bProtect)
	{
		if(!pLoadedMods)
			goto Cleanup;

		pLoadedMods->Start();
		pLoadedMods->EnumLoadedModules();

		if(GetModuleHandle(L"didcore.dll") == NULL)
		{
			pLoadedMods->AttachModuleHooks();
		}

		HMODULE hmBpCore = LoadLibrary(L"BpCore.dll");
		if(!hmBpCore)
		{
			// unable to load BpCore
			goto Cleanup;
		}
	}

	bResult = TRUE;
Cleanup:
	if(!bProtect)
	{
		if(bCanUnloadAuxCore)
			FreeLibrary(hmAuxCore);

		// unload module (self) if protection not required
		HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)FreeLibrary, GetModuleHandle(L"CryModLdr.dll"), 0, NULL);
		if(hThread)
			CloseHandle(hThread);
		
		//if(bUnloadSelf)
		//	FreeLibraryAndExitThread(GetModuleHandle(L"CryModLdr.dll"), 0);
	}

	return bResult;
}

BOOL CrystalModuleLoader::UnInitialize()
{
	return TRUE;
}
