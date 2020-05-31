// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

// nb: configure IE sec settings: HKEY_CURRENT_USER\Software\Microsoft\Internet Explorer\Security

#pragma once

using namespace DetoursFree;

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

namespace DefenseInDepth {
	
	static void ConfigRefresh(BOOL bRefresh);
	static BOOL IsWithinTrustedMemory(LPVOID lpvAddress);
	static BOOL IsJmpOrCallEsp(LPVOID lpvAddress);
	static BOOL DllContainsString(HMODULE hmModule, void *pSearch, BOOL bUnicode = FALSE);

	/* config defaults */
	static BOOL bComMonEnabled = TRUE,
		bClsidBlacklistEnabled = TRUE,
		bClsidWhitelistEnabled = FALSE,
		bEnableAntiROPStack = TRUE,
		bDisableRWXVAMemory = FALSE,
		bDisableRWXStackMemory = FALSE,
		bMonitorAPICallOrigins = TRUE,
		bInjectIntoChildProc = FALSE,
		bPersistJitDecision = TRUE,
		bRandomizeAllocBase = FALSE,
		bCheckExceptionDisp = TRUE,
		bEmuExceptHandler = FALSE,
		bPromptLowIntegApp = FALSE,
		bElevateLowIntegApps = FALSE,
		bPromptUserRelaxDotnet = FALSE,
		bCheckROPReturn = TRUE,
		bProcessDepEnabled = TRUE,
		bAutoAddHookDllRWX = TRUE;
	/* end */
	static int iPersistUserChoiceMade = -1;
	static std::map<CLSID *, std::wstring> mapClsidBlacklist, mapClsidWhitelist, mapClsidRequiresJit;
	static std::map<std::wstring, HMODULE> mapAllowRWXMods;
	static CRITICAL_SECTION csConfigReload = {0}, csRWXMods = {0}, csSehWalk = {0};
	
	static BOOL CheckClsidSFS(REFCLSID rclsid, REFIID riid, IUnknown *pUnknown);
	static BOOL CheckClsidSFI(REFCLSID rclsid, REFIID riid, IUnknown *pUnknown);
	static BOOL CheckClsidSafeForInit(REFCLSID rclsid);
	
	static volatile BOOL SafeToDeferHeapCalls = FALSE;

	typedef LPVOID (WINAPI *FN_HEAPALLOC_DEF)(
	  __in  HANDLE hHeap,
	  __in  DWORD dwFlags,
	  __in  SIZE_T dwBytes
	);

	typedef LPVOID (WINAPI *FN_HEAPREALLOC_DEF)(
	  __in  HANDLE hHeap,
	  __in  DWORD dwFlags,
	  __in  LPVOID lpMem,
	  __in  SIZE_T dwBytes
	);

	typedef BOOL (WINAPI *FN_HEAPFREE_DEF)(
	  __in  HANDLE hHeap,
	  __in  DWORD dwFlags,
	  __in  LPVOID lpMem
	);
	
	typedef PVOID (WINAPI *FN_RTLCREATEHEAP_DEF)(
		__in      ULONG Flags,
		__in_opt  PVOID HeapBase,
		__in_opt  SIZE_T ReserveSize,
		__in_opt  SIZE_T CommitSize,
		__in_opt  PVOID Lock,
		__in_opt  LPVOID /*PRTL_HEAP_PARAMETERS*/ Parameters
	);

	typedef SIZE_T (WINAPI *FN_HEAPSIZE_DEF)(
	  __in  HANDLE hHeap,
	  __in  DWORD dwFlags,
	  __in  LPCVOID lpMem
	);

	typedef BOOL (WINAPI *FN_HEAPDESTROY_DEF)(
	 __in  HANDLE hHeap
	);
	
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

	typedef VOID (WINAPI *FN_FREELIBRARYANDEXITTHREAD_DEF)(
	  __in  HMODULE hModule,
	  __in  DWORD dwExitCode
	);

