// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#pragma once

class CApiFirewall {
	friend BOOL DiDCore::Initialize();
public:
	static CApiFirewall* GetInstance();

	BOOL QueryProcessCreationAllowed(std::wstring wstrProcess, std::wstring wstrParameters);
	BOOL QueryModuleLoadPathAllowed(std::wstring wstrModulePath);

private:
	CApiFirewall();
	
	BOOL CheckPathNotUNC(std::wstring wstrPath);
	BOOL CheckPathNotWinTemp(std::wstring wstrPath);
	BOOL CheckPathNotTempInetFiles(std::wstring wstrPath);
	BOOL CheckPathNotLongName(std::wstring wstrPath);
	BOOL CheckPathNotDownloads(std::wstring wstrPath);
	BOOL CheckPathNotEnvVar(std::wstring wstrPath);
	BOOL CheckPathWhitelist(std::wstring wstrPath);
	BOOL CheckParamDirectoryDlls(std::wstring wstrParameters);

	// function to prompt for user override
	BOOL QueryUserRunExe(std::wstring wstrPath);
	std::list<std::wstring> *ExtractUNCNetworkPaths(std::wstring wstrParameters);
	BOOL RemoteFolderContainsDll(std::wstring wstrDir);
	BOOL TryOpenDllFileDirectory(std::wstring wstrDir);

	static BOOL AddExeToWhitelist(WCHAR *pwszExeName);
	static void ConfigRefresh(BOOL bRefresh);

	static CApiFirewall *m_Instance;
	static CRITICAL_SECTION m_csCreateInst, m_csApiFwOp;

	WCHAR *m_pwszTmpEnvPath, *m_pwszTempEnvPath, *m_pwszUserprofileTmp, *m_pwszWindowDirTmp,
		*m_pwszProtProc;

	BOOL m_bEnabled, m_bAllowCodeFromTemp, m_bAllowCodeFromNetwork, m_bAllowCodeLongPath,
		m_bAllowCodeFromInetTemp, m_bCheckWhitelist, m_bAllowCodeFromDownloads, m_bQueryUserOnExeBlocked,
		m_bCheckDirDllPlant, m_bAllowSpawnProtProcs, m_bAutoAddToWhitelist;
	
	std::vector<std::wstring> m_vecExeWhitelist;

	IEventWriter *m_pEventWriter;
};