// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#include "stdafx.h"
#include "ProcTracker_h.h"
#include "RealtimeLog.h"
#include "..\..\AuxCore\AuxCore\IEventWriter.h"
#include "..\..\Shared\Utils\Utils.h"
#include "ProcessTracker.h"

CRITICAL_SECTION CProcessTracker::m_csCreateInst = {0}, CProcessTracker::m_csHandleOp = {0}, CProcessTracker::m_csConfigOp = {0};
CProcessTracker *CProcessTracker::m_Instance = NULL;

CProcessTracker *CProcessTracker::GetInstance()
{
	if(m_Instance == NULL)
	{
		EnterCriticalSection(&m_csCreateInst);

		if(m_Instance == NULL)
		{
			m_Instance = new CProcessTracker();
		}

		LeaveCriticalSection(&m_csCreateInst);
	}

	return m_Instance;
}

DWORD CProcessTracker::ProcessMonitorProc(LPVOID)
{
	CProcessTracker *pProcTracker = CProcessTracker::GetInstance();

	while(1)
	{
		// first member of this list is an event signifying process handle list change
		DWORD dwWaitResult = WaitForMultipleObjects(pProcTracker->m_cMonProc, pProcTracker->m_hMonProc, FALSE, INFINITE);
		
		switch(dwWaitResult)
		{
		case WAIT_FAILED:
			pProcTracker->StopRPCServer();
			return 1;
		default:
			DWORD Index = dwWaitResult - WAIT_OBJECT_0;

			if(!Index) // process list change event
				continue;
			
			HANDLE hProcess = pProcTracker->m_hMonProc[Index];

			EnterCriticalSection(&m_csHandleOp);
			
			pProcTracker->m_hMonProc[Index] = NULL;
			CProcess proc = pProcTracker->m_mapMonProc[hProcess];
			pProcTracker->m_mapMonProc.erase(hProcess);
			
			// erase the realtime log for this process
			CRealtimeLog::GetInstance()->RemoveProcess(proc.GetPID());
			
			LeaveCriticalSection(&m_csHandleOp);

			DWORD dwExitCode = 0;
			GetExitCodeProcess(hProcess, &dwExitCode);
			EEventRepository repos;
			BOOL bWriteEvent = TRUE;
			std::wstring wstrDescription;
			LPWSTR pwszRtLogMsg = NULL;

			switch(dwExitCode)
			{
			case BPCORE_TERMINATE_HEAP_CORRUPT:
				repos = EEventRepository::HeapMonitor;
				wstrDescription = L"An instance of heap memory corruption was identified within the process %hs.\r\n\r\n"
					L"A probable attempt to execute malicious code within the affected program was successfully prevented. The affected program was terminated.\r\n\r\n"
					L"Memory corruption conditions can often be induced by an attacker to force an otherwise secure program into an insecure state. "
					L"Once in an insecure state it is often possible to force the program to run malicious code.";
				pwszRtLogMsg = L"heap memory corruption";
				break;
			case BPCORE_TERMINATE_DOUBLE_FREE:
				repos = EEventRepository::HeapMonitor;
				wstrDescription = L"A double free condition was identified within the process %hs.\r\n\r\n"
					L"A probable attempt to execute malicious code within the affected program was successfully prevented. The affected program was terminated.\r\n\r\n"
					L"Double free conditions are programming errors which put the affected program into a vulnerable state. Once in such a state it "
					L"is sometimes possible to force the program to run malicious code.";
				pwszRtLogMsg = L"double free";
				break;
			case BPCORE_TERMINATE_USE_AFTER_FREE:
				repos = EEventRepository::HeapMonitor;
				wstrDescription = L"A use-after-free condition was identified within the process %hs.\r\n\r\n"
					L"A probable attempt to execute malicious code within the affected program was successfully prevented. The affected program was terminated.\r\n\r\n"
					L"Use-after-free conditions are programming errors which can often be exploited to run malicious code. This is possible because "
					L"once trusted memory is returned to the operating system its contents becomes unpredictable and can put the affected program into "
					L"an insecure state if re-used.";
				pwszRtLogMsg = L"use-after-free";
				break;
			case BPCORE_TERMINATE_ANTISPRAY:
				repos = EEventRepository::AntiSpray;
				wstrDescription = L"An attempt to run code from a dangerous location within the process %hs was prevented.\r\n\r\n"
					L"A very common method for leveraging a security flaw to install malicious code relies on filling a lot of memory with malicious code in an attempt to "
					L"increase the probability that it will be run successfully. An attempt to run such code was detected and the process was terminated.";
				pwszRtLogMsg = L"heap spray attack";
				break;
			case BPCORE_TERMINATE_UNTRUSTED_CALLER:
				repos = EEventRepository::DfnsInDepth;
				wstrDescription = L"An attempt by untrusted code to access operating system security critical functionality was prevented. The process in question was %hs.\r\n\r\n"
					L"This may indicate that malicious code had managed to begin running and was detected by Crystal, or it could be a false positive. The process was terminated for safety reasons.";
				pwszRtLogMsg = L"untrusted caller";
				break;
			case BPCORE_TERMINATE_ROP_DETECTED:
				repos = EEventRepository::DfnsInDepth;
				wstrDescription = L"An attempt to execute a return-oriented programming attack was detected within the process %hs. The affected program was terminated.\r\n\r\n"
					L"Return-oriented programming (ROP) is a method of executing malicious code without introducing new code into the vulnerable program. This involves repurposing existing "
					L"otherwise safe program code to achieve a malicious end. ROP attacks often share common attack stages and one such stage was detected. Code execution was prevented.";
				pwszRtLogMsg = L"return-oriented programming";
				break;
			case BPCORE_TERMINATE_SEH_FAULT:
				repos = EEventRepository::DfnsInDepth;
				wstrDescription = L"An attempt to execute malicious code through corruption of an error handling mechanism was prevented. The affected process was %hs.\r\n\r\n"
					L"A common technique for running malicious code involves overwriting error handling information within a vulnerable program and then causing it to crash. As the "
					L"program crashes the error handling behaviour is invoked and, having been overwritten to malicious ends, dangerous code can be executed.";
				pwszRtLogMsg = L"structured exception handler corruption";
				break;
			case 0xC0000409:
				repos = EEventRepository::StackMonitor;
				wstrDescription = L"Termination of the process %hs occurred as a result of stack cookie corruption.\r\n\r\n"
					L"This may indicate a failed attempt to execute malicious code, or it may be a false positive.";
				pwszRtLogMsg = L"stack buffer overflow";
				break;
			default:
				// not suspicious
				bWriteEvent = FALSE;
				break;
			}

			if(bWriteEvent)
			{
				// write event
				
				if(GetEventWriter != NULL)
				{
					IEventWriter *pEvent = GetEventWriter();
				
					WCHAR wszProcName[256] = {0};

					mbstowcs(wszProcName, proc.GetExeName(), 255);
					wszProcName[255] = '\0';

					pEvent->WriteEventProcess(
							proc.GetHandle(),
							wszProcName,
							repos, EThreatRiskRating::HighRisk,
							L"Malicious Code Execution Attempt Intercepted",
							const_cast<WCHAR *>(wstrDescription.c_str()),
							proc.GetExeName()
						);
				}

				// add an entry to realtime log
				RealtimeLogEntry rt;

				rt.dwSeverity = 1;
				wsprintf(rt.wszMessage, L"An attempt to execute malicious code using the %s technique was prevented", pwszRtLogMsg);

				CRealtimeLog *pRealtimeLog = CRealtimeLog::GetInstance();
				pRealtimeLog->AddEntryProcess(proc.GetPID(), &rt);
			}

			EnterCriticalSection(&pProcTracker->m_csHandleOp);
			
			// fast way to trim the array. + 1 to avoid event which must be first item in list (or we'll try and get its exit code!)
			qsort(pProcTracker->m_hMonProc + 1, pProcTracker->m_cMonProc - 1, sizeof(HANDLE), HandleComp);

			while(pProcTracker->m_cMonProc >= 1 && pProcTracker->m_hMonProc[pProcTracker->m_cMonProc] == NULL)
				pProcTracker->m_cMonProc--;

			if(pProcTracker->m_hMonProc[pProcTracker->m_cMonProc])
				pProcTracker->m_cMonProc++;

			LeaveCriticalSection(&pProcTracker->m_csHandleOp);

			proc.CloseHandle();
			break;
		}
	}
	
	return 0;
}

