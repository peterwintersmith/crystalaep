// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

// todo: add prompt user on exe blocked behaviour

#include "stdafx.h"
#include "DiDCore.h"
#include "..\..\Shared\Utils\Utils.h"
#include "..\..\AuxCore\AuxCore\ILoadedModules.h"
#include "..\..\AuxCore\AuxCore\IEventWriter.h"
#include "..\..\AuxCore\AuxCore\IConfigReader.h"
#include "..\..\AuxCore\AuxCore\IUiFeedback.h"
#include "..\..\ProcTracker\ProcTracker\ProcTracker_h.h"
#include "ApiFw.h"

CApiFirewall *CApiFirewall::m_Instance = NULL;
CRITICAL_SECTION CApiFirewall::m_csCreateInst = {0}, CApiFirewall::m_csApiFwOp = {0};

CApiFirewall::CApiFirewall()
{
	InitializeCriticalSection(&m_csApiFwOp);
		
	if(m_pwszTmpEnvPath = (WCHAR *)malloc(65600))
	{
		DWORD cch = GetEnvironmentVariable(L"TMP", m_pwszTmpEnvPath, (65600 / sizeof(WCHAR)));
		m_pwszTmpEnvPath[cch] = 0;
	}
	
	if(m_pwszTempEnvPath = (WCHAR *)malloc(65600))
	{
		DWORD cch = GetEnvironmentVariable(L"TEMP", m_pwszTempEnvPath, (65600 / sizeof(WCHAR)));
		m_pwszTempEnvPath[cch] = 0;
	}

	if(m_pwszUserprofileTmp = (WCHAR *)malloc(65600))
	{
		DWORD cch = GetEnvironmentVariable(L"USERPROFILE", m_pwszUserprofileTmp, (65600 / sizeof(WCHAR)));
		m_pwszUserprofileTmp[cch] = 0;
		wcscat(m_pwszUserprofileTmp, L"\\AppData\\Local\\Temp");
	}

	if(m_pwszWindowDirTmp = (WCHAR *)malloc(65600))
	{
		DWORD cch = GetWindowsDirectory(m_pwszWindowDirTmp, (65600 / sizeof(WCHAR)));
		m_pwszWindowDirTmp[cch] = 0;
		wcscat(m_pwszWindowDirTmp, L"\\Temp");
	}

	m_pEventWriter = GetEventWriter();

	// start default configuration
	m_bEnabled = TRUE;
	m_bAllowCodeFromTemp = FALSE;
	m_bAllowCodeFromNetwork = FALSE;
	m_bAllowCodeLongPath = FALSE;
	m_bAllowCodeFromInetTemp = FALSE;
	m_bAllowCodeFromDownloads = FALSE;
	m_bQueryUserOnExeBlocked = TRUE;
	m_bCheckWhitelist = TRUE;
	m_bCheckDirDllPlant = TRUE;
	m_bAutoAddToWhitelist = FALSE;
	m_pwszProtProc = NULL;
	// end default configuration

	IConfigReader *pConfig = GetConfigReader();
	pConfig->Subscribe(ConfigRefresh);
}

CApiFirewall *CApiFirewall::GetInstance()
{
	if(m_Instance == NULL)
	{
		EnterCriticalSection(&m_csCreateInst);

		if(m_Instance == NULL)
		{
			m_Instance = new CApiFirewall();
		}

		LeaveCriticalSection(&m_csCreateInst);
	}

	return m_Instance;
}

