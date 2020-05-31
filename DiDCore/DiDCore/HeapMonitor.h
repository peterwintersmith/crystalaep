// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#pragma once

#include "..\..\Shared\Utils\Utils.h"
#include "..\..\AuxCore\AuxCore\IConfigReader.h"
#include "..\..\AuxCore\AuxCore\IEventWriter.h"
#include "PointerHashtable.h"
#include "DiDCore.h"

class CMonitoredWinAlloc;
class CEventWriter;

class CHeapMonitor {
	friend BOOL DiDCore::Initialize();
	friend LPVOID WINAPI DefenseInDepth::My_HeapAlloc(HANDLE, DWORD, SIZE_T);
	friend LPVOID WINAPI DefenseInDepth::My_HeapReAlloc(HANDLE, DWORD, LPVOID, SIZE_T);
	friend BOOL WINAPI DefenseInDepth::My_HeapFree(HANDLE, DWORD, LPVOID);
	friend SIZE_T WINAPI DefenseInDepth::My_HeapSize(HANDLE, DWORD, LPCVOID);
	friend PVOID WINAPI DefenseInDepth::My_RtlCreateHeap(ULONG, PVOID, SIZE_T, SIZE_T, PVOID, LPVOID);
	friend NTSTATUS NTAPI DefenseInDepth::My_NtAllocateVirtualMemory(HANDLE ProcessHandle, PVOID*, ULONG, PULONG, ULONG, ULONG);
	friend NTSTATUS NTAPI DefenseInDepth::My_LdrLoadDll(PWCHAR, ULONG, PUNICODE_STRING, PHANDLE);
public:

#define MINIMUM_HEAP_PAD_SIZE	64
#define MEM_384MB_IN_BYTES		(384 * 1024 * 1024)
#define MEM_512MB_IN_BYTES		(512 * 1024 * 1024)
#define MEM_768MB_IN_BYTES		(768 * 1024 * 1024)
#define MEM_1GB_IN_BYTES		(1024 * 1024 * 1024)

#define	MEM_CLEAR_CHAR			0xec

	static CHeapMonitor *GetInstance();
	
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

	LPVOID WinHeapAlloc(HANDLE hHeap, DWORD dwFlags, SIZE_T dwBytes);
	LPVOID WinHeapReAlloc(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem, SIZE_T dwBytes);
	BOOL WinHeapFree(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem);
	PVOID WinHeapCreate(ULONG Flags, PVOID HeapBase, SIZE_T ReserveSize, SIZE_T CommitSize, PVOID Lock, LPVOID Parameters);
	SIZE_T WINAPI WinHeapSize(HANDLE hHeap, DWORD dwFlags, LPCVOID lpMem);
	BOOL WINAPI WinHeapDestroy(__in  HANDLE hHeap);
	
	static FN_HEAPALLOC_DEF		Real_HeapAlloc;
	static FN_HEAPREALLOC_DEF	Real_HeapReAlloc;
	static FN_HEAPFREE_DEF		Real_HeapFree;
	static FN_RTLCREATEHEAP_DEF	Real_RtlCreateHeap;
	static FN_HEAPSIZE_DEF		Real_HeapSize;
	static FN_HEAPDESTROY_DEF	Real_HeapDestroy;

	void SetTerminateOnCorruptionAllHeaps();
	BOOL IsAllocFromExecutableHeap(LPVOID lpvAlloc); // determine if m_htMonHeapAllocs[x] != NULL for: x = range - PAGE_SIZE; x++ <= range (slow, but not often invoked)
	
private:
	CHeapMonitor() : m_dwPadBytesOffs(0)
	{
		InitializeCriticalSection(&m_csMonitorOp);
		//InitializeCriticalSection(&m_csHistoricOp);
		
		HANDLE hThread = CreateThread(NULL, 0, PruneHistoricAllocs, NULL, 0, NULL);
		if(hThread)
			CloseHandle(hThread);

		for(int i=0; i<sizeof(m_PaddingRndBytes); i++)
		{
			// 0x40 to avoid spraying the heap with possibly useful addresses (i.e. 0x0c0c0c0c)
			m_PaddingRndBytes[i] = (BYTE)Utils::GetRandomInteger(0x40, 256);
		}

		m_hIntlHeap = HeapCreate(0, 0, 0);
		
#define HEAP_LFH	2
		ULONG ulHeapInformation = HEAP_LFH;
		HeapSetInformation(m_hIntlHeap, HeapCompatibilityInformation, &ulHeapInformation, sizeof(ULONG));

		m_htMonHeapAllocs.allocator = HeapAllocCallback;
		m_htMonHeapAllocs.deallocator = HeapFreeCallback;

		m_htHistoricAllocs.allocator = HeapAllocCallback;
		m_htHistoricAllocs.deallocator = HeapFreeCallback;
		
		m_htNxHeapThreads.allocator = HeapAllocCallback;
		m_htNxHeapThreads.deallocator = HeapFreeCallback;

		m_htExecuteAllowedHeaps.allocator = HeapAllocCallback;
		m_htExecuteAllowedHeaps.deallocator = HeapFreeCallback;
		
		// start default configuration
		m_bEnabled = TRUE;
		m_bTermCorrHeaps = TRUE;
		m_bVaryAllocSize = TRUE;
		m_bCustomValidateAlloc = TRUE;
		m_bWinValidateAlloc = FALSE;
		m_bAntiUseAfterFree = FALSE;
		m_bAntiDoubleFree = FALSE;
		m_bEnableProcDep = FALSE;
		m_bSensitiveMonitoring = TRUE;
		m_bWipeAllocsOnFree = TRUE;
		m_bRemoveExecuteHeaps = FALSE;
		// end default configuration

		IConfigReader *pConfig = GetConfigReader();
		pConfig->Subscribe(ConfigRefresh);

		m_pEventWriter = GetEventWriter();
	}

