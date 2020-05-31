// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#include "stdafx.h"
#include "DiDCore.h"
#include "AntiSpray.h"

CRITICAL_SECTION CAntiSpray::m_csCreateInst = {0};
CAntiSpray *CAntiSpray::m_Instance = NULL;

CAntiSpray* CAntiSpray::GetInstance()
{
	if(m_Instance == NULL)
	{
		EnterCriticalSection(&m_csCreateInst);

		if(m_Instance == NULL)
		{
			m_Instance = new CAntiSpray();
		}

		LeaveCriticalSection(&m_csCreateInst);
	}

	return m_Instance;
}

BOOL CAntiSpray::ReserveMemRange(ReservedHeapSprayRegion& hsr)
{
	BOOL bSuccess = FALSE;
	MEMORY_BASIC_INFORMATION mbi = {0};
	SYSTEM_INFO si = {0};
	LPVOID lpvAddress = hsr.lpvSprayAddress;

	GetSystemInfo(&si);

	// round down to page boundary
	lpvAddress = (LPVOID)((PBYTE)lpvAddress - ((size_t)lpvAddress % si.dwPageSize));

	LPVOID lpvBase = VirtualAlloc(
			lpvAddress,
			hsr.ReservedBaseSize,
			MEM_RESERVE | MEM_COMMIT,
			PAGE_READWRITE
		);
	if(!lpvBase)
		goto Cleanup;

	/*
	WCHAR wszRegion[0x100];
	wsprintf(wszRegion, L"Region 0x%p allocated\n", lpvAddress);
	OutputDebugString(wszRegion);
	*/

	hsr.lpvReservedBase = lpvBase;

	size_t sizeOfMbi = VirtualQuery(lpvBase, &mbi, sizeof(mbi));
	if(sizeOfMbi)
	{
		hsr.ReservedBaseSize = mbi.RegionSize < hsr.ReservedBaseSize ?
			hsr.ReservedBaseSize : mbi.RegionSize;
	}

	// intercept execution and direct to hsr.ExecutionHandler
	memset(lpvBase, 0x0c, hsr.ReservedBaseSize);
	memset((PBYTE)lpvBase + hsr.ReservedBaseSize - 0x100 - 16, 0x90, 16); // nops for code alignment
	PBYTE pbHandlerCode = (PBYTE)lpvBase + hsr.ReservedBaseSize - 0x100;
	pbHandlerCode[0] = 0xb8; // mov eax, imm32/64
	*(size_t *)(pbHandlerCode + 1) = (size_t)hsr.ExecutionHandler;
	pbHandlerCode[1 + sizeof(size_t)] = 0xff; // call eax
	pbHandlerCode[2 + sizeof(size_t)] = 0xd0;

	DWORD dwOldProtect;
	if(!VirtualProtect(lpvBase, hsr.ReservedBaseSize, PAGE_EXECUTE_READ, &dwOldProtect))
		goto Cleanup;

	/*
	wsprintf(wszRegion, L"Success!\n");
	OutputDebugString(wszRegion);
	*/

	lpvBase = NULL;
	bSuccess = TRUE;
Cleanup:
	if(lpvBase)
		VirtualFree(lpvBase, hsr.ReservedBaseSize, MEM_RELEASE);

	return bSuccess;
}