RPC_STATUS CProcessTracker::StartRPCServer()
{
	RPC_STATUS rpcStatus;
	PSECURITY_DESCRIPTOR pSecDesc = NULL;
	ULONG cbSecDesc = 0;
	
	// allow Everyone access
	if(ConvertStringSecurityDescriptorToSecurityDescriptor(L"D:(A;OICI;GA;;;WD)", SDDL_REVISION_1, &pSecDesc, &cbSecDesc) == FALSE)
	{
		rpcStatus = RPC_S_INVALID_SECURITY_DESC;
		goto Cleanup;
	}
	
	rpcStatus = RpcServerUseProtseqEp(
			(RPC_WSTR)L"ncalrpc",
			RPC_C_LISTEN_MAX_CALLS_DEFAULT,
			(RPC_WSTR)L"ProcTracker",
			pSecDesc
		);
	if(rpcStatus != RPC_S_OK)
		goto Cleanup;

	rpcStatus = RpcServerRegisterIf(IProcTracker_v1_0_s_ifspec, NULL, NULL);
	if(rpcStatus != RPC_S_OK)
		goto Cleanup;

	rpcStatus = RpcServerListen(1, RPC_C_LISTEN_MAX_CALLS_DEFAULT, FALSE);
	
	// server will be ongoing

Cleanup:
	if(pSecDesc)
		LocalFree(pSecDesc);

	return rpcStatus;
}