	typedef DWORD (WINAPI *FN_CREATEPROCESSINTERNALA_DEF)(
		__in HANDLE	hToken,
		__in_opt	char* lpApplicationName,
		__inout_opt	char* lpCommandLine,
		__in_opt	LPSECURITY_ATTRIBUTES lpProcessAttributes,
		__in_opt	LPSECURITY_ATTRIBUTES lpThreadAttributes,
		__in		BOOL bInheritHandles,
		__in		DWORD dwCreationFlags,
		__in_opt	LPVOID lpEnvironment,
		__in_opt	char* lpCurrentDirectory,
		__in		LPSTARTUPINFO lpStartupInfo,
		__out		LPPROCESS_INFORMATION lpProcessInformation,
		__in		PHANDLE hNewToken
	);
	
	typedef DWORD (WINAPI *FN_CREATEPROCESSINTERNALW_DEF)(
		__in HANDLE	hToken,
		__in_opt	LPWSTR lpApplicationName,
		__inout_opt	LPWSTR lpCommandLine,
		__in_opt	LPSECURITY_ATTRIBUTES lpProcessAttributes,
		__in_opt	LPSECURITY_ATTRIBUTES lpThreadAttributes,
		__in		BOOL bInheritHandles,
		__in		DWORD dwCreationFlags,
		__in_opt	LPVOID lpEnvironment,
		__in_opt	LPWSTR lpCurrentDirectory,
		__in		LPSTARTUPINFO lpStartupInfo,
		__out		LPPROCESS_INFORMATION lpProcessInformation,
		__in		PHANDLE hNewToken
	);

	typedef FARPROC (WINAPI *FN_GETPROCADDRESS_DEF)(
		__in	HMODULE hModule,
		__in	LPCSTR lpProcName
	);

	typedef HMODULE (WINAPI *FN_GETMODULEHANDLEA_DEF)(
		__in_opt	char* lpModuleName
	);

	typedef HMODULE (WINAPI *FN_GETMODULEHANDLEW_DEF)(
		__in_opt	LPWSTR lpModuleName
	);

	typedef BOOL (WINAPI *FN_CREATEPROCESSA_DEF)(
	  __in_opt     char* lpApplicationName,
	  __inout_opt  char* lpCommandLine,
	  __in_opt     LPSECURITY_ATTRIBUTES lpProcessAttributes,
	  __in_opt     LPSECURITY_ATTRIBUTES lpThreadAttributes,
	  __in         BOOL bInheritHandles,
	  __in         DWORD dwCreationFlags,
	  __in_opt     LPVOID lpEnvironment,
	  __in_opt     char* lpCurrentDirectory,
	  __in         LPSTARTUPINFO lpStartupInfo,
	  __out        LPPROCESS_INFORMATION lpProcessInformation
	);

	typedef BOOL (WINAPI *FN_CREATEPROCESSW_DEF)(
	  __in_opt     LPWSTR lpApplicationName,
	  __inout_opt  LPWSTR lpCommandLine,
	  __in_opt     LPSECURITY_ATTRIBUTES lpProcessAttributes,
	  __in_opt     LPSECURITY_ATTRIBUTES lpThreadAttributes,
	  __in         BOOL bInheritHandles,
	  __in         DWORD dwCreationFlags,
	  __in_opt     LPVOID lpEnvironment,
	  __in_opt     LPWSTR lpCurrentDirectory,
	  __in         LPSTARTUPINFO lpStartupInfo,
	  __out        LPPROCESS_INFORMATION lpProcessInformation
	);

	typedef BOOL (WINAPI *FN_VIRTUALPROTECT_DEF)(
		__in	LPVOID lpAddress,
		__in	SIZE_T dwSize,
		__in	DWORD flNewProtect,
		__out	PDWORD lpflOldProtect
	);

	typedef BOOL (WINAPI *FN_VIRTUALPROTECTEX_DEF)(
		__in	HANDLE hProcess,
		__in	LPVOID lpAddress,
		__in	SIZE_T dwSize,
		__in	DWORD flNewProtect,
		__out	PDWORD lpflOldProtect
	);

