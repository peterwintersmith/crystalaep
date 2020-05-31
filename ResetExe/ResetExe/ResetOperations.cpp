// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#include "stdafx.h"
#include "MinUtils.h"
#include "ResetOperations.h"

void ResetOperations::AbortWithMsgError(std::wstring strError, UINT uiExitCode)
{
	MessageBoxW(NULL, strError.c_str(), L"Fatal Error", MB_OK | MB_ICONERROR);
	TerminateProcess(GetCurrentProcess(), uiExitCode);
}

void ResetOperations::AddDirectoriesRecursive(std::list<std::wstring> *plist, std::wstring wstrFolder, BOOL bIncludeDirs)
{
	WIN32_FIND_DATA w32fd = {0};

	DWORD cch = wstrFolder.size();
	
	if(wstrFolder[cch - 1] != '\\')
	{
		wstrFolder += L"\\";
	}

	HANDLE hFindFile = FindFirstFile((wstrFolder + L"*").c_str(), &w32fd);
	if(hFindFile == INVALID_HANDLE_VALUE)
		goto Cleanup;

	do
	{
		if(!wcscmp(w32fd.cFileName, L".") || !wcscmp(w32fd.cFileName, L".."))
		{
			// skip the current/parent directory references
			continue;
		}

		if(w32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			// dir
			AddDirectoriesRecursive(plist, wstrFolder + w32fd.cFileName, bIncludeDirs);
			if(bIncludeDirs)
				plist->push_back((wstrFolder + w32fd.cFileName) + L"\\");
		}
		else
		{
			// file
			plist->push_back(wstrFolder + w32fd.cFileName);
		}
	}
	while(FindNextFile(hFindFile, &w32fd) != FALSE);

Cleanup:
	FindClose(hFindFile);
}

std::list<std::wstring> *ResetOperations::GetFilesInFolder(std::wstring wstrFolder)
{
	std::list<std::wstring> *plist = new std::list<std::wstring>();
	if(!plist)
		goto Cleanup_NullPtr;

	if(wstrFolder.empty())
		goto Cleanup;

	AddDirectoriesRecursive(plist, wstrFolder, TRUE);
	
Cleanup:
	if(plist->empty())
	{
		// NULL on failure
		delete plist;
		plist = NULL;
	}

Cleanup_NullPtr:
	return plist;
}

BOOL ResetOperations::GetUserMsgYesNo(std::wstring wstrQuery)
{
	return MessageBoxW(NULL, wstrQuery.c_str(), L"Input Required", MB_YESNO | MB_ICONQUESTION) == IDYES;
}

void ResetOperations::RaiseNonFatalMsgError(std::wstring wstrError)
{
	MessageBoxW(NULL, wstrError.c_str(), L"Information", MB_OK | MB_ICONINFORMATION);
}

DWORD ResetOperations::GetProcessPid(std::wstring wstrProcess, BOOL bPartial)
{
	DWORD dwProcessId = 0;
	HANDLE hProcSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if(hProcSnap == INVALID_HANDLE_VALUE)
		goto Cleanup;

	PROCESSENTRY32 pe32 = {0};
	pe32.dwSize = sizeof(PROCESSENTRY32);

	BOOL bResult = Process32First(hProcSnap, &pe32);
	
	while(bResult)
	{
		WCHAR *pwszExeName = wcsrchr(pe32.szExeFile, '\\');
		if(!pwszExeName)
		{
			pwszExeName = wcsrchr(pe32.szExeFile, '/');
			if(!pwszExeName)
			{
				pwszExeName = pe32.szExeFile;
			}
			else
			{
				pwszExeName++;
			}
		}
		else
		{
			pwszExeName++;
		}

		if(!bPartial)
		{
			if(!wcsicmp(pe32.szExeFile, wstrProcess.c_str()))
			{
				dwProcessId = pe32.th32ProcessID;
				goto Cleanup;
			}
		}
		else
		{
			if(!wcsnicmp(pe32.szExeFile, wstrProcess.c_str(), wstrProcess.size()))
			{
				dwProcessId = pe32.th32ProcessID;
				goto Cleanup;
			}
		}

Continue:
		pe32.dwSize = sizeof(PROCESSENTRY32);
		bResult = Process32Next(hProcSnap, &pe32);
	}

Cleanup:
	if(hProcSnap)
		CloseHandle(hProcSnap);

	return dwProcessId;
}

BOOL ResetOperations::KillProcess(DWORD dwPid)
{
	BOOL bSuccess = FALSE;
	
	HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, dwPid);
	if(!hProcess)
		goto Cleanup;

	if(TerminateProcess(hProcess, 0) == FALSE)
		goto Cleanup;

	bSuccess = TRUE;
