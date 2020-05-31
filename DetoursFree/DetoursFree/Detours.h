// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#pragma once

#include <set>
#include <hash_map>
#include <map>

namespace DetoursFree {

	typedef struct {
		
		typedef struct {
			
			PVOID pfnToDetour;
			PVOID pfnDetourTarget;
			PVOID *ppfnOriginalFunction;

		} DT;

		std::vector<DT> Detours;

		void Clear()
		{
			Detours.clear();
		}

	} ThreadDetours;

	static std::hash_map<DWORD, ThreadDetours *> hmPendingDetours;
	static std::hash_map<DWORD, ThreadDetours *> hmPendingRemove;
	
	void InitStubProvider();
	PVOID GetFreeStub();
	void ReleaseStub(PVOID pvStub);

	BOOL PrepareForWrite(PVOID pvStub);
	BOOL PrepareForExecute(PVOID pvStub);
	BOOL SetPagePermissons(PVOID pvStub, DWORD dwNewProtect, DWORD *pdwOldProtect);
	BOOL CheckWriteable(PVOID pvStub);
	BOOL SuspendThreads();
	BOOL ResumeThreads();

	size_t InstructionBoundary(PVOID pv);
	BOOL WriteDetour(PVOID pvStub, ThreadDetours::DT dt);
	BOOL RemoveDetour(ThreadDetours::DT dt);

#define STUB_RANGE_SIZE_BYTES	0x10000
#define STUB_SIZE				0x10
#define STUB_MAX_ENTRIES		(STUB_RANGE_SIZE_BYTES / STUB_SIZE)
#define MAX_ITERATIONS_WAIT_RX	24

	static PVOID pvStubRange = NULL;
	static std::set<size_t> setStubOffsTaken;

	LONG WINAPI DetourTransactionBegin();
	LONG WINAPI DetourTransactionCommit();

	LONG WINAPI DetourUpdateThread(HANDLE hThread);

	LONG WINAPI DetourAttach(PVOID *ppPointer,
							 PVOID pDetour);

	LONG WINAPI DetourDetach(PVOID *ppPointer,
	                         PVOID pDetour);

}