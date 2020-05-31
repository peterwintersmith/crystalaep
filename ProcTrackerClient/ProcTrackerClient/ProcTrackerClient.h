// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#pragma once

namespace ProcTrackerClient {
	BOOL Initialize();
	BOOL UnInitialize();
}

extern "C" __declspec(dllexport) DWORD DllGetEnabledFeaturesProcess(DWORD dwProcessID);

extern "C" __declspec(dllexport) BOOL DllAddEntryProcess(DWORD dwProcessID, RealtimeLogEntry *pRtLogEnt);
extern "C" __declspec(dllexport) DWORD DllGetProcessLogs(DWORD dwProcessID, DWORD cMaxLogEntries, RealtimeLogEntry rg_RtLogEnts[  ]);

extern "C" __declspec(dllexport) DWORD DllGetProcessIDs(DWORD cUniqueProcIDs, char rg_pszUniqueProcID[  ][ 512 ]);
