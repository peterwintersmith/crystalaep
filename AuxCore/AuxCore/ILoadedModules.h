// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#pragma once

// CLoadedModules Interface Class

struct ILoadedModules;

#ifdef CRYSTAL_LATE_BINDING
#ifdef DEBUG_BUILD
extern "C" static ILoadedModules *(*GetLoadedModules)() = (ILoadedModules *(*)())GetProcAddress(
		GetModuleHandle(L"C:\\Work\\crystal\\BP\\Impl\\AuxCore\\Debug\\auxcore.dll") ?
			GetModuleHandle(L"C:\\Work\\crystal\\BP\\Impl\\AuxCore\\Debug\\auxcore.dll") :
			LoadLibrary(L"C:\\Work\\crystal\\BP\\Impl\\AuxCore\\Debug\\auxcore.dll"), 
		"GetLoadedModules"
	);
#else
extern "C" static ILoadedModules *(*GetLoadedModules)() = (ILoadedModules *(*)())GetProcAddress(
		GetModuleHandle(L"auxcore.dll") ?
			GetModuleHandle(L"auxcore.dll") :
			LoadLibrary(L"auxcore.dll"), 
		"GetLoadedModules"
	);
#endif
#else
extern "C" __declspec(dllimport) ILoadedModules *GetLoadedModules();
#endif

namespace ModuleLoadHooks {
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
}

struct ISingleLoadedModule {
	virtual WCHAR *DllName() = 0;
	virtual WCHAR *DllPath() = 0;
	virtual LPVOID DllBase() = 0;
	virtual DWORD DllImageSize() = 0;
	virtual LPVOID DllEntryPoint() = 0;
};

struct ILoadedModules {
	virtual std::map<HMODULE, ISingleLoadedModule *>& GetLoadedModuleList() = 0;
	virtual void AddModule(HMODULE hm) = 0;
	virtual void AddModule(HMODULE hm, BOOL bLock) = 0;
	virtual void RemoveModule(HMODULE hm) = 0;
	virtual LPVOID PebAddress() = 0;
	virtual BOOL IsAddressInLoadedModule(LPVOID lpvAddress) = 0;
	virtual BOOL IsAddressInHModule(HMODULE hmModule, LPVOID lpvAddress) = 0;
	virtual void EnumLoadedModules() = 0;
	virtual ISingleLoadedModule *FindDllWithPartialName(LPWSTR pwszPartialName) = 0;
	virtual BOOL AttachModuleHooks() = 0;
	virtual BOOL RemoveModuleHooks() = 0;
	virtual BOOL Start() = 0;
	virtual void Terminate() = 0;

//#ifdef DEBUG_BUILD
	virtual void DebugOutputAllModules() = 0;
//#endif
};