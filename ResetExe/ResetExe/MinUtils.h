// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#pragma once

namespace User32
{
	static int (WINAPI *FN_MessageBoxW)(
		__in_opt HWND hWnd,
		__in_opt LPCWSTR lpText,
		__in_opt LPCWSTR lpCaption,
		__in UINT uType
		) = NULL;

	static void InitUser32()
	{
		HMODULE hModule = GetModuleHandle(L"user32.dll") ? GetModuleHandle(L"user32.dll") : LoadLibrary(L"user32.dll");
		FARPROC p = GetProcAddress(hModule, "MessageBoxW");
		*(FARPROC *)&FN_MessageBoxW = p;
	}
};

namespace MinUtils
{
	WCHAR *GetRegistryString(HKEY hKey, WCHAR *pwszRegPath, WCHAR *pwszKeyName);
	std::wstring GetRegInstallPath();
	BOOL SetRegistryString(HKEY hKey, WCHAR *pwszRegPath, WCHAR *pwszName, WCHAR *pwszValue);
	BOOL SetRegistryDword(HKEY hKey, WCHAR *pwszRegPath, WCHAR *pwszName, DWORD dwValue);
	BOOL CreateRegKey(HKEY hKey, WCHAR *pwszRegPath);
	std::list<std::wstring> *EnumRegSubKeys(HKEY hKey, WCHAR *pwszRegPath);
	BOOL DeleteRegKeyValue(HKEY hKey, WCHAR *pwszRegPath, WCHAR *pwszName);
	WCHAR *wcsistr(WCHAR *pwszString, WCHAR *pwszSearch, size_t cchString);
	BOOL IsUserAdminRegMethod();
	BOOL FileExists(std::wstring wstrPath);
	BOOL FileCreate(std::wstring wstrPath);
	std::wstring GetTempPath();
}