// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#pragma once

namespace MinUtils
{
	WCHAR *GetRegistryString(HKEY hKey, WCHAR *pwszRegPath, WCHAR *pwszKeyName);
	std::wstring GetRegInstallPath();
	BOOL GetEnvVar(std::wstring wstrVarName, std::wstring& wstrVarValue);
	BOOL SetEnvVar(std::wstring wstrVarName, std::wstring wstrVarValue);
	BOOL GetCurDir(std::wstring& wstrCurDir);
}