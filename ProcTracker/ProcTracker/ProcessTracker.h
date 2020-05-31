// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#pragma once

class CProcess {
public:
	CProcess(char *pszProcess, DWORD dwPID, HANDLE hProcess) :
	  m_dwPID(dwPID), m_hProcess(hProcess)
	{
		strncpy(m_szProcess, pszProcess, 260);
		m_szProcess[259] = '\0';
	}
	
	CProcess() : m_dwPID(0), m_hProcess(0)
	{}

	DWORD GetPID(){ return m_dwPID; }
	char *GetExeName(){ return m_szProcess; }
	HANDLE GetHandle(){ return m_hProcess; }
	void CloseHandle(){ ::CloseHandle(m_hProcess); }

private:
	char m_szProcess[260];
	DWORD m_dwPID;
	HANDLE m_hProcess;
};

class CProcessTracker {
	friend int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow);
public:
	
	~CProcessTracker()
	{
		if(m_hProcessEvent)
			CloseHandle(m_hProcessEvent);
	}

	static CProcessTracker *GetInstance();
	static DWORD ProcessMonitorProc(LPVOID);

	RPC_STATUS StartRPCServer();
	RPC_STATUS StopRPCServer();

	BOOL AddProcess(DWORD dwProcessID);
	DWORD GetProcessIDs(DWORD cUniqueProcIDs, char *rg_pszUniqueProcID[]);

	BOOL AddExeToWhitelist(WCHAR *pwszExeName);

private:
	CProcessTracker() {
		InitializeCriticalSection(&m_csConfigOp);
		InitializeCriticalSection(&m_csHandleOp);

		memset(m_hMonProc, 0, sizeof(m_hMonProc));
		
		m_hProcessEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		m_hMonProc[0] = m_hProcessEvent;
		m_cMonProc = 1;
		
		HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ProcessMonitorProc, NULL, 0, NULL);
		CloseHandle(hThread);
	}

	std::string GetUniqueProcessIDString(HANDLE hProcess, char *pszProcName, DWORD dwProcessID);
	BOOL CProcessTracker::GetProcessName(std::string& strProcName, HANDLE hProcess);

	static int __cdecl HandleComp(const void *v1, const void *v2);

	static CRITICAL_SECTION m_csCreateInst, m_csHandleOp, m_csConfigOp;
	static CProcessTracker *m_Instance;
	
	HANDLE m_hMonProc[4096], m_hProcessEvent;
	size_t m_cMonProc;

	std::map<HANDLE, CProcess> m_mapMonProc;
};