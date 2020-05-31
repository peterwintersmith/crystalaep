// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#include "stdafx.h"
#include "..\..\AuxCore\AuxCore\IRealtimeLog.h"
#include "..\..\AuxCore\AuxCore\IProcTrackerServices.h"
#include "..\..\ProcTracker\ProcTracker\ProcTracker_h.h"
#include "DfnsInDepth.h"
#include "..\..\Shared\Utils\Utils.h"
#include "HeapMonitor.h"
#include "ApiFw.h"
#include "DynConfig.h"
#include "..\..\AuxCore\AuxCore\IConfigReader.h"
#include "..\..\AuxCore\AuxCore\ILoadedModules.h"
#include "..\..\AuxCore\AuxCore\IUiFeedback.h"
#include "..\..\BpCore\BpCore\IBrwsrInstDetails.h"
#include "StackMonitor.h"
#include "..\..\libpdasm\libpdasm\disasm.h"
#include "..\..\SimpleEspEmu\SimpleEspEmu\StackEmu.h"

extern "C" void __RPC_FAR * __RPC_USER midl_user_allocate(size_t len)
{
	return(malloc(len));
}

extern "C" void __RPC_USER midl_user_free(void __RPC_FAR * ptr)
{
	free(ptr);
}

BOOL CreateProtectFileProcess(DWORD dwPID)
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
		bSuccess = ::CreateProtectFileProcess(IProcTracker_v1_0_c_ifspec, dwPID);
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

BOOL DefenseInDepth::AttachAll()
{
	hmNtdll = GetModuleHandle(L"ntdll.dll");
	hmKernel32 = GetModuleHandle(L"kernel32.dll");
	hmKernelBase = GetModuleHandle(L"kernelbase.dll");
	hmDiDCore = GetModuleHandle(L"didcore.dll");
	hmAuxCore = GetModuleHandle(L"auxcore.dll");
	hmBpCore = GetModuleHandle(L"bpcore.dll");
	
	InitializeCriticalSection(&csRWXMods);
	InitializeCriticalSection(&csSehWalk);
	
	GetSystemInfo(&stSystemInfo);

	ILoadedModules *pLoadedMods = GetLoadedModules();

	ISingleLoadedModule *pSlm = pLoadedMods->FindDllWithPartialName(L"ntdll");
	if(pSlm)
		dwNtdllImageSize = pSlm->DllImageSize();

	pSlm = pLoadedMods->FindDllWithPartialName(L"kernel32");
	if(pSlm)
		dwKernel32ImageSize = pSlm->DllImageSize();

	pSlm = pLoadedMods->FindDllWithPartialName(L"kernelbase");
	if(pSlm)
		dwKernelBaseImageSize = pSlm->DllImageSize();

	pvLdrLoadDll = (PVOID)GetProcAddress(hmNtdll, "LdrLoadDll");
	pvRtlUserThreadStart = (PVOID)GetProcAddress(hmNtdll, "RtlUserThreadStart");
	
	LONG lResult = 0;
	
	IProcTrackerServices *pProcTracker = GetProcTrackerServices();
	IRealtimeLog *pRealtimeLog = GetRealtimeLog();
	
	DETOUR_TRANS_BEGIN
	Real_RtlDispatchException = FindRtlDispatchException();
	DetourAttach((PVOID *)&Real_RtlDispatchException, My_RtlDispatchException);
	lResult = DETOUR_TRANS_COMMIT
	
	DETOUR_TRANS_BEGIN
	ATTACH_DETOUR_GPA(L"ntdll.dll", "RtlCreateHeap", RtlCreateHeap);
	ATTACH_DETOUR_GPA(L"ntdll.dll", "RtlAllocateHeap", HeapAlloc);
	ATTACH_DETOUR_GPA(L"ntdll.dll", "RtlReAllocateHeap", HeapReAlloc);
	ATTACH_DETOUR_GPA(L"ntdll.dll", "RtlFreeHeap", HeapFree);
	ATTACH_DETOUR_GPA(L"ntdll.dll", "RtlSizeHeap", HeapSize);
	ATTACH_DETOUR_GPA(L"ntdll.dll", "RtlDestroyHeap", HeapDestroy);
	lResult = DETOUR_TRANS_COMMIT
	
	switch(lResult)
	{
	case NO_ERROR:
		pRealtimeLog->LogMessage(0, L"Enabled the HeapMonitor feature (WinHeap)");
		Globals::g_dwEnabledFeatures = pProcTracker->GetEnabledFeaturesProcess(GetCurrentProcessId());
		Globals::g_dwEnabledFeatures |= EEnabledFeatures::HeapMonitorFeature;
		pProcTracker->SetEnabledFeaturesProcess(GetCurrentProcessId(), Globals::g_dwEnabledFeatures);
		break;
	case ERROR_INVALID_BLOCK:
		pRealtimeLog->LogMessage(1, L"Failed to enable fully the HeapMonitor feature");
		break;
	default:
		pRealtimeLog->LogMessage(1, L"Fatal error in HeapMonitor feature");
		break;
	}
	
	DETOUR_TRANS_BEGIN
	ATTACH_DETOUR_GPA(L"kernel32.dll", "LoadLibraryA", LoadLibraryA);
	ATTACH_DETOUR_GPA(L"kernel32.dll", "LoadLibraryW", LoadLibraryW);
	ATTACH_DETOUR_GPA(L"kernel32.dll", "LoadLibraryExA", LoadLibraryExA);
	ATTACH_DETOUR_GPA(L"kernel32.dll", "LoadLibraryExW", LoadLibraryExW);
	ATTACH_DETOUR_GPA(L"ntdll.dll", "LdrLoadDll", LdrLoadDll);
	ATTACH_DETOUR_GPA(L"kernel32.dll", "FreeLibrary", FreeLibrary);
	ATTACH_DETOUR_GPA(L"kernel32.dll", "FreeLibraryAndExitThread", FreeLibraryAndExitThread);
	lResult = DETOUR_TRANS_COMMIT
	
	switch(lResult)
	{
	case NO_ERROR:
		pRealtimeLog->LogMessage(0, L"Enabled the APIMonitor feature (Modules)");
		Globals::g_dwEnabledFeatures = pProcTracker->GetEnabledFeaturesProcess(GetCurrentProcessId());
		Globals::g_dwEnabledFeatures |= EEnabledFeatures::APIMonitorFeature;
		pProcTracker->SetEnabledFeaturesProcess(GetCurrentProcessId(), Globals::g_dwEnabledFeatures);
		break;
	case ERROR_INVALID_BLOCK:
		pRealtimeLog->LogMessage(1, L"Failed to enable fully the APIMonitor feature");
		break;
	default:
		pRealtimeLog->LogMessage(1, L"Fatal error in APIMonitor feature");
		break;
	}
	
	DETOUR_TRANS_BEGIN
	ATTACH_DETOUR_GPA(L"kernel32.dll", "CreateProcessA", CreateProcessA);
	ATTACH_DETOUR_GPA(L"kernel32.dll", "CreateProcessW", CreateProcessW);
	ATTACH_DETOUR_GPA(L"kernel32.dll", "CreateProcessInternalA", CreateProcessInternalA);
	ATTACH_DETOUR_GPA(L"kernel32.dll", "CreateProcessInternalW", CreateProcessInternalW);
	lResult = DETOUR_TRANS_COMMIT
	
	switch(lResult)
	{
	case NO_ERROR:
		pRealtimeLog->LogMessage(0, L"Enabled the APIMonitor feature (Process)");
		Globals::g_dwEnabledFeatures = pProcTracker->GetEnabledFeaturesProcess(GetCurrentProcessId());
		Globals::g_dwEnabledFeatures |= EEnabledFeatures::APIMonitorFeature;
		pProcTracker->SetEnabledFeaturesProcess(GetCurrentProcessId(), Globals::g_dwEnabledFeatures);
		break;
	case ERROR_INVALID_BLOCK:
		pRealtimeLog->LogMessage(1, L"Failed to enable fully the APIMonitor feature");
		break;
	default:
		pRealtimeLog->LogMessage(1, L"Fatal error in APIMonitor feature");
		break;
	}
	
	DETOUR_TRANS_BEGIN
	ATTACH_DETOUR_GPA(L"kernel32.dll", "GetProcAddress", GetProcAddress);
	ATTACH_DETOUR_GPA(L"kernel32.dll", "GetModuleHandleA", GetModuleHandleA);
	ATTACH_DETOUR_GPA(L"kernel32.dll", "GetModuleHandleW", GetModuleHandleW);
	lResult = DETOUR_TRANS_COMMIT
	
	switch(lResult)
	{
	case NO_ERROR:
		pRealtimeLog->LogMessage(0, L"Enabled the APIMonitor feature (Exports)");
		Globals::g_dwEnabledFeatures = pProcTracker->GetEnabledFeaturesProcess(GetCurrentProcessId());
		Globals::g_dwEnabledFeatures |= EEnabledFeatures::APIMonitorFeature;
		pProcTracker->SetEnabledFeaturesProcess(GetCurrentProcessId(), Globals::g_dwEnabledFeatures);
		break;
	case ERROR_INVALID_BLOCK:
		pRealtimeLog->LogMessage(1, L"Failed to enable fully the APIMonitor feature");
		break;
	default:
		pRealtimeLog->LogMessage(1, L"Fatal error in APIMonitor feature");
		break;
	}

	DETOUR_TRANS_BEGIN
	ATTACH_DETOUR_GPA(L"kernel32.dll", "VirtualProtect", VirtualProtect);
	ATTACH_DETOUR_GPA(L"kernel32.dll", "VirtualProtectEx", VirtualProtectEx);
	ATTACH_DETOUR_GPA(L"ntdll.dll", "NtProtectVirtualMemory", NtProtectVirtualMemory);
	ATTACH_DETOUR_GPA(L"kernel32.dll", "VirtualAlloc", VirtualAlloc);
	ATTACH_DETOUR_GPA(L"kernel32.dll", "VirtualAllocEx", VirtualAllocEx);
	ATTACH_DETOUR_GPA(L"ntdll.dll", "NtAllocateVirtualMemory", NtAllocateVirtualMemory);
	lResult = DETOUR_TRANS_COMMIT
	
	switch(lResult)
	{
	case NO_ERROR:
		pRealtimeLog->LogMessage(0, L"Enabled the ROPMonitor feature (API)");
		Globals::g_dwEnabledFeatures = pProcTracker->GetEnabledFeaturesProcess(GetCurrentProcessId());
		Globals::g_dwEnabledFeatures |= EEnabledFeatures::ROPMonitorFeature;
		pProcTracker->SetEnabledFeaturesProcess(GetCurrentProcessId(), Globals::g_dwEnabledFeatures);
		break;
	case ERROR_INVALID_BLOCK:
		pRealtimeLog->LogMessage(1, L"Failed to enable fully the ROPMonitor feature");
		break;
	default:
		pRealtimeLog->LogMessage(1, L"Fatal error in ROPMonitor feature");
		break;
	}

	// this can prove an easy technique for bypassing DEP on older Windows!
	DETOUR_TRANS_BEGIN
	ATTACH_DETOUR_GPA(L"kernel32.dll", "SetProcessDEPPolicy", SetProcessDEPPolicy);
	lResult = DETOUR_TRANS_COMMIT
	
	switch(lResult)
	{
	case NO_ERROR:
		pRealtimeLog->LogMessage(0, L"Enabled the ROPMonitor feature (DEP [1])");
		break;
	case ERROR_INVALID_BLOCK:
		pRealtimeLog->LogMessage(1, L"Failed to enable fully the ROPMonitor feature");
		break;
	default:
		pRealtimeLog->LogMessage(1, L"Serious error in ROPMonitor feature");
		break;
	}

	// as can this!
	DETOUR_TRANS_BEGIN
	ATTACH_DETOUR_GPA(L"ntdll.dll", "NtSetInformationProcess", NtSetInformationProcess);
	lResult = DETOUR_TRANS_COMMIT
	
	switch(lResult)
	{
	case NO_ERROR:
		pRealtimeLog->LogMessage(0, L"Enabled the ROPMonitor feature (DEP [2])");
		break;
	case ERROR_INVALID_BLOCK:
		pRealtimeLog->LogMessage(1, L"Failed to enable fully the ROPMonitor feature");
		break;
	default:
		pRealtimeLog->LogMessage(1, L"Serious error in ROPMonitor feature");
		break;
	}
	DETOUR_TRANS_BEGIN
	ATTACH_DETOUR_GPA(L"ole32.dll", "CoCreateInstance", CoCreateInstance);
	ATTACH_DETOUR_GPA(L"ole32.dll", "CoGetClassObject", CoGetClassObject);
	lResult = DETOUR_TRANS_COMMIT
	
	switch(lResult)
	{
	case NO_ERROR:
		pRealtimeLog->LogMessage(0, L"Enabled the COMMonitor feature (API)");
		Globals::g_dwEnabledFeatures = pProcTracker->GetEnabledFeaturesProcess(GetCurrentProcessId());
		Globals::g_dwEnabledFeatures |= EEnabledFeatures::COMMonitorFeature;
		pProcTracker->SetEnabledFeaturesProcess(GetCurrentProcessId(), Globals::g_dwEnabledFeatures);
		break;
	case ERROR_INVALID_BLOCK:
		pRealtimeLog->LogMessage(1, L"Failed to enable fully the COMMonitor feature");
		break;
	default:
		pRealtimeLog->LogMessage(1, L"Fatal error in COMMonitor feature");
		break;
	}
	
	DETOUR_TRANS_BEGIN
	ATTACH_DETOUR_GPA(L"kernel32.dll", "CreateThread", CreateThread);
	//ATTACH_DETOUR_GPA(L"kernel32.dll", "CreateRemoteThread", CreateRemoteThread);
	ATTACH_DETOUR_GPA(L"ntdll.dll", "NtCreateThread", NtCreateThread);
	ATTACH_DETOUR_GPA(L"ntdll.dll", "NtCreateThreadEx", NtCreateThreadEx);
	ATTACH_DETOUR_GPA(L"kernel32.dll", "ExitProcess", ExitProcess);
	//ATTACH_DETOUR_GPA(L"kernel32.dll", "ExitThread", ExitThread);
	ATTACH_DETOUR_GPA(L"ntdll.dll", "NtTerminateThread", NtTerminateThread);
	lResult = DETOUR_TRANS_COMMIT
	
	switch(lResult)
	{
	case NO_ERROR:
		pRealtimeLog->LogMessage(0, L"Enabled the APIMonitor feature (Thread)");
		Globals::g_dwEnabledFeatures = pProcTracker->GetEnabledFeaturesProcess(GetCurrentProcessId());
		Globals::g_dwEnabledFeatures |= EEnabledFeatures::APIMonitorFeature;
		pProcTracker->SetEnabledFeaturesProcess(GetCurrentProcessId(), Globals::g_dwEnabledFeatures);
		break;
	case ERROR_INVALID_BLOCK:
		pRealtimeLog->LogMessage(1, L"Failed to enable fully the APIMonitor feature");
		break;
	default:
		pRealtimeLog->LogMessage(1, L"Fatal error in APIMonitor feature");
		break;
	}

	CHeapMonitor *pHeapMon = CHeapMonitor::GetInstance();
	
	pHeapMon->Real_HeapAlloc		= Real_HeapAlloc;
	pHeapMon->Real_HeapReAlloc		= Real_HeapReAlloc;
	pHeapMon->Real_HeapFree			= Real_HeapFree;
	pHeapMon->Real_RtlCreateHeap	= Real_RtlCreateHeap;
	pHeapMon->Real_HeapDestroy		= Real_HeapDestroy;
	pHeapMon->Real_HeapSize			= Real_HeapSize;
	
	pHeapMon->SetTerminateOnCorruptionAllHeaps();

	IConfigReader *pConfig = GetConfigReader();
	
	InitializeCriticalSection(&csConfigReload);
	pConfig->Subscribe(ConfigRefresh);

	SafeToDeferHeapCalls = TRUE;

	return TRUE;
}

