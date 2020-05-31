// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]
#include "stdafx.h"
#include "Detours.h"
#include "..\..\libpdasm\libpdasm\disasm.h"

void DetoursFree::InitStubProvider()
{
	if(pvStubRange)
		return;

	LARGE_INTEGER li = {0, 0};

	QueryPerformanceCounter(&li);
	srand(GetTickCount() ^ li.LowPart ^ li.HighPart);

	pvStubRange = (PVOID)VirtualAlloc(
			NULL,
			STUB_RANGE_SIZE_BYTES,
			MEM_RESERVE | MEM_COMMIT,
			PAGE_READWRITE
		);
}

PVOID DetoursFree::GetFreeStub()
{
	BOOL bValidSlot = FALSE;
	size_t offs = 0;

	for(size_t i=0; i < 32; i++) // arbitrary number of times
	{
		offs = (size_t)(rand() % STUB_MAX_ENTRIES);
		
		if(setStubOffsTaken.find(offs) == setStubOffsTaken.end())
		{
			bValidSlot = TRUE;
			break;
		}
	}

	if(!bValidSlot)
	{
		for(size_t i=0; i < STUB_MAX_ENTRIES; i++)
		{
			if(setStubOffsTaken.find(i) == setStubOffsTaken.end())
			{
				offs = i;
				bValidSlot = TRUE;
				
				break;
			}
		}
	}

	if(!bValidSlot)
	{
		// no free slots!
		return NULL;
	}

	setStubOffsTaken.insert(offs);

	return ((PBYTE)pvStubRange + (offs * STUB_SIZE));
}

void DetoursFree::ReleaseStub(PVOID pvStub)
{
	size_t offs = ((PBYTE)pvStub - (PBYTE)pvStubRange) / STUB_SIZE;

	// !!fix this later!!
	// ZeroMemory(pvStub, STUB_SIZE);

	setStubOffsTaken.erase(offs);
}

BOOL DetoursFree::PrepareForWrite(PVOID pvStub)
{
	DWORD old = 0;

	// must be execute, as mem protection has page granularity, removing execute
	// permissions will lead to crash executing nearby stub

	if(!VirtualProtect(pvStub, STUB_SIZE, PAGE_EXECUTE_READWRITE, &old))
		return FALSE;

	return TRUE;
}

BOOL DetoursFree::PrepareForExecute(PVOID pvStub)
{
	DWORD old = 0;

	if(!VirtualProtect(pvStub, STUB_SIZE, PAGE_EXECUTE_READ, &old))
		return FALSE;

	return TRUE;
}

BOOL DetoursFree::SetPagePermissons(PVOID pvStub, DWORD dwNewProtect, DWORD *pdwOldProtect)
{
	DWORD old = 0;

	if(!VirtualProtect(pvStub, STUB_SIZE, dwNewProtect, pdwOldProtect ? pdwOldProtect : &old))
		return FALSE;

	return TRUE;
}

BOOL DetoursFree::CheckWriteable(PVOID pvStub)
{
	MEMORY_BASIC_INFORMATION mbi = {0};

	if(!VirtualQuery(pvStub, &mbi, sizeof(mbi)))
		return FALSE;

	return ( ((mbi.Protect & PAGE_READWRITE) != 0) ||
			 ((mbi.Protect & PAGE_WRITECOPY) != 0) ||
			 ((mbi.Protect & PAGE_EXECUTE_READWRITE) != 0) ||
			 ((mbi.Protect & PAGE_EXECUTE_WRITECOPY) != 0 ) );
}

