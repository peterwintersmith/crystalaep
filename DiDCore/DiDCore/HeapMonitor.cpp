// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

/*
	TODO: implement use-after-free/double-free prevention:

		1. change all the win heap free functions to overwrite zero the memory to be freed and instead of
			freeing memory, add the memory to a recently freed alloc pool. Return success.

		2. have a background thread that sleeps for a second (or how long?) then:
			a. picks a random bucket (they should be fairly evenly filled)
			b. processes (num_buckets / 60) buckets hence (as we want all allocs freed within 1 minute)
			c. checks a refcount on each stored alloc to ensure it has not been freed more than once
				i. if so: DOUBLE FREE; terminate
			d. checks that memory is still fully zeroed
				i. if not: USE-AFTER-FREE; terminate
			e. actually frees each alloc in the recently freed pool using win heap api
*/

#include "stdafx.h"
#include "..\..\Shared\Utils\Utils.h"
#include "DfnsInDepth.h"
#include "DiDCore.h"
#include "..\..\AuxCore\AuxCore\IConfigReader.h"
#include "..\..\AuxCore\AuxCore\IEventWriter.h"
#include "HeapMonitor.h"
#include "..\..\AuxCore\AuxCore\IRealtimeLog.h"
#include "..\..\AuxCore\AuxCore\IProcTrackerServices.h"
#include "..\..\ProcTracker\ProcTracker\ProcTracker_h.h"

CHeapMonitor *CHeapMonitor::m_Instance = NULL;
CRITICAL_SECTION CHeapMonitor::m_csCreateInst = {0};
HANDLE CHeapMonitor::m_hIntlHeap = 0;

CHeapMonitor::FN_HEAPALLOC_DEF		CHeapMonitor::Real_HeapAlloc = NULL;
CHeapMonitor::FN_HEAPREALLOC_DEF	CHeapMonitor::Real_HeapReAlloc = NULL;
CHeapMonitor::FN_HEAPFREE_DEF		CHeapMonitor::Real_HeapFree = NULL;
CHeapMonitor::FN_RTLCREATEHEAP_DEF	CHeapMonitor::Real_RtlCreateHeap = NULL;
CHeapMonitor::FN_HEAPSIZE_DEF		CHeapMonitor::Real_HeapSize = NULL;
CHeapMonitor::FN_HEAPDESTROY_DEF	CHeapMonitor::Real_HeapDestroy = NULL;

CHeapMonitor *CHeapMonitor::GetInstance()
{
	if(m_Instance == NULL)
	{
		EnterCriticalSection(&m_csCreateInst);

		if(m_Instance == NULL)
		{
			m_Instance = new CHeapMonitor();
		}

		LeaveCriticalSection(&m_csCreateInst);
	}

	return m_Instance;
}

void CHeapMonitor::SetTerminateOnCorruptionAllHeaps()
{
	if(!m_bEnabled)
		return;

	IRealtimeLog *pRealtimeLog = GetRealtimeLog();

	if(m_bTermCorrHeaps || m_bRemoveExecuteHeaps)
	{
		DWORD dwHeapCount = GetProcessHeaps(0, NULL);

		if(dwHeapCount)
		{
			HANDLE *phHeaps = (HANDLE *)Real_HeapAlloc(m_hIntlHeap, 0, (dwHeapCount * sizeof(HANDLE)));
			
			if(phHeaps)
			{
				dwHeapCount = GetProcessHeaps(dwHeapCount, phHeaps);
				
				for(DWORD i=0; i<dwHeapCount; i++)
				{
					if(m_bTermCorrHeaps)
					{
						pRealtimeLog->LogMessage(0, L"Setting terminate-on-corruption on heap 0x%p", phHeaps[i]);
						HeapSetInformation(phHeaps[i], HeapEnableTerminationOnCorruption, NULL, 0);
					}

					if(m_bRemoveExecuteHeaps)
					{
						size_t cbRegionSize = 0;

						LPVOID lpvRegion = RegionSize(phHeaps[i], &cbRegionSize);

						if(lpvRegion)
						{
							if(!RegionRemoveExecute(lpvRegion, cbRegionSize))
							{
#ifdef DEBUG
								DebugBreak();
#endif
								pRealtimeLog->LogMessage(0, L"Unable to remove execute from heap 0x%p", phHeaps[i]);
							}
							else
							{
								pRealtimeLog->LogMessage(0, L"Removed execute from heap 0x%p (size %u)", phHeaps[i], cbRegionSize);
							}
						}
						else
						{
#ifdef DEBUG
							DebugBreak();
#endif
							pRealtimeLog->LogMessage(0, L"Unable to get size of heap 0x%p", phHeaps[i]);
						}
					}
				}

				Real_HeapFree(m_hIntlHeap, 0, phHeaps);
			}
		}
		else
		{
			pRealtimeLog->LogMessage(1, L"Unable to enumerate process heaps!");
		}
	}

	if(m_bEnableProcDep)
	{
		// not the best place for this call

		pRealtimeLog->LogMessage(1, L"Enabling data execution prevention for process self (0x%x)", GetCurrentProcessId());
		SetProcessDEPPolicy(PROCESS_DEP_ENABLE);
	}
}