RPC_STATUS CProcessTracker::StopRPCServer()
{
	RPC_STATUS rpcStatus;
	
	rpcStatus = RpcMgmtStopServerListening(IProcTracker_v1_0_s_ifspec);
	if(rpcStatus != RPC_S_OK)
		goto Cleanup;

	rpcStatus = RpcServerUnregisterIf(IProcTracker_v1_0_s_ifspec, NULL, TRUE);

Cleanup:
	return rpcStatus;
}

BOOL CProcessTracker::AddProcess(DWORD dwProcessID)
{
	BOOL bRet = FALSE;
	char szProcess[260] = {0};
	std::string strProc = "";
	DWORD dwProcCount = 0;
	HANDLE hProcess = NULL;

	EnterCriticalSection(&m_csHandleOp);

	for(dwProcCount=0; dwProcCount < (m_cMonProc - 1); dwProcCount++)
	{
		hProcess = m_hMonProc[dwProcCount + 1];
		CProcess proc = m_mapMonProc[hProcess];

		if(proc.GetPID() == dwProcessID)
		{
			// we're already watching this process
			LeaveCriticalSection(&m_csHandleOp);
			return TRUE;
		}
	}
	
	LeaveCriticalSection(&m_csHandleOp);

	hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | SYNCHRONIZE, FALSE, dwProcessID);
	if(!hProcess)
		goto Cleanup;

	if(!GetModuleFileNameExA(hProcess, NULL, szProcess, 259))
		goto Cleanup;

	strProc = szProcess;
	strProc = strProc.substr(strProc.find_last_of('\\') + 1);

	EnterCriticalSection(&m_csHandleOp);

	m_mapMonProc[hProcess] = CProcess(const_cast<char *>(strProc.c_str()), dwProcessID, hProcess);
	
	// add to monitored handles array
	m_hMonProc[m_cMonProc] = hProcess;
	m_cMonProc++;

	// refresh tracker thread
	SetEvent(m_hProcessEvent);

	LeaveCriticalSection(&m_csHandleOp);

	hProcess = NULL;
	bRet = TRUE;
Cleanup:
	if(hProcess)
		CloseHandle(hProcess);
	
	return bRet;
}

#define RPC_PROCESS_ID_STRING_SIZE	512
DWORD CProcessTracker::GetProcessIDs(DWORD cUniqueProcIDs, char *rg_pszUniqueProcID[]) // 512-byte arrays
{
	DWORD dwProcCount = 0;
	
	if(rg_pszUniqueProcID == NULL || cUniqueProcIDs == 0)
		return (DWORD)(m_cMonProc - 1);
	
	SecureZeroMemory(rg_pszUniqueProcID, cUniqueProcIDs * RPC_PROCESS_ID_STRING_SIZE);

	if(cUniqueProcIDs >= (sizeof(m_hMonProc) / sizeof(HANDLE))) // >= to avoid first event object
		goto Cleanup;

	EnterCriticalSection(&m_csHandleOp);

	for(dwProcCount=0; dwProcCount < (m_cMonProc - 1) && dwProcCount < cUniqueProcIDs; dwProcCount++)
	{
		HANDLE hProcess = m_hMonProc[dwProcCount + 1];
		CProcess proc = m_mapMonProc[hProcess];

		std::string strUniqueProcID = GetUniqueProcessIDString(hProcess, proc.GetExeName(), proc.GetPID());

		strncpy((char *)rg_pszUniqueProcID + (dwProcCount * RPC_PROCESS_ID_STRING_SIZE), strUniqueProcID.c_str(), 511);
		*((char *)rg_pszUniqueProcID + (dwProcCount * RPC_PROCESS_ID_STRING_SIZE) + (RPC_PROCESS_ID_STRING_SIZE - 1)) = '\0';
	}
	
	LeaveCriticalSection(&m_csHandleOp);

Cleanup:
	return dwProcCount;
}

