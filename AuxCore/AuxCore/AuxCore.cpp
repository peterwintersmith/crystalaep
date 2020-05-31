// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]
// AuxCore.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "..\..\ProcTracker\ProcTracker\ProcTracker_h.h"
#include "LoadedModules.h"
#include "EventWriter.h"
#include "ConfigReader.h"
#include "UiFeedback.h"
#include "ProcTrackerServices.h"
#include "RealtimeLog.h"
#include "..\..\Shared\Utils\Utils.h"
#include "AuxCore.h"

BOOL AuxCore::Initialize()
{
	InitializeCriticalSection(&CLoadedModules::m_csModOpLock);
	InitializeCriticalSection(&CLoadedModules::m_csCreateInst);
	InitializeCriticalSection(&CEventWriter::m_csCreateInst);
	InitializeCriticalSection(&CConfigReader::m_csCreateInst);
	InitializeCriticalSection(&CConfigReader::m_csReloadConfig);
	InitializeCriticalSection(&CUiFeedback::m_csCreateInst);
	InitializeCriticalSection(&CProcTrackerServices::m_csCreateInst);
	InitializeCriticalSection(&CRealtimeLog::m_csCreateInst);
	
	CLoadedModules *pLoadedMods = CLoadedModules::GetInstance();
	CEventWriter *pEvent = CEventWriter::GetInstance();
	CConfigReader *pConfig = CConfigReader::GetInstance();
	CUiFeedback *pUiFeedback = CUiFeedback::GetInstance();
	CProcTrackerServices *pProcTracker = CProcTrackerServices::GetInstance();
	CRealtimeLog *pRealtimeLog = CRealtimeLog::GetInstance();

	// for reasons entirely unbeknownst to me, when:
	// GetRegInstallPath => RegOpenKeyEx => RegCloseKey RtlEnterCriticalSection(&advapi32_cs)
	//   &advapi32_cs = NULL
	// not necessary to set DLL path here, in actual fact, as DLL is in local directory anyway.
	// perhaps we should drop it in system32 at install time (auxcore.dll) ?

	/*
	std::wstring wstrInstallPath = Utils::GetRegInstallPath();
	
	if(!wstrInstallPath.empty())
	{
		SetDllDirectory(wstrInstallPath.c_str());
	}
	*/

	return TRUE;
}

BOOL AuxCore::UnInitialize(BOOL bExiting)
{
	if(!bExiting)
	{
		CLoadedModules *pLoadedMods = CLoadedModules::GetInstance();
		pLoadedMods->Terminate();

		CRealtimeLog *pRealtimeLog = CRealtimeLog::GetInstance();
		pRealtimeLog->Terminate();
	}
	
	return TRUE;
}