BOOL CHeapMonitor::IsAllocFromExecutableHeap(LPVOID lpvAlloc)
{
	BOOL bExecutableHeapAlloc = FALSE, bFoundAlloc = FALSE;
	CMonitoredWinAlloc *pmwha = NULL;

	for(DWORD i=0; i < (2 * DEFAULT_PAGE_SIZE); i++)
	{
		if((pmwha = m_htMonHeapAllocs.GetEntry((PBYTE)lpvAlloc - i)) != NULL)
		{
			bFoundAlloc = TRUE;
			break;
		}
	}

	if(!bFoundAlloc)
		goto Cleanup;

	if(m_htExecuteAllowedHeaps.GetEntry(pmwha->HeapHandle()) != NULL)
	{
		bExecutableHeapAlloc = TRUE;
	}

Cleanup:
	return bExecutableHeapAlloc;
}

BOOL CHeapMonitor::IsAllocThread()
{
	if(!m_bRemoveExecuteHeaps)
	{
		return FALSE;
	}
	
	PBOOL pBOOL = NULL;

	//EnterCriticalSection(&m_csMonitorOp);
	pBOOL = m_htNxHeapThreads.GetEntry((LPVOID)GetCurrentThreadId());
	//LeaveCriticalSection(&m_csMonitorOp);
	
	return (pBOOL != NULL);
}

LPVOID CHeapMonitor::RegionSize(LPVOID lpvRegion, size_t *pcbRegionSize)
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

BOOL CHeapMonitor::RegionRemoveExecute(LPVOID lpvRegion, size_t cbRegionSize)
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

PVOID CHeapMonitor::WinHeapAlloc(HANDLE hHeap, DWORD dwFlags, SIZE_T dwBytes)
{
	if(!m_bEnabled)
	{
		return Real_HeapAlloc(hHeap, dwFlags, dwBytes);
	}

	size_t cbPad = MINIMUM_HEAP_PAD_SIZE + m_PaddingRndBytes[m_dwPadBytesOffs % sizeof(m_PaddingRndBytes)];
	BYTE bytePad = m_PaddingRndBytes[m_dwPadBytesOffs % sizeof(m_PaddingRndBytes)];
	LPVOID lpvAddress = NULL;

	cbPad += 16 - (cbPad % 16); // align 16
	m_dwPadBytesOffs++;

	if(!m_bVaryAllocSize)
		cbPad = 0;
	
	if(dwBytes + cbPad < dwBytes)
		return NULL;
	
	BOOL bUndo = FALSE;

	if(m_bRemoveExecuteHeaps)
	{
		if(m_htExecuteAllowedHeaps.GetEntry(hHeap) == NULL)
		{
			bUndo = TRUE;

			//EnterCriticalSection(&m_csMonitorOp);
			m_htNxHeapThreads.SetEntry((LPVOID)GetCurrentThreadId(), &bUndo);
			//LeaveCriticalSection(&m_csMonitorOp);
		}
	}

	if((lpvAddress = Real_HeapAlloc(hHeap, dwFlags, dwBytes + cbPad)) == NULL)
		return NULL;
	
	if(bUndo)
	{
		//EnterCriticalSection(&m_csMonitorOp);
		m_htNxHeapThreads.RemoveEntry((LPVOID)GetCurrentThreadId());
		//LeaveCriticalSection(&m_csMonitorOp);
	}

	if((DWORD)lpvAddress >= 0xc0000000) // this function can return error codes too (>= 0xc0000000)
		return lpvAddress;

	memset((PBYTE)lpvAddress + dwBytes, bytePad, cbPad);

	CMonitoredWinAlloc mwha(lpvAddress, dwBytes, cbPad, bytePad, dwFlags, AllocType::HeapAllocType, hHeap, NULL);// (LPVOID)Utils::GetReturnAddress(2));
	
	//EnterCriticalSection(&m_csMonitorOp);
	m_htMonHeapAllocs.SetEntry(lpvAddress, &mwha);
	//LeaveCriticalSection(&m_csMonitorOp);

	return (PBYTE)lpvAddress;
}