BOOL CProcessTracker::GetProcessName(std::string& strProcName, HANDLE hProcess)
{
	BOOL bSuccess = FALSE;

	if(!hProcess)
		hProcess = GetCurrentProcess();

	char *pszExePath = (char *)malloc(8192);
	if(!pszExePath)
		goto Cleanup;

	DWORD nSize = GetModuleFileNameExA(hProcess, NULL, pszExePath, 8191);
	
	if(!nSize || nSize == 8191)
		goto Cleanup;

	char *psz = strrchr(pszExePath, '\\');
	if(psz)
	{
		memmove(pszExePath, psz + 1, strlen(psz + 1) + 1); // + 1 for NUL
	}

	strProcName.assign(pszExePath);
	
	bSuccess = TRUE;
Cleanup:
	if(pszExePath)
		free(pszExePath);

	return bSuccess;
}

std::string CProcessTracker::GetUniqueProcessIDString(HANDLE hProcess, char *pszProcName, DWORD dwProcessID)
{
	std::string strProc, strUniqueProcID;
	char szDateBuf[128] = {0};

	SYSTEMTIME sysTime = {0};
	GetSystemTime(&sysTime);

	sprintf(szDateBuf,
			"%.02u/%.02u/%.02u %.02u:%.02u:%.02u",
			sysTime.wDay, sysTime.wMonth, sysTime.wYear,
			sysTime.wHour, sysTime.wMinute, sysTime.wSecond);

	if(pszProcName)
		strProc = pszProcName;
	else
		strProc = "(unknown process name)";
	
	for(int i=0; i<strProc.size(); i++)
	{
		if(strProc[i] == '[' || strProc[i] == ']')
			strProc[i] = '_';
	}

	strUniqueProcID = "[" + strProc + "]";
	
	char szPID[32];
	sprintf(szPID, "%u", dwProcessID);
	
	strUniqueProcID += "[" + std::string(szPID) + "]";

	FILETIME ftCreationTime = {0}, ftExitTime = {0}, ftKernelTime = {0}, ftUserTime = {0};

	if(GetProcessTimes(hProcess, &ftCreationTime, &ftExitTime, &ftKernelTime, &ftUserTime))
	{
		char szCreationTime[128];
		sprintf(szCreationTime, "%.08x:%.08x", ftCreationTime.dwHighDateTime, ftCreationTime.dwLowDateTime);
		strUniqueProcID += "[" + std::string(szCreationTime) + "]";
	}
	else
	{
		strUniqueProcID += "[(unknown process time)]";
	}

	return strUniqueProcID;
}

int __cdecl CProcessTracker::HandleComp(const void *v1, const void *v2)
{
	HANDLE h1 = *(HANDLE *)v1, h2 = *(HANDLE *)v2;
	return (int)h2 - (int)h1;
}