	typedef NTSTATUS (NTAPI *FN_NTPROTECTVIRTUALMEMORY_DEF)(
		__in HANDLE	ProcessHandle,
		__inout PVOID	*BaseAddress,
		__inout PULONG	NumberOfBytesToProtect,
		__in ULONG		NewAccessProtection,
		__out PULONG	OldAccessProtection
	);

	typedef LPVOID (WINAPI *FN_VIRTUALALLOC_DEF)(
		__in_opt	LPVOID lpAddress,
		__in		SIZE_T dwSize,
		__in		DWORD flAllocationType,
		__in		DWORD flProtect
	);

	typedef LPVOID (WINAPI *FN_VIRTUALALLOCEX_DEF)(
		__in		HANDLE hProcess,
		__in_opt	LPVOID lpAddress,
		__in		SIZE_T dwSize,
		__in		DWORD flAllocationType,
		__in		DWORD flProtect
	);

	typedef NTSTATUS (NTAPI *FN_NTALLOCATEVIRTUALMEMORY_DEF)(
		__in HANDLE		ProcessHandle,
		__inout PVOID	*BaseAddress,
		__in ULONG		ZeroBits,
		__inout PULONG	RegionSize,
		__in ULONG		AllocationType,
		__in ULONG		Protect
	);

	typedef BOOL (WINAPI *FN_SETPROCESSDEPPOLICY_DEF)(
		__in DWORD dwFlags
	);

	typedef DWORD PROCESS_INFORMATION_CLASS;

	typedef NTSTATUS (NTAPI *FN_NTSETINFORMATIONPROCESS_DEF)(
		__in HANDLE ProcessHandle,
		__in PROCESS_INFORMATION_CLASS ProcessInformationClass,
		__in PVOID ProcessInformation,
		__in ULONG ProcessInformationLength
	);

	typedef HRESULT (WINAPI *FN_COCREATEINSTANCE_DEF)(
	  __in   REFCLSID rclsid,
	  __in   LPUNKNOWN pUnkOuter,
	  __in   DWORD dwClsContext,
	  __in   REFIID riid,
	  __out  LPVOID *ppv
	);

	typedef HRESULT (WINAPI *FN_COGETCLASSOBJECT_DEF)(
	  __in      REFCLSID rclsid,
	  __in      DWORD dwClsContext,
	  __in_opt  COSERVERINFO *pServerInfo,
	  __in      REFIID riid,
	  __out     LPVOID *ppv
	);

	typedef HANDLE (WINAPI *FN_CREATETHREAD_DEF)(
	  __in_opt   LPSECURITY_ATTRIBUTES lpThreadAttributes,
	  __in       SIZE_T dwStackSize,
	  __in       LPTHREAD_START_ROUTINE lpStartAddress,
	  __in_opt   LPVOID lpParameter,
	  __in       DWORD dwCreationFlags,
	  __out_opt  LPDWORD lpThreadId
	);

	typedef HANDLE (WINAPI *FN_CREATEREMOTETHREAD_DEF)(
		__in   HANDLE hProcess,
		__in   LPSECURITY_ATTRIBUTES lpThreadAttributes,
		__in   SIZE_T dwStackSize,
		__in   LPTHREAD_START_ROUTINE lpStartAddress,
		__in   LPVOID lpParameter,
		__in   DWORD dwCreationFlags,
		__out  LPDWORD lpThreadId
	);

	typedef void* POBJECT_ATTRIBUTES;
	typedef void* PCLIENT_ID;
	typedef void* PINITIAL_TEB;

	typedef NTSTATUS (NTAPI *FN_NTCREATETHREAD_DEF)(
		__out PHANDLE			ThreadHandle,
		__in ACCESS_MASK		DesiredAccess,
		__in_opt POBJECT_ATTRIBUTES	ObjectAttributes,
		__in HANDLE				ProcessHandle,
		__out PCLIENT_ID		ClientId,
		__in PCONTEXT			ThreadContext,
		__in PINITIAL_TEB		InitialTeb,
		__in BOOLEAN			CreateSuspended
	);

