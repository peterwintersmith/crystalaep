// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

// important todo:
//  1. push HMODULEs and CSingleLoadedModules onto a queue to be erased periodically
//     to avoid horrible situations when performing checks for heap/virtual mem
//     functions
//
//  2. check that this code still works on XP without the defered erase thread

#include "stdafx.h"
#include "AuxCore.h"
#include "..\..\Shared\Utils\Utils.h"
#include "LoadedModulesBase.h"
#include "LoadedModules.h"

CLoadedModules *CLoadedModules::m_Instance = NULL;
CRITICAL_SECTION CLoadedModules::m_csCreateInst = {0}, CLoadedModules::m_csModOpLock = {0};

BOOL ModuleLoadHooks::AttachAll()
{
	DETOUR_TRANS_BEGIN
	ATTACH_DETOUR_GPA(L"kernel32.dll", "LoadLibraryA", LoadLibraryA);
	ATTACH_DETOUR_GPA(L"kernel32.dll", "LoadLibraryW", LoadLibraryW);
	ATTACH_DETOUR_GPA(L"kernel32.dll", "LoadLibraryExA", LoadLibraryExA);
	ATTACH_DETOUR_GPA(L"kernel32.dll", "LoadLibraryExW", LoadLibraryExW);
	ATTACH_DETOUR_GPA(L"ntdll.dll", "LdrLoadDll", LdrLoadDll);
	ATTACH_DETOUR_GPA(L"kernel32.dll", "FreeLibrary", FreeLibrary);
	DETOUR_TRANS_COMMIT
	return TRUE;
}

BOOL ModuleLoadHooks::DetachAll()
{
	DETOUR_TRANS_BEGIN
	DETACH_DETOUR(LoadLibraryA);
	DETACH_DETOUR(LoadLibraryW);
	DETACH_DETOUR(LoadLibraryExA);
	DETACH_DETOUR(LoadLibraryExW);
	DETACH_DETOUR(LdrLoadDll);
	DETACH_DETOUR(FreeLibrary);
	DETOUR_TRANS_COMMIT
	return TRUE;
}

HMODULE WINAPI ModuleLoadHooks::My_LoadLibraryA(
	__in	char* lpFileName
)
{
	CLoadedModules *pLoadedMods = CLoadedModules::GetInstance();
	HMODULE hm = NULL;
	wchar_t *pwszFile = Utils::WcFromMultiByte(lpFileName);
	
	hm = Real_LoadLibraryExA(lpFileName, NULL, 0);
	if(hm)
	{
		pLoadedMods->AddModule(hm);
	}
	
	if(pwszFile)
		free(pwszFile);

	return hm;
}

HMODULE WINAPI ModuleLoadHooks::My_LoadLibraryW(
	__in	LPWSTR lpFileName
)
{
	CLoadedModules *pLoadedMods = CLoadedModules::GetInstance();
	HMODULE hm = NULL;
	
	hm = Real_LoadLibraryExW(lpFileName, NULL, 0);
	if(hm)
	{
		pLoadedMods->AddModule(hm);
	}
	
	return hm;
}

HMODULE WINAPI ModuleLoadHooks::My_LoadLibraryExA(
	__in        char* lpFileName,
	__reserved  HANDLE hFile,
	__in        DWORD dwFlags
)
{
	CLoadedModules *pLoadedMods = CLoadedModules::GetInstance();
	HMODULE hm = NULL;
	wchar_t *pwszFile = Utils::WcFromMultiByte(lpFileName);
	
	hm = Real_LoadLibraryExA(lpFileName, hFile, dwFlags);
	if(hm && !(dwFlags & LOAD_LIBRARY_AS_DATAFILE))
	{
		pLoadedMods->AddModule(hm);
	}
	
	if(pwszFile)
		free(pwszFile);

	return hm;
}

HMODULE WINAPI ModuleLoadHooks::My_LoadLibraryExW(
	__in        LPWSTR lpFileName,
	__reserved  HANDLE hFile,
	__in        DWORD dwFlags
)
{
	CLoadedModules *pLoadedMods = CLoadedModules::GetInstance();
	HMODULE hm = NULL;
	
	hm = Real_LoadLibraryExW(lpFileName, hFile, dwFlags);
	if(hm && !(dwFlags & LOAD_LIBRARY_AS_DATAFILE))
	{
		pLoadedMods->AddModule(hm);
	}
	
	return hm;
}

