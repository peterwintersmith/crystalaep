// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#include "stdafx.h"
#include "DiDCore.h"
#include "DfnsInDepth.h"
#include "StackMonitor.h"
#include "..\..\Shared\Utils\Utils.h"
#include "..\..\AuxCore\AuxCore\IRealtimeLog.h"
#include "..\..\AuxCore\AuxCore\IProcTrackerServices.h"
#include "..\..\ProcTracker\ProcTracker\ProcTracker_h.h"

CStackMonitor *CStackMonitor::m_Instance = NULL;
CRITICAL_SECTION CStackMonitor::m_csCreateInst = {0}, CStackMonitor::m_csStackRegisterOp = {0};

CStackMonitor *CStackMonitor::GetInstance()
{
	if(m_Instance == NULL)
	{
		EnterCriticalSection(&m_csCreateInst);

		if(m_Instance == NULL)
		{
			m_Instance = new CStackMonitor();
		}

		LeaveCriticalSection(&m_csCreateInst);
	}

	return m_Instance;
}

BOOL CStackMonitor::EnumAndRegisterThreads(BOOL bRemoveExecute, BOOL bRegister)
{
	BOOL bSuccess = FALSE, bErrored = FALSE;
	
	m_bRemoveExecute = bRemoveExecute;

	HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if(hThreadSnap != INVALID_HANDLE_VALUE)
	{
		THREADENTRY32 te32 = {0};
		te32.dwSize = sizeof(THREADENTRY32);

		if(Thread32First(hThreadSnap, &te32))
		{
			do
			{
				if(te32.th32OwnerProcessID == GetCurrentProcessId())
				{
					HANDLE hThread = OpenThread(THREAD_QUERY_INFORMATION, FALSE, te32.th32ThreadID);
					if(hThread)
					{
						LPVOID lpvTIBStackBase = NULL, lpvTIBStackTop = NULL;

						if(Utils::ThreadGetStackRegion(hThread, &lpvTIBStackBase, &lpvTIBStackTop))
						{
							size_t cbTIBStackRegion = (PBYTE)lpvTIBStackBase - (PBYTE)lpvTIBStackTop;

							if(bRemoveExecute && !RegionRemoveExecute(lpvTIBStackTop, cbTIBStackRegion))
								bErrored = TRUE;

							if(bRegister && !RegisterThreadStack(Utils::ThreadIdFromHandle(hThread), lpvTIBStackTop, cbTIBStackRegion))
								bErrored = TRUE;
						}
						else
						{
#ifdef DEBUG_BUILD
							DebugBreak();
#endif
						}

						CloseHandle(hThread);
					}
				}
			}
			while(Thread32Next(hThreadSnap, &te32));
		}

		CloseHandle(hThreadSnap);
	}

	IRealtimeLog *pRealtimeLog = GetRealtimeLog();

	if(!bErrored)
	{
		IProcTrackerServices *pProcTracker = GetProcTrackerServices();

		pRealtimeLog->LogMessage(0, L"Enabled the StackMonitor feature");
		Globals::g_dwEnabledFeatures = pProcTracker->GetEnabledFeaturesProcess(GetCurrentProcessId());
		Globals::g_dwEnabledFeatures |= EEnabledFeatures::StackMonitorFeature;
		pProcTracker->SetEnabledFeaturesProcess(GetCurrentProcessId(), Globals::g_dwEnabledFeatures);

		bSuccess = TRUE;
	}
	else
	{
		pRealtimeLog->LogMessage(1, L"Unable to enable the StackMonitor feature");
	}

	return bSuccess;
}

BOOL CStackMonitor::RemoveExecuteFromThreadStack(DWORD dwThreadId)
{
	BOOL bSuccess = FALSE;

	HANDLE hThread = OpenThread(THREAD_QUERY_INFORMATION, FALSE, dwThreadId);
	if(!hThread)
		goto Cleanup;
	
	bSuccess = RemoveExecuteFromThreadStack(hThread);

Cleanup:
	if(hThread)
		CloseHandle(hThread);

	return bSuccess;
}

BOOL CStackMonitor::RemoveExecuteFromThreadStack(HANDLE hThread)
{
	BOOL bSuccess = FALSE;

	LPVOID lpvTIBStackBase = NULL, lpvTIBStackTop = NULL;

	if(Utils::ThreadGetStackRegion(hThread, &lpvTIBStackBase, &lpvTIBStackTop))
	{
		size_t cbTIBStackRegion = (PBYTE)lpvTIBStackBase - (PBYTE)lpvTIBStackTop;
		
		bSuccess = RegionRemoveExecute(lpvTIBStackTop, cbTIBStackRegion);
	}
	else
	{
#ifdef DEBUG_BUILD
		DebugBreak();
#endif
	}

	return bSuccess;
}