BOOL CProcessTracker::AddExeToWhitelist(WCHAR *pwszExeName)
{
	// horribly hacky code to add a process to the executable whitelist entry in a config file
	
	BOOL bSuccess = FALSE, bAlreadyExists = FALSE;
	std::wstring wstrConfigFile = Utils::GetRegInstallPath() + L"\\config.xml";
	FILE *fp = NULL;
	char *pszConfigRaw = NULL, *pszToken = NULL, szExeName[260] = {0};
	DWORD cchExeName = 0, cchExistingList = 0;

	wcstombs(szExeName, pwszExeName, 258);
	szExeName[258] = '\0';
	cchExeName = strlen(szExeName);

	EnterCriticalSection(&m_csConfigOp);

	fp = _wfopen(wstrConfigFile.c_str(), L"rb");
	if(!fp)
		goto Cleanup;

	fseek(fp, 0, SEEK_END);
	long fileLen = ftell(fp);

	if(fileLen <= 0 || (fileLen + 260 + 1) < fileLen)
		goto Cleanup;

	rewind(fp);

	pszConfigRaw = (char *)malloc(fileLen + 260 + 1);
	if(!pszConfigRaw)
		goto Cleanup;

	if(fread(pszConfigRaw, 1, fileLen, fp) != fileLen)
		goto Cleanup;

	fclose(fp); // done with read mode
	fp = NULL;

	pszConfigRaw[fileLen] = '\0';

	// horrible but should always work unless user hand edits file significantly
	pszToken = strstr(pszConfigRaw, "<property name=\"executableWhitelist\" value=\"");
	if(!pszToken)
		goto Cleanup;

	pszToken += 44;

	while(pszToken[cchExistingList] && pszToken[cchExistingList] != '\"')
	{
		if(!strnicmp(pszToken + cchExistingList, szExeName, cchExeName))
		{
			bAlreadyExists = TRUE;
			break;
		}

		cchExistingList++;
	}

	if(!bAlreadyExists)
	{
		fp = _wfopen(wstrConfigFile.c_str(), L"w+b");
		if(!fp)
			goto Cleanup;

		if(cchExistingList)
		{
			// delimit if not sole entry in whitelist
			strcat(szExeName, "|");
			cchExeName++;
		}

		memmove(pszToken + cchExeName, pszToken, (DWORD)fileLen - (pszToken - pszConfigRaw) + 1);
		memcpy(pszToken, szExeName, cchExeName);

		if(fwrite(pszConfigRaw, 1, (DWORD)fileLen + cchExeName, fp) != (DWORD)fileLen + cchExeName)
			goto Cleanup;
	}

	bSuccess = TRUE;
Cleanup:
	if(pszConfigRaw)
		free(pszConfigRaw);
	
	if(fp)
		fclose(fp);

	LeaveCriticalSection(&m_csConfigOp);

	return bSuccess;
}

extern "C" void __RPC_FAR * __RPC_USER midl_user_allocate(size_t len)
{
	return(malloc(len));
}

extern "C" void __RPC_USER midl_user_free(void __RPC_FAR * ptr)
{
	free(ptr);
}

BOOL AddProcess( 
    /* [in] */ handle_t IDL_handle,
    /* [in] */ DWORD dwProcessID)
{
	CProcessTracker *pProcTracker = CProcessTracker::GetInstance();
	return pProcTracker->AddProcess(dwProcessID);
}

DWORD GetProcessIDs( 
    /* [in] */ handle_t IDL_handle,
    /* [in] */ DWORD cUniqueProcIDs,
    /* [size_is][size_is][out][in] */ unsigned char rg_pszUniqueProcID[  ][ 512 ])
{
	CProcessTracker *pProcTracker = CProcessTracker::GetInstance();
	return pProcTracker->GetProcessIDs(cUniqueProcIDs, (char **)rg_pszUniqueProcID);
}

void Stop(
    /* [in] */ handle_t IDL_handle)
{
	CProcessTracker *pProcTracker = CProcessTracker::GetInstance();
	pProcTracker->StopRPCServer();
}

BOOL CreateProtectFileProcess( 
    /* [in] */ handle_t IDL_handle,
    DWORD dwPID)
{
	if(Utils::Is64BitProcess(dwPID) == FALSE)
	{
		std::wstring wstrProcFile = Utils::GetRegInstallPath() + L"\\Temp\\Proc_";

		WCHAR wszPID[32] = {0};
		wsprintf(wszPID, L"%u", dwPID);
			
		wstrProcFile += wszPID;
		wstrProcFile += L".protect";

		// c:\work\crystal\bp\install\Proc_1234.protect
		return Utils::CreateEmptyFile(wstrProcFile.c_str());
	}

	return FALSE;
}

BOOL CheckProtectFileProcess( 
    /* [in] */ handle_t IDL_handle,
    DWORD dwPID)
{
	std::wstring wstrProcFile = Utils::GetRegInstallPath() + L"\\Temp\\Proc_";
	
	WCHAR wszPID[32] = {0};
	wsprintf(wszPID, L"%u", dwPID);
	
	wstrProcFile += wszPID;
	wstrProcFile += L".protect";

	if(Utils::OpenEmptyFile(wstrProcFile.c_str()))
	{
		// this is a protected child process
		Utils::DeleteEmptyFile(wstrProcFile.c_str());
		return TRUE;
	}

	return FALSE;
}

BOOL AddExeToWhitelist( 
    /* [in] */ handle_t IDL_handle,
    /* [string][in] */ WCHAR *pwszExeName)
{
	CProcessTracker *pProcTracker = CProcessTracker::GetInstance();
	return pProcTracker->AddExeToWhitelist(pwszExeName);
}