BOOL DetoursFree::SuspendThreads()
{
	BOOL bSuccess = FALSE;

	HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if(hThreadSnap == INVALID_HANDLE_VALUE)
		goto Cleanup;

	THREADENTRY32 te32 = {0};
	te32.dwSize = sizeof(THREADENTRY32);

	if(!Thread32First(hThreadSnap, &te32))
		goto Cleanup;

	bSuccess = TRUE;

	do
	{
		if(te32.th32OwnerProcessID == GetCurrentProcessId())
		{
			if(te32.th32ThreadID != GetCurrentThreadId())
			{
				HANDLE hThread = OpenThread(THREAD_GET_CONTEXT | THREAD_SUSPEND_RESUME | THREAD_QUERY_INFORMATION, FALSE, te32.th32ThreadID);
				if(!hThread)
				{
					bSuccess = FALSE;
					continue;
				}

				if(SuspendThread(hThread) == (DWORD)-1)
				{
					bSuccess = FALSE;
				}

				CloseHandle(hThread);
			}
		}
	}
	while(Thread32Next(hThreadSnap, &te32) != FALSE);
	
Cleanup:
	return bSuccess;
}

BOOL DetoursFree::ResumeThreads()
{
	BOOL bSuccess = FALSE;

	HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if(hThreadSnap == INVALID_HANDLE_VALUE)
		goto Cleanup;

	THREADENTRY32 te32 = {0};
	te32.dwSize = sizeof(THREADENTRY32);

	if(!Thread32First(hThreadSnap, &te32))
		goto Cleanup;

	bSuccess = TRUE;

	do
	{
		if(te32.th32OwnerProcessID == GetCurrentProcessId())
		{
			if(te32.th32ThreadID != GetCurrentThreadId())
			{
				HANDLE hThread = OpenThread(THREAD_GET_CONTEXT | THREAD_SUSPEND_RESUME | THREAD_QUERY_INFORMATION, FALSE, te32.th32ThreadID);
				if(!hThread)
				{
					bSuccess = FALSE;
					continue;
				}

				if(ResumeThread(hThread) == (DWORD)-1)
				{
					bSuccess = FALSE;
				}

				CloseHandle(hThread);
			}
		}
	}
	while(Thread32Next(hThreadSnap, &te32) != FALSE);
	
Cleanup:
	return bSuccess;
}

size_t DetoursFree::InstructionBoundary(PVOID pv)
{
	size_t bound = 0, needed = 5; // jmp near 32
	PBYTE pb = (PBYTE)pv;

	for( ; bound < needed; )
	{
		DWORD cbInstr = Disasm::GetInstructionLength(pb + bound);
		if(!cbInstr)
		{
			bound = 0;
			break;
		}

		bound += cbInstr;
	}

	return bound;
}

BOOL DetoursFree::WriteDetour(PVOID pvStub, ThreadDetours::DT dt)
{
	PBYTE pbStub = (PBYTE)pvStub;
	//PVOID pvFirstTarget = *dt.ppfnOriginalFunction;
	DWORD diff = 0;
	BOOL bJmpThunk = FALSE;
	
	*dt.ppfnOriginalFunction = pbStub;

	size_t bound = InstructionBoundary(dt.pfnToDetour);
	if(bound == 0)
		return FALSE;
	
	if(!SetPagePermissons(pbStub, PAGE_EXECUTE_READWRITE, NULL))
		return ERROR_NOT_ENOUGH_MEMORY;
	
	memcpy(pbStub, dt.pfnToDetour, bound);
	
	if(pbStub[0] == 0xe8 || pbStub[0] == 0xe9) // call/jmp rel-32
	{
		diff = *(DWORD *)(pbStub + 1);
		PBYTE pbOriginalFunc = (PBYTE)dt.pfnToDetour + diff + 5;
		*(DWORD *)(pbStub + 1) = (DWORD)pbOriginalFunc - (DWORD)pbStub - 5;
	}
	
	diff = (DWORD)((PBYTE)dt.pfnToDetour - 5 - pbStub);
	pbStub[bound] = 0xe9; // jmp near
	*(DWORD *)((PBYTE)pbStub + bound + 1) = diff;
	
	if(!SetPagePermissons(pvStub, PAGE_EXECUTE_READ, NULL))
		return ERROR_NOT_ENOUGH_MEMORY;
	
	DWORD dwOldProtect = 0;

	if(!SetPagePermissons(dt.pfnToDetour, PAGE_EXECUTE_READWRITE, &dwOldProtect))
		return ERROR_NOT_ENOUGH_MEMORY;

	diff = (DWORD)((PBYTE)dt.pfnDetourTarget - (PBYTE)dt.pfnToDetour - 5);
	
	PBYTE pb = (PBYTE)dt.pfnToDetour;
	pb[0] = 0xe9;
	*(DWORD *)(pb + 1) = diff;

	if(!SetPagePermissons(dt.pfnToDetour, dwOldProtect, NULL))
		return ERROR_NOT_ENOUGH_MEMORY;

	return TRUE;
}

