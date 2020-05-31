// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]
// ResetExe.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ResetOperations.h"
#include "MinUtils.h"

int _tmain(int argc, _TCHAR* argv[])
{
	//WCHAR *ppwsz[3] = {L"foobar.exe", L"-setup-regkeys", L"c:\\work\\temp"};
	//WCHAR *ppwsz[3] = {L"foobar.exe", L"-check-dotnet", L"c:\\work\\temp"};
	//WCHAR *ppwsz[3] = {L"foobar.exe", L"-sched-update", L"c:\\work\\temp"};
	//argc = 3;
	//argv = ppwsz;

	User32::InitUser32();

	if(argc < 2)
	{
		printf("Crystal AEP Removal Utility\n");
		return 1;
	}

	if(!wcsicmp(argv[1], L"-remove"))
	{
		ResetOperations::Remove();
	}
	else if(!wcsicmp(argv[1], L"-remove-quiet"))
	{
		ResetOperations::Remove(TRUE);
	}
	else if(!wcsicmp(argv[1], L"-setup-regkeys"))
	{
		// set HKCU\Software\CrystalBP [name=InstallPath, value=argv[2]]
		// set HKLM\Software\Microsoft\Windows\CurrentVersion\Run [name=CrystalProcTracker, value=argv[2] + '\\ProcTracker.exe']
		// set HKLM\Software\Microsoft\Windows NT\CurrentVersion\Windows [name=AppInit_DLLs, value=<existing> + ' ' + crymodldr.dll
		// set HKLM\Software\Microsoft\Windows NT\CurrentVersion\Windows [name=LoadAppInit_DLLs, value=1]
		if(argc != 3)
		{
			printf("No install path supplied\n");
			return 1;
		}

		ResetOperations::SetupReg(argv[2]);
	}
	else if(!wcsicmp(argv[1], L"-copy-ldr"))
	{
		// copy crymodldr.dll to c:\windows\system32
		if(argc != 3)
		{
			printf("No install path supplied\n");
			return 1;
		}

		ResetOperations::CopyLdr(argv[2]);
	}
	else if(!wcsicmp(argv[1], L"-check-dotnet"))
	{
		// check for .net 2.0 and shell download URL if not installed
		// check: HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\NET Framework Setup\NDP\v2.<*>
		// shell: http://www.microsoft.com/download/en/details.aspx?id=19
		ResetOperations::CheckDotNet();
	}
	else if(!wcsicmp(argv[1], L"-sched-update"))
	{
		if(argc != 3)
		{
			printf("No install path supplied\n");
			return 1;
		}

		ResetOperations::SchedUpdate(argv[2]);
	}
	else if(!wcsicmp(argv[1], L"-check-admin"))
	{
		ResetOperations::CheckAdminTermInstall();
	}
	else if(!wcsicmp(argv[1], L"-check-reboot"))
	{
		ResetOperations::CheckPendingRebootTermInstall();
	}
	else if(!wcsicmp(argv[1], L"-set-appinit"))
	{
		if(argc != 3)
		{
			printf("No install path supplied\n");
			return 1;
		}

		ResetOperations::SetAppInit(argv[2]);
	}
	else if(!wcsicmp(argv[1], L"-enable-appinit"))
	{
		ResetOperations::EnableAppInit();
	}
	else if(!wcsicmp(argv[1], L"-disable-appinit"))
	{
		ResetOperations::DisableAppInit();
	}
	else if(!wcsicmp(argv[1], L"-check-already-installed"))
	{
		ResetOperations::CheckAlreadyInstalled();
	}

	return 0;
}