PVOID CHeapMonitor::WinHeapReAlloc(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem, SIZE_T dwBytes)
{
	if(!m_bEnabled)
	{
		return Real_HeapReAlloc(hHeap, dwFlags, lpMem, dwBytes);
	}

	size_t cbPad = MINIMUM_HEAP_PAD_SIZE + m_PaddingRndBytes[m_dwPadBytesOffs % sizeof(m_PaddingRndBytes)];
	BYTE bytePad = m_PaddingRndBytes[m_dwPadBytesOffs % sizeof(m_PaddingRndBytes)];
	LPVOID lpvAddress = NULL;
	
	cbPad += 16 - (cbPad % 16); // align 16
	m_dwPadBytesOffs++;

	if(!m_bVaryAllocSize)
		cbPad = 0;
	
	if(dwBytes + cbPad < dwBytes)
		return NULL;
	
	if(!lpMem)
		return NULL;
	
	if(m_bWinValidateAlloc && HeapValidate(hHeap, dwFlags, lpMem) == FALSE)
	{
		//CLogger::Break();
		TerminateProcess(GetCurrentProcess(), BPCORE_TERMINATE_HEAP_CORRUPT);
	}

	//EnterCriticalSection(&m_csMonitorOp);
	CMonitoredWinAlloc *pmwha = m_htMonHeapAllocs.GetEntry(lpMem);
	//LeaveCriticalSection(&m_csMonitorOp);

	DWORD dwPrevAllocSize = 0;
	BOOL bPrevAlloc = FALSE;

	if(pmwha)
	{
		if(m_bCustomValidateAlloc)
		{
			switch(pmwha->IsCorrupted())
			{
			case ECorruptedMemoryState::MemFatallyCorrupted:
				//CLogger::Break();
				//DebugBreak();
				TerminateProcess(GetCurrentProcess(), BPCORE_TERMINATE_HEAP_CORRUPT);

				break;
			case ECorruptedMemoryState::MemNotCorrupted:
				break;
			}
		}

		bPrevAlloc = TRUE;
		dwPrevAllocSize = pmwha->Size();
		
		//EnterCriticalSection(&m_csMonitorOp);
		m_htMonHeapAllocs.RemoveEntry(lpMem);
		//LeaveCriticalSection(&m_csMonitorOp);

		pmwha = NULL;
	}
	
	BOOL bUndo = FALSE;

	if(m_bRemoveExecuteHeaps)
	{
		if(m_htExecuteAllowedHeaps.GetEntry(hHeap) == NULL)
		{
			bUndo = TRUE;

			//EnterCriticalSection(&m_csMonitorOp);
			m_htNxHeapThreads.SetEntry((LPVOID)GetCurrentThreadId(), &bUndo);
			//LeaveCriticalSection(&m_csMonitorOp);
		}
	}

	if((lpvAddress = Real_HeapReAlloc(hHeap, dwFlags, lpMem, dwBytes + cbPad)) == NULL)
		return NULL;
	
	if(bUndo)
	{
		//EnterCriticalSection(&m_csMonitorOp);
		m_htNxHeapThreads.RemoveEntry((LPVOID)GetCurrentThreadId());
		//LeaveCriticalSection(&m_csMonitorOp);
	}

	if((DWORD)lpvAddress >= 0xc0000000) // this function can return error codes too (>= 0xc0000000)
		return lpvAddress;

	memset((PBYTE)lpvAddress + dwBytes, bytePad, cbPad);

	if(bPrevAlloc && (dwFlags & HEAP_ZERO_MEMORY))
	{
		DWORD cbZero = (dwPrevAllocSize >= dwBytes) ? 0 : (dwBytes - dwPrevAllocSize);
		
		if(cbZero)
			memset((PBYTE)lpvAddress + dwPrevAllocSize, 0, cbZero);
	}

	CMonitoredWinAlloc mwha(lpvAddress, dwBytes, cbPad, bytePad, dwFlags, AllocType::HeapReAllocType, hHeap, NULL);// (LPVOID)Utils::GetReturnAddress(2));
	
	//EnterCriticalSection(&m_csMonitorOp);
	m_htMonHeapAllocs.SetEntry(lpvAddress, &mwha);
	//LeaveCriticalSection(&m_csMonitorOp);

	return lpvAddress;
}