	typedef NTSTATUS (NTAPI *FN_NTCREATETHREADEX_DEF) 
	(
		__out PHANDLE hThread,
		__in ACCESS_MASK DesiredAccess,
		__in LPVOID ObjectAttributes,
		__in HANDLE ProcessHandle,
		__in LPTHREAD_START_ROUTINE lpStartAddress,
		__in LPVOID lpParameter,
		__in BOOL CreateSuspended, 
		__in ULONG StackZeroBits,
		__in ULONG SizeOfStackCommit,
		__in ULONG SizeOfStackReserve,
		__out LPVOID lpBytesBuffer
	);

	typedef VOID (WINAPI *FN_EXITPROCESS_DEF)(
	  __in  UINT uExitCode
	);

	typedef VOID (WINAPI *FN_EXITTHREAD_DEF)(
	  __in  DWORD dwExitCode
	);

	typedef NTSTATUS (NTAPI *FN_NTTERMINATETHREAD_DEF)(
		__in HANDLE               ThreadHandle,
		__in NTSTATUS             ExitStatus
	);

	typedef BOOLEAN (NTAPI *FN_RTLDISPATCHEXCEPTION_DEF)(
		__in PEXCEPTION_RECORD  ExceptionRecord,  
		__in PCONTEXT           Context   
	);

	static FN_HEAPALLOC_DEF					Real_HeapAlloc = NULL;
	static FN_HEAPREALLOC_DEF				Real_HeapReAlloc = NULL;
	static FN_HEAPFREE_DEF					Real_HeapFree = NULL;
	static FN_RTLCREATEHEAP_DEF				Real_RtlCreateHeap = NULL;
	static FN_HEAPSIZE_DEF					Real_HeapSize = NULL;
	static FN_HEAPDESTROY_DEF				Real_HeapDestroy = NULL;
	static FN_LOADLIBRARYA_DEF				Real_LoadLibraryA = NULL;
	static FN_LOADLIBRARYW_DEF				Real_LoadLibraryW = NULL;
	static FN_LOADLIBRARYEXA_DEF			Real_LoadLibraryExA = NULL;
	static FN_LOADLIBRARYEXW_DEF			Real_LoadLibraryExW = NULL;
	static FN_LDRLOADDLL_DEF				Real_LdrLoadDll = NULL;
	static FN_FREELIBRARY_DEF				Real_FreeLibrary = NULL;
	static FN_FREELIBRARYANDEXITTHREAD_DEF  Real_FreeLibraryAndExitThread = NULL;
	static FN_CREATEPROCESSINTERNALA_DEF	Real_CreateProcessInternalA = NULL;
	static FN_CREATEPROCESSINTERNALW_DEF	Real_CreateProcessInternalW = NULL;
	static FN_GETPROCADDRESS_DEF			Real_GetProcAddress = NULL;
	static FN_GETMODULEHANDLEA_DEF			Real_GetModuleHandleA = NULL;
	static FN_GETMODULEHANDLEW_DEF			Real_GetModuleHandleW = NULL;
	static FN_CREATEPROCESSA_DEF			Real_CreateProcessA = NULL;
	static FN_CREATEPROCESSW_DEF			Real_CreateProcessW = NULL;
	static FN_VIRTUALPROTECT_DEF			Real_VirtualProtect = NULL;
	static FN_VIRTUALPROTECTEX_DEF			Real_VirtualProtectEx = NULL;
	static FN_NTPROTECTVIRTUALMEMORY_DEF	Real_NtProtectVirtualMemory = NULL;
	static FN_VIRTUALALLOC_DEF				Real_VirtualAlloc = NULL;
	static FN_VIRTUALALLOCEX_DEF			Real_VirtualAllocEx = NULL;
	static FN_NTALLOCATEVIRTUALMEMORY_DEF	Real_NtAllocateVirtualMemory = NULL;
	static FN_SETPROCESSDEPPOLICY_DEF		Real_SetProcessDEPPolicy = NULL;
	static FN_NTSETINFORMATIONPROCESS_DEF	Real_NtSetInformationProcess = NULL;
	static FN_COCREATEINSTANCE_DEF			Real_CoCreateInstance = NULL;
	static FN_COGETCLASSOBJECT_DEF			Real_CoGetClassObject = NULL;
	static FN_CREATETHREAD_DEF				Real_CreateThread = NULL;
	static FN_CREATEREMOTETHREAD_DEF		Real_CreateRemoteThread = NULL;
	static FN_NTCREATETHREAD_DEF			Real_NtCreateThread = NULL;
	static FN_NTCREATETHREADEX_DEF			Real_NtCreateThreadEx = NULL;
	static FN_EXITPROCESS_DEF				Real_ExitProcess = NULL;
	static FN_EXITTHREAD_DEF				Real_ExitThread = NULL;
	static FN_NTTERMINATETHREAD_DEF			Real_NtTerminateThread = NULL;
	static FN_RTLDISPATCHEXCEPTION_DEF      Real_RtlDispatchException = NULL;

