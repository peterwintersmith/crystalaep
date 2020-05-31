// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#include "stdafx.h"
#include "AuxCore.h"
#include "..\..\ProcTracker\ProcTracker\ProcTracker_h.h"
#include "..\..\Shared\Utils\Utils.h"
#include "EventWriter.h"

CRITICAL_SECTION CEventWriter::m_csCreateInst = {0};
CEventWriter *CEventWriter::m_Instance = NULL;

LPWSTR CEventWriter::m_rgwszFileNames[10] = {
	L"AntiSpray",
	L"ApiFirewall",
	L"AudioValidator",
	L"ConnectionMonitor",
	L"DfnsInDepth",
	L"HeapMonitor",
	L"ImageValidator",
	L"StackMonitor",
	L"VideoValidator",
	L"WebAppMonitor"
};

LPSTR CEventWriter::m_rgszRiskRatings[4] = {
	"Information",
	"LowRisk",
	"MediumRisk",
	"HighRisk"
};

CEventWriter* CEventWriter::GetInstance()
{
	if(m_Instance == NULL)
	{
		EnterCriticalSection(&m_csCreateInst);

		if(m_Instance == NULL)
		{
			m_Instance = new CEventWriter();
		}

		LeaveCriticalSection(&m_csCreateInst);
	}

	return m_Instance;
}

EEventWriteStatus CEventWriter::WriteEvent(
		EEventRepository repos,
		EThreatRiskRating risk,
		LPWSTR pwszTitle,
		LPWSTR pwszBodyFormat,
		...
	)
{
	va_list ap;
	WCHAR wszFmt[8192] = {0};
	
	va_start(ap, pwszBodyFormat);
	StringCchVPrintf(wszFmt, 8191, pwszBodyFormat, ap);
	wszFmt[8191] = '\0';
	va_end(ap);

	return WriteEventInternal(NULL, NULL, repos, risk, pwszTitle, wszFmt);
}

EEventWriteStatus CEventWriter::WriteEventProcess(
		HANDLE hProcess,
		LPWSTR pwszProcessName,
		EEventRepository repos,
		EThreatRiskRating risk,
		LPWSTR pwszTitle,
		LPWSTR pwszBodyFormat,
		...
	)
{
	va_list ap;
	WCHAR wszFmt[8192] = {0};
	
	va_start(ap, pwszBodyFormat);
	StringCchVPrintf(wszFmt, 8191, pwszBodyFormat, ap);
	wszFmt[8191] = '\0';
	va_end(ap);

	return WriteEventInternal(hProcess, pwszProcessName, repos, risk, pwszTitle, wszFmt);
}

EEventWriteStatus CEventWriter::WriteEventInternal(
		HANDLE hProcess,
		LPWSTR pwszProcessName,
		EEventRepository repos,
		EThreatRiskRating risk,
		LPWSTR pwszTitle,
		LPWSTR pwszBody
	)
{
	Utils::WinClientVer ver = Utils::GetWinVersion();

	BOOL bIsLow = FALSE;
	BOOL bCheckSuccess = Utils::IsLowIntegrityProcess(GetCurrentProcess(), &bIsLow);

	if(ver == Utils::WinClientVer::Windows2000 || ver == Utils::WinClientVer::WindowsXP || (bCheckSuccess && !bIsLow))
	{
		// either Win2k or XP which do not support low integrity processes or is not low integrity process

		if(m_hMutex != NULL)
		{
			return WriteEventInProc(
				hProcess,
				pwszProcessName,
				repos,
				risk,
				pwszTitle,
				pwszBody	
			);
		}
		else
		{
			// mutex was not created: either privilege issue or we attempted to write an event before proctracker
			// had started. Let's fall back to RPC for this case.

			// (do nothing)
		}
	}

	// for newer OS and/or the error case for IsLowIntegrityProcess we should fall back to RPC event writer
	return WriteEventRPC(
		hProcess,
		pwszProcessName,
		repos,
		risk,
		pwszTitle,
		pwszBody	
	);
}