//http://www.matcode.com/undocwin.h.txt

NTSTATUS NTAPI ModuleLoadHooks::My_LdrLoadDll(
	__in PWCHAR				PathToFile,
	__in ULONG				Flags,
	__in PUNICODE_STRING	ModuleFileName,
	__out PHANDLE			ModuleHandle
)
{
	CLoadedModules *pLoadedMods = CLoadedModules::GetInstance();
	NTSTATUS ntStatus = 0xc0000135; // UNABLE_TO_LOAD_DLL
	
	std::wstring wstrModuleName((WCHAR *)ModuleFileName->Buffer, ModuleFileName->Length / sizeof(WCHAR));
	WCHAR *pwszModuleName = const_cast<WCHAR *>(wstrModuleName.c_str());

	if(NT_SUCCESS(ntStatus = Real_LdrLoadDll(PathToFile, Flags, ModuleFileName, ModuleHandle)))
	{
#ifdef DEBUG_BUILD
		//WCHAR wszTmp[512];
		//wsprintf(wszTmp, L"LdrLoadDll(HMODULE = 0x%p)\n", ModuleHandle ? *ModuleHandle : NULL);
		//OutputDebugString(wszTmp);
#endif
		if(*ModuleHandle && !(Flags & LOAD_LIBRARY_AS_DATAFILE))
		{
			pLoadedMods->AddModule((HMODULE)*ModuleHandle);
		}
	}
	
	return ntStatus;
}

BOOL WINAPI ModuleLoadHooks::My_FreeLibrary(
  __in  HMODULE hModule
)
{
	CLoadedModules *pLoadedMods = CLoadedModules::GetInstance();
	
	BOOL bRetVal = Real_FreeLibrary(hModule);
	
	if(bRetVal)
	{
		pLoadedMods->RemoveModule(hModule);
	}

	return bRetVal;
}

CLoadedModules *CLoadedModules::GetInstance()
{
	if(m_Instance == NULL)
	{
		EnterCriticalSection(&m_csCreateInst);

		if(m_Instance == NULL)
		{
			m_Instance = new CLoadedModules();
		}

		LeaveCriticalSection(&m_csCreateInst);
	}

	return m_Instance;
}

DWORD WINAPI CLoadedModules::CleanupQueueProc(LPVOID)
{
	CLoadedModules *pLoadedMods = CLoadedModules::GetInstance();
	
	while(!pLoadedMods->m_bTerminate)
	{
		while(!pLoadedMods->m_listCleanupMods.empty())
		{
			CSingleLoadedModuleBase *pSlm = pLoadedMods->m_listCleanupMods.back();
			pLoadedMods->m_listCleanupMods.pop_back();

			if(pSlm)
				delete pSlm;
		}

		Sleep(DEFER_POLL_INTERVAL_MILLIS);
	}
	
	SetEvent(pLoadedMods->m_hEvent);

	return 0;
}

BOOL CLoadedModules::Start()
{
	BOOL bSuccess = FALSE;
	DWORD dwTID = 0;

	if(m_hThread)
	{
		// started already
		bSuccess = TRUE;
		goto Cleanup;
	}

	m_hThread = CreateThread(NULL, 0, CleanupQueueProc, (LPVOID)'AUXQ', 0, &dwTID);
	bSuccess = (m_hThread != NULL);
	
Cleanup:
	return bSuccess;
}

void CLoadedModules::Terminate()
{
	if(!m_hThread || !this)
		return;

	m_bTerminate = TRUE;
	
	TerminateThread(m_hThread, 0);
	//DWORD dwWaitStatus = WaitForSingleObject(m_hEvent, 2 * DEFER_POLL_INTERVAL_MILLIS);
	
	/*
	switch(dwWaitStatus)
	{
	case WAIT_OBJECT_0:
	case WAIT_TIMEOUT:
		CloseHandle(m_hThread);
		m_hThread = NULL;

		CloseHandle(m_hEvent);
		m_hEvent = NULL;

		break;
	default:
		break;
	}
	*/
}

