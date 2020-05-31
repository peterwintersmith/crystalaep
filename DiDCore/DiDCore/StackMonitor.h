// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#pragma once

#include "PointerHashtable.h"

class CStackMonitor {
	friend BOOL DiDCore::Initialize();
	friend NTSTATUS NTAPI DefenseInDepth::My_NtAllocateVirtualMemory(
		__in HANDLE		ProcessHandle,
		__inout PVOID	*BaseAddress,
		__in ULONG		ZeroBits,
		__inout PULONG	RegionSize,
		__in ULONG		AllocationType,
		__in ULONG		Protect
	);
	friend NTSTATUS NTAPI DefenseInDepth::My_NtProtectVirtualMemory(
		__in HANDLE	ProcessHandle,
		__inout PVOID	*BaseAddress,
		__inout PULONG	NumberOfBytesToProtect,
		__in ULONG		NewAccessProtection,
		__out PULONG	OldAccessProtection
	);
public:
	static CStackMonitor* GetInstance();

	BOOL EnumAndRegisterThreads(BOOL bRemoveExecute, BOOL bRegister);
	BOOL RemoveExecuteFromThreadStack(DWORD dwThreadId);
	BOOL RemoveExecuteFromThreadStack(HANDLE hThread);

	BOOL RegisterThreadStack(DWORD dwThreadId, LPVOID lpvRegion, size_t cbRegionSize);
	BOOL IsValidStackPointer(DWORD dwThreadId, LPVOID lpvStackPointer);
	BOOL IsValidStackBase(DWORD dwThreadId, LPVOID lpvStackPointer);
	void RefreshThreadStack(DWORD dwThreadId, HANDLE hThreadHandle = NULL);
	void UnregisterThreadStack(DWORD dwThreadId);
	
protected:
	void DisplayThreadStacks();

private:
	CStackMonitor() : m_bRemoveExecute(FALSE)
	{
		InitializeCriticalSection(&m_csStackRegisterOp);
	}

	BOOL RegionRemoveExecute(LPVOID lpvRegion, size_t cbRegionSize);
	LPVOID RegionSize(LPVOID lpvRegion, size_t *pcbRegionSize);

	typedef struct {
		size_t sizeOfStack;
		LPVOID lpvStackBase;
	} StackRegionInfo;

	//std::map<DWORD, StackRegionInfo> m_mapThreadStacks;
	std::hash_map<DWORD, StackRegionInfo> m_mapThreadStacks;
	BOOL m_bRemoveExecute;

	static CStackMonitor *m_Instance;
	static CRITICAL_SECTION m_csCreateInst, m_csStackRegisterOp;
};