BOOL DefenseInDepth::DetachAll()
{
	IRealtimeLog *pRealtimeLog = GetRealtimeLog();
	pRealtimeLog->LogMessage(1, L"Removing all defense-in-depth aspects");

	DETOUR_TRANS_BEGIN
	DETACH_DETOUR(RtlCreateHeap);
	DETACH_DETOUR(HeapAlloc);
	DETACH_DETOUR(HeapReAlloc);
	DETACH_DETOUR(HeapFree);
	DETACH_DETOUR(HeapSize);
	DETACH_DETOUR(HeapDestroy);
	DETACH_DETOUR(LoadLibraryA);
	DETACH_DETOUR(LoadLibraryW);
	DETACH_DETOUR(LoadLibraryExA);
	DETACH_DETOUR(LoadLibraryExW);
	DETACH_DETOUR(LdrLoadDll);
	DETACH_DETOUR(FreeLibrary);
	DETACH_DETOUR(FreeLibraryAndExitThread);
	DETACH_DETOUR(CreateProcessA);
	DETACH_DETOUR(CreateProcessW);
	DETACH_DETOUR(CreateProcessInternalA);
	DETACH_DETOUR(CreateProcessInternalW);
	DETACH_DETOUR(GetProcAddress);
	DETACH_DETOUR(GetModuleHandleA);
	DETACH_DETOUR(GetModuleHandleW);
	DETACH_DETOUR(VirtualProtect);
	DETACH_DETOUR(VirtualProtectEx);
	DETACH_DETOUR(NtProtectVirtualMemory);
	DETACH_DETOUR(VirtualAlloc);
	DETACH_DETOUR(VirtualAllocEx);
	DETACH_DETOUR(NtAllocateVirtualMemory);
	DETACH_DETOUR(SetProcessDEPPolicy);
	DETACH_DETOUR(NtSetInformationProcess);
	DETACH_DETOUR(CoCreateInstance);
	DETACH_DETOUR(CoGetClassObject);
	DETACH_DETOUR(CreateThread);
	//DETACH_DETOUR(CreateRemoteThread);
	DETACH_DETOUR(NtCreateThread);
	DETACH_DETOUR(NtCreateThreadEx);
	DETACH_DETOUR(ExitProcess);
	//DETACH_DETOUR(ExitThread);
	DETACH_DETOUR(NtTerminateThread);
	DETACH_DETOUR(RtlDispatchException);
	DETOUR_TRANS_COMMIT
	return TRUE;
}

BOOL DefenseInDepth::IsWithinTrustedMemory(LPVOID lpvAddress)
{
	CHeapMonitor *pHeapMon = CHeapMonitor::GetInstance();
	
	return (
			Utils::IsMemMappedOrModule(lpvAddress)
			|| pHeapMon->IsAllocFromExecutableHeap(lpvAddress)
		);
}

// it doesn't seem to be an export, so I have to disassemble KiUserExceptionDispatcher
DefenseInDepth::FN_RTLDISPATCHEXCEPTION_DEF DefenseInDepth::FindRtlDispatchException()
{
	PBYTE pb = (PBYTE)GetProcAddress(GetModuleHandle(L"ntdll.dll"), "KiUserExceptionDispatcher");
	if(!pb)
		return NULL;

	for(DWORD i=0; i < 10; i++)
	{
		DWORD dwInstrLen = Disasm::GetInstructionLength(pb);
		if(!dwInstrLen)
			return NULL;

		if(pb[0] == 0xe8) // call
		{
			return (DefenseInDepth::FN_RTLDISPATCHEXCEPTION_DEF)(pb + 5 + *(DWORD *)(pb + 1));
		}

		pb += dwInstrLen;
	}

	return NULL;
}

BOOL DefenseInDepth::GetSEHHandlerAndNext(DWORD dwNthEntry, LPVOID *plpvHandler, LPVOID *plpvPrev)
{
	BOOL bFoundEntry = FALSE;
	PEXCEPTION_REGISTRATION pSehChain = NULL;

	__asm
	{
		push eax
		mov eax, dword ptr fs:[0]
		mov pSehChain, eax
		pop eax
	}

	for(DWORD i=0; i < dwNthEntry; i++)
	{
		if(Utils::IsBadReadPtr(pSehChain, sizeof(EXCEPTION_REGISTRATION)))
		{
			// i don't know if this should be trusted
			TerminateProcess(GetCurrentProcess(), BPCORE_TERMINATE_SEH_FAULT);
		}

		pSehChain = pSehChain->Prev;
	}

	if(pSehChain && pSehChain != (PEXCEPTION_REGISTRATION)0xffffffff)
	{
		if(Utils::IsBadReadPtr(pSehChain, sizeof(EXCEPTION_REGISTRATION)))
		{
			TerminateProcess(GetCurrentProcess(), BPCORE_TERMINATE_SEH_FAULT);
		}

		*plpvHandler = pSehChain->Handler;
		*plpvPrev = pSehChain->Prev;

		bFoundEntry = TRUE;
	}
	
Cleanup:
	return bFoundEntry;
}

BOOL DefenseInDepth::IsCodeSEHExploitSequence(CONTEXT *pContext, LPVOID lpvAddress)
{
	BOOL bIsExplSeq = FALSE, bEmuSuccess = FALSE;
	int iStackDelta = 0;

	if(!StackEmu::CalculateStackDelta(pContext, (PBYTE)lpvAddress, &iStackDelta, &bEmuSuccess, FALSE, FALSE))
		goto Cleanup; // return safe on failure
	
	if(bEmuSuccess == FALSE)
		goto Cleanup; // can't really determine if it's an exploit attempt - return safe

	if(iStackDelta == 8)
	{
		// pop [+4]
		// pop [+4]
		// ret [0 if emulate_retn = FALSE]
		// => esp = esp + 8 bytes
		bIsExplSeq = TRUE;
	}

Cleanup:
	return bIsExplSeq;
}

/*BOOL DefenseInDepth::IsCodeSEHExploitSequence(LPVOID lpvAddress)
{
	PBYTE pbPtr = (PBYTE)lpvAddress;

	// (1) POP/POP/RET
	// (2) ADD ESP, 8/RET
	// (3) SUB ESP, -8/RET

	// this whole function needs to be replaced with a simple state machine, parsing commands which alter
	// esp and recording the deltas til RET, then checking whether they are equivalent to add esp, 8/ret
	
#define ISPOP(ptr)			((ptr[0] - 0x58) >= 0 && (ptr[0] - 0x58) <= 8)
#define ISADDESP8(ptr)		(ptr[0] == 0x83 && ptr[1] == 0xC4 && ptr[2] == 0x08)
#define ISSUBESPMINUS8(ptr)	(ptr[0] == 0x83 && ptr[1] == 0xEC && ptr[2] == 0xF8)
#define ISADDESP4(ptr)		(ptr[0] == 0x83 && ptr[1] == 0xC4 && ptr[2] == 0x04)
#define ISSUBESPMINUS4(ptr)	(ptr[0] == 0x83 && ptr[1] == 0xEC && ptr[2] == 0xFC)
#define ISRET(ptr)			(ptr[0] == 0xC2 || ptr[0] == 0xC3)

	/*	
		779801B8   58               POP EAX                                  ; iexplore.<ModuleEntryPoint>
		779801B9   58               POP EAX
		779801BA   C3               RETN

		779801BB   5E               POP ESI
		779801BC   5E               POP ESI
		779801BD   C2 0800          RETN 8

		779801C0   83C4 08          ADD ESP,8
		779801C3   C2 1000          RETN 10

		779801C6   83EC F8          SUB ESP,-8
		779801C9   C2 2000          RETN 20
	*/
	/*
	if(ISPOP(pbPtr) && ISPOP((pbPtr + 1)) && ISRET((pbPtr + 2)))
	{
		return TRUE;
	}

	if(ISADDESP8(pbPtr) && ISRET((pbPtr + 3)))
	{
		return TRUE;
	}

	if(ISSUBESPMINUS8(pbPtr) && ISRET((pbPtr + 3)))
	{
		return TRUE;
	}

	return FALSE;
}*/


// x ntdll*!RtlDispatchException

#define	MEM_CLEAR_CHAR			0xec

BOOLEAN NTAPI DefenseInDepth::My_RtlDispatchException(
	__in PEXCEPTION_RECORD  ExceptionRecord,  
	__in PCONTEXT           Context   
)
{
	if(bCheckExceptionDisp)
	{
		// check fault is not STATUS_ACCESS_VIOLATION and faulting address is not a stack base

		if(ExceptionRecord->ExceptionCode == STATUS_ACCESS_VIOLATION)
		{
			if(ExceptionRecord->ExceptionInformation[0] == 1) // this is a write AV
			{
				CStackMonitor *pStackMon = CStackMonitor::GetInstance();
				pStackMon->RefreshThreadStack(GetCurrentThreadId(), GetCurrentThread());

				LPVOID lpvFaultingAddress = (LPVOID)ExceptionRecord->ExceptionInformation[1];

				if(pStackMon->IsValidStackBase(GetCurrentThreadId(), lpvFaultingAddress))
				{
					// probable stack overflow overwriting SEH handler
					TerminateProcess(GetCurrentProcess(), BPCORE_TERMINATE_SEH_FAULT);
				}
			}
			else if(ExceptionRecord->ExceptionInformation[0] == 0) // this is a read AV
			{
				LPVOID lpvFaultingAddress = (LPVOID)ExceptionRecord->ExceptionInformation[1];

				if(((DWORD)lpvFaultingAddress >> 24) == MEM_CLEAR_CHAR)
				{
					// address of form 0xECxxxxxx indiciates it was probably memory we cleared - a use after free
					TerminateProcess(GetCurrentProcess(), BPCORE_TERMINATE_USE_AFTER_FREE);
				}
			}
		}

		// walk SEH chain, validate all pointers (for lack of a better method at this stage)

		ILoadedModules *pLoadedMods = GetLoadedModules();

		EnterCriticalSection(&csSehWalk);

		LPVOID lpvHandler = NULL, lpvNext = NULL;

		for(DWORD i=0; GetSEHHandlerAndNext(i, &lpvHandler, &lpvNext); i++)
		{
			// check dispatch address is in loaded module (i.e. not stack or heap)

			if(!pLoadedMods->IsAddressInLoadedModule(lpvHandler))
			{
				pLoadedMods->EnumLoadedModules();
			
				if(!pLoadedMods->IsAddressInLoadedModule(lpvHandler))
				{
					TerminateProcess(GetCurrentProcess(), BPCORE_TERMINATE_SEH_FAULT);
				}
			}

			if(bEmuExceptHandler)
			{
				// check dispatch address does not point to POP/POP/RET, or ADD ESP, 8/RET, or SUB ESP, -8/RET

				if(IsCodeSEHExploitSequence(Context, lpvHandler))
				{
					TerminateProcess(GetCurrentProcess(), BPCORE_TERMINATE_SEH_FAULT);
				}
			}
		}

		LeaveCriticalSection(&csSehWalk);
	}

	return Real_RtlDispatchException(ExceptionRecord, Context);
}

LPVOID WINAPI DefenseInDepth::My_HeapAlloc(
	__in  HANDLE hHeap,
	__in  DWORD dwFlags,
	__in  SIZE_T dwBytes
)
{
	CHeapMonitor *pHeapMon = CHeapMonitor::GetInstance();
	DWORD dwRetnAddress = 0;
	
	GETRETNADDR(dwRetnAddress);
	
	if(!SafeToDeferHeapCalls || ((dwRetnAddress - (DWORD)hmNtdll) < dwNtdllImageSize) ||
		(
		 pHeapMon->m_bSensitiveMonitoring && 
		   ((dwRetnAddress - (DWORD)hmKernel32) < dwKernel32ImageSize ||
		   (dwRetnAddress - (DWORD)hmKernelBase) < dwKernelBaseImageSize)
		)
	)
	{
		LPVOID lpMem = Real_HeapAlloc(hHeap, dwFlags, dwBytes);

		//EnterCriticalSection(&pHeapMon->m_csMonitorOp);
		
		if(pHeapMon->m_htMonHeapAllocs.GetEntry(lpMem) != NULL)
		{
			pHeapMon->m_htMonHeapAllocs.RemoveEntry(lpMem);
		}

		//LeaveCriticalSection(&pHeapMon->m_csMonitorOp);

		return lpMem;
	}

	return pHeapMon->WinHeapAlloc(hHeap, dwFlags, dwBytes);
}

LPVOID WINAPI DefenseInDepth::My_HeapReAlloc(
	__in  HANDLE hHeap,
	__in  DWORD dwFlags,
	__in  LPVOID lpMem,
	__in  SIZE_T dwBytes
)
{
	CHeapMonitor *pHeapMon = CHeapMonitor::GetInstance();
	DWORD dwRetnAddress = 0;
	
	GETRETNADDR(dwRetnAddress);
	
	if(!SafeToDeferHeapCalls || ((dwRetnAddress - (DWORD)hmNtdll) < dwNtdllImageSize) ||
		(
		 pHeapMon->m_bSensitiveMonitoring && 
		   ((dwRetnAddress - (DWORD)hmKernel32) < dwKernel32ImageSize ||
		   (dwRetnAddress - (DWORD)hmKernelBase) < dwKernelBaseImageSize)
		)
	)
	{
		//EnterCriticalSection(&pHeapMon->m_csMonitorOp);
		
		if(pHeapMon->m_htMonHeapAllocs.GetEntry(lpMem) != NULL)
		{
			// if memory is reallocated by a function we can't watch (for reasons of risking infinite recursion)
			// with our heap hooks, let's stop watching this particular allocation. Shouldn't affect our ability
			// to protect third party apps or large parts of Windows.
			
			pHeapMon->m_htMonHeapAllocs.RemoveEntry(lpMem);
		}

		//LeaveCriticalSection(&pHeapMon->m_csMonitorOp);

		return Real_HeapReAlloc(hHeap, dwFlags, lpMem, dwBytes);
	}

	return pHeapMon->WinHeapReAlloc(hHeap, dwFlags, lpMem, dwBytes);
}

BOOL WINAPI DefenseInDepth::My_HeapFree(
	__in  HANDLE hHeap,
	__in  DWORD dwFlags,
	__in  LPVOID lpMem
)
{
	CHeapMonitor *pHeapMon = CHeapMonitor::GetInstance();
	DWORD dwRetnAddress = 0;
	
	GETRETNADDR(dwRetnAddress);

	if(!SafeToDeferHeapCalls || ((dwRetnAddress - (DWORD)hmNtdll) < dwNtdllImageSize) ||
		(
		 pHeapMon->m_bSensitiveMonitoring && 
		   ((dwRetnAddress - (DWORD)hmKernel32) < dwKernel32ImageSize ||
		   (dwRetnAddress - (DWORD)hmKernelBase) < dwKernelBaseImageSize)
		)
	)
	{
		//EnterCriticalSection(&pHeapMon->m_csMonitorOp);
		
		if(pHeapMon->m_htMonHeapAllocs.GetEntry(lpMem) != NULL)
		{
			// if memory allocated by a region for which allocs are monitored, and later free by
			// a region for which memory is not monitored, and then a later alloc returns the
			// same base, the monitored alloc will be incorrect and lead to IsCorrupt() => TRUE
			
			pHeapMon->m_htMonHeapAllocs.RemoveEntry(lpMem);
		}

		//LeaveCriticalSection(&pHeapMon->m_csMonitorOp);

		return Real_HeapFree(hHeap, dwFlags, lpMem);
	}

	return pHeapMon->WinHeapFree(hHeap, dwFlags, lpMem);
}