EEventWriteStatus CEventWriter::WriteEventInProc(
		HANDLE hProcess,
		LPWSTR pwszProcessName,
		EEventRepository repos,
		EThreatRiskRating risk,
		LPWSTR pwszTitle,
		LPWSTR pwszBody
	)
{
	EEventWriteStatus status = EEventWriteStatus::StatusFailure;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	std::string strXmlTag = "", strProc, strUniqueProcID;
	char *pszTitle = NULL;
	DWORD dwBytesWritten;
	char szDateBuf[128] = {0};

	std::wstring wstrXmlFile = Utils::GetRegInstallPath() + L"\\";  //L"C:\\Work\\crystal\\BP\\Impl\\BpCore\\";
	
	wstrXmlFile += m_rgwszFileNames[repos];
	wstrXmlFile += L".xml";

	DWORD dwWaitResult = WaitForSingleObject(m_hMutex, INFINITE);

	switch(dwWaitResult)
	{
	case WAIT_OBJECT_0:
		break;
	default:
		goto Cleanup;
	}

	hFile = CreateFile(
			wstrXmlFile.c_str(), 
			GENERIC_READ | GENERIC_WRITE,
			0, // deny sharing
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL
		);

	if(hFile == INVALID_HANDLE_VALUE)
	{
		hFile = CreateFile(
			wstrXmlFile.c_str(), 
			GENERIC_READ | GENERIC_WRITE,
			0, // deny sharing
			NULL,
			CREATE_NEW,
			FILE_ATTRIBUTE_NORMAL,
			NULL
		);

		if(hFile == INVALID_HANDLE_VALUE)
		{
			goto Cleanup;
		}

		char *pszXmlHeader = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n<events>\r\n</events>";
		WriteFile(hFile, pszXmlHeader, strlen(pszXmlHeader), &dwBytesWritten, NULL);
	}

	LARGE_INTEGER li = {0};
	
	// let's not fail if this doesn't work, although i can't see how it wouldn't

	if(GetFileSizeEx(hFile, &li))
	{
		PBYTE pbEventFileData = NULL;
		DWORD dwEventFileSize = (DWORD)li.u.LowPart;

		if(dwEventFileSize >= EVENT_XML_FILE_24MB_MAX_SIZE)
		{
			// truncate event log file
			
			if(dwEventFileSize + 1 < dwEventFileSize)
				goto Abort;

			pbEventFileData = (PBYTE)malloc(dwEventFileSize + 1);
			if(pbEventFileData == NULL)
				goto Abort;

			DWORD dwBytesRead = 0;

			if(!ReadFile(hFile, pbEventFileData, dwEventFileSize, &dwBytesRead, NULL) || dwBytesRead != dwEventFileSize)
				goto Abort;

			pbEventFileData[dwEventFileSize] = '\0';

			PBYTE pbRewrite = pbEventFileData + (dwEventFileSize - EVENT_XML_FILE_24MB_MAX_SIZE) + EVENT_XML_FILE_TRUNCATE_SIZE;
			while(*pbRewrite != '\0')
			{
				if(!memcmp(pbRewrite, "</event>\r\n", 10))
				{
					pbRewrite += 10;

					SetFilePointer(hFile, 0, NULL, FILE_BEGIN);

					char *pszXmlPartialHeader = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n<events>\r\n";

					WriteFile(hFile, pszXmlPartialHeader, strlen(pszXmlPartialHeader), &dwBytesWritten, NULL);
					WriteFile(hFile, pbRewrite, dwEventFileSize - (pbRewrite - pbEventFileData), &dwBytesWritten, NULL);

					SetEndOfFile(hFile);

					break;
				}

				pbRewrite++;
			}
		}
Abort:
		if(pbEventFileData != NULL)
			free(pbEventFileData);
	}

	SetFilePointer(hFile, -9, NULL, FILE_END);

	SYSTEMTIME sysTime = {0};
	GetSystemTime(&sysTime);

	sprintf(szDateBuf,
			"%.02u/%.02u/%.02u %.02u:%.02u:%.02u",
			sysTime.wDay, sysTime.wMonth, sysTime.wYear,
			sysTime.wHour, sysTime.wMinute, sysTime.wSecond);

	if(!Utils::GetProcessName(strProc, hProcess))
	{
		char *pszProcName = pwszProcessName ? Utils::MbFromWideChar(pwszProcessName) : NULL;
		if(pszProcName)
			strProc = pszProcName;
		else
			strProc = "(unknown process name)";

		if(pszProcName)
			free(pszProcName);
	}

	for(int i=0; i<strProc.size(); i++)
	{
		if(strProc[i] == '[' || strProc[i] == ']')
			strProc[i] = '_';
	}

	strUniqueProcID = "[" + strProc + "]";
	
	char szPID[32];
	sprintf(szPID, "%u", GetProcessId(hProcess ? hProcess : GetCurrentProcess()));
	
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

	BYTE rgGuidBytes[16];
	WCHAR wszEventGuid[48];

	if(Utils::GetRandomBytes(rgGuidBytes, 16))
	{
		if(!StringFromGUID2((GUID &)rgGuidBytes, wszEventGuid, 48))
		{
			wcscpy(wszEventGuid, L"(error creating event ID)");
		}
	}
	else
	{
		wcscpy(wszEventGuid, L"(error creating event ID)");
	}

	char *pszEventGuid = Utils::MbFromWideChar(wszEventGuid);

	pszTitle = Utils::MbFromWideChar(pwszTitle);
	strXmlTag += " <event date=\"";
	strXmlTag += szDateBuf;
	strXmlTag += "\" procid=\"";
	strXmlTag += strUniqueProcID;
	strXmlTag += "\" eventid=\"";
	strXmlTag += pszEventGuid;
	strXmlTag += "\" risk=\"";
	strXmlTag += m_rgszRiskRatings[risk];
	strXmlTag += "\" title=\"";
	strXmlTag += pszTitle;
	strXmlTag += "\">";

	if(pszEventGuid)
		free(pszEventGuid);

	WCHAR rgwchPreserveChars[] = L"!£$%^*()_+-={}:;@~#,./?\\|";

	for(int i=0; i<8192; i++)
	{
		if(!pwszBody[i])
			break;

		if((pwszBody[i] > 0 && pwszBody[i] <= 0x7f) && (iswalpha(pwszBody[i]) || iswdigit(pwszBody[i]) || iswspace(pwszBody[i]) || wcschr(rgwchPreserveChars, pwszBody[i])))
		{
			strXmlTag += pwszBody[i];
		}
		else if(pwszBody[i] == '"')
		{
			strXmlTag += "&quot;";
		}
		else if(pwszBody[i] == '<')
		{
			strXmlTag += "&lt;";
		}
		else if(pwszBody[i] == '>')
		{
			strXmlTag += "&gt;";
		}
		else
		{
			char tmp[32];
			sprintf(tmp, "&amp;#x%.4x;", pwszBody[i]);
			strXmlTag += tmp;
		}
	}

	strXmlTag += "</event>\r\n</events>";

	WriteFile(hFile, strXmlTag.c_str(), strlen(strXmlTag.c_str()), &dwBytesWritten, NULL);
		
	status = EEventWriteStatus::StatusSuccess;
Cleanup:
	ReleaseMutex(m_hMutex);

	if(hFile != INVALID_HANDLE_VALUE)
		CloseHandle(hFile);

	if(pszTitle)
		free(pszTitle);

	return status;
}