	static HMODULE hmNtdll = NULL, hmKernel32 = NULL, hmKernelBase = NULL, hmDiDCore = NULL,
		hmAuxCore = NULL, hmBpCore = NULL;
	static DWORD dwNtdllImageSize = 0x200000, dwKernel32ImageSize = 0x200000, dwKernelBaseImageSize = 0x200000;
	static PVOID pvLdrLoadDll = NULL, pvRtlUserThreadStart = NULL;
	static SYSTEM_INFO stSystemInfo = {0};
	static BOOL bQueriedReIntegLevels = FALSE, bElevationChoice = FALSE;
	static WCHAR *pwszProtProcList = NULL;

	static FN_RTLDISPATCHEXCEPTION_DEF FindRtlDispatchException();
	static BOOL GetSEHHandlerAndNext(DWORD dwNthEntry, LPVOID *plpvHandler, LPVOID *plpvPrev);
	static BOOL IsCodeSEHExploitSequence(CONTEXT *pContext, LPVOID lpvAddress);

	typedef struct _EXCEPTION_REGISTRATION {
		struct _EXCEPTION_REGISTRATION *Prev;
		LPVOID Handler;
	} EXCEPTION_REGISTRATION, *PEXCEPTION_REGISTRATION;

	LPVOID WINAPI My_HeapAlloc(
	  __in  HANDLE hHeap,
	  __in  DWORD dwFlags,
	  __in  SIZE_T dwBytes
	);

	LPVOID WINAPI My_HeapReAlloc(
	  __in  HANDLE hHeap,
	  __in  DWORD dwFlags,
	  __in  LPVOID lpMem,
	  __in  SIZE_T dwBytes
	);

	BOOL WINAPI My_HeapFree(
	  __in  HANDLE hHeap,
	  __in  DWORD dwFlags,
	  __in  LPVOID lpMem
	);

	SIZE_T WINAPI My_HeapSize(
	  __in  HANDLE hHeap,
	  __in  DWORD dwFlags,
	  __in  LPCVOID lpMem
	);

	BOOL WINAPI My_HeapDestroy(
		 __in  HANDLE hHeap
	);

	PVOID WINAPI My_RtlCreateHeap(
		__in      ULONG Flags,
		__in_opt  PVOID HeapBase,
		__in_opt  SIZE_T ReserveSize,
		__in_opt  SIZE_T CommitSize,
		__in_opt  PVOID Lock,
		__in_opt  LPVOID /*PRTL_HEAP_PARAMETERS*/ Parameters
	);
	
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

	VOID WINAPI My_FreeLibraryAndExitThread(
	  __in  HMODULE hModule,
	  __in  DWORD dwExitCode
	);

