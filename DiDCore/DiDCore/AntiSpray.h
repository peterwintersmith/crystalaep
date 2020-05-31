// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#pragma once

#include "..\..\AuxCore\AuxCore\IRealtimeLog.h"
#include "..\..\AuxCore\AuxCore\IProcTrackerServices.h"
#include "..\..\ProcTracker\ProcTracker\ProcTracker_h.h"

class CAntiSpray {
	friend BOOL DiDCore::Initialize();
public:
	static CAntiSpray* GetInstance();

private:
	typedef struct
	{
		LPVOID lpvSprayAddress;
		LPVOID lpvReservedBase;
		size_t ReservedBaseSize;
		void (*ExecutionHandler)();

	} ReservedHeapSprayRegion;

//#define IE_ALLOCATION_REGION_SIZE	0x40000
//#define IE_ALLOCATION_REGION_SIZE	0x1000

	CAntiSpray()
	{
		LPVOID rglpvSprayAddresses[21] = {
			(LPVOID)0x01010101, // ADD DWORD PTR DS:[ECX],EAX
			(LPVOID)0x02020202, // ADD AL,BYTE PTR DS:[EDX]
			(LPVOID)0x03030303, // ADD EAX,DWORD PTR DS:[EBX]
			(LPVOID)0x04040404, // ADD AL,4
			(LPVOID)0x05050505, // ADD EAX,5050505
			(LPVOID)0x06060606, // PUSH ES
			(LPVOID)0x07070707, // POP ES                                   ; Modification of segment register
			(LPVOID)0x08080808, // OR BYTE PTR DS:[EAX],CL
			(LPVOID)0x09090909, // OR DWORD PTR DS:[ECX],ECX
			(LPVOID)0x0a0a0a0a, // OR CL,BYTE PTR DS:[EDX]
			(LPVOID)0x0b0b0b0b, // OR ECX,DWORD PTR DS:[EBX]
			(LPVOID)0x0c0c0c0c, // OR AL,0C
			(LPVOID)0x0d0d0d0d, // OR EAX,0D0D0D0D
			(LPVOID)0x0e0e0e0e, // PUSH CS
			(LPVOID)0x10101010, // ADC BYTE PTR DS:[EAX],DL
			(LPVOID)0x11111111, // ADC DWORD PTR DS:[ECX],EDX
			(LPVOID)0x12121212, // ADC DL,BYTE PTR DS:[EDX]
			(LPVOID)0x13131313, // ADC EDX,DWORD PTR DS:[EBX]
			(LPVOID)0x14141414, // ADC AL,14
			(LPVOID)0x15151515, // ADC EAX,15151515
			(LPVOID)0x16161616  // PUSH SS
			// and more ...
		};

		ReservedHeapSprayRegion hsr = {0};
		//WCHAR wszLog[256] = {0};
		
		SYSTEM_INFO si = {0};
		GetSystemInfo(&si);

		for(int i=0; i<sizeof(rglpvSprayAddresses)/sizeof(LPVOID); i++)
		{
			hsr.lpvSprayAddress = rglpvSprayAddresses[i];
			hsr.ReservedBaseSize = si.dwPageSize;
			hsr.ExecutionHandler = DefaultExecutionHandler;
			
			if(ReserveMemRange(hsr))
			{
				//wsprintf(wszLog, L"AntiSpray: Successfully reserved memory at 0x%p (%u bytes)\n", rglpvSprayAddresses[i], IE_ALLOCATION_REGION_SIZE);
				m_vecReservedRegions.push_back(hsr);
			}
			else
			{
				//wsprintf(wszLog, L"AntiSpray: Unable to reserve memory at 0x%p (%u bytes)\n", rglpvSprayAddresses[i], IE_ALLOCATION_REGION_SIZE);
			}

			//OutputDebugString(wszLog);
		}

		IProcTrackerServices *pProcTracker = GetProcTrackerServices();
		IRealtimeLog *pRealtimeLog = GetRealtimeLog();

		if(m_vecReservedRegions.size() != 0)
		{
			pRealtimeLog->LogMessage(0, L"Enabled the AntiSpray feature");
			Globals::g_dwEnabledFeatures = pProcTracker->GetEnabledFeaturesProcess(GetCurrentProcessId());
			Globals::g_dwEnabledFeatures |= EEnabledFeatures::AntiSprayFeature;
			pProcTracker->SetEnabledFeaturesProcess(GetCurrentProcessId(), Globals::g_dwEnabledFeatures);
		}
	}

	BOOL ReserveMemRange(ReservedHeapSprayRegion& hsr);

	static void DefaultExecutionHandler()
	{
		//MessageBox(NULL, L"Code execution attempt successfully intercepted.\nProcess will now Terminate.", L"Crystal Default Anti-Spray Handler", MB_ICONSTOP);
		TerminateProcess(GetCurrentProcess(), BPCORE_TERMINATE_ANTISPRAY);
	}

	static CAntiSpray *m_Instance;
	static CRITICAL_SECTION m_csCreateInst;
	
	std::vector<ReservedHeapSprayRegion> m_vecReservedRegions;
};