BOOL DetoursFree::RemoveDetour(ThreadDetours::DT dt)
{
	PBYTE pbStub = (PBYTE)*dt.ppfnOriginalFunction;
	PBYTE pbOriginalFunction = NULL;
	DWORD diff = 0;

	size_t bound = InstructionBoundary(pbStub);
	if(bound == 0)
		return FALSE;
	
	DWORD dwOldProtect = 0;

	if(pbStub[0] == 0xe8 || pbStub[0] == 0xe9) // call/jmp rel-32
	{
		diff = *(DWORD *)(pbStub + 1);
		pbOriginalFunction = (PBYTE)pbStub + diff + 5;
		
		diff = *(DWORD *)(pbStub + bound + 1);
		PBYTE pbThunk = pbStub + bound + diff;
		
		if(!SetPagePermissons(pbThunk, PAGE_EXECUTE_READWRITE, &dwOldProtect))
			return FALSE;
		
		*(DWORD *)(pbThunk + 1) = (DWORD)(pbOriginalFunction - pbThunk - 5);

		if(!SetPagePermissons(pbThunk, dwOldProtect, NULL))
			return FALSE;

		pbOriginalFunction = pbThunk;
	}
	else
	{
		diff = *(DWORD *)(pbStub + bound + 1); // jmp
		pbOriginalFunction = pbStub + diff + 5;

		if(!SetPagePermissons(pbOriginalFunction, PAGE_EXECUTE_READWRITE, &dwOldProtect))
			return FALSE;
		
		// write back stub code over jump near 32
		memcpy(pbOriginalFunction, pbStub, bound);

		if(!SetPagePermissons(pbOriginalFunction, dwOldProtect, NULL))
			return FALSE;
	}

	// restore user pointer to function
	*dt.ppfnOriginalFunction = pbOriginalFunction;

	// free stub
	ReleaseStub(pbStub);

	return TRUE;
}

LONG WINAPI DetoursFree::DetourTransactionBegin()
{
	// returns NO_ERROR or ERROR_INVALID_OPERATION
	DWORD dwTid = GetCurrentThreadId();

	ThreadDetours *ptdt = hmPendingDetours[dwTid];

	if(!ptdt)
	{
		ptdt = new ThreadDetours();
		hmPendingDetours[dwTid] = ptdt;
	}

	ptdt->Clear();

	if(!(ptdt = hmPendingRemove[dwTid]))
	{
		ptdt = new ThreadDetours();
		hmPendingRemove[dwTid] = ptdt;
	}

	ptdt->Clear();

	return NO_ERROR;
}