BOOL CHeapMonitor::WinHeapFree(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem)
{
	if(!m_bEnabled)
	{
		return Real_HeapFree(hHeap, dwFlags, lpMem);
	}

	if(!lpMem)
		return Real_HeapFree(hHeap, dwFlags, NULL);
	
	BOOL bRet = TRUE, bFreeOverride = FALSE;
	
	//EnterCriticalSection(&m_csMonitorOp);
	CMonitoredWinAlloc *pmwha = m_htMonHeapAllocs.GetEntry(lpMem);
	//LeaveCriticalSection(&m_csMonitorOp);
	
	if(pmwha)
	{
		if(m_bCustomValidateAlloc)
		{
			switch(pmwha->IsCorrupted())
			{
			case ECorruptedMemoryState::MemFatallyCorrupted:
				//CLogger::Break();
				//DebugBreak();
				TerminateProcess(GetCurrentProcess(), BPCORE_TERMINATE_HEAP_CORRUPT);

				break;
			case ECorruptedMemoryState::MemNotCorrupted:
				break;
			}
		}
		
		size_t cbClean = 0;

		if(m_bWipeAllocsOnFree)
		{
			cbClean = pmwha->Size();
			memset(lpMem, MEM_CLEAR_CHAR, cbClean);
		}

		if(m_bAntiUseAfterFree)
		{
			if(!m_bWipeAllocsOnFree) // no point in doing it twice!
			{
				cbClean = pmwha->Size();
				memset(lpMem, MEM_CLEAR_CHAR, cbClean);
			}

			//EnterCriticalSection(&m_csMonitorOp);

			if((GetWorkingSetSize() < MEM_512MB_IN_BYTES) && !(dwFlags & HEAP_NO_SERIALIZE))
			{
				m_htHistoricAllocs.SetEntry(lpMem, pmwha); // copies pwmha
				m_htMonHeapAllocs.RemoveEntry(lpMem);
			}
			else
			{
				bFreeOverride = TRUE;
			}

			//LeaveCriticalSection(&m_csMonitorOp);
		}
	}
	else
	{
		size_t cbClean = 0;

		if(m_bWipeAllocsOnFree)
		{
			// we need to get the real alloc size for non-monitored alloc
			cbClean = Real_HeapSize(hHeap, 0, lpMem);
			memset(lpMem, MEM_CLEAR_CHAR, cbClean);
		}

		if(m_bAntiUseAfterFree)
		{
			if(!m_bWipeAllocsOnFree)
			{
				cbClean = Real_HeapSize(hHeap, 0, lpMem);
				memset(lpMem, MEM_CLEAR_CHAR, cbClean);
			}

			if(m_bAntiDoubleFree)
			{
				//EnterCriticalSection(&m_csMonitorOp);

				if(m_htHistoricAllocs.GetEntry(lpMem))
				{
					// an alloc which has already been freed is being freed again
					TerminateProcess(GetCurrentProcess(), BPCORE_TERMINATE_DOUBLE_FREE);
				}
				else
				{
					// either m_bAntiDoubleFree=false or not an alloc we were able to track
					// we could create a dummy entry and add it to historic alloc pool, but
					// it's unlikely we'll hit this case often enough for it to count for much
				}
				
				//LeaveCriticalSection(&m_csMonitorOp);
			}

			CMonitoredWinAlloc mwha(lpMem, cbClean, 0, 0, dwFlags, AllocType::HeapAllocType, hHeap, NULL);

			//EnterCriticalSection(&m_csMonitorOp);

			if((GetWorkingSetSize() < MEM_512MB_IN_BYTES) && !(dwFlags & HEAP_NO_SERIALIZE))
			{
				m_htHistoricAllocs.SetEntry(lpMem, &mwha); // copies pwmha
				//bRet = Real_HeapFree(hHeap, dwFlags, lpMem);
			}
			else
			{
				bFreeOverride = TRUE;
			}

			//LeaveCriticalSection(&m_csMonitorOp);
		}
	}

	if(!m_bAntiUseAfterFree || bFreeOverride)
	{
		bRet = Real_HeapFree(hHeap, dwFlags, lpMem);

		if(pmwha)
		{
			//EnterCriticalSection(&m_csMonitorOp);
			m_htMonHeapAllocs.RemoveEntry(lpMem);
			//LeaveCriticalSection(&m_csMonitorOp);
		}
	}
	else
	{
		// in this case the alloc has been added to the historic allocs pool for deferred free
	}

	return bRet;
}