BOOL CStackMonitor::RegionRemoveExecute(LPVOID lpvRegion, size_t cbRegionSize)
{
	BOOL bSuccess = FALSE;
	DWORD dwOldProtect = 0;
	
	if(lpvRegion)
	{
		if(!VirtualProtect(lpvRegion, cbRegionSize, PAGE_READWRITE, &dwOldProtect))
			goto Cleanup;
	}
	else
	{
#ifdef DEBUG_BUILD
		CLogger::Break();
#endif
	}

	bSuccess = TRUE;
Cleanup:
	return bSuccess;
}

LPVOID CStackMonitor::RegionSize(LPVOID lpvRegion, size_t *pcbRegionSize)
{
	MEMORY_BASIC_INFORMATION mbi = {0};
	LPVOID lpvRegionBase = NULL;
	size_t sizeOfRegion  = 0;

	if(!VirtualQuery(lpvRegion, &mbi, sizeof(mbi)))
		goto Cleanup;

	lpvRegionBase = mbi.BaseAddress;
	sizeOfRegion = mbi.RegionSize;

	while(1)
	{
		if(VirtualQuery((PBYTE)lpvRegionBase - DEFAULT_PAGE_SIZE, &mbi, sizeof(mbi)))
		{
			if(mbi.RegionSize <= sizeOfRegion)
			{
				// past the stack region
				break;
			}

			sizeOfRegion += DEFAULT_PAGE_SIZE;
			lpvRegionBase = mbi.BaseAddress;
		}
		else
		{
			// unallocated page past bottom of stack
			break;
		}
	}

	*pcbRegionSize = sizeOfRegion;
Cleanup:
	return lpvRegionBase;
}

BOOL CStackMonitor::RegisterThreadStack(DWORD dwThreadId, LPVOID lpvRegion, size_t cbRegionSize)
{
	BOOL bSuccess = FALSE;
	size_t sizeOfStack  = 0;
	LPVOID lpvStackBase = NULL;
	
	EnterCriticalSection(&m_csStackRegisterOp);
	
	//WCHAR wszTmp[256];
	//wsprintf(wszTmp, L"RegisterThreadStack(): TID = 0x%x; stack pointer = 0x%p\n", dwThreadId, lpvStackPointer);
	//OutputDebugString(wszTmp);

	if(!lpvRegion || !cbRegionSize)
		goto Cleanup;
	
	m_mapThreadStacks[dwThreadId].lpvStackBase = lpvRegion;
	m_mapThreadStacks[dwThreadId].sizeOfStack = cbRegionSize;
	
	bSuccess = TRUE;
Cleanup:
	LeaveCriticalSection(&m_csStackRegisterOp);

	return bSuccess;
}

BOOL CStackMonitor::IsValidStackPointer(DWORD dwThreadId, LPVOID lpvStackPointer)
{
	BOOL bValid = TRUE;
	
	// this causes deadlocks with VP/VA race conditions. We'll have to surrender (alternative: kill process!)
	if(!TryEnterCriticalSection(&m_csStackRegisterOp))
		return TRUE;
	
	//WCHAR wszTmp[256];
	//wsprintf(wszTmp, L"IsValidStackPointer(): TID = 0x%x; stack pointer = 0x%p\n", dwThreadId, lpvStackPointer);
	//OutputDebugString(wszTmp);

	if(m_mapThreadStacks.find(dwThreadId) != m_mapThreadStacks.end())
	{
		LPVOID lpvStackBase = m_mapThreadStacks[dwThreadId].lpvStackBase;
		size_t sizeOfStack = m_mapThreadStacks[dwThreadId].sizeOfStack;

		if(lpvStackPointer < lpvStackBase || lpvStackPointer > (PBYTE)lpvStackBase + sizeOfStack)
		{
			bValid = FALSE;
		}
	}
	else
	{
		// not necessarily valid, but we can't error because this will occasionally happen. We could register the stack here
		// but that may introduce an attack vector.
		// RegisterThreadStack(dwThreadId, lpvStackPointer);
	}

	LeaveCriticalSection(&m_csStackRegisterOp);

	return bValid;
}

BOOL CStackMonitor::IsValidStackBase(DWORD dwThreadId, LPVOID lpvStackPointer)
{
	BOOL bValid = FALSE;
	
	if(!TryEnterCriticalSection(&m_csStackRegisterOp))
		return TRUE;
	
	if(m_mapThreadStacks.find(dwThreadId) != m_mapThreadStacks.end())
	{
		LPVOID lpvStackBase = m_mapThreadStacks[dwThreadId].lpvStackBase;
		size_t sizeOfStack = m_mapThreadStacks[dwThreadId].sizeOfStack;

		if(lpvStackPointer == ((PBYTE)lpvStackBase + sizeOfStack))
		{
			bValid = TRUE;
		}
	}
	
	LeaveCriticalSection(&m_csStackRegisterOp);

	return bValid;
}