PVOID WINAPI DefenseInDepth::My_RtlCreateHeap(
	__in      ULONG Flags,
	__in_opt  PVOID HeapBase,
	__in_opt  SIZE_T ReserveSize,
	__in_opt  SIZE_T CommitSize,
	__in_opt  PVOID Lock,
	__in_opt  LPVOID /*PRTL_HEAP_PARAMETERS*/ Parameters
)
{
	CHeapMonitor *pHeapMon = CHeapMonitor::GetInstance();
	DWORD dwRetnAddress = 0;
	
	GETRETNADDR(dwRetnAddress);
	
	if(!SafeToDeferHeapCalls || ((dwRetnAddress - (DWORD)hmNtdll) < dwNtdllImageSize))
	{
		return Real_RtlCreateHeap(Flags, HeapBase, ReserveSize, CommitSize, Lock, Parameters);
	}

	return pHeapMon->WinHeapCreate(Flags, HeapBase, ReserveSize, CommitSize, Lock, Parameters);
}

SIZE_T WINAPI DefenseInDepth::My_HeapSize(
	__in  HANDLE hHeap,
	__in  DWORD dwFlags,
	__in  LPCVOID lpMem
)
{
	CHeapMonitor *pHeapMon = CHeapMonitor::GetInstance();
	DWORD dwRetnAddress = 0;
	
	GETRETNADDR(dwRetnAddress);
	
	if(!SafeToDeferHeapCalls || ((dwRetnAddress - (DWORD)hmNtdll) < dwNtdllImageSize) ||
		(
		 pHeapMon->m_bSensitiveMonitoring && 
		   ((dwRetnAddress - (DWORD)hmKernel32) < dwKernel32ImageSize ||
		   (dwRetnAddress - (DWORD)hmKernelBase) < dwKernelBaseImageSize)
		)
	)
	{
		//EnterCriticalSection(&pHeapMon->m_csMonitorOp);
		
		if(pHeapMon->m_htMonHeapAllocs.GetEntry(const_cast<LPVOID>(lpMem)) != NULL)
		{
			// if memory allocated by a region for which allocs are monitored, and later free by
			// a region for which memory is not monitored, and then a later alloc returns the
			// same base, the monitored alloc will be incorrect and lead to IsCorrupt() => TRUE
			
			pHeapMon->m_htMonHeapAllocs.RemoveEntry(const_cast<LPVOID>(lpMem));
		}

		//LeaveCriticalSection(&pHeapMon->m_csMonitorOp);

		return Real_HeapSize(hHeap, dwFlags, lpMem);
	}

	return pHeapMon->WinHeapSize(hHeap, dwFlags, lpMem);
}

BOOL WINAPI DefenseInDepth::My_HeapDestroy(
	 __in  HANDLE hHeap
)
{
	CHeapMonitor *pHeapMon = CHeapMonitor::GetInstance();
	DWORD dwRetnAddress = 0;
	
	GETRETNADDR(dwRetnAddress);

	if(!SafeToDeferHeapCalls || ((dwRetnAddress - (DWORD)hmNtdll) < dwNtdllImageSize))
	{
		return Real_HeapDestroy(hHeap);
	}

	return pHeapMon->WinHeapDestroy(hHeap);
}

HMODULE WINAPI DefenseInDepth::My_LoadLibraryA(
	__in	char* lpFileName
)
{
	ILoadedModules *pLoadedMods = GetLoadedModules();
	CApiFirewall *pApiFw = CApiFirewall::GetInstance();
	HMODULE hm = NULL;
	wchar_t *pwszFile = Utils::WcFromMultiByte(lpFileName);
	DWORD dwRetnAddress = 0;

	GETRETNADDR(dwRetnAddress);

	if(bMonitorAPICallOrigins && !IsWithinTrustedMemory((LPVOID)dwRetnAddress))
	{
		pLoadedMods->EnumLoadedModules();

		if(!pLoadedMods->IsAddressInLoadedModule((LPVOID)dwRetnAddress))
		{
#ifdef DEBUG_BUILD
			CLogger::Break();
#endif
			TerminateProcess(GetCurrentProcess(), BPCORE_TERMINATE_UNTRUSTED_CALLER);
		}
	}

	BOOL bContinue = pApiFw->QueryModuleLoadPathAllowed(pwszFile ? pwszFile : L"");
	if(bContinue)
	{
		hm = Real_LoadLibraryExA(lpFileName, NULL, 0);
		if(hm)
		{
			pLoadedMods->AddModule(hm);
		}
	}

	if(pwszFile)
		free(pwszFile);

	return hm;
}

HMODULE WINAPI DefenseInDepth::My_LoadLibraryW(
	__in	LPWSTR lpFileName
)
{
	ILoadedModules *pLoadedMods = GetLoadedModules();
	CApiFirewall *pApiFw = CApiFirewall::GetInstance();
	HMODULE hm = NULL;
	DWORD dwRetnAddress = 0;

	GETRETNADDR(dwRetnAddress);

	if(bMonitorAPICallOrigins && !IsWithinTrustedMemory((LPVOID)dwRetnAddress))
	{
		pLoadedMods->EnumLoadedModules();

		if(!pLoadedMods->IsAddressInLoadedModule((LPVOID)dwRetnAddress))
		{
#ifdef DEBUG_BUILD
			CLogger::Break();
#endif
			TerminateProcess(GetCurrentProcess(), BPCORE_TERMINATE_UNTRUSTED_CALLER);
		}
	}

	BOOL bContinue = pApiFw->QueryModuleLoadPathAllowed(lpFileName ? lpFileName : L"");
	if(bContinue)
	{
		hm = Real_LoadLibraryExW(lpFileName, NULL, 0);
		if(hm)
		{
			pLoadedMods->AddModule(hm);
		}
	}

	return hm;
}

HMODULE WINAPI DefenseInDepth::My_LoadLibraryExA(
	__in        char* lpFileName,
	__reserved  HANDLE hFile,
	__in        DWORD dwFlags
)
{
	ILoadedModules *pLoadedMods = GetLoadedModules();
	CApiFirewall *pApiFw = CApiFirewall::GetInstance();
	HMODULE hm = NULL;
	wchar_t *pwszFile = Utils::WcFromMultiByte(lpFileName);
	DWORD dwRetnAddress = 0;

	GETRETNADDR(dwRetnAddress);

	if(bMonitorAPICallOrigins && !IsWithinTrustedMemory((LPVOID)dwRetnAddress))
	{
		pLoadedMods->EnumLoadedModules();

		if(!pLoadedMods->IsAddressInLoadedModule((LPVOID)dwRetnAddress))
		{
#ifdef DEBUG_BUILD
			CLogger::Break();
#endif
			TerminateProcess(GetCurrentProcess(), BPCORE_TERMINATE_UNTRUSTED_CALLER);
		}
	}

	BOOL bContinue = pApiFw->QueryModuleLoadPathAllowed(pwszFile ? pwszFile : L"");
	if(bContinue)
	{
		hm = Real_LoadLibraryExA(lpFileName, hFile, dwFlags);
		if(hm && !(dwFlags & LOAD_LIBRARY_AS_DATAFILE))
		{
			pLoadedMods->AddModule(hm);
		}
	}

	if(pwszFile)
		free(pwszFile);

	return hm;
}

HMODULE WINAPI DefenseInDepth::My_LoadLibraryExW(
	__in        LPWSTR lpFileName,
	__reserved  HANDLE hFile,
	__in        DWORD dwFlags
)
{
	ILoadedModules *pLoadedMods = GetLoadedModules();
	CApiFirewall *pApiFw = CApiFirewall::GetInstance();
	HMODULE hm = NULL;
	DWORD dwRetnAddress = 0;

	GETRETNADDR(dwRetnAddress);

	if(bMonitorAPICallOrigins && !IsWithinTrustedMemory((LPVOID)dwRetnAddress))
	{
		pLoadedMods->EnumLoadedModules();

		if(!pLoadedMods->IsAddressInLoadedModule((LPVOID)dwRetnAddress))
		{
#ifdef DEBUG_BUILD
			CLogger::Break();
#endif
			TerminateProcess(GetCurrentProcess(), BPCORE_TERMINATE_UNTRUSTED_CALLER);
		}
	}

	BOOL bContinue = pApiFw->QueryModuleLoadPathAllowed(lpFileName ? lpFileName : L"");
	if(bContinue)
	{
		hm = Real_LoadLibraryExW(lpFileName, hFile, dwFlags);
		if(hm && !(dwFlags & LOAD_LIBRARY_AS_DATAFILE))
		{
			pLoadedMods->AddModule(hm);
		}
	}

	return hm;
}

BOOL DefenseInDepth::DllContainsString(HMODULE hmModule, void *pSearch, BOOL bUnicode)
{
	BOOL bSuccess = FALSE;
	MODULEINFO mi = {0};
	DWORD cbSearch = bUnicode ? (wcslen((PWSTR)pSearch) * sizeof(WCHAR)) : strlen((char *)pSearch);

	if(cbSearch >= DEFAULT_PAGE_SIZE) // this isn't safe [1]
		goto Cleanup;

	if(!GetModuleInformation(GetCurrentProcess(), hmModule, &mi, sizeof(mi)))
		goto Cleanup;

	PBYTE pbDllBase = (PBYTE)mi.lpBaseOfDll;

	while(pbDllBase < (PBYTE)mi.lpBaseOfDll + mi.SizeOfImage)
	{
		MEMORY_BASIC_INFORMATION mbi = {0};

		if(!VirtualQuery(pbDllBase, &mbi, sizeof(mbi)))
			goto Cleanup;

		// if accessible
		if((mbi.State & MEM_COMMIT) && !(mbi.Protect & PAGE_GUARD) && !(mbi.Protect & PAGE_NOACCESS))
		{
			// if accessible for read
			if((mbi.Protect & PAGE_EXECUTE_READ) || (mbi.Protect & PAGE_EXECUTE_READWRITE) ||
				(mbi.Protect & PAGE_READONLY) || (mbi.Protect & PAGE_READWRITE))
			{
				for(DWORD i=0; i < mbi.RegionSize - cbSearch; i++) // [1]
				{
					if(!memcmp(pbDllBase + i, pSearch, cbSearch))
					{
						//WCHAR wszDebugOut[256] = {0};
						//wsprintf(wszDebugOut, L"Detoured tag @ 0x%p\n", pbDllBase + i);
						//OutputDebugString(wszDebugOut);
						
						// found string bytes
						bSuccess = TRUE;
						goto Cleanup;
					}
				}
			}
		}

		// advance to next section
		pbDllBase += mbi.RegionSize;
	}

Cleanup:
	return bSuccess;
}

//http://www.matcode.com/undocwin.h.txt

NTSTATUS NTAPI DefenseInDepth::My_LdrLoadDll(
	__in PWCHAR				PathToFile,
	__in ULONG				Flags,
	__in PUNICODE_STRING	ModuleFileName,
	__out PHANDLE			ModuleHandle
)
{
	ILoadedModules *pLoadedMods = GetLoadedModules();
	CApiFirewall *pApiFw = CApiFirewall::GetInstance();
	CHeapMonitor *pHeapMon = CHeapMonitor::GetInstance();
	NTSTATUS ntStatus = 0xc0000135; // UNABLE_TO_LOAD_DLL
	DWORD dwRetnAddress = 0;
	
	GETRETNADDR(dwRetnAddress);

	if(bMonitorAPICallOrigins && !IsWithinTrustedMemory((LPVOID)dwRetnAddress))
	{
		pLoadedMods->EnumLoadedModules();

		if(!pLoadedMods->IsAddressInLoadedModule((LPVOID)dwRetnAddress))
		{
			//DebugBreak();
			//pLoadedMods->DebugOutputAllModules();
			//if(!pLoadedMods->IsAddressInLoadedModule((LPVOID)dwRetnAddress))
			//	DebugBreak();
#ifdef DEBUG_BUILD
			CLogger::Break();
#endif
			TerminateProcess(GetCurrentProcess(), BPCORE_TERMINATE_UNTRUSTED_CALLER);
		}
	}

	std::wstring wstrModuleName((WCHAR *)ModuleFileName->Buffer, ModuleFileName->Length / sizeof(WCHAR));
	WCHAR *pwszModuleName = const_cast<WCHAR *>(wstrModuleName.c_str());

	std::map<std::wstring, HMODULE>::iterator it;
	std::wstring wstrToAdd;

	if(pwszModuleName)
	{
		EnterCriticalSection(&csRWXMods);

		for(it = mapAllowRWXMods.begin(); it != mapAllowRWXMods.end(); it++)
		{
			if(Utils::wcsistr(pwszModuleName, const_cast<WCHAR *>(it->first.c_str())))
			{
				wstrToAdd = it->first;
				break;
			}
		}

		LeaveCriticalSection(&csRWXMods);
	}

	BOOL bContinue = pApiFw->QueryModuleLoadPathAllowed(pwszModuleName ? pwszModuleName : L"");

	// temporary fix for Silverlight/.NET
	if(bContinue)
	{
		if(bMonitorAPICallOrigins || bDisableRWXVAMemory)
		{
			BOOL bRequiresSecDowngrade = FALSE;
			std::wstring wstrChoiceMade, wstrChoice, wstrTitle, wstrCaption;
			/*
			if(Utils::wcsistr(pwszModuleName ? pwszModuleName : L"", L"npctrl") != NULL)
			{
				wstrChoiceMade = L"SilverlightReduceProtSetting_ChoiceMade";
				wstrChoice = L"SilverlightReduceProtSetting_Choice";
				wstrTitle = L"Relax Protection for Silverlight";
				wstrCaption = L"The application is attempting to load Silverlight. This is likely to happen if you are using a Silverlight based media player (e.g. Netflix) or browser-based game.\r\n\r\n"
							L"Silverlight requires that Crystal relax some of its protection methods (Monitor Call Origins). "
							L"Would you like allow Silverlight to run?\r\n\r\nPlease choose 'No' if you did not expect to run Silverlight.";

				bRequiresSecDowngrade = TRUE;
			}
			else */
			if(Utils::wcsistr(pwszModuleName ? pwszModuleName : L"", L"mscoree") != NULL ||
					Utils::wcsistr(pwszModuleName ? pwszModuleName : L"", L"coreclr") != NULL)
			{
				wstrChoiceMade = L"DotNetReduceProtSetting_ChoiceMade";
				wstrChoice = L"DotNetReduceProtSetting_Choice";
				wstrTitle = L"Relax Protection for .NET";
				wstrCaption = L"The application is attempting to load the Microsoft .NET runtime components. "
							L"This may be completely normal, but requires Crystal to relax some of its protection methods (Monitor Call Origins, Disable RWX Memory) to ensure that the application functions as expected.\r\n\r\n"
							L"Would you like to relax these protection methods, allowing the application to run?";

				bRequiresSecDowngrade = TRUE;
			}

			if(bRequiresSecDowngrade)
			{
				if(bPromptUserRelaxDotnet)
				{
					BOOL bUserChoiceMade = FALSE, bUserChoice = FALSE;
				
					bUserChoiceMade = CDynamicConfig::GetInstance()->GetProperty<BOOL>(
						CDynamicConfig::EDynamicRepository::DfnsInDepth,
						wstrChoiceMade.c_str()
						);

					if(bUserChoiceMade)
					{
						bUserChoice =  CDynamicConfig::GetInstance()->GetProperty<BOOL>(CDynamicConfig::EDynamicRepository::DfnsInDepth, wstrChoice.c_str());
						bContinue = bUserChoice;
					}
					else
					{
						BOOL bToggleCallOrigins = GetUiFeedback()->UserBoolFeedback(
								UiStyle_IconQuestion,
								const_cast<WCHAR *>(wstrTitle.c_str()),
								const_cast<WCHAR *>(wstrCaption.c_str())
							);

						if(bToggleCallOrigins)
						{
							// no need to persist choice as the prompt above is never shown when bMonitorAPICallOrigins=FALSE
							
							bMonitorAPICallOrigins = FALSE;
							bDisableRWXVAMemory = FALSE;
						}
						else
						{
							bContinue = FALSE;

							CDynamicConfig::GetInstance()->SetProperty<BOOL>(CDynamicConfig::EDynamicRepository::DfnsInDepth, wstrChoiceMade.c_str(), TRUE);
							CDynamicConfig::GetInstance()->SetProperty<BOOL>(CDynamicConfig::EDynamicRepository::DfnsInDepth, wstrChoice.c_str(), bContinue);
						}
					}
				}
				else
				{
					bMonitorAPICallOrigins = FALSE;
					bDisableRWXVAMemory = FALSE;
				}
			}
		}
	}
	// end temporary fix

	if(bContinue)
	{
		if(NT_SUCCESS(ntStatus = Real_LdrLoadDll(PathToFile, Flags, ModuleFileName, ModuleHandle)))
		{
			//WCHAR wszTmp[512];
			//wsprintf(wszTmp, L"LdrLoadDll(HMODULE = 0x%p)\n", ModuleHandle ? *ModuleHandle : NULL);
			//OutputDebugString(wszTmp);

			if(*ModuleHandle && !(Flags & LOAD_LIBRARY_AS_DATAFILE))
			{
				pLoadedMods->AddModule((HMODULE)*ModuleHandle);
			}

			EnterCriticalSection(&csRWXMods);

			if(!wstrToAdd.empty())
			{
				mapAllowRWXMods[wstrToAdd] = (HMODULE)*ModuleHandle;
			}
			else
			{
				if(bAutoAddHookDllRWX)
				{
					if(DllContainsString((HMODULE)*ModuleHandle, "detoured.dll"))
					{
						WCHAR wszHandleValue[32] = {0};
						wsprintf(wszHandleValue, L"0x%p.dll", (HMODULE)*ModuleHandle);
						
						// the name need not be correct (only unique) as it's not validated by My_Nt*VirtualMemory
						mapAllowRWXMods[wszHandleValue] = (HMODULE)*ModuleHandle;
					}
				}
			}

			LeaveCriticalSection(&csRWXMods);
		}
	}

	return ntStatus;
}