SIZE_T CHeapMonitor::WinHeapSize(
  __in  HANDLE hHeap,
  __in  DWORD dwFlags,
  __in  LPCVOID lpMem
)
{
	if(!m_bEnabled)
	{
		return Real_HeapSize(hHeap, dwFlags, lpMem);
	}

	size_t cbAllocation = Real_HeapSize(hHeap, dwFlags, lpMem);

#ifdef DEBUG_BUILD
	if(!lpMem)
		DebugBreak();
#endif

	//EnterCriticalSection(&m_csMonitorOp);
	CMonitoredWinAlloc *pmwha = m_htMonHeapAllocs.GetEntry(const_cast<LPVOID>(lpMem));
	//LeaveCriticalSection(&m_csMonitorOp);
	
	if(pmwha)
	{
		if(cbAllocation < pmwha->RandomPadSize())
		{
			// was reallocated using functions we're not monitoring, and will definitely lead to problems
			// further down the line. Let's remove from monitored list.
			// CLogger::Break();

			//DebugBreak();

			//EnterCriticalSection(&m_csMonitorOp);
			m_htMonHeapAllocs.RemoveEntry(const_cast<LPVOID>(lpMem));
			//LeaveCriticalSection(&m_csMonitorOp);
		}
		else
		{
			if(m_bCustomValidateAlloc)
			{
				switch(pmwha->IsCorrupted())
				{
				case ECorruptedMemoryState::MemFatallyCorrupted:
					//CLogger::Break();
					//DebugBreak();
					TerminateProcess(GetCurrentProcess(), BPCORE_TERMINATE_HEAP_CORRUPT);
					
					break;
				case ECorruptedMemoryState::MemNotCorrupted:
					break;
				}
			}
			
			cbAllocation -= pmwha->RandomPadSize();
		}
	}

	return cbAllocation;
}

PVOID CHeapMonitor::WinHeapCreate(
  __in      ULONG Flags,
  __in_opt  PVOID HeapBase,
  __in_opt  SIZE_T ReserveSize,
  __in_opt  SIZE_T CommitSize,
  __in_opt  PVOID Lock,
  __in_opt  LPVOID Parameters
)
{
	PVOID pvHeap = Real_RtlCreateHeap(Flags, HeapBase, ReserveSize, CommitSize, Lock, Parameters);
	
	if(!m_bEnabled)
	{
		return pvHeap;
	}

	if(m_bTermCorrHeaps || m_bRemoveExecuteHeaps)
	{
		IRealtimeLog *pRealtimeLog = GetRealtimeLog();

		if(m_bTermCorrHeaps)
		{
			pRealtimeLog->LogMessage(0, L"Setting terminate on corruption on new heap with handle 0x%p", pvHeap);
			HeapSetInformation((HANDLE)pvHeap, HeapEnableTerminationOnCorruption, NULL, 0);
		}

		if(m_bRemoveExecuteHeaps)
		{
			if(!(Flags & HEAP_CREATE_ENABLE_EXECUTE))
			{
				size_t cbRegionSize = 0;
				
				LPVOID lpvRegion = RegionSize(pvHeap, &cbRegionSize);

				if(lpvRegion)
				{
					if(!RegionRemoveExecute(lpvRegion, cbRegionSize))
					{
#ifdef DEBUG
						DebugBreak();
#endif
						pRealtimeLog->LogMessage(0, L"Unable to remove execute from heap 0x%p", pvHeap);
					}
					else
					{
						pRealtimeLog->LogMessage(0, L"Removed execute from new heap 0x%p (size %u)", pvHeap, cbRegionSize);
					}
				}
				else
				{
#ifdef DEBUG
					DebugBreak();
#endif
					pRealtimeLog->LogMessage(0, L"Unable to get size of heap 0x%p", pvHeap);
				}
			}
			else
			{
				// we have to allow these, as there's no way the app can function otherwise. This should be OK as this isn't an anti-ROP measure.
				BOOL bTRUE = TRUE;

				//EnterCriticalSection(&m_csMonitorOp);
				m_htExecuteAllowedHeaps.SetEntry(pvHeap, &bTRUE);
				//LeaveCriticalSection(&m_csMonitorOp);
			}
		}
	}

	return pvHeap;
}