void CStackMonitor::RefreshThreadStack(DWORD dwThreadId, HANDLE hThreadHandle)
{
	HANDLE hThread = hThreadHandle ? hThreadHandle : OpenThread(THREAD_GET_CONTEXT | THREAD_SUSPEND_RESUME | THREAD_QUERY_INFORMATION, FALSE, dwThreadId);
	if(hThread)
	{
		LPVOID lpvTIBStackBase = NULL, lpvTIBStackTop = NULL;

		if(!Utils::ThreadGetStackRegion(hThread, &lpvTIBStackBase, &lpvTIBStackTop))
			goto Cleanup; // we can't possibly trust this thread (invalid TEB)
		
		size_t cbTIBStackRegion = (PBYTE)lpvTIBStackBase - (PBYTE)lpvTIBStackTop;

		LPVOID lpvRegionStackBase = NULL, lpvRegionStackTop = NULL;
		size_t cbRegionStackSize = 0;

		if(GetCurrentThreadId() == dwThreadId)
		{
			DWORD dwDummyInEspRegion = 0;
			lpvRegionStackTop = RegionSize(&dwDummyInEspRegion, &cbRegionStackSize);
			lpvRegionStackBase = (PBYTE)lpvRegionStackTop + cbRegionStackSize;
		}
		else
		{
			// we can suspend this thread
			if(SuspendThread(hThread) == (DWORD)-1)
				goto Cleanup;

			CONTEXT ctx = {0};
			ctx.ContextFlags = CONTEXT_FULL;

			if(!GetThreadContext(hThread, &ctx))
			{
				ResumeThread(hThread);
				goto Cleanup;
			}

			lpvRegionStackTop = RegionSize((LPVOID)ctx.Esp, &cbRegionStackSize);
			lpvRegionStackBase = (PBYTE)lpvRegionStackTop + cbRegionStackSize;
			
			ResumeThread(hThread);
		}

		if((lpvTIBStackTop && lpvRegionStackTop && lpvTIBStackBase && lpvRegionStackBase) &&
			(cbTIBStackRegion == cbRegionStackSize) &&
			(lpvTIBStackBase == lpvRegionStackBase))
		{
			// the stack has expanded downwards, just re-register
			
			//WCHAR wsz[256] = {0}; wsprintf(wsz, L"lpvTIBStackTop 0x%p, lpvRegionStackTop 0x%p, lpvTIBStackBase 0x%p, lpvRegionStackBase 0x%p, cbTIBStackRegion 0x%p, cbRegionStackSize 0x%p\n",
			//	lpvTIBStackTop, lpvRegionStackTop, lpvTIBStackBase, lpvRegionStackBase, cbTIBStackRegion, cbRegionStackSize); OutputDebugString(wsz);

			if(m_bRemoveExecute)
			{
				// remove execute from this expanded region
				//RegionRemoveExecute(lpvTIBStackTop, cbTIBStackRegion);
				
				if(DefenseInDepth::Real_NtProtectVirtualMemory != NULL)
				{
					ULONG ulOldProtect = 0;
					PVOID pvBaseAddress = lpvTIBStackTop;
					ULONG ulNumberBytes = cbTIBStackRegion;

					DefenseInDepth::Real_NtProtectVirtualMemory(GetCurrentProcess(), &pvBaseAddress, &ulNumberBytes, PAGE_READWRITE, &ulOldProtect);
				}
			}

			RegisterThreadStack(dwThreadId, lpvTIBStackTop, cbTIBStackRegion);
		}

Cleanup:
		if(hThread != hThreadHandle)
		{
			// otherwise caller is responsible for closing handle
			CloseHandle(hThread);
		}
	}
}

void CStackMonitor::UnregisterThreadStack(DWORD dwThreadId)
{
	std::hash_map<DWORD, StackRegionInfo>::iterator it;

	EnterCriticalSection(&m_csStackRegisterOp);
	
	//WCHAR wszTmp[256];
	//wsprintf(wszTmp, L"UnregisterThreadStack(): TID = 0x%x; ", dwThreadId);
	
	it = m_mapThreadStacks.find(dwThreadId);
	if(it != m_mapThreadStacks.end())
	{
		//wsprintf(wszTmp + wcslen(wszTmp), L"stack base = 0x%p\n", it->second.lpvStackBase);
		m_mapThreadStacks.erase(it);
	}
	else
	{
		//wsprintf(wszTmp + wcslen(wszTmp), L" [NOT FOUND]\n");
	}

	//OutputDebugString(wszTmp);
	
	LeaveCriticalSection(&m_csStackRegisterOp);
}

void CStackMonitor::DisplayThreadStacks()
{
	std::hash_map<DWORD, StackRegionInfo>::iterator it;
	WCHAR wsz[256] = {0};

	for(it = m_mapThreadStacks.begin(); it != m_mapThreadStacks.end(); it++)
	{
		wsprintf(wsz, L"ThreadId: 0x%.08x - StackBase: 0x%p - StackSize: 0x%.08x\n", it->first, it->second.lpvStackBase, it->second.sizeOfStack);
		OutputDebugString(wsz);
	}
}