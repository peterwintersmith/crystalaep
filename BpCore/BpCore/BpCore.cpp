// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]
// BpCore.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "..\..\AuxCore\AuxCore\IRealtimeLog.h"
#include "WininetHooks.h"
#include "Stream.h"
#include "BrwsrInstDetails.h"
#include "ConnectionPool.h"
#include "DemoPolicies.h"
#include "BrwsrInstDetails.h"
#include "..\..\AuxCore\AuxCore\IConfigReader.h"
#include "..\..\AuxCore\AuxCore\IUiFeedback.h"
#include "..\..\AuxCore\AuxCore\ILoadedModules.h"
#include "..\..\AuxCore\AuxCore\IEventWriter.h"
#include "..\..\Shared\Utils\Utils.h"
#include "..\..\AuxCore\AuxCore\IProcTrackerServices.h"
#include "BpCore.h"

BOOL AddFilters(IConfigReader *pConfig)
{
	BOOL bSuccess = FALSE;
	IRealtimeLog *pRealtimeLog = GetRealtimeLog();
	CConnectionPool *pConnPool = CConnectionPool::GetInstance();

	std::map<std::wstring, std::list<IConfigReader::NameValuePair *>>::const_iterator it;
	std::list<IConfigReader::NameValuePair *>::const_iterator it2;
	
	HMODULE hModule = NULL;
	const wchar_t *pwszFunctionName = NULL, *pwszModule = NULL;
	char *pszFunctionName = NULL;

	if(!pConnPool || !pRealtimeLog)
		goto Cleanup;

	for(it = pConfig->GetConfigRepository()->begin(); it != pConfig->GetConfigRepository()->end(); it++)
	{
		if(!wcsicmp(it->first.c_str(), L"connectFilters"))
		{
			for(it2 = it->second.begin(); it2 != it->second.end(); it2++)
			{
				pwszFunctionName = (*it2)->wstrName.c_str();
				pwszModule = (*it2)->wstrValue.c_str();
				
				std::wstring wstrDllPathFull = Utils::GetRegInstallPath() + L"\\";
				wstrDllPathFull += pwszModule;

				hModule = GetModuleHandle(wstrDllPathFull.c_str());
				if(!hModule)
				{
					hModule = LoadLibrary(wstrDllPathFull.c_str());
					if(!hModule)
					{
						// can't find checks dll
						continue;
					}
				}

				if(wcsnicmp(pwszFunctionName, L"Disabled:", 9) == 0)
					continue;

				if(wcsnicmp(pwszFunctionName, L"Enabled:", 8) == 0)
					pwszFunctionName += 8; // skip enabled state

				pszFunctionName = Utils::MbFromWideChar(const_cast<wchar_t *>(pwszFunctionName));
				if(!pszFunctionName)
					continue;
				
				pRealtimeLog->LogMessage(0, L"Loading connect check %S (DLL %s)", pszFunctionName, pwszModule);

				PFN_CP_CALLBACK pCallback = (PFN_CP_CALLBACK)GetProcAddress(hModule, pszFunctionName);
				if(!pCallback)
				{
					pRealtimeLog->LogMessage(0, L"Check %S does not seem to exist in module (DLL %s)", pszFunctionName, pwszModule);
					free(pszFunctionName);
					continue;
				}

				pConnPool->AddOpenCallback((PFN_CP_CALLBACK)GetProcAddress(hModule, pszFunctionName), 1);
				free(pszFunctionName);
			}
		}
		else if(!wcsicmp(it->first.c_str(), L"requestFilters"))
		{
			for(it2 = it->second.begin(); it2 != it->second.end(); it2++)
			{
				pwszFunctionName = (*it2)->wstrName.c_str();
				pwszModule = (*it2)->wstrValue.c_str();
				
				std::wstring wstrDllPathFull = Utils::GetRegInstallPath() + L"\\";
				wstrDllPathFull += pwszModule;

				hModule = GetModuleHandle(wstrDllPathFull.c_str());
				if(!hModule)
				{
					hModule = LoadLibrary(wstrDllPathFull.c_str());
					if(!hModule)
					{
						// can't find checks dll
						continue;
					}
				}

				if(wcsnicmp(pwszFunctionName, L"Disabled:", 9) == 0)
					continue;

				if(wcsnicmp(pwszFunctionName, L"Enabled:", 8) == 0)
					pwszFunctionName += 8; // skip enabled state

				pszFunctionName = Utils::MbFromWideChar(const_cast<wchar_t *>(pwszFunctionName));
				if(!pszFunctionName)
					continue;
				
				pRealtimeLog->LogMessage(0, L"Loading request check %S (DLL %s)", pszFunctionName, pwszModule);

				PFN_CP_CALLBACK pCallback = (PFN_CP_CALLBACK)GetProcAddress(hModule, pszFunctionName);
				if(!pCallback)
				{
					pRealtimeLog->LogMessage(0, L"Check %S does not seem to exist in module (DLL %s)", pszFunctionName, pwszModule);
					free(pszFunctionName);
					continue;
				}

				pConnPool->AddRequestCallback((PFN_CP_CALLBACK)GetProcAddress(hModule, pszFunctionName), 1);
				free(pszFunctionName);
			}
		}
		else if(!wcsicmp(it->first.c_str(), L"responseFilters"))
		{
			for(it2 = it->second.begin(); it2 != it->second.end(); it2++)
			{
				pwszFunctionName = (*it2)->wstrName.c_str();
				pwszModule = (*it2)->wstrValue.c_str();
				
				std::wstring wstrDllPathFull = Utils::GetRegInstallPath() + L"\\";
				wstrDllPathFull += pwszModule;

				hModule = GetModuleHandle(wstrDllPathFull.c_str());
				if(!hModule)
				{
					hModule = LoadLibrary(wstrDllPathFull.c_str());
					if(!hModule)
					{
						// can't find checks dll
						continue;
					}
				}
					
				if(wcsnicmp(pwszFunctionName, L"Disabled:", 9) == 0)
					continue;

				if(wcsnicmp(pwszFunctionName, L"Enabled:", 8) == 0)
					pwszFunctionName += 8; // skip enabled state

				pszFunctionName = Utils::MbFromWideChar(const_cast<wchar_t *>(pwszFunctionName));
				if(!pszFunctionName)
					continue;
				
				pRealtimeLog->LogMessage(0, L"Loading response check %S (DLL %s)", pszFunctionName, pwszModule);

				PFN_CP_CALLBACK pCallback = (PFN_CP_CALLBACK)GetProcAddress(hModule, pszFunctionName);
				if(!pCallback)
				{
					pRealtimeLog->LogMessage(0, L"Check %S does not seem to exist in module (DLL %s)", pszFunctionName, pwszModule);
					free(pszFunctionName);
					continue;
				}

				pConnPool->AddResponseCallback((PFN_CP_CALLBACK)GetProcAddress(hModule, pszFunctionName), 1);
				free(pszFunctionName);
			}
		}
	}
	
	bSuccess = TRUE;
Cleanup:
	return bSuccess;
}