BOOL CApiFirewall::AddExeToWhitelist(WCHAR *pwszExeName)
{
	BOOL bSuccess = FALSE;
	
	RPC_WSTR pwszBinding = NULL;
	RPC_STATUS rpcStatus = RpcStringBindingCompose(NULL, (RPC_WSTR)L"ncalrpc", NULL, (RPC_WSTR)L"ProcTracker", NULL, &pwszBinding);
	if(rpcStatus != RPC_S_OK)
		goto Done_RPC;
	
	rpcStatus = RpcBindingFromStringBinding(pwszBinding, &::IProcTracker_v1_0_c_ifspec);
	if(rpcStatus != RPC_S_OK)
		goto Done_RPC;
	
	RpcTryExcept
	{
		bSuccess = ::AddExeToWhitelist(IProcTracker_v1_0_c_ifspec, pwszExeName);
	}
	RpcExcept(1)
	{
	}
	RpcEndExcept
	
	rpcStatus = RpcStringFree(&pwszBinding);
	if(rpcStatus != RPC_S_OK)
		goto Done_RPC;
	
	rpcStatus = RpcBindingFree(&::IProcTracker_v1_0_c_ifspec);
	if(rpcStatus != RPC_S_OK)
		goto Done_RPC;
	
Done_RPC:
	return bSuccess;
}