std::map<HMODULE, CSingleLoadedModuleBase *>& CLoadedModules::GetLoadedModuleList()
{
	return m_mapLoadedMods;
}

void CLoadedModules::AddModule(HMODULE hm)
{
	// locking here causes deadlock (presumably something we call calls heap or
	// mem function which checks caller ret is valid/enums modules which locks)
	
	AddModule(hm, FALSE);
}

void CLoadedModules::AddModule(HMODULE hm, BOOL bLock)
{
	WCHAR *pwszModulePath = NULL, *pwszModuleName = NULL;
	MODULEINFO moduleInfo = {0};
#ifdef DEBUG_BUILD
	CLogger *logger = NULL;
#endif

	//if(bLock)
	//	EnterCriticalSection(&m_csModOpLock);

	//if(m_mapLoadedMods.find(hm) != m_mapLoadedMods.end())
	//	goto Cleanup;

	try
	{
#ifdef DEBUG_BUILD
		logger = CLogger::GetInstance("bpcore_loadedmods");
		logger->Log("AddModule(HMODULE hm = 0x%x)\n", hm);
#endif
	}
	catch(...)
	{
	}

	if(!(pwszModulePath = (WCHAR *)calloc(65600, sizeof(WCHAR))))
		goto Cleanup;

	DWORD cchModulePath = GetModuleFileName(hm, pwszModulePath, 65600 - 1);
	if(!cchModulePath)
	{
		WCHAR wszTempName[32];

		//do
		//{
		//	wsprintf(wszTempName, L"unnamed_dll_%u.dll", Utils::GetRandomInteger(0, 0xffffffff));
		//}
		//while(m_mapLoadedMods.find(hm) != m_mapLoadedMods.end()); // BUG?! hm never changes => infinite loop
		
		do
		{
			wsprintf(wszTempName, L"unnamed_dll_%u.dll", Utils::GetRandomInteger(0, 0xffffffff));
		}
		while(FindDllWithPartialName(wszTempName) != NULL); // find a unique name
		
		wcscpy(pwszModulePath, wszTempName);

		pwszModuleName = pwszModulePath;
	}
	else
	{
		pwszModuleName = wcsrchr(pwszModulePath, '\\');
		if(!pwszModuleName)
		{
			pwszModuleName = pwszModulePath;
		}
		else
		{
			// skip '\\'
			pwszModuleName++;
		}
	}

	if(!GetModuleInformation(
			GetCurrentProcess(),
			hm,
			&moduleInfo,
			sizeof(moduleInfo)
		))
	{
		// just store zero values
		memset(&moduleInfo, 0, sizeof(moduleInfo));
	}

	try
	{
#ifdef DEBUG_BUILD
		if(logger)
			logger->Log("\tSaving module info hm(0x%x) {%S, %S, 0x%p, 0x%p, 0x%p}\n",
				hm,
				pwszModuleName,
				pwszModulePath,
				moduleInfo.lpBaseOfDll,
				moduleInfo.SizeOfImage,
				moduleInfo.EntryPoint
			);
#endif
		// should never throw as modname should always be unique

#ifdef DEBUG_BUILD
		{
		WCHAR wsz[256];
		wsprintf(wsz, L"Registering DLL: 0x%p (0x%s)\n", moduleInfo.lpBaseOfDll, pwszModuleName);
		OutputDebugString(wsz);
		}
#endif
		if(m_mapLoadedMods[hm] != NULL)
		{
			//m_listCleanupMods.push_back(m_mapLoadedMods[hm]);
			m_mapLoadedMods[hm] = NULL;
		}

		m_mapLoadedMods[hm] = new CSingleLoadedModule(
				pwszModuleName,
				pwszModulePath,
				moduleInfo.lpBaseOfDll,
				moduleInfo.SizeOfImage,
				moduleInfo.EntryPoint
			);
	}
	catch(...)
	{
#ifdef DEBUG_BUILD
		{
		WCHAR wsz[256];
		wsprintf(wsz, L"Exception registering module hm = 0x%p\n", hm);
		OutputDebugString(wsz);
		}
#endif
		goto Cleanup;
	}

Cleanup:
	if(pwszModulePath)
		free(pwszModulePath);

	//if(bLock)
	//	LeaveCriticalSection(&m_csModOpLock);
}