BOOL WINAPI DefenseInDepth::My_FreeLibrary(
  __in  HMODULE hModule
)
{
	ILoadedModules *pLoadedMods = GetLoadedModules();
	DWORD dwRetnAddress = 0;

	GETRETNADDR(dwRetnAddress);

	// this leads to crash during shutdown when some modules are unloading. This is probably a bug in my code somehow.
	// should fix this properly because there may be an attack to be had freeing an arbitrary in-use library (think
	// data-section disappearing). Seems improbable that it'd be exploitable.
	
	/*
	pLoadedMods->EnumLoadedModules();

	if(bMonitorAPICallOrigins && !pLoadedMods->IsAddressInLoadedModule((LPVOID)dwRetnAddress))
	{
		//CLogger::Break();
		//TerminateProcess(GetCurrentProcess(), BPCORE_TERMINATE_UNTRUSTED_CALLER);
	}
	*/

	BOOL bRetVal = Real_FreeLibrary(hModule);

	if(bRetVal)
	{
		pLoadedMods->RemoveModule(hModule);
	}

	return bRetVal;
}

VOID WINAPI DefenseInDepth::My_FreeLibraryAndExitThread(
  __in  HMODULE hModule,
  __in  DWORD dwExitCode
)
{
	ILoadedModules *pLoadedMods = GetLoadedModules();
	DWORD dwRetnAddress = 0;

	GETRETNADDR(dwRetnAddress);

	// this leads to crash during shutdown when some modules are unloading. This is probably a bug in my code somehow.
	// should fix this properly because there may be an attack to be had freeing an arbitrary in-use library (think
	// data-section disappearing). Seems improbable that it'd be exploitable.
	
	/*
	pLoadedMods->EnumLoadedModules();

	if(bMonitorAPICallOrigins && !pLoadedMods->IsAddressInLoadedModule((LPVOID)dwRetnAddress))
	{
		//CLogger::Break();
		//TerminateProcess(GetCurrentProcess(), BPCORE_TERMINATE_UNTRUSTED_CALLER);
	}
	*/

	pLoadedMods->RemoveModule(hModule);
	
	CStackMonitor *pStackMon = CStackMonitor::GetInstance();
	pStackMon->UnregisterThreadStack(GetCurrentThreadId());
	
	Real_FreeLibraryAndExitThread(hModule, dwExitCode);
}