BOOL CApiFirewall::QueryProcessCreationAllowed(std::wstring wstrProcess, std::wstring wstrParameters)
{
	BOOL bRet = FALSE;
	std::wstring wstrExePath = wstrProcess, wstrExeName = L"";
	
	if(!wstrProcess.empty())
	{
		if(wstrProcess[0] == '\"' && wstrProcess.find('"', 1) != std::wstring::npos)
		{
			wstrExePath = wstrProcess.substr(1, wstrProcess.find('"', 1) - 1);
		}
	}
	else
	{
		if(wstrParameters.empty())
		{
			goto Cleanup;
		}

		WCHAR ch = wstrParameters[0];
		if(ch == '"' && wstrParameters.find('"', 1) != std::wstring::npos)
		{
			wstrExePath = wstrParameters.substr(1, wstrParameters.find('"', 1) - 1);
		}
		else
		{
			if(wstrParameters.find(' ', 1) != std::wstring::npos)
			{
				wstrExePath = wstrParameters.substr(0, wstrParameters.find(' ', 1));
				
				if(!wcsicmp(wstrExePath.c_str() + 1, L":\\Program"))
				{
					wstrExePath = wstrParameters.substr(0, wstrParameters.find(L".exe", 1) + 4);
				}
			}
			else
			{
				wstrExePath = wstrParameters;
			}
		}
	}

	if(wstrExePath.rfind('\\') != std::wstring::npos)
	{
		wstrExeName = wstrExePath.substr(wstrExePath.rfind('\\') + 1);
	}
	else
	{
		wstrExeName = wstrExePath;
	}

	if(!CheckPathNotUNC(wstrExePath))
	{
		if(m_bQueryUserOnExeBlocked && QueryUserRunExe(wstrExePath))
		{
			bRet = TRUE;
			goto Cleanup;
		}

		m_pEventWriter->WriteEvent(
				EEventRepository::ApiFirewall,
				EThreatRiskRating::MediumRisk,
				L"Process Creation Attempt Blocked",
				L"An attempt was made to launch a program from a remote share. The program attempted for launch was %s (path: \"%s\" args: \"%s\")",
				wstrExeName.c_str(),
				wstrProcess.c_str(),
				wstrParameters.c_str()
			);

		goto Cleanup;
	}

	if(!CheckPathNotLongName(wstrExePath))
	{
		if(m_bQueryUserOnExeBlocked && QueryUserRunExe(wstrExePath))
		{
			bRet = TRUE;
			goto Cleanup;
		}

		m_pEventWriter->WriteEvent(
				EEventRepository::ApiFirewall,
				EThreatRiskRating::MediumRisk,
				L"Process Creation Attempt Blocked",
				L"An attempt was made to lauch a program with a suspicious path. The program attempted for launch was %s (path: \"%s\" args: \"%s\")",
				wstrExeName.c_str(),
				wstrProcess.c_str(),
				wstrParameters.c_str()
			);

		goto Cleanup;
	}

	if(!CheckPathNotWinTemp(wstrExePath))
	{
		if(m_bQueryUserOnExeBlocked && QueryUserRunExe(wstrExePath))
		{
			bRet = TRUE;
			goto Cleanup;
		}

		m_pEventWriter->WriteEvent(
				EEventRepository::ApiFirewall,
				EThreatRiskRating::MediumRisk,
				L"Process Creation Attempt Blocked",
				L"An attempt was made to launch a program from a local temporary folder. The program attempted for launch was %s (path: \"%s\" args: \"%s\")",
				wstrExeName.c_str(),
				wstrProcess.c_str(),
				wstrParameters.c_str()
			);

		goto Cleanup;
	}

	if(!CheckPathNotTempInetFiles(wstrExePath))
	{
		if(m_bQueryUserOnExeBlocked && QueryUserRunExe(wstrExePath))
		{
			bRet = TRUE;
			goto Cleanup;
		}

		m_pEventWriter->WriteEvent(
				EEventRepository::ApiFirewall,
				EThreatRiskRating::MediumRisk,
				L"Process Creation Attempt Blocked",
				L"An attempt was made to launch a program from the Temporary Internet Files folder. The program attempted for launch was %s (path: \"%s\" args: \"%s\")",
				wstrExeName.c_str(),
				wstrProcess.c_str(),
				wstrParameters.c_str()
			);

		goto Cleanup;
	}

	if(CheckParamDirectoryDlls(wstrParameters))
	{
		// if true, remote directory contains .dll file (or reports to)
		
		m_pEventWriter->WriteEvent(
				EEventRepository::ApiFirewall,
				EThreatRiskRating::HighRisk,
				L"Process Creation Attempt Blocked",
				L"A probable attempt to execute a binary planting attack was prevented. The program under attack was %s (path: \"%s\" args: \"%s\")",
				wstrExeName.c_str(),
				wstrProcess.c_str(),
				wstrParameters.c_str()
			);

		goto Cleanup;
	}

	// wrap this in a function later
	if(m_bAllowSpawnProtProcs)
	{
		if(m_pwszProtProc != NULL && Utils::IsContainedInPipeSeparatedList(m_pwszProtProc, (WCHAR *)wstrExeName.c_str()))
		{
			bRet = TRUE;
			goto Cleanup;
		}
	}

	// check against whitelist
	if(!CheckPathWhitelist(wstrExeName))
	{
		if(m_bQueryUserOnExeBlocked && QueryUserRunExe(wstrExePath))
		{
			if(m_bAutoAddToWhitelist)
			{
				// persist user decision temporarily and to config file
				EnterCriticalSection(&m_csApiFwOp);
				m_vecExeWhitelist.push_back(wstrExeName);
				LeaveCriticalSection(&m_csApiFwOp);

				// RPC call to add to executable whitelist
				AddExeToWhitelist(const_cast<WCHAR *>(wstrExeName.c_str()));
			}

			bRet = TRUE;
			goto Cleanup;
		}

		m_pEventWriter->WriteEvent(
				EEventRepository::ApiFirewall,
				EThreatRiskRating::MediumRisk,
				L"Process Creation Attempt Blocked",
				L"An attempt was made to launch a program not on the application white-list. The program attempted for launch was %s (path: \"%s\" args: \"%s\")",
				wstrExeName.c_str(),
				wstrProcess.c_str(),
				wstrParameters.c_str()
			);

		goto Cleanup;
	}

	bRet = TRUE;
Cleanup:
	
	if(bRet)
	{
		m_pEventWriter->WriteEvent(
			EEventRepository::ApiFirewall,
			EThreatRiskRating::Information,
			L"Process Creation Attempt Permitted",
			L"An program was permitted to run: %s (path: \"%s\" args: \"%s\")",
			wstrExeName.c_str(),
			wstrProcess.c_str(),
			wstrParameters.c_str()
		);
	}

	return bRet;
}

