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

BOOL MinUtils::SetRegistryString(HKEY hKey, WCHAR *pwszRegPath, WCHAR *pwszName, WCHAR *pwszValue)
{
	BOOL bSuccess = FALSE;
	HKEY hkResult = 0;
	
	if(RegOpenKeyEx(hKey, pwszRegPath, 0, KEY_SET_VALUE, &hkResult) != ERROR_SUCCESS)
		goto Cleanup;

	if(RegSetValueEx(hkResult, pwszName, NULL, REG_SZ, (PBYTE)pwszValue,
			sizeof(WCHAR) * (wcslen(pwszValue) + 1)) != ERROR_SUCCESS)
		goto Cleanup;

	bSuccess = TRUE;
Cleanup:
	if(hkResult)
		RegCloseKey(hkResult);

	return bSuccess;
}

BOOL MinUtils::SetRegistryDword(HKEY hKey, WCHAR *pwszRegPath, WCHAR *pwszName, DWORD dwValue)
{
	BOOL bSuccess = FALSE;
	HKEY hkResult = 0;
	
	if(RegOpenKeyEx(hKey, pwszRegPath, 0, KEY_SET_VALUE, &hkResult) != ERROR_SUCCESS)
		goto Cleanup;

	if(RegSetValueEx(hkResult, pwszName, NULL, REG_DWORD, (PBYTE)&dwValue, sizeof(DWORD)) != ERROR_SUCCESS)
		goto Cleanup;

	bSuccess = TRUE;
Cleanup:
	if(hkResult)
		RegCloseKey(hkResult);

	return bSuccess;
}

BOOL MinUtils::CreateRegKey(HKEY hKey, WCHAR *pwszRegPath)
{
	BOOL bSuccess = FALSE;
	DWORD dwDisposition = 0;
	HKEY hkResult = 0;
	
	if(RegCreateKeyEx(hKey, pwszRegPath, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE,
			NULL, &hkResult, &dwDisposition) != ERROR_SUCCESS)
		goto Cleanup;
	
	bSuccess = TRUE;
Cleanup:
	if(hkResult)
		RegCloseKey(hkResult);

	return bSuccess;
}

std::list<std::wstring> *MinUtils::EnumRegSubKeys(HKEY hKey, WCHAR *pwszRegPath)
{
	std::list<std::wstring> *plist = new std::list<std::wstring>();
	if(!plist)
		goto Cleanup_NullPtr;

	HKEY hkResult = 0;
	
	if(RegOpenKeyEx(hKey, pwszRegPath, 0, KEY_READ, &hkResult) != ERROR_SUCCESS)
		goto Cleanup;

	WCHAR wszSubKeyName[256];
	DWORD dwIndex = 0, dwSubKeyNameLen = (sizeof(wszSubKeyName) / sizeof(WCHAR));
	
	while(RegEnumKeyEx(hkResult, dwIndex, wszSubKeyName, &dwSubKeyNameLen,
		NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
	{
		plist->push_back(wszSubKeyName);
		dwIndex++;
		dwSubKeyNameLen = (sizeof(wszSubKeyName) / sizeof(WCHAR));
	}
	
Cleanup:
	if(plist->empty())
	{
		delete plist;
		plist = NULL;
	}

Cleanup_NullPtr:
	return plist;
}

BOOL MinUtils::DeleteRegKeyValue(HKEY hKey, WCHAR *pwszRegPath, WCHAR *pwszName)
{
	BOOL bSuccess = FALSE;
	HKEY hkResult = 0;
	WCHAR *pwszKeyValue = NULL;
	DWORD cbKeyValue = 0;

	if(RegOpenKeyEx(hKey, pwszRegPath, 0, KEY_SET_VALUE,
			&hkResult) != ERROR_SUCCESS)
		goto Cleanup;

	if(RegDeleteValue(hkResult, pwszName) != ERROR_SUCCESS)
		goto Cleanup;

	bSuccess = TRUE;
Cleanup:
	if(hkResult)
		RegCloseKey(hkResult);

	return bSuccess;
}

WCHAR *MinUtils::wcsistr(WCHAR *pwszString, WCHAR *pwszSearch, size_t cchString)
{
	size_t cchSearch = wcslen(pwszSearch);

	if(cchString != -1)
	{
		if(cchString == cchSearch)
		{
			if(!wcsicmp(pwszString, pwszSearch))
			{
				return pwszString;
			}
		}
		else if(cchString < cchSearch)
		{
			return NULL;
		}
	}

	for(size_t i=0; cchString == -1 ? pwszString[i] != '\0' : i < cchString - cchSearch; i++)
	{
		if(!wcsnicmp(pwszString + i, pwszSearch, cchSearch))
		{
			return pwszString + i;
		}
	}

	return NULL;
}

BOOL MinUtils::IsUserAdminRegMethod()
{
	BOOL bIsAdmin = FALSE;
	HKEY hkResult = 0;

	if(RegOpenKey(HKEY_USERS, L"", &hkResult) != ERROR_SUCCESS)
		goto Cleanup;

	bIsAdmin = TRUE;
Cleanup:
	return bIsAdmin;
}

BOOL MinUtils::FileExists(std::wstring wstrPath)
{
	char szPath[260] = {0};
	
	wcstombs(szPath, wstrPath.c_str(), 259); szPath[259] = '\0';

	FILE *fp = fopen(szPath, "rb");
	if(fp)
	{
		fclose(fp);
		return TRUE;
	}

	return FALSE;
}

BOOL MinUtils::FileCreate(std::wstring wstrPath)
{
	char szPath[260] = {0};
	
	wcstombs(szPath, wstrPath.c_str(), 259); szPath[259] = '\0';

	FILE *fp = fopen(szPath, "wb");
	if(fp)
	{
		fclose(fp);
		return TRUE;
	}

	return FALSE;
}

std::wstring MinUtils::GetTempPath()
{
	WCHAR wszPath[260] = {0};

	DWORD dwCch = ::GetTempPathW(259, wszPath);
	if(!dwCch)
	{
		return std::wstring();
	}

	wszPath[dwCch] = '\0';

	return wszPath;
}