	DWORD WINAPI My_CreateProcessInternalA(
		__in HANDLE	hToken,
		__in_opt	char* lpApplicationName,
		__inout_opt	char* lpCommandLine,
		__in_opt	LPSECURITY_ATTRIBUTES lpProcessAttributes,
		__in_opt	LPSECURITY_ATTRIBUTES lpThreadAttributes,
		__in		BOOL bInheritHandles,
		__in		DWORD dwCreationFlags,
		__in_opt	LPVOID lpEnvironment,
		__in_opt	char* lpCurrentDirectory,
		__in		LPSTARTUPINFO lpStartupInfo,
		__out		LPPROCESS_INFORMATION lpProcessInformation,
		__in		PHANDLE hNewToken
	);
	
	DWORD WINAPI My_CreateProcessInternalW(
		__in HANDLE	hToken,
		__in_opt	LPWSTR lpApplicationName,
		__inout_opt	LPWSTR lpCommandLine,
		__in_opt	LPSECURITY_ATTRIBUTES lpProcessAttributes,
		__in_opt	LPSECURITY_ATTRIBUTES lpThreadAttributes,
		__in		BOOL bInheritHandles,
		__in		DWORD dwCreationFlags,
		__in_opt	LPVOID lpEnvironment,
		__in_opt	LPWSTR lpCurrentDirectory,
		__in		LPSTARTUPINFO lpStartupInfo,
		__out		LPPROCESS_INFORMATION lpProcessInformation,
		__in		PHANDLE hNewToken
	);

	FARPROC WINAPI My_GetProcAddress(
		__in	HMODULE hModule,
		__in	LPCSTR lpProcName
	);

	HMODULE WINAPI My_GetModuleHandleA(
		__in_opt	char* lpModuleName
	);
	
	HMODULE WINAPI My_GetModuleHandleW(
		__in_opt	LPWSTR lpModuleName
	);

	BOOL WINAPI My_CreateProcessA(
	  __in_opt     char* lpApplicationName,
	  __inout_opt  char* lpCommandLine,
	  __in_opt     LPSECURITY_ATTRIBUTES lpProcessAttributes,
	  __in_opt     LPSECURITY_ATTRIBUTES lpThreadAttributes,
	  __in         BOOL bInheritHandles,
	  __in         DWORD dwCreationFlags,
	  __in_opt     LPVOID lpEnvironment,
	  __in_opt     char* lpCurrentDirectory,
	  __in         LPSTARTUPINFO lpStartupInfo,
	  __out        LPPROCESS_INFORMATION lpProcessInformation
	);

	BOOL WINAPI My_CreateProcessW(
	  __in_opt     LPWSTR lpApplicationName,
	  __inout_opt  LPWSTR lpCommandLine,
	  __in_opt     LPSECURITY_ATTRIBUTES lpProcessAttributes,
	  __in_opt     LPSECURITY_ATTRIBUTES lpThreadAttributes,
	  __in         BOOL bInheritHandles,
	  __in         DWORD dwCreationFlags,
	  __in_opt     LPVOID lpEnvironment,
	  __in_opt     LPWSTR lpCurrentDirectory,
	  __in         LPSTARTUPINFO lpStartupInfo,
	  __out        LPPROCESS_INFORMATION lpProcessInformation
	);

	BOOL WINAPI My_VirtualProtect(
		__in	LPVOID lpAddress,
		__in	SIZE_T dwSize,
		__in	DWORD flNewProtect,
		__out	PDWORD lpflOldProtect
	);

	BOOL WINAPI My_VirtualProtectEx(
		__in	HANDLE hProcess,
		__in	LPVOID lpAddress,
		__in	SIZE_T dwSize,
		__in	DWORD flNewProtect,
		__out	PDWORD lpflOldProtect
	);

	NTSTATUS NTAPI My_NtProtectVirtualMemory(
		__in HANDLE	ProcessHandle,
		__inout PVOID	*BaseAddress,
		__inout PULONG	NumberOfBytesToProtect,
		__in ULONG		NewAccessProtection,
		__out PULONG	OldAccessProtection
	);

	LPVOID WINAPI My_VirtualAlloc(
		__in_opt	LPVOID lpAddress,
		__in		SIZE_T dwSize,
		__in		DWORD flAllocationType,
		__in		DWORD flProtect
	);