BOOL CApiFirewall::QueryModuleLoadPathAllowed(std::wstring wstrModulePath)
{
	BOOL bRet = FALSE;
	
	if(!CheckPathNotLongName(wstrModulePath))
	{
		m_pEventWriter->WriteEvent(
				EEventRepository::ApiFirewall,
				EThreatRiskRating::MediumRisk,
				L"Module Load Attempt Blocked",
				L"An attempt was made to load a code module with a suspicious path. The module path being loaded was \"%s\"",
				wstrModulePath.c_str()
			);

		goto Cleanup;
	}

	if(!CheckPathNotUNC(wstrModulePath))
	{
		m_pEventWriter->WriteEvent(
				EEventRepository::ApiFirewall,
				EThreatRiskRating::MediumRisk,
				L"Module Load Attempt Blocked",
				L"An attempt was made to load a code module from a remote share. The module path being loaded was \"%s\"",
				wstrModulePath.c_str()
			);

		goto Cleanup;
	}

	if(!CheckPathNotWinTemp(wstrModulePath))
	{
		m_pEventWriter->WriteEvent(
				EEventRepository::ApiFirewall,
				EThreatRiskRating::MediumRisk,
				L"Module Load Attempt Blocked",
				L"An attempt was made to load a code module from a local temporary folder. The module path being loaded was \"%s\"",
				wstrModulePath.c_str()
			);

		goto Cleanup;
	}

	if(!CheckPathNotTempInetFiles(wstrModulePath))
	{
		m_pEventWriter->WriteEvent(
				EEventRepository::ApiFirewall,
				EThreatRiskRating::MediumRisk,
				L"Module Load Attempt Blocked",
				L"An attempt was made to load a code module from the Temporary Internet Folder. The module path being loaded was \"%s\"",
				wstrModulePath.c_str()
			);

		goto Cleanup;
	}

	if(!CheckPathNotDownloads(wstrModulePath))
	{
		m_pEventWriter->WriteEvent(
				EEventRepository::ApiFirewall,
				EThreatRiskRating::MediumRisk,
				L"Module Load Attempt Blocked",
				L"An attempt was made to load a code module from a Downloads folder. The module path being loaded was \"%s\"",
				wstrModulePath.c_str()
			);

		goto Cleanup;
	}

	if(!CheckPathNotEnvVar(wstrModulePath))
	{
		m_pEventWriter->WriteEvent(
				EEventRepository::ApiFirewall,
				EThreatRiskRating::MediumRisk,
				L"Module Load Attempt Blocked",
				L"An attempt was made to load a code module from a path containing an unexpanded environment variable. The module path being loaded was \"%s\"",
				wstrModulePath.c_str()
			);

		goto Cleanup;
	}

	bRet = TRUE;
Cleanup:
	return bRet;
}