Cleanup:
	if(hProcess)
		CloseHandle(hProcess);

	return bSuccess;
}

void ResetOperations::Remove(BOOL bQuiet)
{
	std::wstring wstrInstPath = MinUtils::GetRegInstallPath();
	if(wstrInstPath.empty())
	{
		if(bQuiet)
			TerminateProcess(GetCurrentProcess(), Failure_RegInstallPath);

		AbortWithMsgError(L"Product install path not found in registry. The install path should reside in HKCU\\Software\\CrystalBP [name=InstallPath].", Failure_RegInstallPath);
	}

	std::list<std::wstring> *plistFiles = GetFilesInFolder(wstrInstPath);
	if(!plistFiles)
	{
		if(bQuiet)
			TerminateProcess(GetCurrentProcess(), Failure_FilesNotFound);

		AbortWithMsgError(L"No files found in Crystal install directory.", Failure_FilesNotFound);
	}

	WCHAR wszSystemDir[260];
	DWORD dwCch = GetSystemDirectory(wszSystemDir, 260 - sizeof(L"\\CryModLdr.dll"));
	if(dwCch)
	{
		wcscat(wszSystemDir, L"\\CryModLdr.dll");
		plistFiles->push_back(wszSystemDir);
	}

	BOOL bCommence = bQuiet ? TRUE : GetUserMsgYesNo(L"To remove Crystal all processes on the protected process list will need to be closed. Please close all processes and click 'Yes', or choose 'No' to abort.");
	if(!bCommence)
	{
		delete plistFiles;
		
		if(bQuiet)
			TerminateProcess(GetCurrentProcess(), Failure_UserAbortBeforeRemove);

		AbortWithMsgError(L"The uninstall will not go ahead. If you wish to uninstall Crystal please re-run the Remove/Uninstall feature.", Failure_UserAbortBeforeRemove);
	}

	DWORD dwPid = GetProcessPid(L"ProcTracker.exe");
	KillProcess(dwPid);

	dwPid = GetProcessPid(L"CrystalAEPUI.exe");
	KillProcess(dwPid);

	BOOL bRemovedAll = TRUE;

	if(!bQuiet)
		printf("Beginning Uninstall ...\n");

	WCHAR *pwszAppInitDlls = MinUtils::GetRegistryString(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Windows", L"AppInit_DLLs");
	if(pwszAppInitDlls)
	{
		std::wstring wstrAppInitDlls, wstrComp = L"CryModLdr.dll";

		BOOL bSkipIfNextSpace = FALSE;

		for(DWORD i=0; pwszAppInitDlls[i] != '\0'; i++)
		{
			if(bSkipIfNextSpace && pwszAppInitDlls[i] == ' ')
			{
				bSkipIfNextSpace = FALSE;
				continue;
			}

			bSkipIfNextSpace = FALSE;

			if(!wcsnicmp(pwszAppInitDlls + i, wstrComp.c_str(), wstrComp.size()))
			{
				i += wstrComp.size();
				bSkipIfNextSpace = TRUE;
			}
			else
			{
				wstrAppInitDlls += pwszAppInitDlls[i];
			}
		}

		dwCch = wstrAppInitDlls.size();
		
		if(dwCch != 0 && wstrAppInitDlls[dwCch - 1] == ' ')
		{
			wstrAppInitDlls.erase(dwCch - 1, 1);
		}

		if(MinUtils::SetRegistryString(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Windows",
			L"AppInit_DLLs", (WCHAR *)wstrAppInitDlls.c_str()) == FALSE)
		{
			if(!bQuiet)
				RaiseNonFatalMsgError(L"There was an error removing Crystal from the AppInit_DLLs registry key. This should be performed manually. The uninstall should still succeed.");
		}
		
		free(pwszAppInitDlls);
		pwszAppInitDlls = NULL;
	}

	MinUtils::DeleteRegKeyValue(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", L"CrystalAEPProcTracker");
	MinUtils::DeleteRegKeyValue(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", L"CrystalAEPUI");

	while(!plistFiles->empty())
	{
		std::wstring wstrFile = plistFiles->front();
		
		if(MinUtils::wcsistr((WCHAR *)wstrFile.c_str(), L"uninstall", -1) != NULL ||
			MinUtils::wcsistr((WCHAR *)wstrFile.c_str(), L"update", -1) != NULL)
		{			
			if(!bQuiet)
				printf("Skipping: %S\n", wstrFile.c_str());

			plistFiles->pop_front();

			continue;
		}

		if(!bQuiet)
			printf("Removing: %S", wstrFile.c_str());

		dwCch = wstrFile.size();
		
		if(wstrFile[dwCch - 1] == '\\')
		{
			if(RemoveDirectory(wstrFile.c_str()) == FALSE)
			{
				if(!bQuiet)
					printf(" - Failed. Directory is in use.\n");
				
				MoveFileEx(wstrFile.c_str(), NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
			}
			else
			{
				if(!bQuiet)
					printf(" - Success.\n");
			}
		}
		else
		{
			if(DeleteFile(wstrFile.c_str()) == FALSE && GetLastError() != ERROR_FILE_NOT_FOUND)
			{
				if(!bQuiet)
					printf(" - Failed. File is in use.\n");
				
				MoveFileEx(wstrFile.c_str(), NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
				bRemovedAll = FALSE;
			}
			else
			{
				if(!bQuiet)
					printf(" - Success.\n");
			}
		}

		plistFiles->pop_front();
	}

	if(!bQuiet)
		printf("Complete.\n");

	if(!bRemovedAll)
	{
		if(!bQuiet)
			RaiseNonFatalMsgError(L"The uninstall process was not able to remove all files. This probably indicates that some processes are being protected.\r\n"
				L"Please reboot now to finish the uninstall process.");
	}

	delete plistFiles;

	MinUtils::FileCreate(MinUtils::GetTempPath() + L"\\reboot.lock");
	MoveFileEx((MinUtils::GetTempPath() + L"\\reboot.lock").c_str(), NULL, MOVEFILE_DELAY_UNTIL_REBOOT);

	if(!bQuiet)
		Sleep(5000);
}

void ResetOperations::SetupReg(WCHAR *pwszInstallPath, BOOL bQuiet)
{
	MinUtils::DeleteRegKeyValue(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", L"CrystalUpdate");

	if(MinUtils::CreateRegKey(HKEY_CURRENT_USER, L"Software\\CrystalBP") == FALSE)
	{
		if(!bQuiet)
			AbortWithMsgError(L"Unable to create or open CrystalBP registry key.", Failure_CreateCrystalBPRegKey);

		TerminateProcess(GetCurrentProcess(), Failure_CreateCrystalBPRegKey);
	}

	if(MinUtils::SetRegistryString(HKEY_CURRENT_USER, L"Software\\CrystalBP", L"InstallPath", pwszInstallPath) == FALSE)
	{
		if(!bQuiet)
			AbortWithMsgError(L"Unable to create or open CrystalBP\\InstallPath registry key.", Failure_CreateInstallPathRegKey);

		TerminateProcess(GetCurrentProcess(), Failure_CreateInstallPathRegKey);
	}

	WCHAR *pwszAppInit = MinUtils::GetRegistryString(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Windows", L"AppInit_DLLs");
	if(!pwszAppInit)
	{
		if(!MinUtils::SetRegistryString(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Windows", L"AppInit_DLLs", L""))
		{
			if(!bQuiet)
				AbortWithMsgError(L"Unable to query AppInit_DLLs registry key.", Failure_QueryAppInitKey);

			TerminateProcess(GetCurrentProcess(), Failure_QueryAppInitKey);
		}

		pwszAppInit = wcsdup(L"");
	}

	WCHAR *pwszCryModLdr = MinUtils::wcsistr(pwszAppInit, L"crymodldr.dll", -1);
	if(!pwszCryModLdr)
	{
		std::wstring wstrAppInit = pwszAppInit;
		wstrAppInit += L" crymodldr.dll";

		if(!MinUtils::SetRegistryString(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Windows", L"AppInit_DLLs", (WCHAR *)wstrAppInit.c_str()))
		{
			if(!bQuiet)
				AbortWithMsgError(L"Unable to set the AppInit_DLLs registry key.", Failure_SetAppInitKey);

			TerminateProcess(GetCurrentProcess(), Failure_SetAppInitKey);
		}

		free(pwszAppInit);
	}

	if(!MinUtils::SetRegistryDword(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Windows", L"LoadAppInit_DLLs", 1))
	{
		if(!bQuiet)
			AbortWithMsgError(L"Unable to set the LoadAppInit_DLLs registry key.", Failure_SetLoadAppInitKey);

		TerminateProcess(GetCurrentProcess(), Failure_SetLoadAppInitKey);
	}

	std::wstring wstrProcTracker = pwszInstallPath;
	wstrProcTracker += L"\\ProcTracker.exe";

	if(!MinUtils::SetRegistryString(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", L"CrystalAEPProcTracker", (WCHAR *)wstrProcTracker.c_str()))
	{
		if(!bQuiet)
			AbortWithMsgError(L"Unable to set ProcTracker.exe in the Run registry key.", Failure_SetProcTrackerRunKey);

		TerminateProcess(GetCurrentProcess(), Failure_SetProcTrackerRunKey);
	}

	std::wstring wstrUi = pwszInstallPath;
	wstrUi += L"\\CrystalAEPUI.exe";

	if(!MinUtils::SetRegistryString(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", L"CrystalAEPUI", (WCHAR *)wstrUi.c_str()))
	{
		if(!bQuiet)
			AbortWithMsgError(L"Unable to set CrystalAEPUI.exe in the Run registry key.", Failure_SetCrystalUIRunKey);

		TerminateProcess(GetCurrentProcess(), Failure_SetCrystalUIRunKey);
	}

	WCHAR *pwsz = MinUtils::GetRegistryString(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", L"CrystalUpdate");
	if(pwsz)
	{
		MinUtils::DeleteRegKeyValue(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", L"CrystalUpdate");
		free(pwsz);
	}
}

void ResetOperations::CopyLdr(WCHAR *pwszInstallPath, BOOL bQuiet)
{
	std::wstring wstrLdrPath = pwszInstallPath;
	wstrLdrPath += L"\\CryModLdr.dll";

	WCHAR wszSystemDir[260];
	DWORD dwCch = GetSystemDirectory(wszSystemDir, 260 - sizeof(L"\\CryModLdr.dll"));
	if(dwCch)
	{
		wcscat(wszSystemDir, L"\\CryModLdr.dll");
	}

	if(!CopyFile(wstrLdrPath.c_str(), wszSystemDir, FALSE))
	{
		if(!bQuiet)
			AbortWithMsgError(L"Unable to copy Crystal Loader to System32 directory.", Failure_CopyCryModLdrSystem);

		TerminateProcess(GetCurrentProcess(), Failure_CopyCryModLdrSystem);
	}
}

void ResetOperations::CheckDotNet(BOOL bQuiet)
{
	BOOL bHasDotNet = FALSE;

	std::list<std::wstring> *plist = MinUtils::EnumRegSubKeys(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\NET Framework Setup\\NDP");
	if(plist)
	{
		while(!plist->empty())
		{
			if(!wcsnicmp(plist->front().c_str(), L"v2.", 3))
			{
				// has .NET 2.0
				bHasDotNet = TRUE;
				break;
			}

			plist->pop_front();
		}

		delete plist;
	}

	if(!bHasDotNet)
	{
RestartInstallCheck:
		ShellExecute(NULL, L"open", L"http://www.microsoft.com/download/en/details.aspx?id=19", NULL, NULL, SW_SHOWNORMAL);
		
		if(GetUserMsgYesNo(L"The Microsoft .NET framework runtime version 2.0 does not appear to be installed. Crystal requires the .NET framework to run.\r\n"
			L"Please wait for the browser to open on the download page for .NET 2.0 now, and download and install the .NET framework.\r\n\r\n"
			L"Please click 'Yes' when .NET has finished installing. If you click 'No' Crystal will fail to run until .NET is installed."))
		{
			std::list<std::wstring> *plist = MinUtils::EnumRegSubKeys(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\NET Framework Setup\\NDP");
			if(plist)
			{
				while(!plist->empty())
				{
					if(!wcsnicmp(plist->front().c_str(), L"v2.", 3))
					{
						// has .NET 2.0
						bHasDotNet = TRUE;
						break;
					}

					plist->pop_front();
				}

				delete plist;
			}

			if(bHasDotNet)
			{
				RaiseNonFatalMsgError(L"The .NET framework was installed with no problem.");
			}
			else
			{
				if(GetUserMsgYesNo(L"The .NET framework was not installed, please select 'Yes' to try again, or 'No' if you wish to install at a later date."))
				{
					goto RestartInstallCheck;
				}
				else
				{
					if(!TerminateInstall())
						RaiseNonFatalMsgError(L"Installation could not be terminated. Please terminate manually and re-install as an administrative user.");
				}
			}
		}
		else
		{
			if(!TerminateInstall())
				RaiseNonFatalMsgError(L"Installation could not be terminated. Please terminate manually and re-install as an administrative user.");
		}
	}
}

void ResetOperations::SchedUpdate(WCHAR *pwszInstallPath, BOOL bQuiet)
{
	std::wstring wstrUpdate = pwszInstallPath;
	wstrUpdate += L"\\update.exe";

	if(!MinUtils::SetRegistryString(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", L"CrystalUpdate", (WCHAR *)wstrUpdate.c_str()))
	{
		if(!bQuiet)
			AbortWithMsgError(L"Unable to set update.exe in the Run registry key.", Failure_SetUpdateRunKey);

		TerminateProcess(GetCurrentProcess(), Failure_SetUpdateRunKey);
	}
}

BOOL ResetOperations::TerminateInstall()
{
	HWND hWnd = FindWindow(NULL, L"Installing Crystal Anti-Exploit Protection 2012");
	if(hWnd == NULL)
	{
		hWnd = FindWindow(NULL, L"Installing Crystal Anti-Exploit Protection 2012 (Beta 0.1)");
		if(hWnd == NULL)
		{
			hWnd = FindWindow(NULL, L"WinRAR self-extracting archive");
		}
	}
	
	if(hWnd != NULL)
	{
		SendMessage(hWnd, WM_QUIT, 1, 0);
	}

	DWORD dwPid = GetProcessPid(L"crystal_aep_installer", TRUE);
	if(dwPid)
	{
		if(!KillProcess(dwPid))
			return FALSE;
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

void ResetOperations::CheckAdminTermInstall()
{
	if(MinUtils::IsUserAdminRegMethod() == FALSE)
	{
		RaiseNonFatalMsgError(L"Unfortunately Crystal needs to be installed and run under an administrative account (but should work fine with Windows UAC). The installation will now be terminated.");

		if(!TerminateInstall())
			RaiseNonFatalMsgError(L"Installation could not be terminated. Please terminate manually and re-install as an administrative user.");
	}
}

BOOL ResetOperations::IsPendingReboot()
{
	std::wstring wstrPath = MinUtils::GetTempPath() + L"\\reboot.lock";
	
	if(MinUtils::FileExists(wstrPath))
	{
		return TRUE;
	}

	return FALSE;
}

void ResetOperations::CheckPendingRebootTermInstall()
{
	if(IsPendingReboot())
	{
		RaiseNonFatalMsgError(L"Crystal is pending a reboot, please restart your computer to proceed with installation. The installation will now be terminated.");

		//ResetOperations::Remove(TRUE);
		
		// ensure that this doesn't happen over and over
		MoveFileEx((MinUtils::GetTempPath() + L"\\reboot.lock").c_str(), NULL, MOVEFILE_DELAY_UNTIL_REBOOT);

		if(!TerminateInstall())
			RaiseNonFatalMsgError(L"Installation could not be terminated. Please terminate manually and re-install after restarting your computer.");
	}
}

void ResetOperations::SetAppInit(WCHAR *pwszKeyValue)
{
	//RaiseNonFatalMsgError(pwszKeyValue);

	if(!MinUtils::SetRegistryString(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Windows", L"AppInit_DLLs", pwszKeyValue))
	{
		TerminateProcess(GetCurrentProcess(), Failure_SetAppInitKey);
	}
}

void ResetOperations::EnableAppInit()
{	
	if(!MinUtils::SetRegistryDword(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Windows", L"LoadAppInit_DLLs", 1))
	{
		TerminateProcess(GetCurrentProcess(), Failure_SetLoadAppInitKey);
	}
}

void ResetOperations::DisableAppInit()
{
	if(!MinUtils::SetRegistryDword(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Windows", L"LoadAppInit_DLLs", 0))
	{
		TerminateProcess(GetCurrentProcess(), Failure_SetLoadAppInitKey);
	}
}

void ResetOperations::CheckAlreadyInstalled()
{
	std::wstring wstrInstPath = MinUtils::GetRegInstallPath();
	if(wstrInstPath.empty())
		return;

	if(!MinUtils::FileExists(wstrInstPath + L"\\auxcore.dll"))
		return;

	if(MinUtils::FileExists(wstrInstPath + L"\\uninstall.exe"))
	{
		RaiseNonFatalMsgError(L"Crystal appears to already be installed, please uninstall the version you already have before continuing with this installation.");
		
		MinUtils::FileCreate(MinUtils::GetTempPath() + L"\\reboot.lock");
		MoveFileEx((MinUtils::GetTempPath() + L"\\reboot.lock").c_str(), NULL, MOVEFILE_DELAY_UNTIL_REBOOT);

		std::wstring wstrUninstaller = L"\"" + wstrInstPath + L"\\uninstall.exe\"";
		
		char szUninstaller[260] = {0};
		wcstombs(szUninstaller, wstrUninstaller.c_str(), 259);
		szUninstaller[259] = '\0';

		system(szUninstaller);
	}
}