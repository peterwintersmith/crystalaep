// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#include "stdafx.h"
#include "MinUtils.h"

WCHAR *MinUtils::GetRegistryString(HKEY hKey, WCHAR *pwszRegPath, WCHAR *pwszKeyName)
{
	BOOL bErr = TRUE;
	HKEY hkResult = 0;
	WCHAR *pwszKeyValue = NULL;
	DWORD cbKeyValue = 0;

	if(RegOpenKeyEx(hKey, pwszRegPath, 0, KEY_QUERY_VALUE,
			&hkResult) != ERROR_SUCCESS)
		goto Cleanup;

	DWORD dwType = 0;

	if(RegQueryValueEx(hkResult, pwszKeyName, NULL, &dwType, NULL,
			&cbKeyValue) != ERROR_SUCCESS)
		goto Cleanup;

	if(dwType != REG_SZ)
		goto Cleanup;

	pwszKeyValue = (WCHAR *)malloc(cbKeyValue + sizeof(WCHAR));
	if(!pwszKeyValue)
		goto Cleanup;

	memset(pwszKeyValue, 0, cbKeyValue + sizeof(WCHAR));

	if(RegQueryValueEx(hkResult, pwszKeyName, NULL, &dwType, (PBYTE)pwszKeyValue,
			&cbKeyValue) != ERROR_SUCCESS)
		goto Cleanup;

	bErr = FALSE;
Cleanup:
	if(hkResult)
		RegCloseKey(hkResult);

	if(bErr && pwszKeyValue)
	{
		free(pwszKeyValue);
		pwszKeyValue = NULL;
	}

	return pwszKeyValue;
}

std::wstring MinUtils::GetRegInstallPath()
{
	std::wstring wstrPath;
	
	LPWSTR pwszPath = GetRegistryString(HKEY_CURRENT_USER, L"Software\\CrystalBP", L"InstallPath");
	if(pwszPath)
	{
		wstrPath = pwszPath;
		free(pwszPath);
	}
	
	return wstrPath;
}

BOOL MinUtils::GetEnvVar(std::wstring wstrVarName, std::wstring& wstrVarValue)
{
	BOOL bSuccess = FALSE;

	static WCHAR wszEnvVar[16384] = {0};
	const DWORD cchEnvVar = (sizeof(wszEnvVar) / sizeof(WCHAR) - 1);

	DWORD dwLength = GetEnvironmentVariable(wstrVarName.c_str(), wszEnvVar, cchEnvVar);
	if(!dwLength)
		goto Cleanup;
	
	wszEnvVar[dwLength] = '\0';
	wstrVarValue = wszEnvVar;

	bSuccess = TRUE;
Cleanup:
	return bSuccess;
}

BOOL MinUtils::SetEnvVar(std::wstring wstrVarName, std::wstring wstrVarValue)
{
	return SetEnvironmentVariable(wstrVarName.c_str(), wstrVarValue.c_str());
}

BOOL MinUtils::GetCurDir(std::wstring& wstrCurDir)
{
	BOOL bSuccess = FALSE;

	static WCHAR wszCurDir[2048] = {0};
	const DWORD cchCurDir = (sizeof(wszCurDir) / sizeof(WCHAR) - 1);

	DWORD dwLength = GetCurrentDirectory(cchCurDir, wszCurDir);
	if(!dwLength)
		goto Cleanup;

	wszCurDir[dwLength] = '\0';
	wstrCurDir = wszCurDir;

	bSuccess = TRUE;
Cleanup:
	return bSuccess;
}