BOOL CApiFirewall::CheckPathNotUNC(std::wstring wstrPath)
{
	if(!m_bEnabled || m_bAllowCodeFromNetwork)
		return TRUE;

	if(!wcsncmp(wstrPath.c_str(), L"\\\\?\\", 4) && wstrPath.c_str()[5] == ':')
	{
		// only permit this form because it may be possible to craft a UNC path of form:
		//  "\\?\" + "\\remotehost\share"

		return TRUE;
	}

	if(!wcsncmp(wstrPath.c_str(), L"\\\\", 2))
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CApiFirewall::CheckPathNotDownloads(std::wstring wstrPath)
{
	if(!m_bEnabled || m_bAllowCodeFromDownloads)
		return TRUE;

	WCHAR *ppwszDownloadPartialFolderNames[4] = {
		L"\\Downloads\\",
		L"\\Downloads/",
		L"/Downloads\\",
		L"/Downloads/"
	};

	for(size_t i=0; i < (sizeof(ppwszDownloadPartialFolderNames) / sizeof(WCHAR *)); i++)
	{
		if(Utils::wcsistr(const_cast<WCHAR *>(wstrPath.c_str()), ppwszDownloadPartialFolderNames[i]) != NULL)
		{
			return FALSE;
		}
	}

	return TRUE;
}

BOOL CApiFirewall::CheckPathNotEnvVar(std::wstring wstrPath)
{
	if(!m_bEnabled)
		return TRUE;

	DWORD dwNumPercentChars = 0;

	for(size_t i=0; i < wstrPath.size(); i++)
	{
		if(wstrPath[i] == '%')
		{
			dwNumPercentChars++;
		}
	}

	if(dwNumPercentChars >= 2)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CApiFirewall::CheckPathNotWinTemp(std::wstring wstrPath)
{
	if(!m_bEnabled || m_bAllowCodeFromTemp)
		return TRUE;

	// depending on what this check is meant to assert, may be necessary
	// to normalize the path

	if(!wcsncmp(wstrPath.c_str(), m_pwszTmpEnvPath, wcslen(m_pwszTmpEnvPath)))
	{
		return FALSE;
	}

	if(!wcsncmp(wstrPath.c_str(), m_pwszTempEnvPath, wcslen(m_pwszTempEnvPath)))
	{
		return FALSE;
	}

	if(!wcsncmp(wstrPath.c_str(), m_pwszUserprofileTmp, wcslen(m_pwszUserprofileTmp)))
	{
		return FALSE;
	}
	
	if(!wcsncmp(wstrPath.c_str(), m_pwszWindowDirTmp, wcslen(m_pwszWindowDirTmp)))
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CApiFirewall::CheckPathNotTempInetFiles(std::wstring wstrPath)
{
	BOOL bRet = FALSE;
	WCHAR *pwszTempIFPath = NULL;
	
	if(!m_bEnabled || m_bAllowCodeFromInetTemp)
		return TRUE;

	pwszTempIFPath = Utils::GetRegistryString(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders", L"Cache");
	if(!pwszTempIFPath)
		goto Cleanup;
	
	if(!wcsncmp(wstrPath.c_str(), pwszTempIFPath, wcslen(pwszTempIFPath)))
		goto Cleanup;

	bRet = TRUE;
Cleanup:
	if(pwszTempIFPath)
		free(pwszTempIFPath);
	
	return bRet;
}

BOOL CApiFirewall::CheckPathNotLongName(std::wstring wstrPath)
{
	if(!m_bEnabled || m_bAllowCodeLongPath)
		return TRUE;

	if(wcslen(wstrPath.c_str()) >= MAX_PATH)
		return FALSE;

	return TRUE;
}

BOOL CApiFirewall::CheckPathWhitelist(std::wstring wstrExeName)
{
	BOOL bRet = FALSE;
	std::wstring wstrExe;

	if(!m_bEnabled || !m_bCheckWhitelist)
		return TRUE;

	EnterCriticalSection(&m_csApiFwOp);

	wstrExe = wstrExeName;

	for(size_t i=0; i<wstrExe.length(); i++)
	{
		wstrExe[i] = tolower(wstrExe[i]);
	}

	std::vector<std::wstring>::iterator it = std::find(m_vecExeWhitelist.begin(), m_vecExeWhitelist.end(), wstrExe);
	if(it == m_vecExeWhitelist.end())
	{
		goto Cleanup;
	}

	bRet = TRUE;
Cleanup:
	LeaveCriticalSection(&m_csApiFwOp);

	return bRet;
}

BOOL CApiFirewall::QueryUserRunExe(std::wstring wstrPath)
{
	BOOL bDesc = FALSE;
	std::wstring wstrDesc;
	
	bDesc = Utils::GetDescriptionStringFile(wstrPath, wstrDesc) && wstrDesc.size() != 0;
	
	std::wstring wstrMsg = L"An attempt is being made to start the program ";

	if(bDesc)
	{
		wstrMsg += wstrDesc;
		wstrMsg += L" (\"";
		wstrMsg += wstrPath;
		wstrMsg += L"\")";
	}
	else
	{
		wstrMsg += L"\"";
		wstrMsg += wstrPath;
		wstrMsg += L"\"";
	}

	wstrMsg += L" - do you want to allow this?\r\n\r\n";
	wstrMsg += L"Please do not choose 'Yes' unless you trust the program to be launched!";

	BOOL bUserChoice = GetUiFeedback()->UserBoolFeedback(
			UiStyle_IconQuestion,
			L"Allow Program to Execute?",
			const_cast<WCHAR *>(wstrMsg.c_str())
		);

	return bUserChoice;
}

std::list<std::wstring> *CApiFirewall::ExtractUNCNetworkPaths(std::wstring wstrParameters)
{
	std::list<std::wstring> *plistPaths = new std::list<std::wstring>();
	if(!plistPaths || wstrParameters.size() <= 6) // \\a\f. (smallest possible without being risk)
		goto Cleanup;
	
	// foo.exe -o foo -f \\server\folder\foo.txt
	// foo.exe -o foo -f \\?\UNC\server\folder\foo.txt
	// foo.exe -o "foo bar" -f "\\server\folder name\foo doc.txt"
	// foo.exe -o "foo bar" -f "\\?\UNC\server\folder name\foo doc.txt"

	BOOL bInQuote = FALSE, bMoveNext = FALSE;
	DWORD i = 0;

	while(i < wstrParameters.size())
	{
		if(bMoveNext)
		{
			if(bInQuote)
			{
				if(wstrParameters[i] == '\"')
				{
					bInQuote = FALSE;
					bMoveNext = FALSE;
				}
			}
			else
			{
				if(wstrParameters[i] == ' ' || wstrParameters[i] == '\t')
				{
					bMoveNext = FALSE;
				}
			}

			i++;
			continue;
		}

		if(wstrParameters[i] == ' ' || wstrParameters[i] == '\t')
		{
			i++;
			continue;
		}
		
		if(wstrParameters[i] == '\"')
		{
			bInQuote = TRUE;
			i++;
			continue;
		}

		// at start of a parameter string, check UNC path

		if(wstrParameters[i] == '\\')
		{
			// perhaps UNC
			i++;
			
			if(i + 6 >= wstrParameters.size())
			{
				bMoveNext = TRUE;
				continue;
			}

			if(wstrParameters[i] == '\\' && wstrParameters[i + 1] == '?' && wstrParameters[i + 2] == '\\' &&
				toupper(wstrParameters[i + 3]) == 'U' &&
				toupper(wstrParameters[i + 4]) == 'N' &&
				toupper(wstrParameters[i + 5]) == 'C')
			{
				// skip UNC prefix
				i += 6;
			}
		}
		else
		{
			// can't be UNC path
			bMoveNext = TRUE;
			continue;
		}

		if(i + 1 >= wstrParameters.size())
		{
			bMoveNext = TRUE;
			continue;
		}

		if(wstrParameters[i] == '\\')
		{
			// definitely UNC path:
			//  \\server\share
			//  \\?\UNC\server\share
			std::wstring wstrPath = L"\\\\";
			BOOL bValid = FALSE;

			i++;

			while(i < wstrParameters.size())
			{
				if((bInQuote && wstrParameters[i] == '\"') ||
					(!bInQuote && (wstrParameters[i] == ' ' || wstrParameters[i] == '\t')))
				{
					break;
				}

				if(wstrParameters[i] == '\\')
				{
					bValid = TRUE;
				}

				wstrPath += wstrParameters[i];
				i++;
			}

			if(bValid)
				plistPaths->push_back(wstrPath);
		}
		
		// advance whatever the outcome when we reach this stage
		bMoveNext = TRUE;
	}

Cleanup:
	if(plistPaths != NULL && plistPaths->empty())
	{
		delete plistPaths;
		plistPaths = NULL;
	}

	return plistPaths;
}

BOOL CApiFirewall::RemoteFolderContainsDll(std::wstring wstrDir)
{
	BOOL bContainsDll = FALSE;
	WIN32_FIND_DATA w32fd = {0};

	wstrDir += L"*";

	HANDLE hFindFile = FindFirstFile(wstrDir.c_str(), &w32fd);
	if(hFindFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			DWORD dwFilenameLen = wcslen(w32fd.cFileName);

			if(dwFilenameLen > 4)
			{
				dwFilenameLen -= 4;

				if(wcsicmp(w32fd.cFileName + dwFilenameLen, L".dll") == 0)
				{
					// contains dll
					bContainsDll = TRUE;
					break;
				}
			}
		}
		while(FindNextFile(hFindFile, &w32fd) != FALSE);

		FindClose(hFindFile);
	}

Cleanup:
	return bContainsDll;
}

BOOL CApiFirewall::TryOpenDllFileDirectory(std::wstring wstrDir)
{
	wstrDir += L"\\msvcrt.dll";
	
	HANDLE hFile = CreateFile(
			wstrDir.c_str(),
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL
		);
	
	if(hFile == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	CloseHandle(hFile);

	return TRUE;
}

BOOL CApiFirewall::CheckParamDirectoryDlls(std::wstring wstrParameters)
{
	BOOL bRet = FALSE;
	
	if(!m_bEnabled || !m_bCheckDirDllPlant)
		goto Cleanup;

	std::list<std::wstring> *plistPaths = ExtractUNCNetworkPaths(wstrParameters);
	if(!plistPaths)
		goto Cleanup;

	while(!plistPaths->empty())
	{
		std::wstring wstrPath = plistPaths->front();
		DWORD i = wstrPath.size() - 1;

		while(wstrPath[i] != '\\')
			i--;
		
		wstrPath = wstrPath.substr(0, i + 1); // chop off filename

		if(RemoteFolderContainsDll(wstrPath) || TryOpenDllFileDirectory(wstrPath))
		{
			bRet = TRUE;
			break;
		}

		plistPaths->pop_front();
	}

	delete plistPaths;

Cleanup:
	return bRet;
}

void CApiFirewall::ConfigRefresh(BOOL bRefresh)
{
	CApiFirewall *pApiFw = CApiFirewall::GetInstance();
	IConfigReader *pConfig = GetConfigReader();

	WCHAR *pwszTmp, *pwszToken;

	LPWSTR rgwszConfigName[12] = {
		L"enabled",
		L"allowCodeFromTemp",
		L"allowCodeFromNetwork",
		L"allowCodeLongPath",
		L"allowCodeFromInetTemp",
		L"allowCodeFromDownloads",
		L"blockDllPlanting",
		L"promptUserExeBlocked",
		L"checkWhitelist",
		L"allowSpawnProtectedProcs",
		L"autoAddToWhitelist",
		L"executableWhitelist",
	};

	PBOOL rgpbool[11] = {
		&pApiFw->m_bEnabled,
		&pApiFw->m_bAllowCodeFromTemp,
		&pApiFw->m_bAllowCodeFromNetwork,
		&pApiFw->m_bAllowCodeLongPath,
		&pApiFw->m_bAllowCodeFromInetTemp,
		&pApiFw->m_bAllowCodeFromDownloads,
		&pApiFw->m_bCheckDirDllPlant,
		&pApiFw->m_bQueryUserOnExeBlocked,
		&pApiFw->m_bCheckWhitelist,
		&pApiFw->m_bAllowSpawnProtProcs,
		&pApiFw->m_bAutoAddToWhitelist
	};
	
	std::wstring wstrExe;

	EnterCriticalSection(&pApiFw->m_csApiFwOp);

	for(int i=0; i<12; i++)
	{
		pwszTmp = pConfig->GetConfigElement(L"apiMonitor", rgwszConfigName[i]);
		if(!pwszTmp)
			continue;

		if(i < sizeof(rgpbool)/sizeof(BOOL))
		{
			if(!wcsicmp(pwszTmp, L"true"))
			{
				*(rgpbool[i]) = TRUE;
			}
			else
			{
				*(rgpbool[i]) = FALSE;
			}
		}
		else
		{
			pApiFw->m_vecExeWhitelist.clear();
			pwszToken = wcstok(pwszTmp, L"|");
			
			while(pwszToken)
			{
				wstrExe = pwszToken;
				
				for(size_t i=0; i<wstrExe.length(); i++)
				{
					wstrExe[i] = tolower(wstrExe[i]);
				}

				pApiFw->m_vecExeWhitelist.push_back(wstrExe);
				pwszToken = wcstok(NULL, L"|");
			}
		}
	}
	
	// this is for the CreateProcessInternal integrity check
	pwszTmp = pConfig->GetConfigElement(L"protectedProcessList", L"processList");
	if(pwszTmp)
	{
		pApiFw->m_pwszProtProc = wcsdup(pwszTmp);
	}

	LeaveCriticalSection(&pApiFw->m_csApiFwOp);
}