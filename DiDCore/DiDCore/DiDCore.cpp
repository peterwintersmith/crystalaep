// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]
// DiDCore.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "..\..\AuxCore\AuxCore\IRealtimeLog.h"
#include "..\..\AuxCore\AuxCore\IProcTrackerServices.h"
#include "..\..\ProcTracker\ProcTracker\ProcTracker_h.h"
#include "DiDCore.h"
#include "DfnsInDepth.h"
#include "HeapMonitor.h"
#include "ApiFw.h"
#include "AntiSpray.h"
#include "DynConfig.h"
#include "StackMonitor.h"
#include "..\..\AuxCore\AuxCore\IConfigReader.h"
#include "..\..\AuxCore\AuxCore\ILoadedModules.h"

CRITICAL_SECTION m_csMyUef;

void MyUefHandler()
{
	EnterCriticalSection(&m_csMyUef);
	// code to check type of instruction at ExceptionAddress and if JIT code made NX, return safely
	// JIT code will not reside in any other module, and will begin with a regular prologue
	LeaveCriticalSection(&m_csMyUef);
	ExitProcess(0);
}

PBYTE g_pbMyUefHandler = (PBYTE)MyUefHandler;
//PBYTE g_pbExitProcessAddress = (PBYTE)ExitProcess;
//PBYTE g_pbExitThreadAddress  = (PBYTE)ExitThread;

__declspec(naked) PBYTE ExitProcessAsmTemplate()
{
	__asm {
		call here
here:
		pop eax
		add eax, 5
		retn
		push 0
		call [g_pbMyUefHandler]
		_emit 0; _emit 0; _emit 0; _emit 0;
		_emit 0; _emit 0; _emit 0; _emit 0;
	}
}

void DisableUnhandledExceptionFilter()
{
	InitializeCriticalSection(&m_csMyUef);

	DetoursFree::PrepareForWrite(SetUnhandledExceptionFilter);
	BYTE SetUEFPatch[5] = {0x33, 0xc0, 0xc2, 0x04, 0x00}; // xor eax, eax/ret 4
	memcpy(SetUnhandledExceptionFilter, SetUEFPatch, 5);
	DetoursFree::PrepareForExecute(SetUnhandledExceptionFilter);

	DetoursFree::PrepareForWrite(UnhandledExceptionFilter);
	memcpy(UnhandledExceptionFilter, ExitProcessAsmTemplate(), 16);
	DetoursFree::PrepareForExecute(UnhandledExceptionFilter);
}

BOOL DoAddProcess()
{
	IProcTrackerServices *pProcTracker = GetProcTrackerServices();
	return pProcTracker->AddProcess(GetCurrentProcessId());
}

BOOL DiDCore::Initialize()
{
	InitializeCriticalSection(&CHeapMonitor::m_csCreateInst);
	InitializeCriticalSection(&CAntiSpray::m_csCreateInst);
	InitializeCriticalSection(&CDynamicConfig::m_csCreateInst);
	InitializeCriticalSection(&CStackMonitor::m_csCreateInst);
	InitializeCriticalSection(&CApiFirewall::m_csCreateInst);
	InitializeCriticalSection(&CApiFirewall::m_csApiFwOp);
	
	std::wstring wstrInstallPath = Utils::GetRegInstallPath();
	std::wstring wstrConfigPath;

	if(!wstrInstallPath.empty())
	{
		BOOL bCustomConfig = FALSE;
		std::string strThisProc;
		
		if(Utils::GetProcessName(strThisProc))
		{
			WCHAR *pwszProcName = NULL, *pwszExtn = NULL;

			do
			{
				if((pwszProcName = Utils::WcFromMultiByte(const_cast<char *>(strThisProc.c_str()))) == NULL)
					break;

				if((pwszExtn = wcsrchr(pwszProcName, '.')) == NULL)
					break;

				*pwszExtn = '\0';

				wstrConfigPath = wstrInstallPath + L"\\";
				wstrConfigPath += pwszProcName;
				wstrConfigPath += L"_config.xml";

				if(Utils::OpenEmptyFile(wstrConfigPath.c_str()))
				{
					bCustomConfig = TRUE;
				}
			}
			while(FALSE);

			if(pwszProcName != NULL)
				free(pwszProcName);
		}

		if(!bCustomConfig)
			wstrConfigPath = wstrInstallPath + L"\\config.xml";
		
		SetDllDirectory(wstrInstallPath.c_str());
	}

	// nice safe place to initialise these. Super important: Leads to infinite recursion
	// if initialised by a function (i.e. VirtualProtect) used by a heap function when
	// allocating instance (inst_ctor() => heap() => virtual_alloc() => inst_ctor() => etc)

	// just to initialize the MODULE and pfns required in more sensitive code
	Utils::ThreadGetStackRegion(GetCurrentThread(), NULL, NULL);

	CHeapMonitor *pHeapMon = CHeapMonitor::GetInstance();
	CAntiSpray *pAntiSpray = CAntiSpray::GetInstance();
	CDynamicConfig *pDynConfig = CDynamicConfig::GetInstance();
	CApiFirewall *pApiFw = CApiFirewall::GetInstance();
	CStackMonitor *pStackMonitor = CStackMonitor::GetInstance();

	ILoadedModules *pLoadedMods = GetLoadedModules();
	IConfigReader *pConfig = GetConfigReader();
	
	IRealtimeLog *pRealtimeLog = GetRealtimeLog();
	pRealtimeLog->Start();

	BOOL bConfigPathValid = FALSE;
	
	pRealtimeLog->LogMessage(0, L"DiDCore loading configuration from file %s", wstrConfigPath.c_str());
	
	if(pConfig->LoadFromXmlFile((WCHAR *)wstrConfigPath.c_str()))
	{
		bConfigPathValid = TRUE;
	}
	else
	{
		pRealtimeLog->LogMessage(0, L"Config path not found: Using default protection settings");
	}

	BOOL bChangeProtect = FALSE;

	if(!bConfigPathValid || !wcsicmp(pConfig->GetConfigElement(L"defenseInDepth", L"enabled"), L"true"))
	{
		pRealtimeLog->LogMessage(0, L"Enabling defense-in-depth protection (DiDCore)");
		
		// superseded by DiDCore hooks
		GetLoadedModules()->RemoveModuleHooks();

#ifndef DEBUG_BUILD
		// temporary fix for some issues which have been fixed more robustly now
		// DisableUnhandledExceptionFilter();
#endif
		DefenseInDepth::AttachAll();

		if(!wcsicmp(pConfig->GetConfigElement(L"apiMonitor", L"disableRWXVAMemory"), L"true"))
		{
			bChangeProtect = TRUE;
		}

		BOOL bNXStack = FALSE, bAntiROPStack = FALSE;

		if(!wcsicmp(pConfig->GetConfigElement(L"apiMonitor", L"disableRWXStackMemory"), L"true"))
		{
			bNXStack = TRUE;
		}

		if(!wcsicmp(pConfig->GetConfigElement(L"apiMonitor", L"enableAntiROPStack"), L"true"))
		{
			bAntiROPStack = TRUE;
		}

		CStackMonitor::GetInstance()->EnumAndRegisterThreads(bNXStack, bAntiROPStack);
	}

	if(bConfigPathValid)
		pConfig->Reload();
	
	if(bChangeProtect)
		Utils::bChangeProtect = TRUE;

	DoAddProcess();

	return TRUE;
}

BOOL DiDCore::UnInitialize()
{
	DefenseInDepth::DetachAll();
	return TRUE;
}