LONG WINAPI DetoursFree::DetourTransactionCommit()
{
	// returns NO_ERROR or ERROR_INVALID_DATA (target was rewritten by process during rewriting [polymorphic?])
	//     or ERROR_INVALID_OPERATION (DetourTransactionBegin was not called) or any code returned by
	//     DetourAttach/DetourDetach

	// attempt all pending operations. If lRet == ERROR_INVALID_BLOCK then one or more failed.

	BOOL bAdd = FALSE;
	LONG lRet = NO_ERROR;

	ThreadDetours *ptdt = hmPendingDetours[GetCurrentThreadId()];
	if(!ptdt)
		return ERROR_INVALID_OPERATION;

	if(ptdt->Detours.size() == 0)
	{
		ptdt = hmPendingRemove[GetCurrentThreadId()];
		if(!ptdt)
			return ERROR_INVALID_OPERATION;
	}
	else
	{
		bAdd = TRUE;
	}

	InitStubProvider();

	if(bAdd)
	{
		if(ptdt->Detours.size() != 0)
		{
			std::vector<ThreadDetours::DT>::const_iterator it = ptdt->Detours.begin();

			SuspendThreads();

			for(; it != ptdt->Detours.end(); it++)
			{
				PVOID pvStub = GetFreeStub();
				if(!pvStub)
				{
					ResumeThreads();
					return ERROR_NOT_ENOUGH_MEMORY;
				}

				if(!WriteDetour(pvStub, *it))
					lRet = ERROR_INVALID_BLOCK;
			}

			ResumeThreads();
		}
	}

	ptdt = hmPendingRemove[GetCurrentThreadId()];
	if(!ptdt)
		return ERROR_INVALID_OPERATION;

	if(ptdt->Detours.size() != 0)
	{
		std::vector<ThreadDetours::DT>::const_iterator it = ptdt->Detours.begin();

		SuspendThreads();

		for(; it != ptdt->Detours.end(); it++)
		{
			if(!RemoveDetour(*it))
				lRet = ERROR_INVALID_BLOCK;
		}

		ResumeThreads();
	}
	
	return lRet;
}

LONG WINAPI DetoursFree::DetourUpdateThread(HANDLE hThread)
{
	// if a thread enlisted via this function - when suspended - has it's instruction pointer
	// within the region of code being rewritten, the thread is updated to point to a valid
	// instruction (does this prevent it crashing??)
	
	// returns NO_ERROR or ERROR_NOT_ENOUGH_MEMORY (to store thread info)

	return NO_ERROR;
}

LONG WINAPI DetoursFree::DetourAttach(PVOID *ppPointer, PVOID pDetour)
{
	// returns NO_ERROR or ERROR_INVALID_BLOCK (target fn too small) or ERROR_INVALID_HANDLE (*ppPointer == NULL) or
	//     or ERROR_INVALID_OPERATION (DetourTransactionBegin not called) or ERROR_NOT_ENOUGH_MEMORY (generic mem failure)

	ThreadDetours *ptdt = hmPendingDetours[GetCurrentThreadId()];
	if(!ptdt)
		return ERROR_INVALID_OPERATION;

	if(!*ppPointer)
		return ERROR_INVALID_HANDLE;

	ThreadDetours::DT dt;

	dt.ppfnOriginalFunction = ppPointer;
	dt.pfnToDetour = *ppPointer;
	dt.pfnDetourTarget = pDetour;
	
	try
	{
		ptdt->Detours.push_back(dt);
	}
	catch(std::bad_alloc)
	{
		return ERROR_NOT_ENOUGH_MEMORY;
	}

	return NO_ERROR;
}

LONG WINAPI DetoursFree::DetourDetach(PVOID *ppPointer, PVOID pDetour)
{
	// reverse of DetourAttach: same error codes oddly.

	ThreadDetours *ptdt = hmPendingRemove[GetCurrentThreadId()];
	if(!ptdt)
		return ERROR_INVALID_OPERATION;

	if(!*ppPointer)
		return ERROR_INVALID_HANDLE;

	ThreadDetours::DT dt;

	dt.ppfnOriginalFunction = ppPointer;
	dt.pfnToDetour = *ppPointer;
	dt.pfnDetourTarget = pDetour;
	
	try
	{
		ptdt->Detours.push_back(dt);
	}
	catch(std::bad_alloc)
	{
		return ERROR_NOT_ENOUGH_MEMORY;
	}

	return NO_ERROR;
}