void CLoadedModules::RemoveModule(HMODULE hm)
{
#ifdef DEBUG_BUILD
	CLogger *logger = NULL;
#endif	
	try
	{
#ifdef DEBUG_BUILD
		logger = CLogger::GetInstance("bpcore_loadedmods");
		logger->Log("RemoveModule(HMODULE hm = 0x%x)\n", hm);
#endif
	}
	catch(...)
	{
	}

	// applying the lock here leads to a deadlock condition. It shouldn't actually affect the program
	// realistically whether we lock during a RemoveModule or not.

	//EnterCriticalSection(&m_csModOpLock);

	try
	{
		if(m_mapLoadedMods[hm] && GetModuleHandle(m_mapLoadedMods[hm]->DllName()) != hm)
		{
#ifdef DEBUG_BUILD
			if(logger)
				logger->Log("\tModule %S [0x%p] erased from module list\n", m_mapLoadedMods[hm]->DllName(), hm);
#endif
			
			// do not erase here as when module unloads GetModuleHandle returns NULL but its DllMain can still call
			// functions of the DLL (which call the IsAddressInLoadedModule function, which fail)

#ifdef DEBUG_BUILD
			{
			WCHAR wsz[256];
			//wsprintf(wsz, L"Adding module 0x%p (0x%s) to defer erase list\n", hm, m_mapLoadedMods[hm]->DllName());
			wsprintf(wsz, L"Removing module 0x%p (0x%s)\n", hm, m_mapLoadedMods[hm]->DllName());
			OutputDebugString(wsz);
			}
#endif

			//m_listCleanupMods.push_back(m_mapLoadedMods[hm]);
			m_mapLoadedMods[hm] = NULL; // clearing it out suffices to unregister it
		}
	}
	catch(...)
	{
	}

	//LeaveCriticalSection(&m_csModOpLock);
}

void CLoadedModules::EnumLoadedModules()
{
	HMODULE *phmProcessModules;
	std::map<HMODULE, CSingleLoadedModuleBase *>::const_iterator it;

	DWORD cLoadedModules = 0, cbBytesNeeded = 0;
	
	//EnterCriticalSection(&m_csModOpLock);

	EnumProcessModules(GetCurrentProcess(), NULL, 0, &cbBytesNeeded);

	if(!(phmProcessModules = (HMODULE *)malloc(cbBytesNeeded)))
		goto Cleanup;

	cLoadedModules = (cbBytesNeeded / sizeof(HMODULE));

	if(!EnumProcessModules(GetCurrentProcess(), phmProcessModules, cbBytesNeeded, &cbBytesNeeded))
		goto Cleanup;
	
	for(it = m_mapLoadedMods.begin(); it != m_mapLoadedMods.end(); it++)
	{
		//m_listCleanupMods.push_back(m_mapLoadedMods[it->first]);
		m_mapLoadedMods[it->first] = NULL;
	}
	
	for(DWORD i=0; i<cLoadedModules; i++)
	{
		AddModule(phmProcessModules[i], FALSE);
	}
	
Cleanup:
	//LeaveCriticalSection(&m_csModOpLock);

	if(phmProcessModules)
		free(phmProcessModules);
}

void CLoadedModules::StorePebPointer()
{
	PROCESS_BASIC_INFORMATION pbi = {0};
	ULONG ulReturnLength = 0;
	
	if(pfnZwQueryInformationProcess && 
		NT_SUCCESS(pfnZwQueryInformationProcess(
			GetCurrentProcess(),
			ProcessBasicInformation,
			&pbi,
			sizeof(pbi),
			&ulReturnLength)
		))
	{
		// this information isn't integral so don't error if ZwQIP fails
		m_lpvPeb = reinterpret_cast<LPVOID>(pbi.PebBaseAddress);
	}
}