BOOL DoAddProcess()
{
	IProcTrackerServices *pProcTracker = GetProcTrackerServices();
	return pProcTracker->AddProcess(GetCurrentProcessId());
}

BOOL BpCore::Initialize()
{
	InitializeCriticalSection(&CBrowserInstanceDetails::m_csCreateInst);
	InitializeCriticalSection(&CConnectionPool::m_csCreateInst);
	
	std::wstring wstrInstallPath = Utils::GetRegInstallPath();
	std::wstring wstrConfigPath;

	if(!wstrInstallPath.empty())
	{
		wstrConfigPath = wstrInstallPath + L"\\config.xml";
		SetDllDirectory(wstrInstallPath.c_str());
	}

	// nice safe place to initialise these. Not really necessary but makes sense.
	CBrowserInstanceDetails *pBrwsInstDetails = CBrowserInstanceDetails::GetInstance();
	CConnectionPool *pConnPool = CConnectionPool::GetInstance();

	IConfigReader *pConfig = GetConfigReader();
	ILoadedModules *pLoadedMods = GetLoadedModules();
	IEventWriter *pEvent = GetEventWriter();
	IUiFeedback *pUiFeedback = GetUiFeedback();
	IRealtimeLog *pRealtimeLog = GetRealtimeLog();

	pRealtimeLog->Start();

	BOOL bConfigPathValid = FALSE;
	
	pRealtimeLog->LogMessage(0, L"BpCore loading configuration from file %s", wstrConfigPath.c_str());
	
	if(pConfig->LoadFromXmlFile((WCHAR *)wstrConfigPath.c_str()))
	{
		bConfigPathValid = TRUE;
		AddFilters(pConfig);
	}
	else
	{
		pRealtimeLog->LogMessage(0, L"Config path not found: Using default protection settings");
	}
	
	if(!bConfigPathValid || !wcsicmp(pConfig->GetConfigElement(L"browserProtection", L"enabled"), L"true"))
	{
		pRealtimeLog->LogMessage(0, L"Enabling browser protection (BpCore)");
		
		BrowserHooks::AttachAll();
		WininetHooks::AttachAll();
	}

	if(bConfigPathValid)
		pConfig->Reload(); // now subscribers are invoked having been attached

	// if config not valid then load DiDCore anyway (default settings are used)
	if(!bConfigPathValid || !wcsicmp(pConfig->GetConfigElement(L"defenseInDepth", L"enabled"), L"true"))
	{
		pRealtimeLog->LogMessage(0, L"Enabling defense-in-depth protection (DiDCore)");
		
#ifdef DEBUG_BUILD
		std::wstring wstrDiDCoreDll = L"C:\\Work\\crystal\\BP\\Impl\\DiDCore\\Debug\\DiDCore.dll";
#else
		std::wstring wstrDiDCoreDll = wstrInstallPath + L"\\DiDCore.dll";
#endif

		if((GetModuleHandle(wstrDiDCoreDll.c_str()) == NULL) && (LoadLibrary(wstrDiDCoreDll.c_str()) == NULL))
		{
			pRealtimeLog->LogMessage(1, L"Unable to enable defense-in-depth protection (DLL not found)");
		}
		else
		{
			pRealtimeLog->LogMessage(1, L"Defense-in-depth protection enabled successfully");
		}
	}

	DoAddProcess();

	return TRUE;
}

BOOL BpCore::UnInitialize(BOOL bExiting)
{
	IRealtimeLog *pRealtimeLog = GetRealtimeLog();

	pRealtimeLog->LogMessage(0, L"Disabling browser hooks");

	BrowserHooks::DetachAll();
	WininetHooks::DetachAll();

	if(!bExiting)
		CConnectionPool::GetInstance()->Terminate();

	return TRUE;
}