BOOL WINAPI CHeapMonitor::WinHeapDestroy(__in HANDLE hHeap)
{
	if(!m_bEnabled)
	{
		return Real_HeapDestroy(hHeap);
	}
	
	//EnterCriticalSection(&m_csMonitorOp);
	
	if(m_bAntiUseAfterFree)
	{
		// remove all historic allocs from this heap as they'll now never be freed
		
		for(size_t i=0; i<POINTER_HASHTABLE_BUCKET_COUNT; i++)
		{
			m_htHistoricAllocs.LockBucket(i);
			CPointerHashtable<CMonitoredWinAlloc>::ListEntry *ple = m_htHistoricAllocs.Buckets[i].Entries;

			while(ple)
			{
				CMonitoredWinAlloc *pmwha = &ple->Entry;
				LPVOID lpv = ple->Idx;
				BOOL bRemove = FALSE;

				if(pmwha->HeapHandle() == hHeap)
					bRemove = TRUE;

				ple = ple->Next;

				if(bRemove)
					m_htHistoricAllocs.RemoveEntry(lpv);
			}

			m_htHistoricAllocs.UnlockBucket(i);
		}
	}
	
	BOOL bRet = Real_HeapDestroy(hHeap);
	
	if(m_bRemoveExecuteHeaps)
	{
		m_htExecuteAllowedHeaps.RemoveEntry(hHeap);
	}

	//LeaveCriticalSection(&m_csMonitorOp);

	return bRet;
}

SIZE_T CHeapMonitor::GetWorkingSetSize()
{
	PROCESS_MEMORY_COUNTERS pmc = {0};

	if(GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)) != FALSE)
	{
		return pmc.WorkingSetSize;
	}

	return 0;
}

DWORD WINAPI CHeapMonitor::PruneHistoricAllocs(LPVOID param)
{
	CPointerHashtable<CMonitoredWinAlloc> *phtHistoricAllocs = &GetInstance()->m_htHistoricAllocs;
	CHeapMonitor *pHeapMonInst = CHeapMonitor::GetInstance();
	
	//Sleep(5000);

	for(size_t i=0; ; i = ((i + 1) % POINTER_HASHTABLE_BUCKET_COUNT))
	{
		if(/*(GetWorkingSetSize() < MEM_512MB_IN_BYTES) && */!(i % 32))
			Sleep(10); // to lessen the CPU hit from this worker thread. This could be optimised.

		if(!pHeapMonInst->m_bAntiUseAfterFree)
		{
			Sleep(60000);
			continue;
		}
		
		// NB: Add RtlDestroyHeap detour to remove all historical allocs on heaps which have been destroyed. It might make
		// sense to have an array of historic alloc buckets; one for each heap handle (or something similar) [done!]

		//EnterCriticalSection(&pHeapMonInst->m_csMonitorOp);

		phtHistoricAllocs->LockBucket(i);
		CPointerHashtable<CMonitoredWinAlloc>::ListEntry *ple = phtHistoricAllocs->Buckets[i].Entries; // random bucket
		
		while(ple)
		{
			CMonitoredWinAlloc *pmwha = &ple->Entry;
			LPVOID lpv = ple->Idx;

			Real_HeapFree(pmwha->HeapHandle(), 0, lpv);

			ple = ple->Next;
			
			// not safe to lock just here because list may by altered by WinHeapFree while we're running?
			phtHistoricAllocs->RemoveEntry(lpv, FALSE);
		}

		phtHistoricAllocs->UnlockBucket(i);

		//LeaveCriticalSection(&pHeapMonInst->m_csMonitorOp);
	}
}