BOOL CLoadedModules::IsAddressInLoadedModule(LPVOID lpvAddress)
{
	BOOL bInMod = FALSE;
	std::map<HMODULE, CSingleLoadedModuleBase *>::const_iterator it;
	CSingleLoadedModuleBase *pSlm = NULL;

	//EnterCriticalSection(&m_csModOpLock);

	for(it = m_mapLoadedMods.begin(); it != m_mapLoadedMods.end(); it++)
	{
		if((pSlm = (*it).second) != NULL)
		{
			if(lpvAddress >= pSlm->DllBase() && lpvAddress <= ((PBYTE)pSlm->DllBase() + pSlm->DllImageSize()))
			{
				bInMod = TRUE;
				break;
			}
		}
	}

	//LeaveCriticalSection(&m_csModOpLock);
	
	return bInMod;
}

BOOL CLoadedModules::IsAddressInHModule(HMODULE hmModule, LPVOID lpvAddress)
{
	BOOL bInMod = FALSE;
	CSingleLoadedModuleBase *pSlm = NULL;

	//EnterCriticalSection(&m_csModOpLock);

	try
	{
		pSlm = m_mapLoadedMods[hmModule];

		if(pSlm != NULL && lpvAddress >= pSlm->DllBase() && lpvAddress <= ((PBYTE)pSlm->DllBase() + pSlm->DllImageSize()))
		{
			bInMod = TRUE;
		}
	}
	catch(...)
	{
	}

	//LeaveCriticalSection(&m_csModOpLock);

	return bInMod;
}

BOOL CLoadedModules::AttachModuleHooks()
{
	if(!m_bRemoveHooks)
		ModuleLoadHooks::AttachAll();
	
	m_bRemoveHooks = TRUE;

	return TRUE;
}

BOOL CLoadedModules::RemoveModuleHooks()
{
	if(m_bRemoveHooks)
		ModuleLoadHooks::DetachAll();
	
	m_bRemoveHooks = FALSE;

	return TRUE;
}

CSingleLoadedModuleBase *CLoadedModules::FindDllWithPartialName(LPWSTR pwszPartialName)
{
	BOOL bInMod = FALSE;
	std::map<HMODULE, CSingleLoadedModuleBase *>::const_iterator it;
	CSingleLoadedModuleBase *pSlm = NULL;

	//EnterCriticalSection(&m_csModOpLock);

	for(it = m_mapLoadedMods.begin(); it != m_mapLoadedMods.end(); it++)
	{
		if((pSlm = (*it).second) != NULL)
		{
			if(Utils::wcsistr(const_cast<WCHAR*>(pSlm->DllName()), pwszPartialName) != NULL)
			{
				goto Cleanup;
			}
		}
	}

	pSlm = NULL;
Cleanup:
	//LeaveCriticalSection(&m_csModOpLock);

	return pSlm;
}

//#ifdef DEBUG_BUILD
void CLoadedModules::DebugOutputAllModules()
{
	WCHAR *pwszBuffer = (WCHAR *)malloc(0x1000000);
	pwszBuffer[0] = '\0';

	std::map<HMODULE, CSingleLoadedModuleBase *>::const_iterator it;
	CSingleLoadedModuleBase *pSlm = NULL;

	EnumLoadedModules();

	for(it = m_mapLoadedMods.begin(); it != m_mapLoadedMods.end(); it++)
	{
		if((pSlm = (*it).second) != NULL)
		{
			SIZE_T cch = wcslen(pwszBuffer);
			if(cch >= 0x1000000 - 0x1000)
				DebugBreak();

			wsprintf(
					pwszBuffer + cch,
					L"Module: Handle [0x%p]; Base [0x%p]; Size [0x%x]; Name [%s]\r\n",
					it->first,
					pSlm->DllBase(),
					pSlm->DllImageSize(),
					pSlm->DllName()
				);
		}
	}

	OutputDebugString(pwszBuffer);

	free(pwszBuffer);
	pwszBuffer = NULL;
}
//#endif

extern "C" __declspec(dllexport) CLoadedModulesBase *GetLoadedModules()
{
	return reinterpret_cast<CLoadedModulesBase *>(CLoadedModules::GetInstance());
}