	~CHeapMonitor()
	{
		// clean up all floating ListEntries at once
		HeapDestroy(m_hIntlHeap);
		m_hIntlHeap = NULL;
	}

	BOOL IsAllocThread();
	LPVOID RegionSize(LPVOID lpvRegion, size_t *pcbRegionSize);
	BOOL RegionRemoveExecute(LPVOID lpvRegion, size_t cbRegionSize);
	
	static DWORD WINAPI PruneHistoricAllocs(LPVOID param);
	static SIZE_T GetWorkingSetSize();
	
	static void ConfigRefresh(BOOL bRefresh);
	
	static LPVOID HeapAllocCallback(size_t cb)
	{
		return Real_HeapAlloc(m_hIntlHeap, 0, cb);
	}

	static void HeapFreeCallback(LPVOID lpv)
	{
		Real_HeapFree(m_hIntlHeap, 0, lpv);
	}

	BOOL m_bEnabled, m_bTermCorrHeaps, m_bVaryAllocSize, m_bCustomValidateAlloc,
		m_bWinValidateAlloc, m_bAntiUseAfterFree, m_bAntiDoubleFree, m_bEnableProcDep,
		m_bSensitiveMonitoring, m_bWipeAllocsOnFree, m_bRemoveExecuteHeaps;

	static CHeapMonitor *m_Instance;
	static CRITICAL_SECTION m_csCreateInst;

	CRITICAL_SECTION m_csMonitorOp;//, m_csHistoricOp;
	
	static HANDLE m_hIntlHeap;

	// monitored heap allocations
	CPointerHashtable<CMonitoredWinAlloc> m_htMonHeapAllocs;
	
	// recently freed allocation pool
	CPointerHashtable<CMonitoredWinAlloc> m_htHistoricAllocs;
	
	// pool for NtAllocateVirtualMemory to query to determine whether or not W^X should be enforced
	CPointerHashtable<BOOL, 1009> m_htNxHeapThreads;
	CPointerHashtable<BOOL, 1009> m_htExecuteAllowedHeaps;

	IEventWriter *m_pEventWriter;
	BYTE m_PaddingRndBytes[1024];
	DWORD m_dwPadBytesOffs;
};

enum AllocType {
	HeapAllocType = 0,
	HeapReAllocType,
};

enum ECorruptedMemoryState {
	MemNotCorrupted = 0,
	MemPartiallyCorrupted,
	MemFatallyCorrupted
};

class CMonitoredWinAlloc {
public:
	CMonitoredWinAlloc(
			LPVOID lpvAddress, size_t cbAlloc, size_t cbPad, BYTE bytePadChar,
			DWORD dwFlags, AllocType type, HANDLE hHeap, LPVOID lpvCaller
		) :
			m_lpvAddress(lpvAddress), m_cbAlloc(cbAlloc), m_cbPad(cbPad),
			m_bytePadChar(bytePadChar), m_dwFlags(dwFlags), m_type(type),
			m_hHeap(hHeap), m_lpvCaller(lpvCaller)
	{
	}

	void operator=(CMonitoredWinAlloc &pInst)
	{
		m_lpvAddress	= pInst.m_lpvAddress;
		m_cbAlloc		= pInst.m_cbAlloc;
		m_cbPad			= pInst.m_cbPad;
		m_bytePadChar	= pInst.m_bytePadChar;
		m_dwFlags		= pInst.m_dwFlags;
		m_type			= pInst.m_type;
		m_hHeap			= pInst.m_hHeap;
		m_lpvCaller     = pInst.m_lpvCaller;
	}

	~CMonitoredWinAlloc()
	{
	}

	LPVOID Address() { return m_lpvAddress; }
	size_t Size() { return m_cbAlloc; }
	size_t RandomPadSize() { return m_cbPad; }
	BYTE RandomPadChar() { return m_bytePadChar; }
	DWORD Flags(){ return m_dwFlags; }
	AllocType AllocType(){ return m_type; }
	ECorruptedMemoryState IsCorrupted();
	HANDLE HeapHandle(){ return m_hHeap; }
	LPVOID Caller() { return m_lpvCaller; }
	
private:
	LPVOID m_lpvAddress, m_lpvCaller;
	size_t m_cbAlloc, m_cbPad;
	BYTE m_bytePadChar;
	DWORD m_dwFlags;
	enum AllocType m_type;
	HANDLE m_hHeap;
};