	LPVOID WINAPI My_VirtualAllocEx(
		__in		HANDLE hProcess,
		__in_opt	LPVOID lpAddress,
		__in		SIZE_T dwSize,
		__in		DWORD flAllocationType,
		__in		DWORD flProtect
	);
	
	NTSTATUS NTAPI My_NtAllocateVirtualMemory(
		__in HANDLE		ProcessHandle,
		__inout PVOID	*BaseAddress,
		__in ULONG		ZeroBits,
		__inout PULONG	RegionSize,
		__in ULONG		AllocationType,
		__in ULONG		Protect
	);
	
	BOOL WINAPI My_SetProcessDEPPolicy(
		__in DWORD dwFlags
	);

	NTSTATUS NTAPI My_NtSetInformationProcess(
		__in HANDLE ProcessHandle,
		__in PROCESS_INFORMATION_CLASS ProcessInformationClass,
		__in PVOID ProcessInformation,
		__in ULONG ProcessInformationLength
	);

	HRESULT WINAPI My_CoCreateInstance(
	  __in   REFCLSID rclsid,
	  __in   LPUNKNOWN pUnkOuter,
	  __in   DWORD dwClsContext,
	  __in   REFIID riid,
	  __out  LPVOID *ppv
	);

	HRESULT WINAPI My_CoGetClassObject(
	  __in      REFCLSID rclsid,
	  __in      DWORD dwClsContext,
	  __in_opt  COSERVERINFO *pServerInfo,
	  __in      REFIID riid,
	  __out     LPVOID *ppv
	);

	HANDLE WINAPI My_CreateThread(
	  __in_opt   LPSECURITY_ATTRIBUTES lpThreadAttributes,
	  __in       SIZE_T dwStackSize,
	  __in       LPTHREAD_START_ROUTINE lpStartAddress,
	  __in_opt   LPVOID lpParameter,
	  __in       DWORD dwCreationFlags,
	  __out_opt  LPDWORD lpThreadId
	);

	HANDLE WINAPI My_CreateRemoteThread(
	  __in   HANDLE hProcess,
	  __in   LPSECURITY_ATTRIBUTES lpThreadAttributes,
	  __in   SIZE_T dwStackSize,
	  __in   LPTHREAD_START_ROUTINE lpStartAddress,
	  __in   LPVOID lpParameter,
	  __in   DWORD dwCreationFlags,
	  __out  LPDWORD lpThreadId
	);

	NTSTATUS NTAPI My_NtCreateThread(
		__out PHANDLE			ThreadHandle,
		__in ACCESS_MASK		DesiredAccess,
		__in_opt POBJECT_ATTRIBUTES	ObjectAttributes,
		__in HANDLE				ProcessHandle,
		__out PCLIENT_ID		ClientId,
		__in PCONTEXT			ThreadContext,
		__in PINITIAL_TEB		InitialTeb,
		__in BOOLEAN			CreateSuspended
	);

	NTSTATUS NTAPI My_NtCreateThreadEx(
		__out PHANDLE hThread,
		__in ACCESS_MASK DesiredAccess,
		__in LPVOID ObjectAttributes,
		__in HANDLE ProcessHandle,
		__in LPTHREAD_START_ROUTINE lpStartAddress,
		__in LPVOID lpParameter,
		__in BOOL CreateSuspended, 
		__in ULONG StackZeroBits,
		__in ULONG SizeOfStackCommit,
		__in ULONG SizeOfStackReserve,
		__out LPVOID lpBytesBuffer
	);

	VOID WINAPI My_ExitProcess(
	  __in  UINT uExitCode
	);

	VOID WINAPI My_ExitThread(
	  __in  DWORD dwExitCode
	);

	NTSTATUS NTAPI My_NtTerminateThread(
		__in HANDLE               ThreadHandle,
		__in NTSTATUS             ExitStatus
	);

	BOOLEAN NTAPI My_RtlDispatchException(
		__in PEXCEPTION_RECORD  ExceptionRecord,  
		__in PCONTEXT           Context   
	);

	BOOL AttachAll();
	BOOL DetachAll();
}