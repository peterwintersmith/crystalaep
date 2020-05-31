// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#pragma once

namespace ResetOperations
{
	enum InstallFailureCodes
	{
		Failure_Base = 0x30000000,
		Failure_RegInstallPath,
		Failure_FilesNotFound,
		Failure_UserAbortBeforeRemove,
		Failure_CreateCrystalBPRegKey,
		Failure_CreateInstallPathRegKey,
		Failure_QueryAppInitKey,
		Failure_SetAppInitKey,
		Failure_SetLoadAppInitKey,
		Failure_SetProcTrackerRunKey,
		Failure_CopyCryModLdrSystem,
		Failure_CheckDotNetVersion,
		Failure_SetUpdateRunKey,
		Failure_SetCrystalUIRunKey
	};

	// exports
	void Remove(BOOL bQuiet = FALSE);
	void SetupReg(WCHAR *pwszInstallPath, BOOL bQuiet = FALSE);
	void CopyLdr(WCHAR *pwszInstallPath, BOOL bQuiet = FALSE);
	void CheckDotNet(BOOL bQuiet = FALSE);
	void SchedUpdate(WCHAR *pwszInstallPath, BOOL bQuiet = FALSE);
	void CheckAdminTermInstall();
	void CheckPendingRebootTermInstall();
	void SetAppInit(WCHAR *pwszKeyValue);
	void EnableAppInit();
	void DisableAppInit();
	void CheckAlreadyInstalled();

	// helper routines
	void AbortWithMsgError(std::wstring strError, UINT uiExitCode);
	std::list<std::wstring> *GetFilesInFolder(std::wstring wstrFolder);
	void AddDirectoriesRecursive(std::list<std::wstring> *plist, std::wstring wstrFolder, BOOL bIncludeDirs);
	BOOL GetUserMsgYesNo(std::wstring wstrQuery);
	void RaiseNonFatalMsgError(std::wstring wstrError);
	DWORD GetProcessPid(std::wstring wstrProcess, BOOL bPartial = FALSE);
	BOOL KillProcess(DWORD dwPid);
	BOOL TerminateInstall();
	BOOL IsPendingReboot();
}