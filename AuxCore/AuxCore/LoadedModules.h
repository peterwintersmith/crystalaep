// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#pragma once

class CSingleLoadedModule;

#include "AuxCore.h"
#include "LoadedModulesBase.h"
#include <Detours.h>

using namespace DetoursFree;

extern "C" __declspec(dllexport) CLoadedModulesBase *GetILoadedModules();

namespace ModuleLoadHooks {

#define DETOUR_TRANS_BEGIN    DetourTransactionBegin(); DetourUpdateThread(GetCurrentThread());
#define DETOUR_TRANS_COMMIT   DetourTransactionCommit();

#define ATTACH_DETOUR(func)    {\
	Real_##func = func;\
	DetourAttach((PVOID *)&Real_##func, (PVOID)My_##func);\
}

#define DETACH_DETOUR(func)    {\
	DetourDetach((PVOID *)&Real_##func, (PVOID)My_##func);\
}

#define ATTACH_DETOUR_PTR(mod, offs, func)    {\
	PVOID *pvTmp = (PVOID *)&Real_##func;\
	*pvTmp = (PVOID)((PBYTE)(GetModuleHandle(mod) ? GetModuleHandle(mod) : LoadLibrary(mod)) + (size_t)offs);\
	DetourAttach((PVOID *)&Real_##func, (PVOID)My_##func);\
}

#define ATTACH_DETOUR_GPA(mod, name, func)    {\
	PVOID *pvTmp = (PVOID *)&Real_##func;\
	*pvTmp = (PVOID)GetProcAddress((GetModuleHandle(mod) ? GetModuleHandle(mod) : LoadLibrary(mod)), name);\
	DetourAttach((PVOID *)&Real_##func, (PVOID)My_##func);\
}

#define ATTACH_DETOUR_GPA_ORDINAL(mod, ordinal, func)    {\
	PVOID *pvTmp = (PVOID *)&Real_##func;\
	*pvTmp = (PVOID)GetProcAddress((GetModuleHandle(mod) ? GetModuleHandle(mod) : LoadLibrary(mod)), (char *)ordinal);\
	DetourAttach((PVOID *)&Real_##func, (PVOID)My_##func);\
}

	BOOL AttachAll();
	BOOL DetachAll();

	typedef HMODULE (WINAPI *FN_LOADLIBRARYA_DEF)(
		__in	char* lpFileName
	);
	
	typedef HMODULE (WINAPI *FN_LOADLIBRARYW_DEF)(
		__in	LPWSTR lpFileName
	);

	typedef HMODULE (WINAPI *FN_LOADLIBRARYEXA_DEF)(
	  __in        char* lpFileName,
	  __reserved  HANDLE hFile,
	  __in        DWORD dwFlags
	);

	typedef HMODULE (WINAPI *FN_LOADLIBRARYEXW_DEF)(
	  __in        LPWSTR lpFileName,
	  __reserved  HANDLE hFile,
	  __in        DWORD dwFlags
	);

	typedef NTSTATUS (NTAPI *FN_LDRLOADDLL_DEF)(
		__in PWCHAR				PathToFile,
		__in ULONG				Flags OPTIONAL,
		__in PUNICODE_STRING	ModuleFileName,
		__out PHANDLE			ModuleHandle
	);

	typedef BOOL (WINAPI *FN_FREELIBRARY_DEF)(
	  __in  HMODULE hModule
	);

	static FN_LOADLIBRARYA_DEF				Real_LoadLibraryA = NULL;
	static FN_LOADLIBRARYW_DEF				Real_LoadLibraryW = NULL;
	static FN_LOADLIBRARYEXA_DEF			Real_LoadLibraryExA = NULL;
	static FN_LOADLIBRARYEXW_DEF			Real_LoadLibraryExW = NULL;
	static FN_LDRLOADDLL_DEF				Real_LdrLoadDll = NULL;
	static FN_FREELIBRARY_DEF				Real_FreeLibrary = NULL;
	
	HMODULE WINAPI My_LoadLibraryA(
		__in	char* lpFileName
	);
	
	HMODULE WINAPI My_LoadLibraryW(
		__in	LPWSTR lpFileName
	);

	HMODULE WINAPI My_LoadLibraryExA(
	  __in        char* lpFileName,
	  __reserved  HANDLE hFile,
	  __in        DWORD dwFlags
	);

	HMODULE WINAPI My_LoadLibraryExW(
	  __in        LPWSTR lpFileName,
	  __reserved  HANDLE hFile,
	  __in        DWORD dwFlags
	);