ECorruptedMemoryState CMonitoredWinAlloc::IsCorrupted()
{
	ECorruptedMemoryState state = ECorruptedMemoryState::MemNotCorrupted;
	
	// GlobalAlloc and similar allocate extra memory when call is passed to RtlAllocateHeap
	// for the purpose of maintaining heap specific info. Check user32!ConvertMemHandle to
	// see an example. For this reason, don't examine the last 8 bytes of the alloc.

	// this check just checks an arbitrary qword of padding which isn't overwritten by
	// LocalAlloc/GlobalAlloc and isn't particularly effective; what needs to be done is to
	// check the first 8 bytes of pad, allowing for Local/GlobalAlloc modifications

	// NB: now design is changed to hook each of Local, Global and Heap alloc so can now
	// check first n bytes with no trouble

	// NB(2): do not invoke the realtime log if corruption is detected as it gives opportunity
	// for exploitation of the heap

	if(m_cbPad == 0)
		goto Cleanup;

	if(m_type == AllocType::HeapAllocType || m_type == AllocType::HeapReAllocType)
	{
		LPVOID lpvAddress = m_lpvAddress;

		for(size_t i=0; i<sizeof(size_t); i++)
		{
			if(*(((PBYTE)lpvAddress) + m_cbAlloc + i) != m_bytePadChar)
			{
				state = ECorruptedMemoryState::MemFatallyCorrupted;
				break;
			}
		}
	}
	
Cleanup:
	return state;
}

void CHeapMonitor::ConfigRefresh(BOOL bRefresh)
{
	CHeapMonitor *pHeapMon = CHeapMonitor::GetInstance();
	IConfigReader *pConfig = GetConfigReader();

	WCHAR *pTmp;

#define NUM_BOOL_CONFIG_ELEMENTS	11
	LPWSTR rgwszConfigName[NUM_BOOL_CONFIG_ELEMENTS] = {
		L"enabled",
		L"varyAllocSize",
		L"terminateCorruptHeaps",
		L"winValidateAlloc",
		L"customValidateAlloc",
		L"antiUseAfterFree",
		L"antiDoubleFree",
		L"processDepEnabled",
		L"maxMonSensitivity",
		L"wipeAllocsOnFree",
		L"removeExecuteHeaps"
	};

	PBOOL rgpbool[NUM_BOOL_CONFIG_ELEMENTS] = {
		&pHeapMon->m_bEnabled,
		&pHeapMon->m_bVaryAllocSize,
		&pHeapMon->m_bTermCorrHeaps,
		&pHeapMon->m_bWinValidateAlloc,
		&pHeapMon->m_bCustomValidateAlloc,
		&pHeapMon->m_bAntiUseAfterFree,
		&pHeapMon->m_bAntiDoubleFree,
		&pHeapMon->m_bEnableProcDep,
		&pHeapMon->m_bSensitiveMonitoring,
		&pHeapMon->m_bWipeAllocsOnFree,
		&pHeapMon->m_bRemoveExecuteHeaps
	};

	//EnterCriticalSection(&pHeapMon->m_csMonitorOp);

	for(int i=0; i<NUM_BOOL_CONFIG_ELEMENTS; i++)
	{
		pTmp = pConfig->GetConfigElement(L"memoryMonitor", rgwszConfigName[i]);
		if(!pTmp)
			continue;

		if(!wcsicmp(pTmp, L"true"))
		{
			*(rgpbool[i]) = TRUE;
		}
		else
		{
			*(rgpbool[i]) = FALSE;
		}
	}

	if(!pHeapMon->m_bVaryAllocSize)
		pHeapMon->m_bCustomValidateAlloc = FALSE;

	pHeapMon->SetTerminateOnCorruptionAllHeaps();

	//LeaveCriticalSection(&pHeapMon->m_csMonitorOp);
}