EEventWriteStatus CEventWriter::WriteEventRPC(
		HANDLE hProcess,
		LPWSTR pwszProcessName,
		EEventRepository repos,
		EThreatRiskRating risk,
		LPWSTR pwszTitle,
		LPWSTR pwszBody
	)
{
	EEventWriteStatus status = EEventWriteStatus::StatusFailure;
	
	RPC_WSTR pwszBinding = NULL;
	RPC_STATUS rpcStatus = RpcStringBindingCompose(NULL, (RPC_WSTR)L"ncalrpc", NULL, (RPC_WSTR)L"EventWriterRpc", NULL, &pwszBinding);
	if(rpcStatus != RPC_S_OK)
		goto Done_RPC;
	
	rpcStatus = RpcBindingFromStringBinding(pwszBinding, &::IEventWriterRpc_v1_0_c_ifspec);
	if(rpcStatus != RPC_S_OK)
		goto Done_RPC;
	
	RpcTryExcept
	{
		status = (EEventWriteStatus)::WriteEventInternal(
				::IEventWriterRpc_v1_0_c_ifspec,
				(UINT)hProcess,
				pwszProcessName,
				(UINT)repos,
				(UINT)risk,
				pwszTitle,
				pwszBody
			);
	}
	RpcExcept(1)
	{
	}
	RpcEndExcept
	
	rpcStatus = RpcStringFree(&pwszBinding);
	if(rpcStatus != RPC_S_OK)
		goto Done_RPC;
	
	rpcStatus = RpcBindingFree(&::IEventWriterRpc_v1_0_c_ifspec);
	if(rpcStatus != RPC_S_OK)
		goto Done_RPC;
	
Done_RPC:
	return status;
}

extern "C" __declspec(dllexport) CEventWriterBase *GetEventWriter()
{
	return reinterpret_cast<CEventWriterBase *>(CEventWriter::GetInstance());
}