	NTSTATUS NTAPI My_LdrLoadDll(
		__in PWCHAR				PathToFile,
		__in ULONG				Flags OPTIONAL,
		__in PUNICODE_STRING	ModuleFileName,
		__out PHANDLE			ModuleHandle
	);

	BOOL WINAPI My_FreeLibrary(
	  __in  HMODULE hModule
	);
}

#define DEFER_POLL_INTERVAL_MILLIS	(2 * 1000) // two secs

class CLoadedModules : public CLoadedModulesBase {
	friend BOOL AuxCore::Initialize();
public:
	static CLoadedModules* GetInstance();

	std::map<HMODULE, CSingleLoadedModuleBase *>& GetLoadedModuleList();
	void AddModule(HMODULE hm);
	void AddModule(HMODULE hm, BOOL bLock); // this one is not in the interface for obvious reasons
	void RemoveModule(HMODULE hm);
	LPVOID PebAddress() { return m_lpvPeb; }
	BOOL IsAddressInLoadedModule(LPVOID lpvAddress);
	BOOL IsAddressInHModule(HMODULE hmModule, LPVOID lpvAddress);
	void EnumLoadedModules();
	CSingleLoadedModuleBase *FindDllWithPartialName(LPWSTR pwszPartialName);
	BOOL AttachModuleHooks();
	BOOL RemoveModuleHooks();
	BOOL Start();
	void Terminate();

//#ifdef DEBUG_BUILD
	void DebugOutputAllModules();
//#endif

private:
	CLoadedModules() : m_lpvPeb(0), m_bRemoveHooks(FALSE), m_bTerminate(FALSE), m_hThread(NULL), m_hEvent(NULL)
	{
		pfnZwQueryInformationProcess = (ZWQUERYINFORMATIONPROCESS)GetProcAddress(
				GetModuleHandle(L"ntdll.dll"),
				"ZwQueryInformationProcess"
			);

		InitializeCriticalSection(&m_csModOpLock);

		StorePebPointer();
		//EnumLoadedModules();
		
		m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		//m_hThread = CreateThread(NULL, 0, CleanupQueueProc, (LPVOID)'AXCQ', 0, NULL);
		
		if(GetModuleHandle(L"didcore.dll") == NULL)
		{
			//AttachModuleHooks();
		}
	}

	void StorePebPointer();

	static DWORD WINAPI CleanupQueueProc(LPVOID);

	typedef NTSTATUS (WINAPI *ZWQUERYINFORMATIONPROCESS)(
	  __in       HANDLE ProcessHandle,
	  __in       PROCESSINFOCLASS ProcessInformationClass,
	  __out      PVOID ProcessInformation,
	  __in       ULONG ProcessInformationLength,
	  __out_opt  PULONG ReturnLength
	);

	ZWQUERYINFORMATIONPROCESS pfnZwQueryInformationProcess;

	static CLoadedModules *m_Instance;
	static CRITICAL_SECTION m_csCreateInst, m_csModOpLock;
	
	LPVOID m_lpvPeb;
	std::map<HMODULE, CSingleLoadedModuleBase *> m_mapLoadedMods;
	std::list<CSingleLoadedModuleBase *> m_listCleanupMods;
	BOOL m_bRemoveHooks;

	HANDLE m_hThread, m_hEvent;
	volatile BOOL m_bTerminate;
};

class CSingleLoadedModule : public CSingleLoadedModuleBase {
public:
	CSingleLoadedModule(WCHAR *dllName, WCHAR *dllPath, LPVOID dllBase, DWORD dllSize, LPVOID dllEntryPt) :
	  m_lpvDllBase(dllBase), m_lpvEntryPoint(dllEntryPt), m_dwDllSize(dllSize)
	{
		m_wszDllName = wcsdup(dllName);
		m_wszDllPath = wcsdup(dllPath);
	}
	
	~CSingleLoadedModule()
	{
		if(m_wszDllName)
			free(m_wszDllName);
		
		m_wszDllName = NULL;

		if(m_wszDllPath)
			free(m_wszDllPath);

		m_wszDllPath = NULL;
	}

	WCHAR *DllName() {
		return m_wszDllName;
	}

	WCHAR *DllPath() {
		return m_wszDllPath;
	}

	LPVOID DllBase() {
		return m_lpvDllBase;
	}

	DWORD DllImageSize() {
		return m_dwDllSize;
	}

	LPVOID DllEntryPoint() {
		return m_lpvEntryPoint;
	}

private:
	WCHAR *m_wszDllName, *m_wszDllPath;
	LPVOID m_lpvDllBase, m_lpvEntryPoint;
	DWORD m_dwDllSize;
};