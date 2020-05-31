// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#pragma once

// CLoadedModules virtual base class (identical to interface)

struct CSingleLoadedModuleBase {
	//virtual std::wstring DllName() = 0;
	//virtual std::wstring DllPath() = 0;
	virtual WCHAR *DllName() = 0;
	virtual WCHAR *DllPath() = 0;
	virtual LPVOID DllBase() = 0;
	virtual DWORD DllImageSize() = 0;
	virtual LPVOID DllEntryPoint() = 0;
};

struct CLoadedModulesBase {
	virtual std::map<HMODULE, CSingleLoadedModuleBase *>& GetLoadedModuleList() = 0;
	virtual void AddModule(HMODULE hm) = 0;
	virtual void AddModule(HMODULE hm, BOOL bLock) = 0; // this one is not in the interface for obvious reasons
	virtual void RemoveModule(HMODULE hm) = 0;
	virtual LPVOID PebAddress() = 0;
	virtual BOOL IsAddressInLoadedModule(LPVOID lpvAddress) = 0;
	virtual BOOL IsAddressInHModule(HMODULE hmModule, LPVOID lpvAddress) = 0;
	virtual void EnumLoadedModules() = 0;
	virtual CSingleLoadedModuleBase *FindDllWithPartialName(LPWSTR pwszPartialName) = 0;
	virtual BOOL AttachModuleHooks() = 0;
	virtual BOOL RemoveModuleHooks() = 0;
	virtual BOOL Start() = 0;
	virtual void Terminate() = 0;

//#ifdef DEBUG_BUILD
	virtual void DebugOutputAllModules() = 0;
//#endif
};