DWORD WINAPI DefenseInDepth::My_CreateProcessInternalA(
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
)
{
	ILoadedModules *pLoadedMods = GetLoadedModules();
	CApiFirewall *pApiFw = CApiFirewall::GetInstance();
	BOOL bRetVal = FALSE;
	wchar_t *pwszApp = Utils::WcFromMultiByte(lpApplicationName),
		*pwszCmdLine = Utils::WcFromMultiByte(lpCommandLine);
	DWORD dwRetnAddress = 0;

	GETRETNADDR(dwRetnAddress);

	if(bMonitorAPICallOrigins && !IsWithinTrustedMemory((LPVOID)dwRetnAddress))
	{
		pLoadedMods->EnumLoadedModules();

		if(!pLoadedMods->IsAddressInLoadedModule((LPVOID)dwRetnAddress))
		{
#ifdef DEBUG_BUILD
			CLogger::Break();
#endif
			TerminateProcess(GetCurrentProcess(), BPCORE_TERMINATE_UNTRUSTED_CALLER);
		}
	}

	BOOL bContinue = pApiFw->QueryProcessCreationAllowed(
			pwszApp ? pwszApp : L"",
			pwszCmdLine ? pwszCmdLine : L""
		);
	
	if(bContinue)
	{
		BOOL bElevate = FALSE;
		std::wstring wstrExeName;

		BOOL bFoundExeName = Utils::GetExecutableNameFromParams(
			pwszApp ? pwszApp : L"",
			pwszCmdLine ? pwszCmdLine : L"",
			wstrExeName);
		
		if(hToken != NULL && bElevateLowIntegApps &&
			(  bInjectIntoChildProc ||
			   ( bFoundExeName && pwszProtProcList != NULL && Utils::IsContainedInPipeSeparatedList(pwszProtProcList, (WCHAR *)wstrExeName.c_str()) )
			))
		{
			PTOKEN_MANDATORY_LABEL pTIL = NULL;
			DWORD dwInfoSize = 0, dwIntegrityLevel;

			if(!GetTokenInformation(hToken, TokenIntegrityLevel, NULL, 0, &dwInfoSize))
			{
				if(GetLastError() != ERROR_INSUFFICIENT_BUFFER)
					goto OnError;
		
				pTIL = (PTOKEN_MANDATORY_LABEL)LocalAlloc(0, dwInfoSize);
				if(!pTIL)
					goto OnError;

				if(!GetTokenInformation(hToken, TokenIntegrityLevel, pTIL, dwInfoSize, &dwInfoSize))
					goto OnError;

				dwIntegrityLevel = *GetSidSubAuthority(
						pTIL->Label.Sid,
						(DWORD)(UCHAR)(*GetSidSubAuthorityCount(pTIL->Label.Sid) - 1)
					);

				switch(dwIntegrityLevel)
				{
				case SECURITY_MANDATORY_LOW_RID:

					if(bPromptLowIntegApp)
					{
						if(!bQueriedReIntegLevels)
						{
							bElevationChoice = bElevate = GetUiFeedback()->UserBoolFeedback(
									EUiMessageStyle::UiStyle_IconQuestion,
									L"Elevate Process Integrity Level",
									L"An attempt is being made to run a program in low integrity mode. This is completely normal, however Crystal cannot protect low integrity programs.\r\n\r\n"
									L"Programs which attempt to run in low integrity mode are generally designed to be more secure than those which run at other levels, and may not require Crystal.\r\n\r\n"
									L"Would you like to elevate the integrity level of the program to be run to allow Crystal to run ('Yes'), or run the program as usual without elevation ('No')?"
								);

							bQueriedReIntegLevels = TRUE;
						}
						else
						{
							bElevate = bElevationChoice;
						}
					}
					else
					{
						if(bElevateLowIntegApps)
						{
							bElevate = TRUE;
						}
					}

					break;
				default:
					// should be fine
					break;
				}

				if(bElevate)
				{
					// unfortunately IE does not respond well to being elevated without having Protected Mode disabled (at least on some test machines)
					if(!wcsicmp(wstrExeName.c_str(), L"iexplore.exe"))
					{
						// for IE we need to set the Protected Mode setting in the registry
						DWORD dwProtectedModeSetting = 0;
						
						// internet zone
						if(Utils::GetRegistryDword(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\Zones\\3", L"2500", &dwProtectedModeSetting))
						{
							if(dwProtectedModeSetting == 0)
							{
								// protected mode is enabled
								Utils::SetRegistryDword(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\Zones\\3", L"2500", 3);
							}
						}

						// intranet zone
						if(Utils::GetRegistryDword(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\Zones\\1", L"2500", &dwProtectedModeSetting))
						{
							if(dwProtectedModeSetting == 0)
							{
								// protected mode is enabled
								Utils::SetRegistryDword(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\Zones\\1", L"2500", 3);
							}
						}
					}
				}
OnError:
				if(pTIL)
					LocalFree(pTIL);
			}
		}

		BOOL bAlreadySuspended = (dwCreationFlags & CREATE_SUSPENDED) != 0;

		if(bInjectIntoChildProc)
		{
			dwCreationFlags |= CREATE_SUSPENDED;
		}

		bRetVal = Real_CreateProcessInternalA(
			bElevate ? NULL : hToken,
			lpApplicationName,
			lpCommandLine,
			lpProcessAttributes,
			lpThreadAttributes,
			bInheritHandles,
			dwCreationFlags,
			lpEnvironment,
			lpCurrentDirectory,
			lpStartupInfo,
			lpProcessInformation,
			hNewToken
		);

		if(bRetVal && bInjectIntoChildProc)
		{
			CreateProtectFileProcess(lpProcessInformation->dwProcessId);
			
			if(!bAlreadySuspended)
			{
				ResumeThread(lpProcessInformation->hThread);
			}
		}
	}
	
	if(pwszApp)
		free(pwszApp);

	if(pwszCmdLine)
		free(pwszCmdLine);
	
	return bRetVal;
}

DWORD WINAPI DefenseInDepth::My_CreateProcessInternalW(
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
)
{
	ILoadedModules *pLoadedMods = GetLoadedModules();
	CApiFirewall *pApiFw = CApiFirewall::GetInstance();
	BOOL bRetVal = FALSE;
	DWORD dwRetnAddress = 0;

	GETRETNADDR(dwRetnAddress);

	if(bMonitorAPICallOrigins && !IsWithinTrustedMemory((LPVOID)dwRetnAddress))
	{
		pLoadedMods->EnumLoadedModules();

		if(!pLoadedMods->IsAddressInLoadedModule((LPVOID)dwRetnAddress))
		{
#ifdef DEBUG_BUILD
			CLogger::Break();
#endif
			TerminateProcess(GetCurrentProcess(), BPCORE_TERMINATE_UNTRUSTED_CALLER);
		}
	}

	BOOL bContinue = pApiFw->QueryProcessCreationAllowed(
			lpApplicationName ? lpApplicationName : L"",
			lpCommandLine ? lpCommandLine : L""
		);
	
	if(bContinue)
	{
		BOOL bElevate = FALSE;
		std::wstring wstrExeName;

		BOOL bFoundExeName = Utils::GetExecutableNameFromParams(
			lpApplicationName ? lpApplicationName : L"",
			lpCommandLine ? lpCommandLine : L"",
			wstrExeName);
		
		if(hToken != NULL && bElevateLowIntegApps &&
			(  bInjectIntoChildProc ||
			( bFoundExeName && pwszProtProcList != NULL && Utils::IsContainedInPipeSeparatedList(pwszProtProcList, (WCHAR *)wstrExeName.c_str()) )
			))
		{
			PTOKEN_MANDATORY_LABEL pTIL = NULL;
			DWORD dwInfoSize = 0, dwIntegrityLevel;

			if(!GetTokenInformation(hToken, TokenIntegrityLevel, NULL, 0, &dwInfoSize))
			{
				if(GetLastError() != ERROR_INSUFFICIENT_BUFFER)
					goto OnError;
		
				pTIL = (PTOKEN_MANDATORY_LABEL)LocalAlloc(0, dwInfoSize);
				if(!pTIL)
					goto OnError;

				if(!GetTokenInformation(hToken, TokenIntegrityLevel, pTIL, dwInfoSize, &dwInfoSize))
					goto OnError;

				dwIntegrityLevel = *GetSidSubAuthority(
						pTIL->Label.Sid,
						(DWORD)(UCHAR)(*GetSidSubAuthorityCount(pTIL->Label.Sid) - 1)
					);

				switch(dwIntegrityLevel)
				{
				case SECURITY_MANDATORY_LOW_RID:
			
					if(bPromptLowIntegApp)
					{
						if(!bQueriedReIntegLevels)
						{
							bElevationChoice = bElevate = GetUiFeedback()->UserBoolFeedback(
									EUiMessageStyle::UiStyle_IconQuestion,
									L"Elevate Process Integrity Level",
									L"An attempt is being made to run a program in low integrity mode. This is completely normal, however Crystal cannot protect low integrity programs.\r\n\r\n"
									L"Programs which attempt to run in low integrity mode are generally designed to be more secure than those which run at other levels, and may not require Crystal.\r\n\r\n"
									L"Would you like to elevate the integrity level of the program to be run to allow Crystal to run ('Yes'), or run the program as usual without elevation ('No')?"
								);

							bQueriedReIntegLevels = TRUE;
						}
						else
						{
							bElevate = bElevationChoice;
						}
					}
					else
					{
						if(bElevateLowIntegApps)
						{
							bElevate = TRUE;
						}
					}

					break;
				default:
					// should be fine
					break;
				}

				if(bElevate)
				{
					if(!wcsicmp(wstrExeName.c_str(), L"iexplore.exe"))
					{
						// for IE we need to set the Protected Mode setting in the registry
						DWORD dwProtectedModeSetting = 0;
						
						if(Utils::GetRegistryDword(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\Zones\\3", L"2500", &dwProtectedModeSetting))
						{
							if(dwProtectedModeSetting == 0)
							{
								// protected mode is enabled
								Utils::SetRegistryDword(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\Zones\\3", L"2500", 3);
							}
						}
					}
				}
OnError:
				if(pTIL)
					LocalFree(pTIL);
			}
		}

		BOOL bAlreadySuspended = (dwCreationFlags & CREATE_SUSPENDED) != 0;

		if(bInjectIntoChildProc)
		{
			dwCreationFlags |= CREATE_SUSPENDED;
		}

		bRetVal = Real_CreateProcessInternalW(
			bElevate ? NULL : hToken,
			lpApplicationName,
			lpCommandLine,
			lpProcessAttributes,
			lpThreadAttributes,
			bInheritHandles,
			dwCreationFlags,
			lpEnvironment,
			lpCurrentDirectory,
			lpStartupInfo,
			lpProcessInformation,
			hNewToken
		);

		if(bRetVal && bInjectIntoChildProc)
		{
			CreateProtectFileProcess(lpProcessInformation->dwProcessId);

			if(!bAlreadySuspended)
			{
				ResumeThread(lpProcessInformation->hThread);
			}
		}
	}
	
	return bRetVal;
}

BOOL WINAPI DefenseInDepth::My_CreateProcessA(
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
)
{
	ILoadedModules *pLoadedMods = GetLoadedModules();
	BOOL bRetVal = FALSE;
	DWORD dwRetnAddress = 0;

	GETRETNADDR(dwRetnAddress);

	if(bMonitorAPICallOrigins && !IsWithinTrustedMemory((LPVOID)dwRetnAddress))
	{
		pLoadedMods->EnumLoadedModules();

		if(!pLoadedMods->IsAddressInLoadedModule((LPVOID)dwRetnAddress))
		{
#ifdef DEBUG_BUILD
			CLogger::Break();
#endif
			TerminateProcess(GetCurrentProcess(), BPCORE_TERMINATE_UNTRUSTED_CALLER);
		}
	}

	bRetVal = Real_CreateProcessA(
		lpApplicationName,
		lpCommandLine,
		lpProcessAttributes,
		lpThreadAttributes,
		bInheritHandles,
		dwCreationFlags,
		lpEnvironment,
		lpCurrentDirectory,
		lpStartupInfo,
		lpProcessInformation
	);

	return bRetVal;
}

BOOL WINAPI DefenseInDepth::My_CreateProcessW(
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
)
{
	ILoadedModules *pLoadedMods = GetLoadedModules();
	BOOL bRetVal = FALSE;
	DWORD dwRetnAddress = 0;

	GETRETNADDR(dwRetnAddress);

	if(bMonitorAPICallOrigins && !IsWithinTrustedMemory((LPVOID)dwRetnAddress))
	{
		pLoadedMods->EnumLoadedModules();

		if(!pLoadedMods->IsAddressInLoadedModule((LPVOID)dwRetnAddress))
		{
#ifdef DEBUG_BUILD
			CLogger::Break();
#endif
			TerminateProcess(GetCurrentProcess(), BPCORE_TERMINATE_UNTRUSTED_CALLER);
		}
	}

	bRetVal = Real_CreateProcessW(
		lpApplicationName,
		lpCommandLine,
		lpProcessAttributes,
		lpThreadAttributes,
		bInheritHandles,
		dwCreationFlags,
		lpEnvironment,
		lpCurrentDirectory,
		lpStartupInfo,
		lpProcessInformation
	);

	return bRetVal;
}

FARPROC WINAPI DefenseInDepth::My_GetProcAddress(
	__in	HMODULE hModule,
	__in	LPCSTR lpProcName
)
{
	ILoadedModules *pLoadedMods = GetLoadedModules();
	DWORD dwRetnAddress = 0;

	GETRETNADDR(dwRetnAddress);

	if(bMonitorAPICallOrigins && !IsWithinTrustedMemory((LPVOID)dwRetnAddress))
	{
		pLoadedMods->EnumLoadedModules();

		if(!pLoadedMods->IsAddressInLoadedModule((LPVOID)dwRetnAddress))
		{
#ifdef DEBUG_BUILD
			CLogger::Break();
#endif
			TerminateProcess(GetCurrentProcess(), BPCORE_TERMINATE_UNTRUSTED_CALLER);
		}
	}

	return Real_GetProcAddress(hModule, lpProcName);
}

HMODULE WINAPI DefenseInDepth::My_GetModuleHandleA(
	__in_opt	char* lpModuleName
)
{
	ILoadedModules *pLoadedMods = GetLoadedModules();
	DWORD dwRetnAddress = 0;

	GETRETNADDR(dwRetnAddress);

	if(bMonitorAPICallOrigins && !IsWithinTrustedMemory((LPVOID)dwRetnAddress))
	{
		pLoadedMods->EnumLoadedModules();

		if(!pLoadedMods->IsAddressInLoadedModule((LPVOID)dwRetnAddress))
		{
#ifdef DEBUG_BUILD
			CLogger::Break();
#endif
			TerminateProcess(GetCurrentProcess(), BPCORE_TERMINATE_UNTRUSTED_CALLER);
		}
	}

	return Real_GetModuleHandleA(lpModuleName);
}

HMODULE WINAPI DefenseInDepth::My_GetModuleHandleW(
	__in_opt	LPWSTR lpModuleName
)
{
	ILoadedModules *pLoadedMods = GetLoadedModules();
	DWORD dwRetnAddress = 0;

	GETRETNADDR(dwRetnAddress);

	if(bMonitorAPICallOrigins && !IsWithinTrustedMemory((LPVOID)dwRetnAddress))
	{
		pLoadedMods->EnumLoadedModules();

		if(!pLoadedMods->IsAddressInLoadedModule((LPVOID)dwRetnAddress))
		{
#ifdef DEBUG_BUILD
			CLogger::Break();
#endif
			TerminateProcess(GetCurrentProcess(), BPCORE_TERMINATE_UNTRUSTED_CALLER);
		}
	}

	return Real_GetModuleHandleW(lpModuleName);
}

BOOL DefenseInDepth::IsJmpOrCallEsp(LPVOID lpvAddress)
{
	BOOL bIsJmpEsp = FALSE;
	
	PBYTE pbAddress = (PBYTE)lpvAddress;
	if(!pbAddress)
		goto Cleanup;
	
	/*
		jmp esp          => 0xff 0xe4
		call esp         => 0xff 0xd4
		push esp/retn    => 0x54 0xc3
		push esp/ret <n> => 0x54 0xc2 <adjust>
	*/

	struct {
		BYTE Instr[2];
	} JmpEspEquivalents[4] = {
		{0xff, 0xe4},
		{0xff, 0xd4},
		{0x54, 0xc3},
		{0x54, 0xc2}
	};

	for(DWORD i=0; i < 4; i++)
	{
		if(pbAddress[0] == JmpEspEquivalents[i].Instr[0] && pbAddress[1] == JmpEspEquivalents[i].Instr[1])
		{
			bIsJmpEsp = TRUE;
			goto Cleanup;
		}
	}

Cleanup:
	return bIsJmpEsp;
}

BOOL WINAPI DefenseInDepth::My_VirtualProtect(
	__in	LPVOID lpAddress,
	__in	SIZE_T dwSize,
	__in	DWORD flNewProtect,
	__out	PDWORD lpflOldProtect
)
{
	DWORD dwRetnAddress = 0;

	GETRETNADDR(dwRetnAddress);

	if((bMonitorAPICallOrigins || bCheckROPReturn) && !IsWithinTrustedMemory((LPVOID)dwRetnAddress))
	{
		// a VirtualProtect ROP chain will often return from the call to the address of the now executable shellcode

		TerminateProcess(GetCurrentProcess(), bCheckROPReturn ?
			BPCORE_TERMINATE_ROP_DETECTED : BPCORE_TERMINATE_UNTRUSTED_CALLER);
	}

	if(bCheckROPReturn && IsJmpOrCallEsp((LPVOID)dwRetnAddress))
	{
		TerminateProcess(GetCurrentProcess(), BPCORE_TERMINATE_ROP_DETECTED);
	}

	return Real_VirtualProtect(lpAddress, dwSize, flNewProtect, lpflOldProtect);
}

BOOL WINAPI DefenseInDepth::My_VirtualProtectEx(
	__in	HANDLE hProcess,
	__in	LPVOID lpAddress,
	__in	SIZE_T dwSize,
	__in	DWORD flNewProtect,
	__out	PDWORD lpflOldProtect
)
{
	DWORD dwRetnAddress = 0;

	GETRETNADDR(dwRetnAddress);

	if((bMonitorAPICallOrigins || bCheckROPReturn) && !IsWithinTrustedMemory((LPVOID)dwRetnAddress))
	{
		// a VirtualProtect ROP chain will often return from the call to the address of the now executable shellcode

		TerminateProcess(GetCurrentProcess(), bCheckROPReturn ?
			BPCORE_TERMINATE_ROP_DETECTED : BPCORE_TERMINATE_UNTRUSTED_CALLER);
	}

	if(bCheckROPReturn && IsJmpOrCallEsp((LPVOID)dwRetnAddress))
	{
		TerminateProcess(GetCurrentProcess(), BPCORE_TERMINATE_ROP_DETECTED);
	}

	return Real_VirtualProtectEx(hProcess, lpAddress, dwSize, flNewProtect, lpflOldProtect);
}

NTSTATUS NTAPI DefenseInDepth::My_NtProtectVirtualMemory(
	__in HANDLE	ProcessHandle,
	__inout PVOID	*BaseAddress,
	__inout PULONG	NumberOfBytesToProtect,
	__in ULONG		NewAccessProtection,
	__out PULONG	OldAccessProtection
)
{
	/*
		NOTE:
			Steps need to be taken to prevent ret-to-DiDCore.
			
			A suitable strategy might be to have a rolling cookie that is XORed with the pointer to
			the real NtProtectVirtualMemory, and then re-generated and re-XORed on function exit to
			ensure that ret-to-BP will end up executing an invalid address.

			Moving around the memory storing the Real_NtProtectVirtualMemory pointer may also be a
			good idea.
	*/

	ILoadedModules *pLoadedMods = GetLoadedModules();
	DWORD dwRetnAddress = 0, dwEsp = 0;

	GETRETNADDR(dwRetnAddress);
	GETESP(dwEsp);

	// unnecessary check: code already running in w/x memory won't need to use ROP techniques!
	
	/*
	if(!pLoadedMods->IsAddressInLoadedModule((LPVOID)dwRetnAddress))
	{
		CLogger::Break();
		TerminateProcess(GetCurrentProcess(), BPCORE_TERMINATE_UNTRUSTED_CALLER);
	}
	*/

	//WCHAR wszTmp[512];
	//wsprintf(wszTmp, L"NtProtectVirtualMemory(Base = 0x%p) ", BaseAddress ? *BaseAddress : NULL);
	
	CStackMonitor *pStackMon = CStackMonitor::GetInstance();
	if(pStackMon)
	{
		//wsprintf(wszTmp, L"NtProtectVirtualMemory(tid = %u) BEGIN\n", GetCurrentThreadId());
		//OutputDebugString(wszTmp);

		if(bEnableAntiROPStack && !pStackMon->IsValidStackPointer(GetCurrentThreadId(), (LPVOID)dwEsp))
		{
			pStackMon->RefreshThreadStack(GetCurrentThreadId(), GetCurrentThread());

			if(!pStackMon->IsValidStackPointer(GetCurrentThreadId(), (LPVOID)dwEsp))
			{
				TerminateProcess(GetCurrentProcess(), BPCORE_TERMINATE_ROP_DETECTED);
			}
		}

		//wsprintf(wszTmp, L"NtProtectVirtualMemory(tid = %u) END\n", GetCurrentThreadId());
		//OutputDebugString(wszTmp);
	}

	BOOL bRequestExecute = Utils::GetPageProtectionMinusExecutionFlag(NewAccessProtection, FALSE) != NewAccessProtection;

	if(bDisableRWXVAMemory && bRequestExecute)
	{
		BOOL bAllowRWX = FALSE, bAllowExecute = FALSE;
		std::map<std::wstring, HMODULE>::iterator it;

		for(DWORD i=2; i<14; i++)
		{
			DWORD dwNthRetnAddr = Utils::GetReturnAddress(i);
			if(!dwNthRetnAddr)
				break;

			if(((dwNthRetnAddr - (DWORD)pvLdrLoadDll) < 0x200) || (((DWORD)pvLdrLoadDll - dwNthRetnAddr) < 0x200))
			{
				bAllowRWX = TRUE;
				break;
			}

			if(pLoadedMods->IsAddressInHModule(hmDiDCore, (LPVOID)dwNthRetnAddr) ||
				pLoadedMods->IsAddressInHModule(hmAuxCore, (LPVOID)dwNthRetnAddr) ||
				pLoadedMods->IsAddressInHModule(hmBpCore, (LPVOID)dwNthRetnAddr))
			{
				if((PBYTE)My_VirtualProtect - (PBYTE)dwNthRetnAddr < 0xA0 || (PBYTE)My_VirtualProtectEx - (PBYTE)dwNthRetnAddr < 0xA0)
				{
					// if we detour VirtualProtect(Ex) then this is always true => no anti-RWX for any module!
					// do nothing ...
				}
				else
				{
					bAllowRWX = TRUE;
					break;
				}
			}
			else
			{
				EnterCriticalSection(&csRWXMods);
				
				for(it = mapAllowRWXMods.begin(); it != mapAllowRWXMods.end(); it++)
				{
					if(pLoadedMods->IsAddressInHModule(it->second, (LPVOID)dwNthRetnAddr))
					{
						bAllowRWX = TRUE;
						break;
					}
				}

				LeaveCriticalSection(&csRWXMods);

				if(bAllowRWX)
					break;
			}
		}

		if(BaseAddress != NULL && pLoadedMods->IsAddressInLoadedModule(*BaseAddress))
		{
			// memory in a loaded DLL is being marked executable. This happens when a function is detoured for example.
			// This could be risky, as a ROP attack could:
			//     ret to VirtualProtect(dll_mem, WRITE) => memcpy(dll_mem, shellcode) => VirtualProtect(dll_mem, EXECUTE)
			//     => ret to dll_mem
			// however this would almost certainly (except in fairly favourable circumstances) require a stack-switch
			// which will be caught above (IsValidStackPointer).

			bAllowExecute = TRUE;
		}

		if(!bAllowRWX)
		{
			// if verifiable module load, allow RWX as the loader requires this when performing e.g. reloc fixups
			NewAccessProtection = Utils::GetPageProtectionMinusExecutionFlag(NewAccessProtection, bAllowExecute);
		}
	}

	return Real_NtProtectVirtualMemory(ProcessHandle, BaseAddress, NumberOfBytesToProtect, NewAccessProtection, OldAccessProtection);
}

LPVOID WINAPI DefenseInDepth::My_VirtualAlloc(
	__in_opt	LPVOID lpAddress,
	__in		SIZE_T dwSize,
	__in		DWORD flAllocationType,
	__in		DWORD flProtect
)
{
	// VirtualAlloc with a given base and size = 0x1 reportedly behaves like VirtualProtect - only changing protection on
	// already allocated page! Mona uses this technique
	DWORD dwRetnAddress = 0;

	GETRETNADDR(dwRetnAddress);

	if((bMonitorAPICallOrigins || bCheckROPReturn) && !IsWithinTrustedMemory((LPVOID)dwRetnAddress))
	{
		// a VirtualProtect ROP chain will often return from the call to the address of the now executable shellcode

		TerminateProcess(GetCurrentProcess(), bCheckROPReturn ?
			BPCORE_TERMINATE_ROP_DETECTED : BPCORE_TERMINATE_UNTRUSTED_CALLER);
	}

	if(bCheckROPReturn && IsJmpOrCallEsp((LPVOID)dwRetnAddress))
	{
		TerminateProcess(GetCurrentProcess(), BPCORE_TERMINATE_ROP_DETECTED);
	}

	return Real_VirtualAlloc(lpAddress, dwSize, flAllocationType, flProtect);
}

LPVOID WINAPI DefenseInDepth::My_VirtualAllocEx(
	__in		HANDLE hProcess,
	__in_opt	LPVOID lpAddress,
	__in		SIZE_T dwSize,
	__in		DWORD flAllocationType,
	__in		DWORD flProtect
)
{
	// VirtualAlloc with a given base and size = 0x1 reportedly behaves like VirtualProtect - only changing protection on
	// already allocated page! Mona uses this technique
	DWORD dwRetnAddress = 0;

	GETRETNADDR(dwRetnAddress);

	if((bMonitorAPICallOrigins || bCheckROPReturn) && !IsWithinTrustedMemory((LPVOID)dwRetnAddress))
	{
		// a VirtualProtect ROP chain will often return from the call to the address of the now executable shellcode

		TerminateProcess(GetCurrentProcess(), bCheckROPReturn ?
			BPCORE_TERMINATE_ROP_DETECTED : BPCORE_TERMINATE_UNTRUSTED_CALLER);
	}

	if(bCheckROPReturn && IsJmpOrCallEsp((LPVOID)dwRetnAddress))
	{
		TerminateProcess(GetCurrentProcess(), BPCORE_TERMINATE_ROP_DETECTED);
	}

	return Real_VirtualAllocEx(hProcess, lpAddress, dwSize, flAllocationType, flProtect);
}

NTSTATUS NTAPI DefenseInDepth::My_NtAllocateVirtualMemory(
	__in HANDLE		ProcessHandle,
	__inout PVOID	*BaseAddress,
	__in ULONG		ZeroBits,
	__inout PULONG	RegionSize,
	__in ULONG		AllocationType,
	__in ULONG		Protect
)
{
	ILoadedModules *pLoadedMods = GetLoadedModules();
	DWORD dwRetnAddress = 0, dwEsp = 0;

	GETRETNADDR(dwRetnAddress);
	GETESP(dwEsp);
	
	// unnecessary check: code already running in w/x memory won't need to use ROP techniques!
	
	/*
	if(!pLoadedMods->IsAddressInLoadedModule((LPVOID)dwRetnAddress))
	{
		CLogger::Break();
		TerminateProcess(GetCurrentProcess(), BPCORE_TERMINATE_UNTRUSTED_CALLER);
	}
	*/

	//WCHAR wszTmp[512];
	//wsprintf(wszTmp, L"NtAllocateVirtualMemory(Base = 0x%p) ", BaseAddress ? *BaseAddress : NULL);
	
	CStackMonitor *pStackMon = CStackMonitor::GetInstance();
	if(pStackMon)
	{
		if(bEnableAntiROPStack && !pStackMon->IsValidStackPointer(GetCurrentThreadId(), (LPVOID)dwEsp))
		{
			// bugfix: for some reason I could not always open my own thread handle for
			// GET_CTX|SUSP_RESM|QUERY so pass handle to self
			pStackMon->RefreshThreadStack(GetCurrentThreadId(), GetCurrentThread());

			if(!pStackMon->IsValidStackPointer(GetCurrentThreadId(), (LPVOID)dwEsp))
			{
				TerminateProcess(GetCurrentProcess(), BPCORE_TERMINATE_ROP_DETECTED);
			}
		}
	}

	BOOL bRequestExecute = Utils::GetPageProtectionMinusExecutionFlag(Protect, FALSE) != Protect;

	if(bDisableRWXVAMemory && bRequestExecute)
	{
		// this may undermine security somewhat. Need additional checks.

		BOOL bAllowRWX = FALSE, bAllowExecute = FALSE;
		std::map<std::wstring, HMODULE>::iterator it;

		for(DWORD i=2; i<14; i++)
		{
			DWORD dwNthRetnAddr = Utils::GetReturnAddress(i);
			if(!dwNthRetnAddr)
				break;

			// call should contain a return address in proximity of the loader function
			if(((dwNthRetnAddr - (DWORD)pvLdrLoadDll) < 0x200) || (((DWORD)pvLdrLoadDll - dwNthRetnAddr) < 0x200))
			{
				bAllowRWX = TRUE;
				break;
			}

			EnterCriticalSection(&csRWXMods);

			for(it = mapAllowRWXMods.begin(); it != mapAllowRWXMods.end(); it++)
			{
				if(pLoadedMods->IsAddressInHModule(it->second, (LPVOID)dwNthRetnAddr))
				{
					bAllowRWX = TRUE;
					break;
				}
			}

			LeaveCriticalSection(&csRWXMods);
		}

		if(BaseAddress != NULL && pLoadedMods->IsAddressInLoadedModule(*BaseAddress))
		{
			// memory in a loaded DLL is being marked executable. This happens when a function is detoured for example.
			// This could be risky, as a ROP attack could:
			//     ret to VirtualProtect(dll_mem, WRITE) => memcpy(dll_mem, shellcode) => VirtualProtect(dll_mem, EXECUTE)
			//     => ret to dll_mem
			// however this would almost certainly (except in fairly favourable circumstances) require a stack-switch
			// which will be caught above (IsValidStackPointer).

			bAllowExecute = TRUE;
		}

		if(!bAllowRWX)
		{
			// if verifiable module load, allow RWX as the loader requires this when performing e.g. reloc fixups
			Protect = Utils::GetPageProtectionMinusExecutionFlag(Protect, bAllowExecute);
		}
	}

	NTSTATUS ntStatus;
	BOOL bSuccess = FALSE;
	
	if(bRandomizeAllocBase)
	{
		if(BaseAddress && *BaseAddress == NULL &&
			// allocations from the heap manager, RtlpExtendHeap specifically, lead to trouble when expanding a region
			// from a base we previously handed out, following which there is not sufficient memory for contiguous heap
			// expansion. Just don't take this behaviour for code originating in ntdll.dll
			!((dwRetnAddress - (DWORD)hmNtdll) < dwNtdllImageSize)
			)
		{
			// make eight attempts to base the alloc randomly before just permitting the system to choose the base.
			// Eight being fairly low shouldn't lead to too big a performance hit, there may be an optimal number
			// to be found using stats
			
			for(DWORD i=0; i < 8; i++)
			{
				// random base and make granular with system page size and honour zero bits base requirement
				PVOID pvBaseTemp = (PVOID)(Utils::GetRandomInteger(stSystemInfo.dwPageSize, 0x7fffffff));
				pvBaseTemp = (PVOID)(((DWORD)pvBaseTemp - ((DWORD)pvBaseTemp % stSystemInfo.dwPageSize)) & (DWORD)(0xffffffff << ZeroBits));

				ntStatus = Real_NtAllocateVirtualMemory(ProcessHandle, &pvBaseTemp, ZeroBits, RegionSize, AllocationType | MEM_COMMIT | MEM_RESERVE, Protect);

				if(NT_SUCCESS(ntStatus))
				{
					*BaseAddress = pvBaseTemp;
					bSuccess = TRUE;
					break;
				}
			}
		}
	}
	
	if(!bSuccess)
	{
		CHeapMonitor *pHeapMon = CHeapMonitor::GetInstance();
		
		if(pHeapMon->m_bRemoveExecuteHeaps)
		{
			if(pHeapMon->IsAllocThread())
			{
				Protect = Utils::GetPageProtectionMinusExecutionFlag(Protect);
			}
		}

		ntStatus = Real_NtAllocateVirtualMemory(ProcessHandle, BaseAddress, ZeroBits, RegionSize, AllocationType, Protect);
	}

	return ntStatus;
}

BOOL WINAPI DefenseInDepth::My_SetProcessDEPPolicy(
	__in DWORD dwFlags
)
{
	DWORD dwRetnAddress = 0;

	GETRETNADDR(dwRetnAddress);

	if((bMonitorAPICallOrigins || bCheckROPReturn) && !IsWithinTrustedMemory((LPVOID)dwRetnAddress))
	{
		TerminateProcess(GetCurrentProcess(), bCheckROPReturn ?
			BPCORE_TERMINATE_ROP_DETECTED : BPCORE_TERMINATE_UNTRUSTED_CALLER);
	}

	if(bCheckROPReturn && IsJmpOrCallEsp((LPVOID)dwRetnAddress))
	{
		TerminateProcess(GetCurrentProcess(), BPCORE_TERMINATE_ROP_DETECTED);
	}

	if(bProcessDepEnabled && dwFlags == 0) // zero means disable (probable ROP bypass attempt)
	{
		SetLastError(ERROR_NOT_SUPPORTED);
		return FALSE;
	}

	return Real_SetProcessDEPPolicy(dwFlags);
}

NTSTATUS NTAPI DefenseInDepth::My_NtSetInformationProcess(
	__in HANDLE ProcessHandle,
	__in PROCESS_INFORMATION_CLASS ProcessInformationClass,
	__in PVOID ProcessInformation,
	__in ULONG ProcessInformationLength
)
{
	DWORD dwRetnAddress = 0;

	GETRETNADDR(dwRetnAddress);

	if((bMonitorAPICallOrigins || bCheckROPReturn) && !IsWithinTrustedMemory((LPVOID)dwRetnAddress))
	{
		TerminateProcess(GetCurrentProcess(), bCheckROPReturn ?
			BPCORE_TERMINATE_ROP_DETECTED : BPCORE_TERMINATE_UNTRUSTED_CALLER);
	}

	if(bCheckROPReturn && IsJmpOrCallEsp((LPVOID)dwRetnAddress))
	{
		TerminateProcess(GetCurrentProcess(), BPCORE_TERMINATE_ROP_DETECTED);
	}

#define ProcessExecuteFlags				0x22
#define MEM_EXECUTE_OPTION_ENABLE		2
#define STATUS_ACCESS_DENIED			0xc0000022

	if(bProcessDepEnabled && ProcessInformationClass == ProcessExecuteFlags)
	{
		DWORD *pdwExecuteOption = (DWORD *)ProcessInformation;

		if(pdwExecuteOption && (*pdwExecuteOption & MEM_EXECUTE_OPTION_ENABLE) != 0)
		{
			// an attempt to enable execution of NX memory
			return STATUS_ACCESS_DENIED;
		}
	}

	return Real_NtSetInformationProcess(
			ProcessHandle,
			ProcessInformationClass,
			ProcessInformation,
			ProcessInformationLength
		);
}

HRESULT WINAPI DefenseInDepth::My_CoCreateInstance(
	__in   REFCLSID rclsid,
	__in   LPUNKNOWN pUnkOuter,
	__in   DWORD dwClsContext,
	__in   REFIID riid,
	__out  LPVOID *ppv
)
{
	ILoadedModules *pLoadedMods = GetLoadedModules();
	DWORD dwRetnAddress = 0;
	HRESULT hr = REGDB_E_CLASSNOTREG;
	BOOL bAllow = FALSE;
	
	GETRETNADDR(dwRetnAddress);

	if(bMonitorAPICallOrigins && !IsWithinTrustedMemory((LPVOID)dwRetnAddress))
	{
		pLoadedMods->EnumLoadedModules();

		if(!pLoadedMods->IsAddressInLoadedModule((LPVOID)dwRetnAddress))
		{
#ifdef DEBUG_BUILD
			CLogger::Break();
#endif
			TerminateProcess(GetCurrentProcess(), BPCORE_TERMINATE_UNTRUSTED_CALLER);
		}
	}
	
	if(bComMonEnabled)
	{
		// checking whether SFS/SFI shouldn't increase attack surface dramatically (or directly)
		// but makes user experience much better

		IUnknown *pUnknown = NULL;
		HRESULT hrTemp = Real_CoCreateInstance(rclsid, pUnkOuter, dwClsContext, IID_IUnknown, (void **)&pUnknown);

		if(hrTemp == S_OK && pUnknown != NULL)
		{
			if(CheckClsidSFS(rclsid, riid, pUnknown) == FALSE && CheckClsidSFI(rclsid, riid, pUnknown) == FALSE)
			{
				bAllow = TRUE;
			}

			pUnknown->Release();
			pUnknown = NULL;
		}

		if(!bAllow)
		{
			// is either safe for scripting or safe for initialisation
			// check against whitelist/blacklist

			bAllow = CheckClsidSafeForInit(rclsid);
		}
	}
	else
	{
		bAllow = TRUE;
	}

	if(bAllow)
		hr = Real_CoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv);

	return hr;
}

HRESULT WINAPI DefenseInDepth::My_CoGetClassObject(
  __in      REFCLSID rclsid,
  __in      DWORD dwClsContext,
  __in_opt  COSERVERINFO *pServerInfo,
  __in      REFIID riid,
  __out     LPVOID *ppv
)
{
	ILoadedModules *pLoadedMods = GetLoadedModules();
	DWORD dwRetnAddress = 0;
	HRESULT hr = REGDB_E_CLASSNOTREG;
	BOOL bAllow = FALSE;
	
	GETRETNADDR(dwRetnAddress);

	if(bMonitorAPICallOrigins && !IsWithinTrustedMemory((LPVOID)dwRetnAddress))
	{
		pLoadedMods->EnumLoadedModules();

		if(!pLoadedMods->IsAddressInLoadedModule((LPVOID)dwRetnAddress))
		{
#ifdef DEBUG_BUILD
			CLogger::Break();
#endif
			TerminateProcess(GetCurrentProcess(), BPCORE_TERMINATE_UNTRUSTED_CALLER);
		}
	}

	if(bComMonEnabled)
	{
		// checking whether SFS/SFI shouldn't increase attack surface dramatically (or directly)
		// but makes user experience much better

		IUnknown *pUnknown = NULL;
		HRESULT hrTemp = Real_CoGetClassObject(rclsid, dwClsContext, pServerInfo, IID_IUnknown, (void **)&pUnknown);

		if(hrTemp == S_OK && pUnknown != NULL)
		{
			if(CheckClsidSFS(rclsid, riid, pUnknown) == FALSE && CheckClsidSFI(rclsid, riid, pUnknown) == FALSE)
			{
				bAllow = TRUE;
			}

			pUnknown->Release();
			pUnknown = NULL;
		}

		if(!bAllow)
		{
			// is either safe for scripting or safe for initialisation
			// check against whitelist/blacklist

			bAllow = CheckClsidSafeForInit(rclsid);
		}
	}
	else
	{
		bAllow = TRUE;
	}

	if(bAllow)
		hr = Real_CoGetClassObject(rclsid, dwClsContext, pServerInfo, riid, ppv);

	return hr;
}

static BOOL DefenseInDepth::CheckClsidSFS(REFCLSID rclsid, REFIID riid, IUnknown *pUnknown)
{
	// check SFS key, and iobjectsafety
	BOOL bUnsafe = FALSE;
	wchar_t wszGuid[128] = {0}, wszRegPath[256] = {0};
	HKEY hkResult = {0};
	
	int cch = StringFromGUID2(rclsid, (LPOLESTR)wszGuid, 127);
	if(cch == 0)
		goto Cleanup;

	//if(wcsicmp(wszGuid, L"{CFCDAA03-8BE4-11cf-B84B-0020AFBBCCFA}") == 0)
	//	DebugBreak();

	// {7DD95801-9882-11CF-9FA9-00AA006C42C4} - CATID_SafeForScripting
	wsprintf(wszRegPath, L"CLSID\\%s\\Implemented Categories\\{7DD95801-9882-11CF-9FA9-00AA006C42C4}", wszGuid);
	
	IObjectSafety *pObjectSafety = NULL;
	IClassFactory *pClassFactory = NULL;
	
	// do not forget we may be running on wow64: does KEY_WOW64_32KEY interfere with operation on Win2K/XP 32bit?
	if(RegOpenKeyEx(HKEY_CLASSES_ROOT, wszRegPath, 0, KEY_READ | KEY_WOW64_32KEY, &hkResult) == ERROR_SUCCESS)
	{
		// it is unsafe due to sfs regkey
		RegCloseKey(hkResult);
		bUnsafe = TRUE;
	}
	else
	{
		// check if unsafe due to iobjectsafety flags
		if(!pUnknown)
			goto Cleanup; // no iunknown ptr => assume safe

		pUnknown->QueryInterface(IID_IObjectSafety, (void **)&pObjectSafety);
		if(!pObjectSafety)
		{
			// perhaps we need to get the IObjectSafety interface from an IClassFactory interface

			pUnknown->QueryInterface(IID_IClassFactory, (void **)&pClassFactory);
			if(!pClassFactory)
				goto Cleanup;
			
			pClassFactory->CreateInstance(NULL, IID_IObjectSafety, (void **)&pObjectSafety);
			if(!pObjectSafety)
				goto Cleanup;
		}

		DWORD dwSupportedOptions = 0, dwEnabledOptions = 0;

		if(pObjectSafety->GetInterfaceSafetyOptions(riid, &dwSupportedOptions, &dwEnabledOptions) != S_OK)
			goto Cleanup; // iobjectsafety not implemented properly => assume safe

		if((dwSupportedOptions & INTERFACESAFE_FOR_UNTRUSTED_CALLER) ||
			(dwSupportedOptions & INTERFACESAFE_FOR_UNTRUSTED_DATA))
		{
			// are these all the unsafe settings?
			bUnsafe = TRUE;
		}
	}
	
Cleanup:
	if(pObjectSafety)
		pObjectSafety->Release();

	if(pClassFactory)
		pClassFactory->Release();

	pObjectSafety = NULL;
	pClassFactory = NULL;

	return bUnsafe;
}

static BOOL DefenseInDepth::CheckClsidSFI(REFCLSID rclsid, REFIID riid, IUnknown *pUnknown)
{
	// check SFI key, and iobjectsafety
	BOOL bUnsafe = FALSE;
	wchar_t wszGuid[128] = {0}, wszRegPath[256] = {0};
	HKEY hkResult = {0};
	
	int cch = StringFromGUID2(rclsid, (LPOLESTR)wszGuid, 127);
	if(cch == 0)
		goto Cleanup;

	//if(wcsicmp(wszGuid, L"{CFCDAA03-8BE4-11cf-B84B-0020AFBBCCFA}") == 0)
	//	DebugBreak();

	// {7DD95802-9882-11CF-9FA9-00AA006C42C4} - CATID_SafeForInitialization
	wsprintf(wszRegPath, L"CLSID\\%s\\Implemented Categories\\{7DD95802-9882-11CF-9FA9-00AA006C42C4}", wszGuid);

	IObjectSafety *pObjectSafety = NULL;
	IClassFactory *pClassFactory = NULL;
	
	// do not forget we may be running on wow64: does KEY_WOW64_32KEY interfere with operation on Win2K/XP 32bit?
	if(RegOpenKeyEx(HKEY_CLASSES_ROOT, wszRegPath, 0, KEY_READ | KEY_WOW64_32KEY, &hkResult) == ERROR_SUCCESS)
	{
		// it is unsafe due to sfs regkey
		RegCloseKey(hkResult);
		bUnsafe = TRUE;
	}
	else
	{
		// check if unsafe due to iobjectsafety flags
		if(!pUnknown)
			goto Cleanup; // no iunknown ptr => assume safe

		pUnknown->QueryInterface(IID_IObjectSafety, (void **)&pObjectSafety);
		if(!pObjectSafety)
		{
			// perhaps we need to get the IObjectSafety interface from an IClassFactory interface

			pUnknown->QueryInterface(IID_IClassFactory, (void **)&pClassFactory);
			if(!pClassFactory)
				goto Cleanup;
			
			pClassFactory->CreateInstance(NULL, IID_IObjectSafety, (void **)&pObjectSafety);
			if(!pObjectSafety)
				goto Cleanup;
		}

		DWORD dwSupportedOptions = 0, dwEnabledOptions = 0;

		if(pObjectSafety->GetInterfaceSafetyOptions(riid, &dwSupportedOptions, &dwEnabledOptions) != S_OK)
			goto Cleanup; // iobjectsafety not implemented properly => assume safe

		if((dwSupportedOptions & INTERFACESAFE_FOR_UNTRUSTED_CALLER) ||
			(dwSupportedOptions & INTERFACESAFE_FOR_UNTRUSTED_DATA))
		{
			// are these all the unsafe settings?
			bUnsafe = TRUE;
		}
	}
	
Cleanup:
	if(pObjectSafety)
		pObjectSafety->Release();

	if(pClassFactory)
		pClassFactory->Release();

	pObjectSafety = NULL;
	pClassFactory = NULL;

	return bUnsafe;
}

BOOL DefenseInDepth::CheckClsidSafeForInit(REFCLSID rclsid)
{
	LPOLESTR lposClsid = NULL;
	BOOL bAllow = FALSE, bRequiresJit = FALSE;
	std::map<CLSID *, std::wstring>::iterator it;
	std::wstring wstrName;
	std::wstring wstrNavigatedAddress = L"not recognised";
	BOOL bValidUrl = FALSE;

	if(!TryEnterCriticalSection(&csConfigReload))
		return TRUE;
	
	IRealtimeLog *pRealtimeLog = GetRealtimeLog();

	if(bClsidWhitelistEnabled)
	{
		for(it = mapClsidWhitelist.begin(); it != mapClsidWhitelist.end(); it++)
		{
			if(!memcmp(it->first, &rclsid, sizeof(CLSID)))
			{
				bAllow = TRUE;
				wstrName = it->second;
				break;
			}
		}

		// write an event log
		pRealtimeLog->LogMessage(0, L"Allowed (whitelist) COM class %s (clsid:%S)", wstrName.c_str(),
			Utils::ClsidToString(const_cast<CLSID&>(rclsid)).c_str());
	}
	else if(bClsidBlacklistEnabled)
	{
		bAllow = TRUE;

		for(it = mapClsidBlacklist.begin(); it != mapClsidBlacklist.end(); it++)
		{
			if(!memcmp(it->first, &rclsid, sizeof(CLSID)))
			{
				bAllow = FALSE;
				wstrName = it->second;
				break;
			}
		}
		
		// write an event log
		if(!bAllow)
			pRealtimeLog->LogMessage(0, L"Blocked (blacklist) COM class %s (clsid:%S)", wstrName.c_str(),
				Utils::ClsidToString(const_cast<CLSID&>(rclsid)).c_str());
	}
	else
	{
		bAllow = TRUE;
	}

	if(!bAllow)
	{
		// remove later
#ifdef DEBUG_BUILD
		if(0)
		{
			LPOLESTR lpos = NULL;
			StringFromCLSID(rclsid, &lpos);
			if(lpos)
			{
				std::wstring wstrTmp = L"Clsid:";
				wstrTmp += lpos;
				wstrTmp += L"\r\n";
				OutputDebugString(wstrTmp.c_str());
				CoTaskMemFree(lpos);
			}
		}
#endif
		goto Cleanup;
	}
	// perform checking to see whether jitting is required by the allowed clsid

	for(it = mapClsidRequiresJit.begin(); it != mapClsidRequiresJit.end(); it++)
	{
		if(!memcmp(it->first, &rclsid, sizeof(CLSID)))
		{
			bRequiresJit = TRUE;
			wstrName = it->second;
			break;
		}
	}

	if(GetModuleHandle(L"bpcore.dll"))
	{
		IBrowserInstanceDetails *pBrowser = GetBrowserInstanceDetails();
		
		if(pBrowser && pBrowser->GetNavigatedAddress())
		{
			wstrNavigatedAddress = pBrowser->GetNavigatedAddress();
			int nPosUrl = wstrNavigatedAddress.find(L"://");
			
			if(nPosUrl != std::wstring::npos)
			{
				int nPosEnd = wstrNavigatedAddress.find('/', nPosUrl + 3);

				if(nPosEnd != std::wstring::npos)
					wstrNavigatedAddress = wstrNavigatedAddress.substr(nPosUrl + 3, nPosEnd - nPosUrl - 3);
			}

			bValidUrl = TRUE;
		}
	}

	if(bRequiresJit)
	{
		// ask user whether they would like to reduce security level to allow flash etc.
		// re-enable when browser UI target changes?
		
		if(bPersistJitDecision && iPersistUserChoiceMade != -1)
		{
			bAllow = iPersistUserChoiceMade == 1;
			goto Cleanup;
		}

		std::wstring wstrCaption = L"ActiveX component ";
	
		if(!wstrName.empty())
			wstrCaption += wstrName;
		else
			wstrCaption += L"(unknown name)";

		wstrCaption += (bValidUrl ? (L" hosted by site " + wstrNavigatedAddress) : L"");

		wstrCaption += L" requires protections to be temporarily lowered.\r\n\r\n"
			L"Please only select 'Yes' if you know that the sites you are currently visiting are safe.\r\n\r\n"
			L"";

		if(bPersistJitDecision)
		{
			wstrCaption += L"Due to your configuration settings this decision will be persisted from this point on "
				L"for every site visited by this instance of the browser. If this is not desired please alter the "
				L"COM/ActiveX Monitor settings in the Crystal UI.\r\n\r\n";
		}

		wstrCaption += L"Continue?";

		std::wstring wstrTitle = L"Reduce Protection Level" + (bValidUrl ? (L" (site: " + wstrNavigatedAddress + L")") : L"");
		
		BOOL bUserChoiceMade = FALSE, bUserChoice = FALSE;
		std::wstring wstrDynCName;

		if(bValidUrl)
		{
			wstrDynCName = L"JitSiteSetting_ChoiceMade_" + wstrNavigatedAddress;
			bUserChoiceMade = CDynamicConfig::GetInstance()->GetProperty<BOOL>(CDynamicConfig::EDynamicRepository::DfnsInDepth, wstrDynCName);

			wstrDynCName = L"JitSiteSetting_Choice_" + wstrNavigatedAddress;
			bUserChoice =  CDynamicConfig::GetInstance()->GetProperty<BOOL>(CDynamicConfig::EDynamicRepository::DfnsInDepth, wstrDynCName);
		}

		if(bUserChoiceMade)
		{
			bAllow = bUserChoice;
		}
		else if(GetUiFeedback()->UserBoolFeedback(
				UiStyle_IconQuestion,
				const_cast<WCHAR *>(wstrTitle.c_str()),
				const_cast<WCHAR *>(wstrCaption.c_str())
			))
		{
			bAllow = TRUE;

			// write event log
			pRealtimeLog->LogMessage(0, L"Allowed JIT compilation for COM class %s (clsid:%S)", wstrName.c_str(), 
				Utils::ClsidToString(const_cast<CLSID&>(rclsid)).c_str());
		}
		else
		{
			// otherwise jitting will cause IE to AV (NX on page)
			bAllow = FALSE;
			
			pRealtimeLog->LogMessage(0, L"Blocked JIT compilation for COM class %s (clsid:%S)", wstrName.c_str(), 
				Utils::ClsidToString(const_cast<CLSID&>(rclsid)).c_str());
		}

		if(bPersistJitDecision)
		{
			if(bAllow == TRUE)
				iPersistUserChoiceMade = 1;
			else
				iPersistUserChoiceMade = 0;
		}

		// if this call is made here, if the user is faced with jit two messages and allows one denies the other
		// the first one may lead to a crash when jit com object is allowed to load but mem protection is set to
		// N/X
		
		Utils::SetChangeProtectThread(!bAllow);

		if(bValidUrl)
		{
			wstrDynCName = L"JitSiteSetting_ChoiceMade_" + wstrNavigatedAddress;
			CDynamicConfig::GetInstance()->SetProperty<BOOL>(CDynamicConfig::EDynamicRepository::DfnsInDepth, wstrDynCName, TRUE);

			wstrDynCName = L"JitSiteSetting_Choice_" + wstrNavigatedAddress;
			CDynamicConfig::GetInstance()->SetProperty<BOOL>(CDynamicConfig::EDynamicRepository::DfnsInDepth, wstrDynCName, bAllow);
		}
	}

Cleanup:
	LeaveCriticalSection(&csConfigReload);
	
	return bAllow;
}

HANDLE WINAPI DefenseInDepth::My_CreateThread(
	__in_opt   LPSECURITY_ATTRIBUTES lpThreadAttributes,
	__in       SIZE_T dwStackSize,
	__in       LPTHREAD_START_ROUTINE lpStartAddress,
	__in_opt   LPVOID lpParameter,
	__in       DWORD dwCreationFlags,
	__out_opt  LPDWORD lpThreadId
)
{
	ILoadedModules *pLoadedMods = GetLoadedModules();
	DWORD dwRetnAddress = 0, dwThreadId = 0;
	
	GETRETNADDR(dwRetnAddress);

	if(bMonitorAPICallOrigins && (!IsWithinTrustedMemory((LPVOID)dwRetnAddress) || !IsWithinTrustedMemory((LPVOID)lpStartAddress)))
	{
		pLoadedMods->EnumLoadedModules();

		if(!pLoadedMods->IsAddressInLoadedModule((LPVOID)dwRetnAddress) || !pLoadedMods->IsAddressInLoadedModule((LPVOID)lpStartAddress))
		{
#ifdef DEBUG_BUILD
			CLogger::Break();
#endif
			TerminateProcess(GetCurrentProcess(), BPCORE_TERMINATE_UNTRUSTED_CALLER);
		}
	}

	// threads with small stack frames cause problems for us on more memory-conservative OS as our
	// wrappers require additional stack space
	if(dwStackSize < 0x80000)
		dwStackSize = 0x80000;

	HANDLE hThread = Real_CreateThread(
			lpThreadAttributes,
			dwStackSize,
			lpStartAddress,
			lpParameter,
			dwCreationFlags | CREATE_SUSPENDED,
			&dwThreadId
		);

	if(hThread)
	{
		if(lpThreadId)
			*lpThreadId = dwThreadId;

		CStackMonitor *pStackMon = CStackMonitor::GetInstance();
		if(pStackMon)
		{
			// fix the other calls to RemoveExecuteFromThreadStack to do similar
			if(bDisableRWXStackMemory)
			{
				pStackMon->RemoveExecuteFromThreadStack(hThread);
			}
			
			if(bEnableAntiROPStack)
			{
				LPVOID lpvTIBStackTop = NULL, lpvTIBStackBase = NULL;
				
				if(Utils::ThreadGetStackRegion(hThread, &lpvTIBStackBase, &lpvTIBStackTop))
				{
					size_t cbTIBStackSize = (PBYTE)lpvTIBStackBase - (PBYTE)lpvTIBStackTop;
					pStackMon->RegisterThreadStack(Utils::ThreadIdFromHandle(hThread), lpvTIBStackTop, cbTIBStackSize);
				}
			}
		}
		
		if(!(dwCreationFlags & CREATE_SUSPENDED))
		{
			// caller did not request CREATE_SUSPENDED so resume thread
			
			if(ResumeThread(hThread) == (DWORD)-1)
			{
				CloseHandle(hThread);
				hThread = NULL;
			}
		}
	}

	return hThread;
}

HANDLE WINAPI DefenseInDepth::My_CreateRemoteThread(
	__in   HANDLE hProcess,
	__in   LPSECURITY_ATTRIBUTES lpThreadAttributes,
	__in   SIZE_T dwStackSize,
	__in   LPTHREAD_START_ROUTINE lpStartAddress,
	__in   LPVOID lpParameter,
	__in   DWORD dwCreationFlags,
	__out  LPDWORD lpThreadId
)
{
#ifdef DEBUG_BUILD
	CLogger::Break();
#endif

	return Real_CreateRemoteThread(
			hProcess,
			lpThreadAttributes,
			dwStackSize,
			lpStartAddress,
			lpParameter,
			dwCreationFlags,
			lpThreadId
		);
}

NTSTATUS NTAPI DefenseInDepth::My_NtCreateThread(
	__out PHANDLE			ThreadHandle,
	__in ACCESS_MASK		DesiredAccess,
	__in_opt POBJECT_ATTRIBUTES	ObjectAttributes,
	__in HANDLE				ProcessHandle,
	__out PCLIENT_ID		ClientId,
	__in PCONTEXT			ThreadContext,
	__in PINITIAL_TEB		InitialTeb,
	__in BOOLEAN			CreateSuspended
)
{
	ILoadedModules *pLoadedMods = GetLoadedModules();
	DWORD dwRetnAddress = 0;
	
	GETRETNADDR(dwRetnAddress);

	if(bMonitorAPICallOrigins && (
			!IsWithinTrustedMemory((LPVOID)dwRetnAddress) || !IsWithinTrustedMemory((LPVOID)ThreadContext->Eip) ||
			  (pvRtlUserThreadStart == (PVOID)ThreadContext->Eip && !IsWithinTrustedMemory((LPVOID)ThreadContext->Eax)) )
		)
	{
		pLoadedMods->EnumLoadedModules();

		if(!pLoadedMods->IsAddressInLoadedModule((LPVOID)dwRetnAddress) ||
			!pLoadedMods->IsAddressInLoadedModule((LPVOID)ThreadContext->Eip) ||
			(pvRtlUserThreadStart == (PVOID)ThreadContext->Eip &&
			!pLoadedMods->IsAddressInLoadedModule((LPVOID)ThreadContext->Eax)))
		{
#ifdef DEBUG_BUILD
			CLogger::Break();
#endif
			TerminateProcess(GetCurrentProcess(), BPCORE_TERMINATE_UNTRUSTED_CALLER);
		}
	}

	HANDLE hThread = NULL;

	NTSTATUS ntStatus = Real_NtCreateThread(
		&hThread,
		DesiredAccess,
		ObjectAttributes,
		ProcessHandle,
		ClientId,
		ThreadContext,
		InitialTeb,
		TRUE
	);

	if(NT_SUCCESS(ntStatus))
	{

		if(ThreadHandle != NULL)
			*ThreadHandle = hThread;
		
		//DWORD dwThreadId = GetThreadId(hThread);
		DWORD dwThreadId = Utils::ThreadIdFromHandle(hThread);
		if(dwThreadId)
		{
			CStackMonitor *pStackMon = CStackMonitor::GetInstance();
			if(pStackMon)
			{
				// fix the other calls to RemoveExecuteFromThreadStack to do similar
				if(bDisableRWXStackMemory)
				{
					pStackMon->RemoveExecuteFromThreadStack(hThread);
				}

				if(bEnableAntiROPStack)
				{
					LPVOID lpvTIBStackTop = NULL, lpvTIBStackBase = NULL;
				
					if(Utils::ThreadGetStackRegion(hThread, &lpvTIBStackBase, &lpvTIBStackTop))
					{
						size_t cbTIBStackSize = (PBYTE)lpvTIBStackBase - (PBYTE)lpvTIBStackTop;
						pStackMon->RegisterThreadStack(Utils::ThreadIdFromHandle(hThread), lpvTIBStackTop, cbTIBStackSize);
					}
				}
			}
		}

		if(!CreateSuspended)
		{
			// caller did not request CREATE_SUSPENDED so resume thread
			
			if(ResumeThread(hThread) == (DWORD)-1)
			{
				CloseHandle(hThread);
				
				if(ThreadHandle)
					*ThreadHandle = NULL;

				ntStatus = STATUS_INVALID_PARAMETER;
			}
		}
	}
	
	return ntStatus;
}

NTSTATUS NTAPI DefenseInDepth::My_NtCreateThreadEx(
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
)
{
	ILoadedModules *pLoadedMods = GetLoadedModules();
	DWORD dwRetnAddress = 0;
	
	GETRETNADDR(dwRetnAddress);

	if(bMonitorAPICallOrigins && (!IsWithinTrustedMemory((LPVOID)dwRetnAddress) || !IsWithinTrustedMemory((LPVOID)lpStartAddress)))
	{
		pLoadedMods->EnumLoadedModules();

		if(!pLoadedMods->IsAddressInLoadedModule((LPVOID)dwRetnAddress) ||
			!pLoadedMods->IsAddressInLoadedModule((LPVOID)lpStartAddress))
		{
#ifdef DEBUG_BUILD
			CLogger::Break();
#endif
			TerminateProcess(GetCurrentProcess(), BPCORE_TERMINATE_UNTRUSTED_CALLER);
		}
	}

	HANDLE hThreadHandle = NULL;

	NTSTATUS ntStatus = Real_NtCreateThreadEx(
		&hThreadHandle,
		DesiredAccess,
		ObjectAttributes,
		ProcessHandle,
		lpStartAddress,
		lpParameter,
		TRUE, 
		StackZeroBits,
		SizeOfStackCommit,
		SizeOfStackReserve,
		lpBytesBuffer
	);

	if(NT_SUCCESS(ntStatus))
	{
		if(hThread != NULL)
			*hThread = hThreadHandle;

		if(ProcessHandle == GetCurrentProcess())
		{
			//DWORD dwThreadId = GetThreadId(hThreadHandle);
			DWORD dwThreadId = Utils::ThreadIdFromHandle(hThreadHandle);
			if(dwThreadId)
			{
				CStackMonitor *pStackMon = CStackMonitor::GetInstance();
				if(pStackMon)
				{
					// fix the other calls to RemoveExecuteFromThreadStack to do similar
					if(bDisableRWXStackMemory)
					{
						pStackMon->RemoveExecuteFromThreadStack(hThreadHandle);
					}

					if(bEnableAntiROPStack)
					{
						LPVOID lpvTIBStackTop = NULL, lpvTIBStackBase = NULL;
				
						if(Utils::ThreadGetStackRegion(hThreadHandle, &lpvTIBStackBase, &lpvTIBStackTop))
						{
							size_t cbTIBStackSize = (PBYTE)lpvTIBStackBase - (PBYTE)lpvTIBStackTop;
							pStackMon->RegisterThreadStack(Utils::ThreadIdFromHandle(hThreadHandle), lpvTIBStackTop, cbTIBStackSize);
						}
					}
				}
			}
		}

		if(!CreateSuspended)
		{
			// caller did not request CREATE_SUSPENDED so resume thread
			
			if(ResumeThread(hThreadHandle) == (DWORD)-1)
			{
				CloseHandle(hThreadHandle);
				
				if(hThread)
					*hThread = NULL;

				ntStatus = STATUS_INVALID_PARAMETER;
			}
		}
	}
	
	return ntStatus;
}

VOID WINAPI DefenseInDepth::My_ExitProcess(
	__in  UINT uExitCode
)
{
	// either re-enable W|X or terminate directly. Former may lead to race condition code execution bug.
	// Utils::SetChangeProtectThread(FALSE);
	//TerminateProcess(GetCurrentProcess(), uExitCode);
	return Real_ExitProcess(uExitCode);
}

VOID WINAPI DefenseInDepth::My_ExitThread(
	__in  DWORD dwExitCode
)
{
	// not all threads will terminate by calling ExitThread but this helps keep down the number of
	// entries in the stack monitor valid stack map.

	CStackMonitor *pStackMon = CStackMonitor::GetInstance();
	pStackMon->UnregisterThreadStack(GetCurrentThreadId());

	return Real_ExitThread(dwExitCode);
}

NTSTATUS NTAPI DefenseInDepth::My_NtTerminateThread(
	__in HANDLE               ThreadHandle,
	__in NTSTATUS             ExitStatus
)
{
	CStackMonitor *pStackMon = CStackMonitor::GetInstance();
	pStackMon->UnregisterThreadStack(GetCurrentThreadId());

	return Real_NtTerminateThread(ThreadHandle, ExitStatus);
}

void DefenseInDepth::ConfigRefresh(BOOL bRefresh)
{
	IConfigReader *pConfig = GetConfigReader();

	LPWSTR rgwszConfigName[21] = {
		// activeXMonitor
		L"enabled",
		L"clsidBlacklistEnabled",
		L"clsidWhitelistEnabled",
		L"clsidBlacklist",
		L"clsidWhitelist",
		L"clsidRequiresJit",
		L"persistJitDecision",
		// apiMonitor
		L"enableAntiROPStack",
		L"disableRWXVAMemory",
		L"disableRWXStackMemory",
		L"monitorAPICallOrigins",
		L"injectIntoChildProc",
		L"allowRWXVAModules",
		L"randomizeAllocBase",
		L"exceptHandlerChecks",
		L"emulateSEHHandler",
		L"promptElevateLowInteg",
		L"elevateLowIntegApps",
		L"promptRelaxDotnet",
		L"checkROPReturn",
		L"autoAddHookDllRWX"
	};

	WCHAR *pwszTmp, *pwszToken, *pwszClsid;
	BOOL *pboolTmp = NULL;
	
	std::map<CLSID *, std::wstring> *pmapTmp = NULL;
	std::map<CLSID *, std::wstring>::iterator it;
	
	std::map<std::wstring, HMODULE> *pmapModuleTmp = NULL;
	
	EnterCriticalSection(&csConfigReload);

	for(int i=0; i<21; i++)
	{
		if(i < 7)
		{
			pwszTmp = pConfig->GetConfigElement(L"activeXMonitor", rgwszConfigName[i]);
			if(!pwszTmp)
				continue;
		}
		else if(i >= 7 && i < 21)
		{
			pwszTmp = pConfig->GetConfigElement(L"apiMonitor", rgwszConfigName[i]);
			if(!pwszTmp)
				continue;
		}

		pboolTmp = NULL;
		pmapTmp = NULL;
		pmapModuleTmp = NULL;

		switch(i)
		{
		case 0: // enabled
			pboolTmp = &bComMonEnabled;
			break;
		case 1: // clsidBlacklistEnabled
			pboolTmp = &bClsidBlacklistEnabled;
			break;
		case 2: // clsidWhitelistEnabled
			pboolTmp = &bClsidWhitelistEnabled;
			break;
		case 3: // clsidBlacklist
			pmapTmp = &mapClsidBlacklist;
			break;
		case 4: // clsidWhitelist
			pmapTmp = &mapClsidWhitelist;
			break;
		case 5: // clsidRequiresJit
			pmapTmp = &mapClsidRequiresJit;
			break;
		case 6: // persistJitDecision
			pboolTmp = &bPersistJitDecision;
			break;
		case 7: // enableAntiROPStack
			pboolTmp = &bEnableAntiROPStack;
			break;
		case 8: // disableRWXVAMemory
			pboolTmp = &bDisableRWXVAMemory;
			break;
		case 9: // disableRWXStackMemory
			pboolTmp = &bDisableRWXStackMemory;
			break;
		case 10: // monitorAPICallOrigins
			pboolTmp = &bMonitorAPICallOrigins;
			break;
		case 11: // injectIntoChildProc
			pboolTmp = &bInjectIntoChildProc;
			break;
		case 12:
			pmapModuleTmp = &mapAllowRWXMods;
			break;
		case 13:
			pboolTmp = &bRandomizeAllocBase;
			break;
		case 14:
			pboolTmp = &bCheckExceptionDisp;
			break;
		case 15:
			pboolTmp = &bEmuExceptHandler;
			break;
		case 16:
			pboolTmp = &bPromptLowIntegApp;
			break;
		case 17:
			pboolTmp = &bElevateLowIntegApps;
			break;
		case 18:
			pboolTmp = &bPromptUserRelaxDotnet;
			break;
		case 19:
			pboolTmp = &bCheckROPReturn;
			break;
		case 20:
			pboolTmp = &bAutoAddHookDllRWX;
			break;
		}

		if(pboolTmp)
		{
			if(!wcsicmp(pwszTmp, L"true"))
				*pboolTmp = TRUE;
			else
				*pboolTmp = FALSE;
		}
		else if(pmapTmp)
		{
			for(it = pmapTmp->begin(); it != pmapTmp->end(); it++)
			{
				// new CLSID[1]
				delete it->first;
				//pmapTmp->erase(it);
			}

			pmapTmp->clear();

			pwszToken = wcstok(pwszTmp, L"|");
			
			while(pwszToken)
			{
				std::wstring wstrName;

				// Adobe Flash:{CLSID} name is optional
				pwszClsid = wcschr(pwszToken, ':');

				if(pwszClsid)
				{
					*pwszClsid = '\0';
					wstrName = pwszToken;
					*pwszClsid = ':';
					pwszClsid++;
				}
				else
				{
					wstrName = L"";
					pwszClsid = pwszToken;
				}
				
				CLSID *pclsid = new GUID[1];

				if(CLSIDFromString((LPOLESTR)(pwszClsid), pclsid) == NOERROR)
				{
					(*pmapTmp)[pclsid] = wstrName;
				}
				else
				{
					delete pclsid;
				}

				pwszToken = wcstok(NULL, L"|");
			}
		}
		else if(pmapModuleTmp)
		{
			EnterCriticalSection(&csRWXMods);

			mapAllowRWXMods.clear();

			pwszToken = wcstok(pwszTmp, L"|");
			
			while(pwszToken)
			{
				std::wstring wstrModule;

				for(DWORD n = 0; pwszToken[n] != '\0'; n++)
				{
					wstrModule += tolower(pwszToken[n]);
				}

				if(!wstrModule.empty())
				{
					// valid name
					mapAllowRWXMods[wstrModule] = GetModuleHandle(wstrModule.c_str());
				}

				pwszToken = wcstok(NULL, L"|");
			}

			LeaveCriticalSection(&csRWXMods);
		}
	}

	// this is for the CreateProcessInternal integrity check
	pwszTmp = pConfig->GetConfigElement(L"protectedProcessList", L"processList");
	if(pwszTmp)
	{
		pwszProtProcList = wcsdup(pwszTmp);
	}

	pwszTmp = pConfig->GetConfigElement(L"memoryMonitor", L"processDepEnabled");
	if(pwszTmp)
	{
		if(!wcsicmp(pwszTmp, L"true"))
			bProcessDepEnabled = TRUE;
		else
			bProcessDepEnabled = FALSE;
	}

	if(bAutoAddHookDllRWX)
	{
		// look for libraries that use detours and add them to the exemptions list
		ILoadedModules *pLoadedMods = GetLoadedModules();
		std::map<HMODULE, ISingleLoadedModule *>::iterator itModules = pLoadedMods->GetLoadedModuleList().begin();

		for(; itModules != pLoadedMods->GetLoadedModuleList().end(); itModules++)
		{
			if(DllContainsString(itModules->first, "detoured.dll"))
			{
				WCHAR wszHandleValue[32] = {0};
				wsprintf(wszHandleValue, L"0x%p.dll", itModules->first);
			
				// the name need not be correct (only unique) as it's not validated by My_Nt*VirtualMemory
				mapAllowRWXMods[wszHandleValue] = (HMODULE)itModules->first;
			}
		}
	}

	LeaveCriticalSection(&csConfigReload);
}