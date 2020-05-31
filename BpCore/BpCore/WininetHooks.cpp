// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

/*

very important note:
	
	I must make sure that RequestReply and Connection instances are never deleted before
	their respective callback processors have completely finished with them. I had an
	instance of a race condition (which I have not been able to reproduce) in which
	deleting a RequestReply prematurely lead to an AV (uninit variable) in
	CConnection::GetRequestReply().

	Some kind of mutex, or deferred deletion list with thread is in order.
	(NB: GarbageCollector implemented.)

nb:
	HttpAddRequestHeaders(Ex) detour (I definitely miss request headers).

*/

#include "stdafx.h"
#include "BpCore.h"
#include "WininetHooks.h"
#include "Stream.h"
#include "ConnectionPool.h"
#include "..\..\AuxCore\AuxCore\ILoadedModules.h"
#include "..\..\Shared\Utils\Utils.h"
#include "..\..\AuxCore\AuxCore\IProcTrackerServices.h"
#include "..\..\AuxCore\AuxCore\IRealtimeLog.h"
#include "..\..\ProcTracker\ProcTracker\ProcTracker_h.h"
#include "..\..\AuxCore\AuxCore\IConfigReader.h"

BOOL WininetHooks::AttachAll()
{
	//hmBpcore = GetModuleHandle(L"bpcore.dll");
	hmWininet = GetModuleHandle(L"wininet.dll");
	
	IRealtimeLog *pRealtimeLog = GetRealtimeLog();

	if(0)
	{
		pRealtimeLog->LogMessage(1, L"Writing hooks for browser traffic interception");
	}

	DETOUR_TRANS_BEGIN
	ATTACH_DETOUR(InternetConnectA);
	ATTACH_DETOUR(InternetConnectW);
	ATTACH_DETOUR(HttpOpenRequestA);
	ATTACH_DETOUR(HttpOpenRequestW);
	ATTACH_DETOUR(HttpSendRequestA);
	ATTACH_DETOUR(HttpSendRequestW);
	ATTACH_DETOUR(HttpSendRequestExA);
	ATTACH_DETOUR(HttpSendRequestExW);
	ATTACH_DETOUR(HttpQueryInfoA);
	ATTACH_DETOUR(HttpQueryInfoW);
	ATTACH_DETOUR(InternetQueryDataAvailable);
	ATTACH_DETOUR(InternetReadFile);
	ATTACH_DETOUR(InternetReadFileExA);
	//ATTACH_DETOUR(InternetReadFileExW);
	ATTACH_DETOUR(InternetWriteFile);
	ATTACH_DETOUR(InternetCloseHandle);
	LONG lResult = DETOUR_TRANS_COMMIT
	
	IProcTrackerServices *pProcTracker = GetProcTrackerServices();

	switch(lResult)
	{
	case NO_ERROR:
		pRealtimeLog->LogMessage(0, L"Enabled the ContentFilter feature");
		Globals::g_dwEnabledFeatures = pProcTracker->GetEnabledFeaturesProcess(GetCurrentProcessId());
		Globals::g_dwEnabledFeatures |= EEnabledFeatures::ContentFilterFeature;
		pProcTracker->SetEnabledFeaturesProcess(GetCurrentProcessId(), Globals::g_dwEnabledFeatures);
		break;
	case ERROR_INVALID_BLOCK:
		pRealtimeLog->LogMessage(1, L"Failed to enable fully the ContentFilter feature");
		WininetHooks::DetachAll(); // shall we unwind?
		break;
	default:
		// fatal error: rewind
		pRealtimeLog->LogMessage(1, L"Fatal error in ContentFilter feature");
		WininetHooks::DetachAll();
		break;
	}

	IConfigReader *pConfig = GetConfigReader();
	pConfig->Subscribe(ConfigRefresh);

	return TRUE;
}

BOOL WininetHooks::DetachAll()
{
	IRealtimeLog *pRealtimeLog = GetRealtimeLog();

	if(0)
	{
		pRealtimeLog->LogMessage(1, L"Removing hooks for browser traffic interception");
	}

	DETOUR_TRANS_BEGIN
	DETACH_DETOUR(InternetConnectA);
	DETACH_DETOUR(InternetConnectW);
	DETACH_DETOUR(HttpOpenRequestA);
	DETACH_DETOUR(HttpOpenRequestW);
	DETACH_DETOUR(HttpSendRequestA);
	DETACH_DETOUR(HttpSendRequestW);
	DETACH_DETOUR(HttpSendRequestExA);
	DETACH_DETOUR(HttpSendRequestExW);
	DETACH_DETOUR(HttpQueryInfoA);
	DETACH_DETOUR(HttpQueryInfoW);
	DETACH_DETOUR(InternetQueryDataAvailable);
	DETACH_DETOUR(InternetReadFile);
	DETACH_DETOUR(InternetReadFileExA);
	//DETACH_DETOUR(InternetReadFileExW);
	DETACH_DETOUR(InternetWriteFile);
	DETACH_DETOUR(InternetCloseHandle);
	DETOUR_TRANS_COMMIT
	return TRUE;
}

EStreamReadCallbackError WininetHooks::Callback_InternetReadFile(void *pStateParam, PBYTE pbOutputBuffer, size_t cbReadSize, size_t *pcbBytesRead)
{
	EStreamReadCallbackError err = EStreamReadCallbackError::ReadCallbackSuccess;
#ifdef DEBUG_BUILD
	CLogger *logger = NULL;
#endif

	try
	{
#ifdef DEBUG_BUILD
		logger = CLogger::GetInstance("bpcore_wininet");
		logger->ObtainLock();
		logger->Log("Callback_InternetReadFile(pStateParam = 0x%p, cbReadSize = %u)\n", pStateParam, cbReadSize);
#endif
	}
	catch(...)
	{
	}

	DWORD dwReadSize = cbReadSize, dwTotalBytesRead = 0, dwAvailable = 0;
	
	while(dwTotalBytesRead < dwReadSize)
	{
		BOOL bRet = Real_InternetReadFile((HINTERNET)pStateParam, pbOutputBuffer + dwTotalBytesRead,
			dwReadSize - dwTotalBytesRead, &dwAvailable);
		
		if(!bRet)
		{
#ifdef DEBUG_BUILD
			if(logger)
				logger->Log("\tReal_InternetReadFile[1] returned FALSE. GetLastError() == %u\n", GetLastError());
#endif

			if(GetLastError() == ERROR_IO_PENDING)
			{
				while(dwAvailable == 0)
				{
					Sleep(100);
				}
			}
			else
			{
				err = EStreamReadCallbackError::ReadCallbackFailure;
				goto Cleanup;
			}
		}
		
#ifdef DEBUG_BUILD
		if(logger)
			logger->Log("\tReal_InternetReadFile[1] returned TRUE. %u bytes read (%u total)\n", dwAvailable, dwTotalBytesRead);
#endif

		if(!dwAvailable)
			break;

		dwTotalBytesRead += dwAvailable;
	}

	*pcbBytesRead = dwTotalBytesRead;

Cleanup:
#ifdef DEBUG_BUILD
	if(logger)
		logger->ReleaseLock();
#endif

	return err;
}


HINTERNET STDAPICALLTYPE WininetHooks::My_InternetConnectA(
  __in  HINTERNET hInternet,
  __in  const char* lpszServerName,
  __in  INTERNET_PORT nServerPort,
  __in  const char* lpszUsername,
  __in  const char* lpszPassword,
  __in  DWORD dwService,
  __in  DWORD dwFlags,
  __in  DWORD_PTR dwContext
)
{
	HINTERNET hiRet;
	CONNECTION conn;
	CConnectionPool *pConnPool = CConnectionPool::GetInstance();
	
	hiRet = Real_InternetConnectA(hInternet, lpszServerName, nServerPort, lpszUsername,
			lpszPassword, dwService, dwFlags, dwContext);

	conn = (CONNECTION)hiRet;
	
#ifdef DEBUG_BUILD
	CLogger *logger = NULL;
#endif

	try
	{
#ifdef DEBUG_BUILD
		logger = CLogger::GetInstance("bpcore_wininet");
		logger->ObtainLock();
		logger->Log("My_InternetConnectA: RegisterConnection(0x%x)\n", conn);
#endif
	}
	catch(...)
	{
	}

	try
	{
		pConnPool->RegisterConnection(conn);
	}
	catch(...)
	{
		// implementation flaw - find and fix
#ifdef DEBUG_BUILD
		if(logger)
		{
			logger->Log("RegisterConnection threw\n");
			CLogger::Break();
		}
#endif
	}

#ifdef DEBUG_BUILD
	if(logger)
		logger->ReleaseLock();
#endif

	CConnection *pConn = pConnPool->GetConnection(conn);
	if(!pConn)
	{
		// unable to monitor this connection; must inform user. Shouldn't throw.
		throw new std::exception("Missing connection class instance");
	}
	
	pConn->AddRef();

	pConn->SetDstHost(strdup((char*)lpszServerName));
	pConn->SetDstPort(nServerPort);
	pConn->SetDstIP(Utils::Resolve(pConn->GetDstHost()));

	pConn->Release();

	return hiRet;
}

HINTERNET STDAPICALLTYPE WininetHooks::My_InternetConnectW(
  __in  HINTERNET hInternet,
  __in  LPCTSTR lpszServerName,
  __in  INTERNET_PORT nServerPort,
  __in  LPCTSTR lpszUsername,
  __in  LPCTSTR lpszPassword,
  __in  DWORD dwService,
  __in  DWORD dwFlags,
  __in  DWORD_PTR dwContext
)
{
	HINTERNET hiRet;
	CONNECTION conn;
	CConnectionPool *pConnPool = CConnectionPool::GetInstance();
	ILoadedModules *pLoadedMods = GetLoadedModules();
	DWORD dwRetnAddress = 0;

	GETRETNADDR(dwRetnAddress);

	if(pLoadedMods->IsAddressInHModule(hmWininet, (LPVOID)dwRetnAddress))
	{
#ifdef DEBUG_BUILD
		CLogger::Break();
#endif

		// assume the A version is calling the W version:
		hiRet = Real_InternetConnectW(hInternet, lpszServerName, nServerPort, lpszUsername,
				lpszPassword, dwService, dwFlags, dwContext);
		
		return hiRet;
	}

	hiRet = Real_InternetConnectW(hInternet, lpszServerName, nServerPort, lpszUsername,
			lpszPassword, dwService, dwFlags, dwContext);

	conn = (CONNECTION)hiRet;
	
#ifdef DEBUG_BUILD
	CLogger *logger = NULL;
#endif

	try
	{
#ifdef DEBUG_BUILD
		logger = CLogger::GetInstance("bpcore_wininet");
		logger->ObtainLock();
		logger->Log("My_InternetConnectW: RegisterConnection(0x%x)\n", conn);
#endif
	}
	catch(...)
	{
	}

	try
	{
		pConnPool->RegisterConnection(conn);
	}
	catch(...)
	{
		// implementation flaw - find and fix
#ifdef DEBUG_BUILD
		if(logger)
		{
			logger->Log("RegisterConnection threw\n");
		}
#endif
	}

#ifdef DEBUG_BUILD
	if(logger)
		logger->ReleaseLock();
#endif

	CConnection *pConn = pConnPool->GetConnection(conn);
	if(!pConn)
	{
		// unable to monitor this connection; must inform user. Shouldn't throw.
		throw new std::exception("Missing connection class instance");
	}
	
	pConn->AddRef();

	pConn->SetDstHost(Utils::MbFromWideChar((WCHAR*)lpszServerName));
	pConn->SetDstPort(nServerPort);
	pConn->SetDstIP(Utils::Resolve(pConn->GetDstHost()));

	pConn->Release();

	return hiRet;
}

HINTERNET STDAPICALLTYPE WininetHooks::My_HttpOpenRequestA(
  __in  HINTERNET hConnect,
  __in  const char* lpszVerb,
  __in  const char* lpszObjectName,
  __in  const char* lpszVersion,
  __in  const char* lpszReferer,
  __in  const char* *lplpszAcceptTypes,
  __in  DWORD dwFlags,
  __in  DWORD_PTR dwContext
)
{
	HINTERNET hiRet;
	CONNECTION conn = (CONNECTION)hConnect;
	CConnectionPool *pConnPool = CConnectionPool::GetInstance();

#ifdef DEBUG_BUILD
	CLogger *logger = NULL;
#endif

	try
	{
#ifdef DEBUG_BUILD
		logger = CLogger::GetInstance("bpcore_wininet");
		logger->ObtainLock();
		logger->Log("My_HttpOpenRequestA(conn = 0x%x, dwFlags = 0x%x):\n"
			"\tMethod: %s\n\tPath: %s\n", conn, dwFlags, lpszVerb, lpszObjectName);
#endif
	}
	catch(...)
	{
	}

#ifdef DEBUG_BUILD
	if(logger)
		logger->ReleaseLock();
#endif

	CConnection *pConn = pConnPool->GetConnection(conn);
	if(!pConn)
	{
		// unable to monitor this connection; must inform user. Shouldn't throw.
		throw new std::exception("Missing connection class instance");
	}

	pConn->AddRef();

	// this process slows down the browser UI *SIGNIFICANTLY*
	// find better way of checking security. Perhaps just inspect successful HttpOpen call certs?
#ifdef DEBUG_BUILD
	if(0 && dwFlags & INTERNET_FLAG_SECURE)
	{
		HINTERNET hiInetTmp = NULL, hConnTmp = NULL, hiReqTmp = NULL;
		INTERNET_CERTIFICATE_INFO ici;
		DWORD dwIciLength = sizeof(ici);
		
		pConn->SetSSL(TRUE);
		pConn->SetSSLErrors(TRUE);

		if(!(hiInetTmp = InternetOpen(L"SslCheck/1.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0)))
			goto terminate_ssl_check;

		if(!(hConnTmp = Real_InternetConnectA(hiInetTmp, pConn->GetDstHost(), pConn->GetDstPort(), NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0)))
			goto terminate_ssl_check;

		if(!(hiReqTmp = Real_HttpOpenRequestA(hConnTmp, "HEAD", NULL, NULL, NULL, NULL, INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_SECURE, 0)))
			goto terminate_ssl_check;
		
		if(!Real_HttpSendRequestA(hiReqTmp, "", 0, NULL, 0))
			goto terminate_ssl_check;

		if(!InternetQueryOption(hiReqTmp, INTERNET_OPTION_SECURITY_CERTIFICATE_STRUCT, &ici, &dwIciLength))
			goto terminate_ssl_check;

		pConn->SetCertificateInfo(ici);

		// Utils::ValidateSslCertificate does not do a whole lot. The cert info is attached
		// for proper validation by connect callbacks if required.

		if(!Utils::ValidateSslCertificate(pConn->GetDstHost(), ici))
			goto terminate_ssl_check;
		
		ULONG ulInetSecFlags = 0;
		DWORD dwSizeSecFlags = sizeof(ulInetSecFlags);

		if(!InternetQueryOption(hiReqTmp, INTERNET_OPTION_SECURITY_FLAGS, &ulInetSecFlags, &dwSizeSecFlags))
			goto terminate_ssl_check;

		if(ulInetSecFlags & SECURITY_FLAG_STRENGTH_WEAK || // weak SSL cipher
			ulInetSecFlags & SECURITY_FLAG_IGNORE_CERT_CN_INVALID ||
			ulInetSecFlags & SECURITY_FLAG_IGNORE_CERT_DATE_INVALID ||
			ulInetSecFlags & SECURITY_FLAG_IGNORE_REDIRECT_TO_HTTP ||
			//ulInetSecFlags & SECURITY_FLAG_IGNORE_REVOCATION ||
			ulInetSecFlags & SECURITY_FLAG_IGNORE_UNKNOWN_CA ||
			ulInetSecFlags & SECURITY_FLAG_IGNORE_WRONG_USAGE)
		{
			goto terminate_ssl_check;
		}

		pConn->SetSSLErrors(FALSE);

terminate_ssl_check:
		if(hiReqTmp)  Real_InternetCloseHandle(hiReqTmp);
		if(hConnTmp)  Real_InternetCloseHandle(hConnTmp);
		if(hiInetTmp) Real_InternetCloseHandle(hiInetTmp);
	}
#endif

	if( pConn->IsSSL() && (
		dwFlags & INTERNET_FLAG_IGNORE_CERT_CN_INVALID ||
		dwFlags & INTERNET_FLAG_IGNORE_CERT_DATE_INVALID ) )
	{
		// this is really rough. Should parse certificate chain and validate properties.
		pConn->SetSSLErrors(TRUE);
	}

	pConn->SetObjectName(strdup(lpszObjectName));
	pConn->SetHttpVerb(strdup(lpszVerb));
	pConn->SetHttpVersion(strdup(lpszVersion));
	
	BOOL bContinue = !bConnMonEnabled || !bValidateConnect || pConnPool->QueryConnectAllowed(conn);
	//BOOL bContinue = pConnPool->QueryConnectAllowed(conn);
	if(!bContinue)
	{
		// standard error path
		if(pConnPool->ConnectionExists(conn))
			pConnPool->UnregisterConnection(conn); // shouldn't throw, but could given above code
		
		Real_InternetCloseHandle(hConnect);
		hiRet = NULL;
	}
	else
	{
		hiRet = Real_HttpOpenRequestA(hConnect, lpszVerb, lpszObjectName, lpszVersion, NULL,
			lplpszAcceptTypes, dwFlags, dwContext);

		if(hiRet)
		{
			Utils::WinClientVer ver = Utils::GetWinVersion();

			if(ver != Utils::WinClientVer::Windows2000 && ver != Utils::WinClientVer::WindowsXP)
			{
				// assume newer version
				BOOLEAN bDecode = TRUE;
				InternetSetOption(hiRet, INTERNET_OPTION_HTTP_DECODING, &bDecode, sizeof(bDecode));
			}

			REQUESTREPLY reqrepl = (REQUESTREPLY)hiRet;

			pConnPool->RegisterRequestReply(reqrepl);
			CRequestReply *pReqRepl = pConnPool->GetRequestReply(reqrepl);

			if(!pReqRepl)
			{
				// standard error path
				if(pConnPool->ConnectionExists(conn))
					pConnPool->UnregisterConnection(conn); // shouldn't throw, but could given above code
		
				Real_InternetCloseHandle(hConnect);
				Real_InternetCloseHandle(hiRet);
				hiRet = NULL;
			}
			else
			{
				pReqRepl->AddRef();

				pReqRepl->SetConnection(pConn);
				pConn->SetRequestReply(pReqRepl);

				char *pszReferer = strdup(lpszReferer);
				if(pszReferer)
				{
					pReqRepl->SetRequestHeader("Referer", pszReferer);
					free(pszReferer);
				}

				pReqRepl->Release();
			}
		}
	}

	if(pConn)
		pConn->Release();

	return hiRet;
}

HINTERNET STDAPICALLTYPE WininetHooks::My_HttpOpenRequestW(
  __in  HINTERNET hConnect,
  __in  LPCWSTR lpszVerb,
  __in  LPCWSTR lpszObjectName,
  __in  LPCWSTR lpszVersion,
  __in  LPCWSTR lpszReferer,
  __in  LPCWSTR *lplpszAcceptTypes,
  __in  DWORD dwFlags,
  __in  DWORD_PTR dwContext
)
{
	HINTERNET hiRet;
	CONNECTION conn = (CONNECTION)hConnect;
	CConnectionPool *pConnPool = CConnectionPool::GetInstance();
	ILoadedModules *pLoadedMods = GetLoadedModules();
	DWORD dwRetnAddress = 0;

	GETRETNADDR(dwRetnAddress);

	if(pLoadedMods->IsAddressInHModule(hmWininet, (LPVOID)dwRetnAddress))
	{
#ifdef DEBUG_BUILD
		CLogger::Break();
#endif

		// assume the A version is calling the W version:
		hiRet = Real_HttpOpenRequestW(hConnect, lpszVerb, lpszObjectName, lpszVersion, lpszReferer,
				lplpszAcceptTypes, dwFlags, dwContext);

		return hiRet;
	}
	
#ifdef DEBUG_BUILD
	CLogger *logger = NULL;
#endif

	try
	{
#ifdef DEBUG_BUILD
		logger = CLogger::GetInstance("bpcore_wininet");
		logger->ObtainLock();
		logger->Log("My_HttpOpenRequestW(conn = 0x%x, dwFlags = 0x%x): \n", conn, dwFlags);
#endif
	}
	catch(...)
	{
	}

#ifdef DEBUG_BUILD
	if(logger)
		logger->ReleaseLock();
#endif

	CConnection *pConn = pConnPool->GetConnection(conn);
	if(!pConn)
	{
		// unable to monitor this connection; must inform user. Shouldn't throw.
		throw new std::exception("Missing connection class instance");
	}

	pConn->AddRef();
	
	// this process slows down the browser UI *SIGNIFICANTLY*
#ifdef DEBUG_BUILD
	if(0 && dwFlags & INTERNET_FLAG_SECURE)
	{
		HINTERNET hiInetTmp = NULL, hConnTmp = NULL, hiReqTmp = NULL;
		WCHAR *pwszHost = NULL;
		INTERNET_CERTIFICATE_INFO ici;
		DWORD dwIciLength = sizeof(ici);
		
		pConn->SetSSL(TRUE);
		pConn->SetSSLErrors(TRUE);

		if(!(hiInetTmp = InternetOpen(L"SslCheck/1.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0)))
			goto terminate_ssl_check;

		if(!(pwszHost = Utils::WcFromMultiByte(pConn->GetDstHost())))
			goto terminate_ssl_check;
		
		if(!(hConnTmp = Real_InternetConnectW(hiInetTmp, pwszHost, pConn->GetDstPort(), NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0)))
			goto terminate_ssl_check;

		if(!(hiReqTmp = Real_HttpOpenRequestW(hConnTmp, L"HEAD", NULL, NULL, NULL, NULL, INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_SECURE, 0)))
			goto terminate_ssl_check;
		
		if(!Real_HttpSendRequestW(hiReqTmp, L"", 0, NULL, 0))
			goto terminate_ssl_check;

		if(!InternetQueryOption(hiReqTmp, INTERNET_OPTION_SECURITY_CERTIFICATE_STRUCT, &ici, &dwIciLength))
			goto terminate_ssl_check;

		pConn->SetCertificateInfo(ici);

		// Utils::ValidateSslCertificate does not do a whole lot. The cert info is attached
		// for proper validation by connect callbacks if required.

		if(!Utils::ValidateSslCertificate(pConn->GetDstHost(), ici))
			goto terminate_ssl_check;
		
		ULONG ulInetSecFlags = 0;
		DWORD dwSizeSecFlags = sizeof(ulInetSecFlags);

		if(!InternetQueryOption(hiReqTmp, INTERNET_OPTION_SECURITY_FLAGS, &ulInetSecFlags, &dwSizeSecFlags))
			goto terminate_ssl_check;

		if(ulInetSecFlags & SECURITY_FLAG_STRENGTH_WEAK || // weak SSL cipher
			ulInetSecFlags & SECURITY_FLAG_IGNORE_CERT_CN_INVALID ||
			ulInetSecFlags & SECURITY_FLAG_IGNORE_CERT_DATE_INVALID ||
			ulInetSecFlags & SECURITY_FLAG_IGNORE_REDIRECT_TO_HTTP ||
			//ulInetSecFlags & SECURITY_FLAG_IGNORE_REVOCATION ||
			ulInetSecFlags & SECURITY_FLAG_IGNORE_UNKNOWN_CA ||
			ulInetSecFlags & SECURITY_FLAG_IGNORE_WRONG_USAGE)
		{
			goto terminate_ssl_check;
		}

		pConn->SetSSLErrors(FALSE);

terminate_ssl_check:
		if(pwszHost)  free(pwszHost);
		if(hiReqTmp)  Real_InternetCloseHandle(hiReqTmp);
		if(hConnTmp)  Real_InternetCloseHandle(hConnTmp);
		if(hiInetTmp) Real_InternetCloseHandle(hiInetTmp);
	}
#endif

	if( pConn->IsSSL() && (
		dwFlags & INTERNET_FLAG_IGNORE_CERT_CN_INVALID ||
		dwFlags & INTERNET_FLAG_IGNORE_CERT_DATE_INVALID ) )
	{
		// this is really rough. Should parse certificate chain and validate properties.
		pConn->SetSSLErrors(TRUE);
	}

	pConn->SetObjectName(Utils::MbFromWideChar((WCHAR*)lpszObjectName));
	pConn->SetHttpVerb(Utils::MbFromWideChar((WCHAR*)lpszVerb));
	pConn->SetHttpVersion(Utils::MbFromWideChar((WCHAR*)lpszVersion));
	
	BOOL bContinue = !bConnMonEnabled || !bValidateConnect || pConnPool->QueryConnectAllowed(conn);
	//BOOL bContinue = pConnPool->QueryConnectAllowed(conn);
	if(!bContinue)
	{
		// standard error path
		if(pConnPool->ConnectionExists(conn))
			pConnPool->UnregisterConnection(conn); // shouldn't throw, but could given above code
		
		Real_InternetCloseHandle(hConnect);
		hiRet = NULL;
	}
	else
	{
		Utils::WinClientVer ver = Utils::GetWinVersion();

		if(ver != Utils::WinClientVer::Windows2000 && ver != Utils::WinClientVer::WindowsXP)
		{
			// assume newer version
			BOOLEAN bDecode = TRUE;
			InternetSetOption(hiRet, INTERNET_OPTION_HTTP_DECODING, &bDecode, sizeof(bDecode));
		}

		hiRet = Real_HttpOpenRequestW(hConnect, lpszVerb, lpszObjectName, lpszVersion, NULL,
			lplpszAcceptTypes, dwFlags, dwContext);

		if(hiRet)
		{
			REQUESTREPLY reqrepl = (REQUESTREPLY)hiRet;

			pConnPool->RegisterRequestReply(reqrepl);
			CRequestReply *pReqRepl = pConnPool->GetRequestReply(reqrepl);
			
			if(!pReqRepl)
			{
				// standard error path
				if(pConnPool->ConnectionExists(conn))
					pConnPool->UnregisterConnection(conn); // shouldn't throw, but could given above code
		
				Real_InternetCloseHandle(hConnect);
				Real_InternetCloseHandle(hiRet);
				hiRet = NULL;
			}
			else
			{
				pReqRepl->AddRef();

				pReqRepl->SetConnection(pConn);
				pConn->SetRequestReply(pReqRepl);

				char *pszReferer = Utils::MbFromWideChar((WCHAR *)lpszReferer);
				if(pszReferer)
				{
					pReqRepl->SetRequestHeader("Referer", pszReferer);
					free(pszReferer);
				}

				pReqRepl->Release();
			}
		}
	}

	if(pConn)
		pConn->Release();

	return hiRet;
}

BOOL STDAPICALLTYPE WininetHooks::My_HttpSendRequestA(
	__in  HINTERNET hRequest,
	__in  const char* lpszHeaders,
	__in  DWORD dwHeadersLength,
	__in  LPVOID lpOptional,
	__in  DWORD dwOptionalLength
)
{
	BOOL bRet;
	CConnectionPool *pConnPool = CConnectionPool::GetInstance();
#ifdef DEBUG_BUILD
	CLogger *logger = NULL;
#endif
	REQUESTREPLY reqrepl = (REQUESTREPLY)hRequest;

	try
	{
#ifdef DEBUG_BUILD
		logger = CLogger::GetInstance("bpcore_wininet");
		logger->ObtainLock();
		logger->Log("My_HttpSendRequestA\n");
#endif
	}
	catch(...)
	{
	}

	CRequestReply *pReqRepl = pConnPool->GetRequestReply(reqrepl);
	if(!pReqRepl)
	{
		bRet = FALSE;
		goto Cleanup;
		//throw new std::exception("HttpSendRequest no requestreply instance attached");
	}

	pReqRepl->AddRef();

	if(lpOptional)
	{
		pReqRepl->SetRequestInfo(reqrepl, (PBYTE)lpOptional, dwOptionalLength, NULL);
	}
	
	if(lpszHeaders)
	{
		char *pszHeaders = (char *)lpszHeaders;
		
		if(dwHeadersLength != (DWORD)-1)
		{
			pszHeaders = (char *)malloc(dwHeadersLength + 1);
			if(!pszHeaders)
			{
				throw new std::exception("Allocation failure in send request");
			}

			memcpy(pszHeaders, lpszHeaders, dwHeadersLength);
			pszHeaders[dwHeadersLength] = '\0';
		}

		char *pszHdrs = strtok(pszHeaders, "\n");

		while(pszHdrs)
		{
			char *psz = strdup(pszHdrs);

			if(psz)
			{
				size_t cch = strlen(psz);
				char *pszHeader = psz,
						*pszValue = strchr(psz, ':');

				if(psz[cch - 1] == '\r')
				{
					psz[cch - 1] = 0;
				}

				if(pszValue)
				{
					*pszValue = 0;
					pszValue++;
					pszValue += Utils::TrimCount(pszValue);
					pReqRepl->SetRequestHeader(pszHeader, pszValue);
				}

				free(psz);
			}

			pszHdrs = strtok(NULL, "\n");
		}

		if(pszHeaders != (char *)lpszHeaders)
		{
			free(pszHeaders);
		}
	}
	
	BOOL bContinue = !bConnMonEnabled || !bValidateRequest || pConnPool->QueryRequestAllowed(reqrepl);
	//BOOL bContinue = pConnPool->QueryRequestAllowed(reqrepl);
	if(bContinue)
	{
		// add sanitised headers back
		std::map<std::string, std::string>::const_iterator it;
		std::string strHeaders = "";
		char *pszHeaders = NULL;

		// handle gzip etc encoding
		Utils::WinClientVer ver = Utils::GetWinVersion();

		if(ver == Utils::WinClientVer::Windows2000 || ver == Utils::WinClientVer::WindowsXP)
		{
			// assume newer version. value=NULL with overwrite=TRUE removes header
			pReqRepl->SetRequestHeader("Accept-Encoding", NULL, ';', TRUE);
		}

		for(it = pReqRepl->GetRequestHeaders().begin(); it != pReqRepl->GetRequestHeaders().end(); it++)
		{
			strHeaders += it->first + ": " + it->second + "\r\n";
		}

		pszHeaders = strdup(strHeaders.c_str());

		// should we pass original headers if strdup fails? Risks?
		// should read back changes to lpBuffer made by request callback subscribers. This isn't supported at present.
		bRet = Real_HttpSendRequestA(hRequest, pszHeaders, strHeaders.size(), lpOptional, dwOptionalLength);
		if(pszHeaders)
		{
			free(pszHeaders);
		}
	}
	else
	{
		bRet = FALSE; // standard error path
	}

Cleanup:
	if(pReqRepl)
		pReqRepl->Release();

#ifdef DEBUG_BUILD
	if(logger)
		logger->ReleaseLock();
#endif

	return bRet;
}

BOOL STDAPICALLTYPE WininetHooks::My_HttpSendRequestW(
	__in  HINTERNET hRequest,
	__in  LPCTSTR lpszHeaders,
	__in  DWORD dwHeadersLength,
	__in  LPVOID lpOptional,
	__in  DWORD dwOptionalLength
)
{
	BOOL bRet;
	CConnectionPool *pConnPool = CConnectionPool::GetInstance();
#ifdef DEBUG_BUILD
	CLogger *logger = NULL;
#endif
	REQUESTREPLY reqrepl = (REQUESTREPLY)hRequest;
	ILoadedModules *pLoadedMods = GetLoadedModules();
	DWORD dwRetnAddress = 0;

	GETRETNADDR(dwRetnAddress);

	if(pLoadedMods->IsAddressInHModule(hmWininet, (LPVOID)dwRetnAddress))
	{
#ifdef DEBUG_BUILD
		CLogger::Break();
#endif

		// assume the A version is calling the W version:
		bRet = Real_HttpSendRequestW(hRequest, lpszHeaders, dwHeadersLength, lpOptional, dwOptionalLength);
		
		return bRet;
	}

	try
	{
#ifdef DEBUG_BUILD
		logger = CLogger::GetInstance("bpcore_wininet");
		logger->ObtainLock();
		logger->Log("My_HttpSendRequestW\n");
#endif
	}
	catch(...)
	{
	}

	CRequestReply *pReqRepl = pConnPool->GetRequestReply(reqrepl);
	if(!pReqRepl)
	{
#ifdef DEBUG_BUILD
		if(logger)
			logger->Log("\tNo request/reply object attached\n");
#endif

		bRet = FALSE;
		goto Cleanup;
		//throw new std::exception("HttpSendRequest no requestreply instance attached");
	}

	pReqRepl->AddRef();

	if(lpOptional)
	{
#ifdef DEBUG_BUILD
		if(logger)
		{
			//logger->Log("\tOptional data attached:\n");
			//logger->LogHex((PBYTE)lpOptional, dwOptionalLength);
		}
#endif
		pReqRepl->SetRequestInfo(reqrepl, (PBYTE)lpOptional, dwOptionalLength, NULL);
	}
	
	if(lpszHeaders)
	{
		wchar_t *pwszHeaders = (wchar_t *)lpszHeaders;
		
		if(dwHeadersLength != (DWORD)-1)
		{
			pwszHeaders = (wchar_t *)malloc((dwHeadersLength + 1) * sizeof(wchar_t));
			if(!pwszHeaders)
			{
				throw new std::exception("Allocation failure in send request");
			}

			memcpy(pwszHeaders, lpszHeaders, dwHeadersLength* sizeof(wchar_t));
			pwszHeaders[dwHeadersLength] = '\0';
		}

		wchar_t *pwsz = wcstok((wchar_t *)pwszHeaders, L"\n");

		while(pwsz)
		{
			char *psz = Utils::MbFromWideChar((WCHAR *)pwsz);

			if(psz)
			{
				size_t cch = strlen(psz);
				char *pszHeader = psz,
						*pszValue = strchr(psz, ':');

				if(psz[cch - 1] == '\r')
				{
					psz[cch - 1] = 0;
				}

				if(pszValue)
				{
					*pszValue = 0;
					pszValue++;
					pszValue += Utils::TrimCount(pszValue);
					pReqRepl->SetRequestHeader(pszHeader, pszValue);

#ifdef DEBUG_BUILD
					if(logger)
						logger->Log("\tSetting request header (%s) value (%s)\n", pszHeader, pszValue);
#endif
				}

				free(psz);
			}

			pwsz = wcstok(NULL, L"\n");
		}

		if(pwszHeaders != (wchar_t *)lpszHeaders)
		{
			free(pwszHeaders);
		}
	}
	
	BOOL bContinue = !bConnMonEnabled || !bValidateRequest || pConnPool->QueryRequestAllowed(reqrepl);
	//BOOL bContinue = pConnPool->QueryRequestAllowed(reqrepl);
	if(bContinue)
	{
#ifdef DEBUG_BUILD
		if(logger)
			logger->Log("\tQueryRequestAllowed returned true\n");
#endif

		// add sanitised headers back
		std::map<std::string, std::string>::const_iterator it;
		std::string strHeaders = "";
		WCHAR *pwszHeaders = NULL;

		// handle gzip etc encoding
		Utils::WinClientVer ver = Utils::GetWinVersion();

		if(ver == Utils::WinClientVer::Windows2000 || ver == Utils::WinClientVer::WindowsXP)
		{
			// assume newer version. value=NULL with overwrite=TRUE removes header
			pReqRepl->SetRequestHeader("Accept-Encoding", NULL, ';', TRUE);
		}

		for(it = pReqRepl->GetRequestHeaders().begin(); it != pReqRepl->GetRequestHeaders().end(); it++)
		{
			strHeaders += it->first + ": " + it->second + "\r\n";
		}

#ifdef DEBUG_BUILD
		if(logger && strHeaders.size())
			logger->Log("\tRequest headers read back:\n%s\n", strHeaders.c_str());
#endif

		pwszHeaders = Utils::WcFromMultiByte(const_cast<char *>(strHeaders.c_str()));

		// should we pass original headers if WcFromMultiByte fails? Risks?
		// should read back changes to lpBuffer made by request callback subscribers. This isn't supported at present.
		bRet = Real_HttpSendRequestW(hRequest, pwszHeaders, strHeaders.size(), lpOptional, dwOptionalLength);
		if(pwszHeaders)
		{
			free(pwszHeaders);
		}

#ifdef DEBUG_BUILD
		if(logger)
			logger->Log("\treturned: %s\n", bRet ? "TRUE" : "FALSE");
#endif
	}
	else
	{
#ifdef DEBUG_BUILD
		if(logger)
			logger->Log("\tQueryRequestAllowed returned false\n");
#endif

		bRet = FALSE; // standard error path
	}

Cleanup:
	if(pReqRepl)
		pReqRepl->Release();

#ifdef DEBUG_BUILD
	if(logger)
		logger->ReleaseLock();
#endif

	return bRet;
}

BOOL STDAPICALLTYPE WininetHooks::My_HttpSendRequestExA(
  __in   HINTERNET hRequest,
  __in   LPINTERNET_BUFFERSA lpBuffersIn,
  __out  LPINTERNET_BUFFERSA lpBuffersOut,
  __in   DWORD dwFlags,
  __in   DWORD_PTR dwContext
)
{
	BOOL bRet;
	CConnectionPool *pConnPool = CConnectionPool::GetInstance();
#ifdef DEBUG_BUILD
	CLogger *logger = NULL;
#endif
	REQUESTREPLY reqrepl = (REQUESTREPLY)hRequest;
	ILoadedModules *pLoadedMods = GetLoadedModules();
	DWORD dwRetnAddress = 0;
	LPINTERNET_BUFFERSA lpBufferDupWithHeaders = NULL, lpBuffersTmp = NULL;
	
	GETRETNADDR(dwRetnAddress);

	if(pLoadedMods->IsAddressInHModule(hmWininet, (LPVOID)dwRetnAddress))
	{
#ifdef DEBUG_BUILD
		CLogger::Break();
#endif

		// assume the A version is calling the W version:
		bRet = Real_HttpSendRequestExA(hRequest, lpBuffersIn, lpBuffersOut, dwFlags, dwContext);

		return bRet;
	}

	try
	{
#ifdef DEBUG_BUILD
		logger = CLogger::GetInstance("bpcore_wininet");
		logger->ObtainLock();
		logger->Log("My_HttpSendRequestExA [reqrepl = 0x%x]\n", reqrepl);
#endif
	}
	catch(...)
	{
	}

	CRequestReply *pReqRepl = pConnPool->GetRequestReply(reqrepl);
	if(!pReqRepl)
	{
#ifdef DEBUG_BUILD
		if(logger)
			logger->Log("\tNo request/reply object attached\n");
#endif

		bRet = FALSE;
		goto Cleanup;
		//throw new std::exception("HttpSendRequest no requestreply instance attached");
	}

	pReqRepl->AddRef();

	lpBuffersTmp = lpBuffersIn;
	
	while(lpBuffersTmp)
	{
		if(pReqRepl->GetRequestBody() == NULL)
		{
			if(lpBuffersTmp->lpvBuffer && lpBuffersTmp->dwBufferLength)
			{
#ifdef DEBUG_BUILD
				if(logger)
				{
					//logger->Log("\tBuffer data attached:\n");
					//logger->LogHex((PBYTE)lpBuffersTmp->lpvBuffer, lpBuffersTmp->dwBufferLength);
				}
#endif
				pReqRepl->SetRequestInfo(reqrepl, (PBYTE)lpBuffersTmp->lpvBuffer, lpBuffersTmp->dwBufferLength, NULL);
			}
		}
		else
		{
			if(lpBuffersTmp->lpvBuffer && lpBuffersTmp->dwBufferLength)
			{
				CCommonTypeStreamReader *pStream = pReqRepl->GetRequestBody();

#ifdef DEBUG_BUILD
				if(logger)
				{
					//logger->Log("\tBuffer data attached:\n");
					//logger->LogHex((PBYTE)lpBuffersTmp->lpvBuffer, lpBuffersTmp->dwBufferLength);
				}
#endif
				if(pStream->Write(
						pStream->Length(),
						(PBYTE)lpBuffersTmp->lpvBuffer,
						lpBuffersTmp->dwBufferLength,
						TRUE
					) == EDataStreamError::StreamFailure)
				{
					bRet = FALSE;
					goto Cleanup;
				}
			}
		}

		if(lpBuffersTmp->lpcszHeader)
		{
			DWORD dwHeadersLength = lpBuffersTmp->dwHeadersLength;
			LPCSTR lpszHeaders = lpBuffersTmp->lpcszHeader;

			char *pszHeaders = (char *)lpszHeaders;
			
			if(dwHeadersLength != (DWORD)-1)
			{
				pszHeaders = (char *)malloc(dwHeadersLength + 1);
				if(!pszHeaders)
				{
					throw new std::exception("Allocation failure in send request");
				}

				memcpy(pszHeaders, lpszHeaders, dwHeadersLength);
				pszHeaders[dwHeadersLength] = '\0';
			}

			char *psz = strtok((char *)pszHeaders, "\n");

			while(psz)
			{
				size_t cch = strlen(psz);
				char *pszHeader = psz,
						*pszValue = strchr(psz, ':');

				if(psz[cch - 1] == '\r')
				{
					psz[cch - 1] = 0;
				}

				if(pszValue)
				{
					*pszValue = 0;
					pszValue++;
					pszValue += Utils::TrimCount(pszValue);
					pReqRepl->SetRequestHeader(pszHeader, pszValue);

#ifdef DEBUG_BUILD
					if(logger)
						logger->Log("\tSetting request header (%s) value (%s)\n", pszHeader, pszValue);
#endif
				}

				psz = strtok(NULL, "\n");
			}

			if(pszHeaders != (char *)lpszHeaders)
			{
				free(pszHeaders);
				pszHeaders = NULL;
			}
		}

		lpBuffersTmp = lpBuffersTmp->Next;
	}

	BOOL bContinue = !bConnMonEnabled || !bValidateRequest || pConnPool->QueryRequestAllowed(reqrepl);
	//BOOL bContinue = pConnPool->QueryRequestAllowed(reqrepl);
	if(bContinue)
	{
#ifdef DEBUG_BUILD
		if(logger)
			logger->Log("\tQueryRequestAllowed returned true\n");
#endif

		// add sanitised headers back
		std::map<std::string, std::string>::const_iterator it;
		std::string strHeaders = "";
		char *pszHeaders = NULL;

		// handle gzip etc encoding
		Utils::WinClientVer ver = Utils::GetWinVersion();

		if(ver == Utils::WinClientVer::Windows2000 || ver == Utils::WinClientVer::WindowsXP)
		{
			// assume newer version. value=NULL with overwrite=TRUE removes header
			pReqRepl->SetRequestHeader("Accept-Encoding", NULL, ';', TRUE);
		}

		for(it = pReqRepl->GetRequestHeaders().begin(); it != pReqRepl->GetRequestHeaders().end(); it++)
		{
			strHeaders += it->first + ": " + it->second + "\r\n";
		}

#ifdef DEBUG_BUILD
		if(logger && strHeaders.size())
			logger->Log("\tRequest headers read back:\n%s\n", strHeaders.c_str());
#endif
		pszHeaders = strdup(const_cast<char *>(strHeaders.c_str()));
		if(!pszHeaders)
		{
			bRet = FALSE;
			goto Cleanup;
		}

		lpBuffersTmp = lpBuffersIn;
		lpBufferDupWithHeaders = NULL;

		// fixup buffers so that all headers are attached to first buffer instance. Don't forget to free.
		while(lpBuffersTmp)
		{
			if(!lpBufferDupWithHeaders)
			{
				lpBufferDupWithHeaders = (LPINTERNET_BUFFERSA)malloc(sizeof(INTERNET_BUFFERSA));
				if(!lpBufferDupWithHeaders)
				{
					// don't like this, it's messy.
					if(pszHeaders)
						free(pszHeaders);

					pszHeaders = NULL;

					bRet = FALSE;
					goto Cleanup;
				}

				memcpy(lpBufferDupWithHeaders, lpBuffersTmp, sizeof(INTERNET_BUFFERSA));

				lpBufferDupWithHeaders->Next = NULL;
				lpBufferDupWithHeaders->lpcszHeader = pszHeaders;
				lpBufferDupWithHeaders->dwHeadersLength = strHeaders.size(); // cch
			}
			else
			{
				LPINTERNET_BUFFERSA lpBuffersNext = (LPINTERNET_BUFFERSA)malloc(sizeof(INTERNET_BUFFERSA));
				if(!lpBuffersNext)
				{
					if(pszHeaders)
						free(pszHeaders);
					
					pszHeaders = NULL;

					bRet = FALSE;
					goto Cleanup;
				}

				lpBufferDupWithHeaders->Next = lpBuffersNext;

				memcpy(lpBuffersNext, lpBuffersTmp, sizeof(INTERNET_BUFFERSA));

				lpBuffersNext->Next = NULL;
				lpBuffersNext->lpcszHeader = NULL;
				lpBuffersNext->dwHeadersLength = 0;
			}

			lpBuffersTmp = lpBuffersTmp->Next;
		}

		// should we pass original headers if WcFromMultiByte fails? Risks?
		// should read back changes to lpBuffer made by request callback subscribers. This isn't supported at present.
		bRet = Real_HttpSendRequestExA(hRequest, lpBufferDupWithHeaders, NULL, dwFlags, dwContext);
		
#ifdef DEBUG_BUILD
		if(logger)
			logger->Log("\treturned: %s\n", bRet ? "TRUE" : "FALSE");
#endif
		if(pszHeaders)
			free(pszHeaders);
	}
	else
	{
#ifdef DEBUG_BUILD
		if(logger)
			logger->Log("\tQueryRequestAllowed returned false\n");
#endif
		bRet = FALSE; // standard error path
	}

Cleanup:
	while(lpBufferDupWithHeaders)
	{
		lpBuffersTmp = lpBufferDupWithHeaders;
		lpBufferDupWithHeaders = lpBufferDupWithHeaders->Next;
		free(lpBuffersTmp);
	}
	
	if(pReqRepl)
		pReqRepl->Release();

#ifdef DEBUG_BUILD
	if(logger)
		logger->ReleaseLock();
#endif

	return bRet;
}

BOOL STDAPICALLTYPE WininetHooks::My_HttpSendRequestExW(
  __in   HINTERNET hRequest,
  __in   LPINTERNET_BUFFERSW lpBuffersIn,
  __out  LPINTERNET_BUFFERSW lpBuffersOut,
  __in   DWORD dwFlags,
  __in   DWORD_PTR dwContext
)
{
	BOOL bRet;
	CConnectionPool *pConnPool = CConnectionPool::GetInstance();
#ifdef DEBUG_BUILD
	CLogger *logger = NULL;
#endif
	REQUESTREPLY reqrepl = (REQUESTREPLY)hRequest;
	ILoadedModules *pLoadedMods = GetLoadedModules();
	DWORD dwRetnAddress = 0;
	LPINTERNET_BUFFERSW lpBufferDupWithHeaders = NULL, lpBuffersTmp = NULL;
	
	GETRETNADDR(dwRetnAddress);

	if(pLoadedMods->IsAddressInHModule(hmWininet, (LPVOID)dwRetnAddress))
	{
#ifdef DEBUG_BUILD
		CLogger::Break();
#endif

		// assume the A version is calling the W version:
		bRet = Real_HttpSendRequestExW(hRequest, lpBufferDupWithHeaders, NULL, dwFlags, dwContext);
		
		return bRet;
	}

	try
	{
#ifdef DEBUG_BUILD
		logger = CLogger::GetInstance("bpcore_wininet");
		logger->ObtainLock();
		logger->Log("My_HttpSendRequestExW [reqrepl = 0x%x]\n", reqrepl);
#endif
	}
	catch(...)
	{
	}

	CRequestReply *pReqRepl = pConnPool->GetRequestReply(reqrepl);
	if(!pReqRepl)
	{
#ifdef DEBUG_BUILD
		if(logger)
			logger->Log("\tNo request/reply object attached\n");
#endif

		bRet = FALSE;
		goto Cleanup;
		//throw new std::exception("HttpSendRequest no requestreply instance attached");
	}

	pReqRepl->AddRef();

	lpBuffersTmp = lpBuffersIn;
	
	while(lpBuffersTmp)
	{
		if(pReqRepl->GetRequestBody() == NULL)
		{
			if(lpBuffersTmp->lpvBuffer && lpBuffersTmp->dwBufferLength)
			{
#ifdef DEBUG_BUILD
				if(logger)
				{
					//logger->Log("\tBuffer data attached:\n");
					//logger->LogHex((PBYTE)lpBuffersTmp->lpvBuffer, lpBuffersTmp->dwBufferLength);
				}
#endif

				pReqRepl->SetRequestInfo(reqrepl, (PBYTE)lpBuffersTmp->lpvBuffer, lpBuffersTmp->dwBufferLength, NULL);
			}
		}
		else
		{
			if(lpBuffersTmp->lpvBuffer && lpBuffersTmp->dwBufferLength)
			{
				CCommonTypeStreamReader *pStream = pReqRepl->GetRequestBody();

#ifdef DEBUG_BUILD
				if(logger)
				{
					//logger->Log("\tBuffer data attached:\n");
					//logger->LogHex((PBYTE)lpBuffersTmp->lpvBuffer, lpBuffersTmp->dwBufferLength);
				}
#endif

				if(pStream->Write(
						pStream->Length(),
						(PBYTE)lpBuffersTmp->lpvBuffer,
						lpBuffersTmp->dwBufferLength,
						TRUE
					) == EDataStreamError::StreamFailure)
				{
					bRet = FALSE;
					goto Cleanup;
				}
			}
		}

		if(lpBuffersTmp->lpcszHeader)
		{
			DWORD dwHeadersLength = lpBuffersTmp->dwHeadersLength;
			LPCWSTR lpszHeaders = lpBuffersTmp->lpcszHeader;

			wchar_t *pwszHeaders = (wchar_t *)lpszHeaders;
			
			if(dwHeadersLength != (DWORD)-1)
			{
				pwszHeaders = (wchar_t *)malloc((dwHeadersLength + 1) * sizeof(wchar_t));
				if(!pwszHeaders)
				{
					throw new std::exception("Allocation failure in send request");
				}

				memcpy(pwszHeaders, lpszHeaders, dwHeadersLength* sizeof(wchar_t));
				pwszHeaders[dwHeadersLength] = '\0';
			}

			wchar_t *pwsz = wcstok((wchar_t *)pwszHeaders, L"\n");

			while(pwsz)
			{
				char *psz = Utils::MbFromWideChar((WCHAR *)pwsz);

				if(psz)
				{
					size_t cch = strlen(psz);
					char *pszHeader = psz,
							*pszValue = strchr(psz, ':');

					if(psz[cch - 1] == '\r')
					{
						psz[cch - 1] = 0;
					}

					if(pszValue)
					{
						*pszValue = 0;
						pszValue++;
						pszValue += Utils::TrimCount(pszValue);
						pReqRepl->SetRequestHeader(pszHeader, pszValue);

#ifdef DEBUG_BUILD
						if(logger)
							logger->Log("\tSetting request header (%s) value (%s)\n", pszHeader, pszValue);
#endif
					}

					free(psz);
					psz = NULL;
				}

				pwsz = wcstok(NULL, L"\n");
			}

			if(pwszHeaders != (wchar_t *)lpszHeaders)
			{
				free(pwszHeaders);
				pwszHeaders = NULL;
			}
		}

		lpBuffersTmp = lpBuffersTmp->Next;
	}

	BOOL bContinue = !bConnMonEnabled || !bValidateRequest || pConnPool->QueryRequestAllowed(reqrepl);
	//BOOL bContinue = pConnPool->QueryRequestAllowed(reqrepl);
	if(bContinue)
	{
#ifdef DEBUG_BUILD
		if(logger)
			logger->Log("\tQueryRequestAllowed returned true\n");
#endif

		// add sanitised headers back
		std::map<std::string, std::string>::const_iterator it;
		std::string strHeaders = "";
		WCHAR *pwszHeaders = NULL;

		// handle gzip etc encoding
		Utils::WinClientVer ver = Utils::GetWinVersion();

		if(ver == Utils::WinClientVer::Windows2000 || ver == Utils::WinClientVer::WindowsXP)
		{
			// assume newer version. value=NULL with overwrite=TRUE removes header
			pReqRepl->SetRequestHeader("Accept-Encoding", NULL, ';', TRUE);
		}

		for(it = pReqRepl->GetRequestHeaders().begin(); it != pReqRepl->GetRequestHeaders().end(); it++)
		{
			strHeaders += it->first + ": " + it->second + "\r\n";
		}

#ifdef DEBUG_BUILD
		if(logger && strHeaders.size())
			logger->Log("\tRequest headers read back:\n%s\n", strHeaders.c_str());
#endif

		pwszHeaders = Utils::WcFromMultiByte(const_cast<char *>(strHeaders.c_str()));
		if(!pwszHeaders)
		{
			bRet = FALSE;
			goto Cleanup;
		}

		lpBuffersTmp = lpBuffersIn;
		
		// fixup buffers so that all headers are attached to first buffer instance. Don't forget to free.
		while(lpBuffersTmp)
		{
			if(!lpBufferDupWithHeaders)
			{
				lpBufferDupWithHeaders = (LPINTERNET_BUFFERSW)malloc(sizeof(INTERNET_BUFFERSW));
				if(!lpBufferDupWithHeaders)
				{
					// don't like this, it's messy.
					if(pwszHeaders)
						free(pwszHeaders);

					pwszHeaders = NULL;

					bRet = FALSE;
					goto Cleanup;
				}

				memcpy(lpBufferDupWithHeaders, lpBuffersTmp, sizeof(INTERNET_BUFFERSW));

				lpBufferDupWithHeaders->Next = NULL;
				lpBufferDupWithHeaders->lpcszHeader = pwszHeaders;
				lpBufferDupWithHeaders->dwHeadersLength = strHeaders.size(); // cch
			}
			else
			{
				LPINTERNET_BUFFERSW lpBuffersNext = (LPINTERNET_BUFFERSW)malloc(sizeof(INTERNET_BUFFERSW));
				if(!lpBuffersNext)
				{
					if(pwszHeaders)
						free(pwszHeaders);
					
					pwszHeaders = NULL;

					bRet = FALSE;
					goto Cleanup;
				}

				lpBufferDupWithHeaders->Next = lpBuffersNext;

				memcpy(lpBuffersNext, lpBuffersTmp, sizeof(INTERNET_BUFFERSW));

				lpBuffersNext->Next = NULL;
				lpBuffersNext->lpcszHeader = NULL;
				lpBuffersNext->dwHeadersLength = 0;
			}

			lpBuffersTmp = lpBuffersTmp->Next;
		}

		// should we pass original headers if WcFromMultiByte fails? Risks?
		// should read back changes to lpBuffer made by request callback subscribers. This isn't supported at present.
		bRet = Real_HttpSendRequestExW(hRequest, lpBufferDupWithHeaders, NULL, dwFlags, dwContext);
		
#ifdef DEBUG_BUILD
		if(logger)
			logger->Log("\treturned: %s\n", bRet ? "TRUE" : "FALSE");
#endif

		if(pwszHeaders)
			free(pwszHeaders);
	}
	else
	{
#ifdef DEBUG_BUILD
		if(logger)
			logger->Log("\tQueryRequestAllowed returned false\n");
#endif

		bRet = FALSE; // standard error path
	}

Cleanup:
	while(lpBufferDupWithHeaders)
	{
		lpBuffersTmp = lpBufferDupWithHeaders;
		lpBufferDupWithHeaders = lpBufferDupWithHeaders->Next;
		free(lpBuffersTmp);
	}
	
	if(pReqRepl)
		pReqRepl->Release();

#ifdef DEBUG_BUILD
	if(logger)
		logger->ReleaseLock();
#endif

	return bRet;
}

BOOL STDAPICALLTYPE WininetHooks::My_HttpQueryInfoA(
  __in     HINTERNET hRequest,
  __in     DWORD dwInfoLevel,
  __inout  LPVOID lpvBuffer,
  __inout  LPDWORD lpdwBufferLength,
  __inout  LPDWORD lpdwIndex
)
{
	BOOL bRet;
	DWORD dwInformationLevel = (dwInfoLevel & 0xffff);
	REQUESTREPLY reqrepl = (REQUESTREPLY)hRequest;
	CConnectionPool *pConnPool = CConnectionPool::GetInstance();
	CRequestReply *pReqRepl = NULL;
#ifdef DEBUG_BUILD
	CLogger *logger = NULL;
#endif
	std::multimap<std::string, std::string>::const_iterator it;
	std::string strHeaders = "";
	BOOL bFirstHeader = TRUE;
	
	DWORD dwRequiredBufferSize = *lpdwBufferLength;
	PBYTE pbRequestHeaders = (PBYTE)lpvBuffer;
	
	std::vector<char *> *pvecCType = NULL;
	char *pszTmp = NULL;
	char *psz = NULL, *pszHeader = NULL, *pszValue = NULL;
	size_t cch = 0;

	ILoadedModules *pLoadedMods = GetLoadedModules();
	DWORD dwRetnAddress = 0;

	GETRETNADDR(dwRetnAddress);

	if(pLoadedMods->IsAddressInHModule(hmWininet, (LPVOID)dwRetnAddress))
	{
#ifdef DEBUG_BUILD
		CLogger::Break();
#endif

		// assume the A version is calling the W version:
		bRet = Real_HttpQueryInfoA(hRequest, dwInfoLevel, lpvBuffer, lpdwBufferLength, lpdwIndex);

		return bRet;
	}

	try
	{
		pReqRepl = pConnPool->GetRequestReply(reqrepl);
		if(!pReqRepl)
		{
			bRet = FALSE;
			goto Cleanup;
		}

		pReqRepl->AddRef();
	}
	catch(...)
	{
		bRet = FALSE;
		goto Cleanup;
	}

	try
	{
#ifdef DEBUG_BUILD
		logger = CLogger::GetInstance("bpcore_wininet");
		logger->ObtainLock();
		logger->Log("My_HttpQueryInfoA(reqrepl = 0x%x; dwInformationLevel = %u, *lpdwBufferLength = %u)\n", reqrepl, dwInformationLevel, *lpdwBufferLength);
#endif
	}
	catch(...)
	{
	}

	if(!pReqRepl)
	{
		throw new std::exception("HttpQueryInfo no requestreply instance attached");
	}

	bRet = Real_HttpQueryInfoA(hRequest, dwInfoLevel, lpvBuffer, &dwRequiredBufferSize, lpdwIndex);
	if(!bRet)
	{
		// if the only problem is that the user has supplied a buffer of insufficient size
		// allocate buffer, read headers into buffer and query request allowed, and fail
		// response early

#ifdef DEBUG_BUILD
		if(logger) logger->Log("\tReal_HttpQueryInfoA returned false\n", dwRequiredBufferSize);
#endif

		if(GetLastError() == ERROR_INSUFFICIENT_BUFFER)
		{
#ifdef DEBUG_BUILD
			if(logger) logger->Log("\tGetLastError() == ERROR_INSUFFICIENT_BUFFER [dwRequiredBufferSize = %u]\n", dwRequiredBufferSize);
#endif
			switch(dwInformationLevel)
			{
			case HTTP_QUERY_CONTENT_TYPE:
				// add code to query for this header in the reply headers, if found, read this value out instead
				// of executing Real_HttpQueryInfo
				
				pvecCType = pReqRepl->GetReplyHeader("Content-Type");
				if(pvecCType)
				{
					psz = pvecCType->at(0);
					*lpdwBufferLength = strlen(psz);

					delete pvecCType;
					goto Cleanup;
				}

				// fallthrough
			case HTTP_QUERY_RAW_HEADERS:
			case HTTP_QUERY_RAW_HEADERS_CRLF:
				
#ifdef DEBUG_BUILD
				if(logger) logger->Log("\tResizing buffer to %u bytes\n", dwRequiredBufferSize);
#endif
				pbRequestHeaders = (PBYTE)malloc(dwRequiredBufferSize);
				if(!pbRequestHeaders)
				{
					goto Cleanup;
				}
				
				bRet = Real_HttpQueryInfoA(hRequest, dwInfoLevel, pbRequestHeaders, &dwRequiredBufferSize, lpdwIndex);
				if(!bRet)
				{
#ifdef DEBUG_BUILD
					if(logger) logger->Log("\tReal_HttpQueryInfoA still errors. Aborting.\n");
#endif
					goto Cleanup;
				}

#ifdef DEBUG_BUILD
				if(logger) logger->Log("\tData read (%u bytes)\n", dwRequiredBufferSize);
#endif
				break;
			default:
				goto Cleanup;
			}
		}
		else
		{
			goto Cleanup;
		}
	}

	switch(dwInformationLevel)
	{
	case HTTP_QUERY_CONTENT_TYPE:
#ifdef DEBUG_BUILD
		if(logger) logger->Log("\tHTTP_QUERY_CONTENT_TYPE ");
#endif
		pvecCType = pReqRepl->GetReplyHeader("Content-Type");
		if(pvecCType)
		{
#ifdef DEBUG_BUILD
			if(logger) logger->Log("already exists. Skipping read out.\n");
#endif
			delete pvecCType;
			break;
		}

#ifdef DEBUG_BUILD
		if(logger) logger->Log("does not yet exist. Reading from header store.\n");
#endif
		cch = dwRequiredBufferSize;

		pszTmp = (char *)malloc(cch + 1);
		if(!pszTmp)
		{
			throw new std::exception("Allocation failure in query info");
		}

		memcpy(pszTmp, pbRequestHeaders, dwRequiredBufferSize);
		pszTmp[cch] = '\0';

#ifdef DEBUG_BUILD
		if(logger) logger->Log("\tContent-Type header length %u bytes\n", dwRequiredBufferSize);
#endif
		psz = strdup(pszTmp);
		if(!psz)
		{
			free(pszTmp);
			throw new std::exception("Allocation failure in query info");
		}

#ifdef DEBUG_BUILD
		if(logger) logger->Log("\tContent-Type being read '%s'\n", psz);
#endif
		pszHeader = "Content-Type";
		pszValue = psz;
		
		pReqRepl->SetReplyHeader(pszHeader, pszValue);

		free(pszTmp);
		free(psz);

		break;
	case HTTP_QUERY_RAW_HEADERS:
#ifdef DEBUG_BUILD
		if(logger) logger->Log("\tHTTP_QUERY_RAW_HEADERS ");
#endif
		if(pReqRepl->GetReplyHeaders().size() != 0)
		{
#ifdef DEBUG_BUILD
			if(logger) logger->Log("already read.\n");
#endif
			// already extracted headers
			break;
		}

#ifdef DEBUG_BUILD
		if(logger) logger->Log("reading headers.\n");
#endif
		pszTmp = (char *)pbRequestHeaders;
		
		while(pszTmp)
		{
			if(strlen(pszTmp) == 0)
			{
				break; // end of array
			}

			psz = strdup(pszTmp);
			if(!psz)
			{
				throw new std::exception("Allocation failure in query info");
			}

#ifdef DEBUG_BUILD
			if(logger) logger->Log("\tFound Header: '%s'\n", psz);
#endif			
			if(bFirstHeader == TRUE)
			{
				pReqRepl->SetReplyBanner(psz);
				bFirstHeader = FALSE;
			}
			else
			{
				pszHeader = psz;
				pszValue = strchr(psz, ':');

				if(pszValue)
				{
					*pszValue = 0;
					pszValue++;
					pszValue += Utils::TrimCount(pszValue);
					pReqRepl->SetReplyHeader(pszHeader, pszValue);
				}
			}

			free(psz);
			
			pszTmp = pszTmp + strlen(pszTmp) + 1;
		}

		break;
	case HTTP_QUERY_RAW_HEADERS_CRLF:
#ifdef DEBUG_BUILD
		if(logger) logger->Log("\tHTTP_QUERY_RAW_HEADERS_CRLF");
#endif
		if(pReqRepl->GetReplyHeaders().size() != 0)
		{
#ifdef DEBUG_BUILD
			if(logger) logger->Log("already read.\n");
#endif
			// already extracted headers
			break;
		}

#ifdef DEBUG_BUILD
		if(logger) logger->Log("reading headers.\n");
#endif
		pReqRepl = pConnPool->GetRequestReply(reqrepl);
		if(!pReqRepl)
			break;

		pszTmp = strtok((char *)pbRequestHeaders, "\n");

		while(pszTmp)
		{
			psz = strdup(pszTmp);

			if(psz)
			{
				cch = strlen(psz);
				pszHeader = psz;
				pszValue = strchr(psz, ':');

#ifdef DEBUG_BUILD
				if(logger) logger->Log("\tHeader Found: '%s'", psz);
#endif
				if(psz[cch - 1] == '\r')
				{
					psz[cch - 1] = 0;
				}

				if(bFirstHeader == TRUE)
				{
					pReqRepl->SetReplyBanner(psz);
					bFirstHeader = FALSE;
				}
				else
				{
					if(pszValue)
					{
						*pszValue = 0;
						pszValue++;
						pszValue += Utils::TrimCount(pszValue);
						pReqRepl->SetReplyHeader(pszHeader, pszValue);
					}
				}

				free(psz);
			}

			pszTmp = strtok(NULL, "\n");
		}

		break;
	default:
		goto Cleanup; // if it's not a query for headers or other supported info: return
	}

	char *pszHeaders = NULL;
	size_t cchHeaders = 0;
	bFirstHeader = TRUE;

	BOOL bContinue = !bConnMonEnabled || !bValidateResponse || pConnPool->QueryResponseAllowed(reqrepl);
	//BOOL bContinue = pConnPool->QueryResponseAllowed(reqrepl);
	if(bContinue)
	{
#ifdef DEBUG_BUILD
		if(logger) logger->Log("\tQueryResponseAllowed returned true\n");
#endif
		switch(dwInformationLevel)
		{
		case HTTP_QUERY_CONTENT_TYPE:
#ifdef DEBUG_BUILD
			if(logger) logger->Log("\tHTTP_QUERY_CONTENT_TYPE ");
#endif
			pvecCType = pReqRepl->GetReplyHeader("Content-Type");
			if(pvecCType)
			{
#ifdef DEBUG_BUILD
				if(logger) logger->Log("found header.\n");
#endif
				pszTmp = strdup(pvecCType->at(0)); // first one - risks?
				if(!pszTmp)
				{
					throw new std::exception("Allocation failure in query info");
				}

				cch = strlen(pszTmp);

#ifdef DEBUG_BUILD
				if(logger) logger->Log("\tValue: '%s'. dwRequiredBufferSize = %u; cch = %u\n", pszTmp, dwRequiredBufferSize, cch);
#endif
				//if(pbRequestHeaders == lpvBuffer)
				if(cch <= dwRequiredBufferSize)
				{
#ifdef DEBUG_BUILD
					if(logger) logger->Log("\tCaller allocated sufficient buffer. Copying out.\n");
#endif
					// the caller allocated a sufficiently large buffer
					memcpy(lpvBuffer, pszTmp, cch);
					*lpdwBufferLength = cch;
				}
				else
				{
					// we need to wait for caller to request actual header data, return length only

#ifdef DEBUG_BUILD
					if(logger) logger->Log("\tCaller did not allocate sufficiently large output buffer. Advising %u bytes\n", cch);
#endif
					*lpdwBufferLength = cch;
				}

				delete pvecCType;
				free(pszTmp);
			}
			else
			{
				// not exactly a failure, but this is expected behaviour

#ifdef DEBUG_BUILD
				if(logger) logger->Log("\tContent-Type header not found. Returning false.\n");
#endif
				*lpdwBufferLength = 0;
				bRet = FALSE;
				
				goto Cleanup;
			}

			break;
		case HTTP_QUERY_RAW_HEADERS:
#ifdef DEBUG_BUILD
			if(logger) logger->Log("\tHTTP_QUERY_RAW_HEADERS\n");
#endif
			// add sanitised headers back
			
			cchHeaders = 0;
			
			for(it = pReqRepl->GetReplyHeaders().begin(); it != pReqRepl->GetReplyHeaders().end(); it++)
			{
				strHeaders = "";

				if(bFirstHeader)
				{
					strHeaders = pReqRepl->GetReplyBanner();
					cch = strlen(strHeaders.c_str());

					pszHeaders = (char *)realloc(pszHeaders, cch + 2);
					if(!pszHeaders || cch > Utils::ARBITRARY_STRING_SIZE_LIMIT )
					{
						throw new std::exception("HttpQueryInfoA allocation failure");
					}

					pszTmp = strdup(const_cast<char *>(strHeaders.c_str()));
					if(!pszTmp)
					{
						throw new std::exception("HttpQueryInfoA allocation failure");
					}
					
#ifdef DEBUG_BUILD
					if(logger) logger->Log("\tAdded Header: '%s'\n", pszTmp);
#endif
					memcpy(pszHeaders, pszTmp, cch);
					pszHeaders[cchHeaders + cch] = '\0';

					cchHeaders += cch;
					cchHeaders++; // move past the NULL byte
					free(pszTmp);

					strHeaders = "";
					bFirstHeader = FALSE;
				}

				strHeaders += it->first + ": " + it->second;
				cch = strlen(strHeaders.c_str());

				pszHeaders = (char *)realloc(pszHeaders, cchHeaders + cch + 2);
				if(!pszHeaders || cchHeaders > Utils::ARBITRARY_STRING_SIZE_LIMIT )
				{
					throw new std::exception("HttpQueryInfoA allocation failure");
				}

				pszTmp = strdup(const_cast<char *>(strHeaders.c_str()));
				if(!pszTmp)
				{
					throw new std::exception("HttpQueryInfoA allocation failure");
				}

#ifdef DEBUG_BUILD
				if(logger) logger->Log("\tAdded Header: '%s'\n", pszTmp);
#endif				
				memcpy(pszHeaders + cchHeaders, pszTmp, cch);
				pszHeaders[cchHeaders + cch] = '\0';
				
				cchHeaders += cch;
				cchHeaders++; // move past the NULL byte
				free(pszTmp);
			}

			pszHeaders[cchHeaders] = '\0'; // terminated by double NULL
			cchHeaders++;
			
			if(cchHeaders <= *lpdwBufferLength)
			{
#ifdef DEBUG_BUILD
				if(logger) logger->Log("\tCaller allocated sufficiently large buffer. Copying %u bytes\n", cchHeaders);
#endif
				// the caller allocated a sufficiently large buffer
				memcpy(lpvBuffer, pszHeaders, cchHeaders);
				*lpdwBufferLength = cchHeaders;
			}
			else
			{
#ifdef DEBUG_BUILD
				if(logger) logger->Log("\tCaller did not allocate sufficiently large buffer. Advising %u bytes\n", cchHeaders);
#endif
				// we need to wait for caller to request actual header data, return length only
				*lpdwBufferLength = cchHeaders;
			}

			if(pszHeaders)
			{
				free(pszHeaders);
			}

			break;
		case HTTP_QUERY_RAW_HEADERS_CRLF:
#ifdef DEBUG_BUILD
			if(logger) logger->Log("\tHTTP_QUERY_RAW_HEADERS_CRLF\n");
#endif
			// add sanitised headers back
			
			cchHeaders = 0;
			
			for(it = pReqRepl->GetReplyHeaders().begin(); it != pReqRepl->GetReplyHeaders().end(); it++)
			{
				if(bFirstHeader)
				{
					strHeaders = pReqRepl->GetReplyBanner();
					strHeaders += "\r\n";
					cchHeaders += strlen(strHeaders.c_str());
					bFirstHeader = FALSE;
				}

				std::string strSingleHeader = it->first + ": " + it->second + "\r\n";
				strHeaders += strSingleHeader;
				cchHeaders += strlen(strSingleHeader.c_str());
			}

			strHeaders += "\r\n"; // double crlf terminated
			cchHeaders += strlen("\r\n");

			pszHeaders = strdup(const_cast<char *>(strHeaders.c_str()));
			if(!pszHeaders)
			{
				throw new std::exception("HttpQueryInfoA allocation failure");
			}
			
			// should we pass original headers if WcFromMultiByte fails? Risks?
			
			// copy headers into lpBuffer ...

#ifdef DEBUG_BUILD
			if(logger) logger->Log("\tHeader size %u bytes.\n", cchHeaders);
#endif
			if(cchHeaders <= *lpdwBufferLength)
			{
#ifdef DEBUG_BUILD
				if(logger) logger->Log("\tCaller allocated sufficiently large buffer. Copying %u bytes\n", cchHeaders);
#endif
				// the caller allocated a sufficiently large buffer
				memcpy(lpvBuffer, pszHeaders, cchHeaders);
				*lpdwBufferLength = cchHeaders;
			}
			else
			{
#ifdef DEBUG_BUILD
				if(logger) logger->Log("\tCaller did not allocate sufficiently large buffer. Advising %u bytes\n", cchHeaders);
#endif
				// we need to wait for caller to request actual header data, return length only
				*lpdwBufferLength = cchHeaders;
			}

			if(pszHeaders)
			{
				free(pszHeaders);
			}

			break;
		default:
			goto Cleanup; // should never reach
		}
	}
	else
	{
		// standard error path
#ifdef DEBUG_BUILD
		if(logger && bRet)
		{
			logger->Log("*lpdwBufferLength = 0x%x\n", *lpdwBufferLength);
			//logger->LogHex((PBYTE)lpvBuffer, *lpdwBufferLength);
		}

		if(logger)
			logger->Log("\tFailing query info attempt\n");
#endif
		*lpdwBufferLength = 0;
		bRet = FALSE;
	}

Cleanup:
	if(pbRequestHeaders && pbRequestHeaders != lpvBuffer)
	{
		free(pbRequestHeaders);
	}

#ifdef DEBUG_BUILD
	if(logger && bRet)
	{
		logger->Log("*lpdwBufferLength = 0x%x\n", *lpdwBufferLength);
		//logger->LogHex((PBYTE)lpvBuffer, *lpdwBufferLength);
	}
#endif

	if(pReqRepl)
		pReqRepl->Release();

#ifdef DEBUG_BUILD
	if(logger)
		logger->ReleaseLock();
#endif

	return bRet;
}

BOOL STDAPICALLTYPE WininetHooks::My_HttpQueryInfoW(
  __in     HINTERNET hRequest,
  __in     DWORD dwInfoLevel,
  __inout  LPVOID lpvBuffer,
  __inout  LPDWORD lpdwBufferLength,
  __inout  LPDWORD lpdwIndex
)
{
	BOOL bRet;
	DWORD dwInformationLevel = (dwInfoLevel & 0xffff);
	REQUESTREPLY reqrepl = (REQUESTREPLY)hRequest;
	CConnectionPool *pConnPool = CConnectionPool::GetInstance();
	CRequestReply *pReqRepl = NULL;
#ifdef DEBUG_BUILD
	CLogger *logger = NULL;
#endif
	std::multimap<std::string, std::string>::const_iterator it;
	std::string strHeaders = "";
	BOOL bFirstHeader = TRUE;
	
	DWORD dwRequiredBufferSize = *lpdwBufferLength;
	PBYTE pbRequestHeaders = (PBYTE)lpvBuffer;
	
	std::vector<char *> *pvecCType = NULL;
	wchar_t *pwsz = NULL;
	char *psz = NULL, *pszHeader = NULL, *pszValue = NULL;
	size_t cch = 0;

	ILoadedModules *pLoadedMods = GetLoadedModules();
	DWORD dwRetnAddress = 0;

	GETRETNADDR(dwRetnAddress);

	if(pLoadedMods->IsAddressInHModule(hmWininet, (LPVOID)dwRetnAddress))
	{
#ifdef DEBUG_BUILD
		CLogger::Break();
#endif

		// assume the A version is calling the W version:
		bRet = Real_HttpQueryInfoW(hRequest, dwInfoLevel, lpvBuffer, lpdwBufferLength, lpdwIndex);
		
		return bRet;
	}
	
	try
	{
		pReqRepl = pConnPool->GetRequestReply(reqrepl);
		if(!pReqRepl)
		{
			bRet = FALSE;
			goto Cleanup;
		}

		pReqRepl->AddRef();
	}
	catch(...)
	{
		bRet = FALSE;
		goto Cleanup;
	}

	try
	{
#ifdef DEBUG_BUILD
		logger = CLogger::GetInstance("bpcore_wininet");
		logger->ObtainLock();
		logger->Log("My_HttpQueryInfoW(reqrepl = 0x%x; dwInformationLevel = %u, *lpdwBufferLength = %u)\n", reqrepl, dwInformationLevel, *lpdwBufferLength);
#endif
	}
	catch(...)
	{
	}

	if(!pReqRepl)
	{
		throw new std::exception("HttpQueryInfo no requestreply instance attached");
	}

	bRet = Real_HttpQueryInfoW(hRequest, dwInfoLevel, lpvBuffer, &dwRequiredBufferSize, lpdwIndex);
	if(!bRet)
	{
		// if the only problem is that the user has supplied a buffer of insufficient size
		// allocate buffer, read headers into buffer and query request allowed, and fail
		// response early

#ifdef DEBUG_BUILD
		if(logger) logger->Log("\tReal_HttpQueryInfoW (reqrepl = 0x%x) returned false\n", reqrepl);
#endif

		if(GetLastError() == ERROR_INSUFFICIENT_BUFFER)
		{
#ifdef DEBUG_BUILD
			if(logger) logger->Log("\tGetLastError() == ERROR_INSUFFICIENT_BUFFER [dwRequiredBufferSize = %u]\n", dwRequiredBufferSize);
#endif
			switch(dwInformationLevel)
			{
			case HTTP_QUERY_CONTENT_TYPE:
				// add code to query for this header in the reply headers, if found, read this value out instead
				// of executing Real_HttpQueryInfo
				
				pvecCType = pReqRepl->GetReplyHeader("Content-Type");
				if(pvecCType)
				{
					psz = pvecCType->at(0);
					*lpdwBufferLength = strlen(psz) * sizeof(wchar_t);

					delete pvecCType;
					goto Cleanup;
				}

				// fallthrough
			case HTTP_QUERY_RAW_HEADERS:
			case HTTP_QUERY_RAW_HEADERS_CRLF:
				
#ifdef DEBUG_BUILD
				if(logger) logger->Log("\t(reqrepl = 0x%x) Resizing buffer to %u bytes\n", reqrepl, dwRequiredBufferSize);
#endif
				pbRequestHeaders = (PBYTE)malloc(dwRequiredBufferSize);
				if(!pbRequestHeaders)
				{
					goto Cleanup;
				}
				
				bRet = Real_HttpQueryInfoW(hRequest, dwInfoLevel, pbRequestHeaders, &dwRequiredBufferSize, lpdwIndex);
				if(!bRet)
				{
#ifdef DEBUG_BUILD
					if(logger) logger->Log("\tReal_HttpQueryInfoW (reqrepl = 0x%x) still errors. Aborting.\n", reqrepl);
#endif
					goto Cleanup;
				}

#ifdef DEBUG_BUILD
				if(logger) logger->Log("\t(reqrepl = 0x%x) Data read (%u bytes)\n", reqrepl, dwRequiredBufferSize);
#endif
				break;
			default:
				goto Cleanup;
			}
		}
		else
		{
			goto Cleanup;
		}
	}

	switch(dwInformationLevel)
	{
	case HTTP_QUERY_CONTENT_TYPE:
#ifdef DEBUG_BUILD
		if(logger) logger->Log("\tHTTP_QUERY_CONTENT_TYPE ");
#endif
		pvecCType = pReqRepl->GetReplyHeader("Content-Type");
		if(pvecCType)
		{
#ifdef DEBUG_BUILD
			if(logger) logger->Log("already exists. Skipping read out.\n");
#endif
			delete pvecCType;
			break;
		}

#ifdef DEBUG_BUILD
		if(logger) logger->Log("does not yet exist. Reading from header store.\n");
#endif
		cch = dwRequiredBufferSize / sizeof(wchar_t);

		pwsz = (wchar_t *)malloc((cch + 1) * sizeof(wchar_t));
		if(!pwsz)
		{
			throw new std::exception("Allocation failure in query info");
		}

		memcpy(pwsz, pbRequestHeaders, dwRequiredBufferSize);
		pwsz[cch] = '\0';

#ifdef DEBUG_BUILD
		if(logger) logger->Log("\tContent-Type header length %u bytes\n", dwRequiredBufferSize);
#endif
		psz = Utils::MbFromWideChar((WCHAR *)pwsz);
		if(!psz)
		{
			free(pwsz);
			throw new std::exception("Allocation failure in query info");
		}

#ifdef DEBUG_BUILD
		if(logger) logger->Log("\tContent-Type being read '%s'\n", psz);
#endif
		pszHeader = "Content-Type";
		pszValue = psz;
		
		pReqRepl->SetReplyHeader(pszHeader, pszValue);

		free(pwsz);
		free(psz);

		break;
	case HTTP_QUERY_RAW_HEADERS:
#ifdef DEBUG_BUILD
		if(logger) logger->Log("\tHTTP_QUERY_RAW_HEADERS ");
#endif
		if(pReqRepl->GetReplyHeaders().size() != 0)
		{
#ifdef DEBUG_BUILD
			if(logger) logger->Log("already read.\n");
#endif
			// already extracted headers
			break;
		}

#ifdef DEBUG_BUILD
		if(logger) logger->Log("reading headers.\n");
#endif
		pwsz = (wchar_t *)pbRequestHeaders;
		
		while(pwsz)
		{
			if(wcslen(pwsz) == 0)
			{
				break; // end of array
			}

			psz = Utils::MbFromWideChar((WCHAR *)pwsz);
			if(!psz)
			{
				throw new std::exception("Allocation failure in query info");
			}

#ifdef DEBUG_BUILD
			if(logger) logger->Log("\tFound Header: '%s'\n", psz);
#endif			
			if(bFirstHeader == TRUE)
			{
				pReqRepl->SetReplyBanner(psz);
				bFirstHeader = FALSE;
			}
			else
			{
				pszHeader = psz;
				pszValue = strchr(psz, ':');

				if(pszValue)
				{
					*pszValue = 0;
					pszValue++;
					pszValue += Utils::TrimCount(pszValue);
					pReqRepl->SetReplyHeader(pszHeader, pszValue);
				}
			}

			free(psz);
			
			pwsz = pwsz + wcslen(pwsz) + 1;
		}

		break;
	case HTTP_QUERY_RAW_HEADERS_CRLF:
#ifdef DEBUG_BUILD
		if(logger) logger->Log("\tHTTP_QUERY_RAW_HEADERS_CRLF");
#endif
		if(pReqRepl->GetReplyHeaders().size() != 0)
		{
#ifdef DEBUG_BUILD
			if(logger) logger->Log("already read.\n");
#endif
			// already extracted headers
			break;
		}

#ifdef DEBUG_BUILD
		if(logger) logger->Log("reading headers.\n");
#endif
		pReqRepl = pConnPool->GetRequestReply(reqrepl);
		if(!pReqRepl)
			break;

		pwsz = wcstok((wchar_t *)pbRequestHeaders, L"\n");

		while(pwsz)
		{
			psz = Utils::MbFromWideChar((WCHAR *)pwsz);

			if(psz)
			{
				cch = strlen(psz);
				pszHeader = psz;
				pszValue = strchr(psz, ':');

#ifdef DEBUG_BUILD
				if(logger) logger->Log("\tHeader Found: '%s'", psz);
#endif
				if(psz[cch - 1] == '\r')
				{
					psz[cch - 1] = 0;
				}

				if(bFirstHeader == TRUE)
				{
					pReqRepl->SetReplyBanner(psz);
					bFirstHeader = FALSE;
				}
				else
				{
					if(pszValue)
					{
						*pszValue = 0;
						pszValue++;
						pszValue += Utils::TrimCount(pszValue);
						pReqRepl->SetReplyHeader(pszHeader, pszValue);
					}
				}

				free(psz);
			}

			pwsz = wcstok(NULL, L"\n");
		}

		break;
	default:
		goto Cleanup; // if it's not a query for headers or other supported info: return
	}

	WCHAR *pwszHeaders = NULL;
	size_t cchHeaders = 0;
	bFirstHeader = TRUE;

	BOOL bContinue = !bConnMonEnabled || !bValidateResponse || pConnPool->QueryResponseAllowed(reqrepl);
	//BOOL bContinue = pConnPool->QueryResponseAllowed(reqrepl);
	if(bContinue)
	{
#ifdef DEBUG_BUILD
		if(logger) logger->Log("\tQueryResponseAllowed returned true\n");
#endif
		switch(dwInformationLevel)
		{
		case HTTP_QUERY_CONTENT_TYPE:
#ifdef DEBUG_BUILD
			if(logger) logger->Log("\tHTTP_QUERY_CONTENT_TYPE ");
#endif
			pvecCType = pReqRepl->GetReplyHeader("Content-Type");
			if(pvecCType)
			{
#ifdef DEBUG_BUILD
				if(logger) logger->Log("found header.\n");
#endif
				pwsz = Utils::WcFromMultiByte(pvecCType->at(0)); // first one - risks?
				if(!pwsz)
				{
					throw new std::exception("Allocation failure in query info");
				}

				cch = wcslen(pwsz);

#ifdef DEBUG_BUILD
				if(logger) logger->Log("\tValue: '%s'. dwRequiredBufferSize = %u; cch = %u\n", pwsz, dwRequiredBufferSize, cch);
#endif
				//if(pbRequestHeaders == lpvBuffer)
				if((cch * sizeof(wchar_t)) <= dwRequiredBufferSize)
				{
#ifdef DEBUG_BUILD
					if(logger) logger->Log("\tCaller allocated sufficient buffer. Copying out.\n");
#endif
					// the caller allocated a sufficiently large buffer
					memcpy(lpvBuffer, pwsz, cch * sizeof(wchar_t));
					*lpdwBufferLength = cch * sizeof(wchar_t);
				}
				else
				{
					// we need to wait for caller to request actual header data, return length only

#ifdef DEBUG_BUILD
					if(logger) logger->Log("\tCaller did not allocate sufficiently large output buffer. Advising %u bytes\n", cch * sizeof(WCHAR));
#endif
					*lpdwBufferLength = cch * sizeof(WCHAR);
				}

				delete pvecCType;
				free(pwsz);
			}
			else
			{
				// not exactly a failure, but this is expected behaviour

#ifdef DEBUG_BUILD
				if(logger) logger->Log("\tContent-Type header not found. Returning false.\n");
#endif
				*lpdwBufferLength = 0;
				bRet = FALSE;
				
				goto Cleanup;
			}

			break;
		case HTTP_QUERY_RAW_HEADERS:
#ifdef DEBUG_BUILD
			if(logger) logger->Log("\tHTTP_QUERY_RAW_HEADERS\n");
#endif
			// add sanitised headers back
			
			cchHeaders = 0;
			
			WCHAR *pwszTmp;
			
			for(it = pReqRepl->GetReplyHeaders().begin(); it != pReqRepl->GetReplyHeaders().end(); it++)
			{
				strHeaders = "";

				if(bFirstHeader)
				{
					strHeaders = pReqRepl->GetReplyBanner();
					cch = strlen(strHeaders.c_str());

					pwszHeaders = (WCHAR *)realloc(pwszHeaders, (cch + 2) * sizeof(WCHAR));
					if(!pwszHeaders || cch > Utils::ARBITRARY_STRING_SIZE_LIMIT )
					{
						throw new std::exception("HttpQueryInfoW allocation failure");
					}

					pwszTmp = Utils::WcFromMultiByte(const_cast<char *>(strHeaders.c_str()));
					if(!pwszTmp)
					{
						throw new std::exception("HttpQueryInfoW allocation failure");
					}
					
#ifdef DEBUG_BUILD
					if(logger) logger->Log("\tAdded Header: '%S'\n", pwszTmp);
#endif
					memcpy(pwszHeaders, pwszTmp, cch * sizeof(WCHAR));
					pwszHeaders[cchHeaders + cch] = '\0';

					cchHeaders += cch;
					cchHeaders++; // move past the NULL byte
					free(pwszTmp);

					strHeaders = "";
					bFirstHeader = FALSE;
				}

				strHeaders += it->first + ": " + it->second;
				cch = strlen(strHeaders.c_str());

				pwszHeaders = (WCHAR *)realloc(pwszHeaders, (cchHeaders + cch + 2) * sizeof(WCHAR));
				if(!pwszHeaders || cchHeaders > Utils::ARBITRARY_STRING_SIZE_LIMIT )
				{
					throw new std::exception("HttpQueryInfoW allocation failure");
				}

				pwszTmp = Utils::WcFromMultiByte(const_cast<char *>(strHeaders.c_str()));
				if(!pwszTmp)
				{
					throw new std::exception("HttpQueryInfoW allocation failure");
				}

#ifdef DEBUG_BUILD
				if(logger) logger->Log("\tAdded Header: '%S'\n", pwszTmp);
#endif				
				memcpy(pwszHeaders + cchHeaders, pwszTmp, cch * sizeof(WCHAR));
				pwszHeaders[cchHeaders + cch] = '\0';
				
				cchHeaders += cch;
				cchHeaders++; // move past the NULL byte
				free(pwszTmp);
			}

			pwszHeaders[cchHeaders] = '\0'; // terminated by double NULL
			cchHeaders++;
			
			if((cchHeaders * sizeof(WCHAR)) <= *lpdwBufferLength)
			{
#ifdef DEBUG_BUILD
				if(logger) logger->Log("\tCaller allocated sufficiently large buffer. Copying %u bytes\n", cchHeaders * sizeof(WCHAR));
#endif
				// the caller allocated a sufficiently large buffer
				memcpy(lpvBuffer, pwszHeaders, cchHeaders * sizeof(WCHAR));
				*lpdwBufferLength = cchHeaders * sizeof(WCHAR);
			}
			else
			{
#ifdef DEBUG_BUILD
				if(logger) logger->Log("\tCaller did not allocate sufficiently large buffer. Advising %u bytes\n", cchHeaders * sizeof(WCHAR));
#endif
				// we need to wait for caller to request actual header data, return length only
				*lpdwBufferLength = cchHeaders * sizeof(WCHAR);
			}

			if(pwszHeaders)
			{
				free(pwszHeaders);
			}

			break;
		case HTTP_QUERY_RAW_HEADERS_CRLF:
#ifdef DEBUG_BUILD
			if(logger) logger->Log("\tHTTP_QUERY_RAW_HEADERS_CRLF\n");
#endif
			// add sanitised headers back
			
			cchHeaders = 0;
			
			for(it = pReqRepl->GetReplyHeaders().begin(); it != pReqRepl->GetReplyHeaders().end(); it++)
			{
				if(bFirstHeader)
				{
					strHeaders = pReqRepl->GetReplyBanner();
					strHeaders += "\r\n";
					cchHeaders += strlen(strHeaders.c_str());
					bFirstHeader = FALSE;
				}

				std::string strSingleHeader = it->first + ": " + it->second + "\r\n";
				strHeaders += strSingleHeader;
				cchHeaders += strlen(strSingleHeader.c_str());
			}

			strHeaders += "\r\n"; // double crlf terminated
			cchHeaders += strlen("\r\n");

			pwszHeaders = Utils::WcFromMultiByte(const_cast<char *>(strHeaders.c_str()));
			if(!pwszHeaders)
			{
				throw new std::exception("HttpQueryInfo allocation failure");
			}
			
			// should we pass original headers if WcFromMultiByte fails? Risks?
			
			// copy headers into lpBuffer ...

#ifdef DEBUG_BUILD
			if(logger) logger->Log("\tHeader size %u bytes.\n", cchHeaders);
#endif
			if((cchHeaders * sizeof(WCHAR)) <= *lpdwBufferLength)
			{
#ifdef DEBUG_BUILD
				if(logger) logger->Log("\tCaller allocated sufficiently large buffer. Copying %u bytes\n", cchHeaders * sizeof(WCHAR));
#endif
				// the caller allocated a sufficiently large buffer
				memcpy(lpvBuffer, pwszHeaders, cchHeaders * sizeof(WCHAR));
				*lpdwBufferLength = cchHeaders * sizeof(WCHAR);
			}
			else
			{
#ifdef DEBUG_BUILD
				if(logger) logger->Log("\tCaller did not allocate sufficiently large buffer. Advising %u bytes\n", cchHeaders * sizeof(WCHAR));
#endif
				// we need to wait for caller to request actual header data, return length only
				*lpdwBufferLength = cchHeaders * sizeof(WCHAR);
			}

			if(pwszHeaders)
			{
				free(pwszHeaders);
			}

			break;
		default:
			goto Cleanup; // should never reach
		}
	}
	else
	{
		// standard error path
#ifdef DEBUG_BUILD
		if(logger && bRet)
		{
			logger->Log("*lpdwBufferLength = 0x%x\n", *lpdwBufferLength);
			//logger->LogHex((PBYTE)lpvBuffer, *lpdwBufferLength);
		}

		if(logger)
			logger->Log("\tFailing query info attempt\n");
#endif
		*lpdwBufferLength = 0;
		bRet = FALSE;
	}

Cleanup:
	if(pbRequestHeaders && pbRequestHeaders != lpvBuffer)
	{
		free(pbRequestHeaders);
	}

#ifdef DEBUG_BUILD
	if(logger && bRet)
	{
		logger->Log("*lpdwBufferLength = 0x%x\n", *lpdwBufferLength);
		//logger->LogHex((PBYTE)lpvBuffer, *lpdwBufferLength);
	}
#endif

	if(pReqRepl)
		pReqRepl->Release();

#ifdef DEBUG_BUILD
	if(logger)
		logger->ReleaseLock();
#endif

	return bRet;
}

BOOL STDAPICALLTYPE WininetHooks::My_InternetQueryDataAvailable(
  __in   HINTERNET hFile,
  __out  LPDWORD lpdwNumberOfBytesAvailable,
  __in   DWORD dwFlags,
  __in   DWORD_PTR dwContext
)
{
	BOOL bRet = TRUE;
	CConnectionPool *pConnPool = CConnectionPool::GetInstance();
	REQUESTREPLY reqrepl = (REQUESTREPLY)hFile;
	CRequestReply *pReqRepl = NULL;
#ifdef DEBUG_BUILD
	CLogger *logger = NULL;
#endif

	try
	{
#ifdef DEBUG_BUILD
		logger = CLogger::GetInstance("bpcore_wininet");
		logger->ObtainLock();
		logger->Log("My_InternetQueryDataAvailable: (0x%x)\n", reqrepl);
#endif
	}
	catch(...)
	{
	}
	
	try
	{
		pReqRepl = pConnPool->GetRequestReply(reqrepl);
		if(!pReqRepl)
		{
			bRet = FALSE;
			goto Cleanup;
		}

		pReqRepl->AddRef();
	}
	catch(...)
	{
		bRet = FALSE;
		goto Cleanup;
	}

	// No need to do security checking here. May lead to a lot of redundant work, esp when response.
	// Defer to legitimate call first time, and if we have run out of data in our stream buffer

	if(pReqRepl->GetReplyDataConsumed() &&
		pReqRepl->GetReplyDataOffset() < pReqRepl->GetReplyBody()->Length())
	{
		DWORD dwReplyLength = pReqRepl->GetReplyBody()->Length();
		DWORD dwReplyOffs   = pReqRepl->GetReplyDataOffset();

		DWORD dwDataAvailable = dwReplyLength - dwReplyOffs; // might as well project the lot
		*lpdwNumberOfBytesAvailable = dwDataAvailable;
			
#ifdef DEBUG_BUILD
		if(logger)
			logger->Log("\tStating that %u bytes are available to be read (%u total in buffer [offs = %u])\n",
				dwDataAvailable, pReqRepl->GetReplyBody()->Length(), pReqRepl->GetReplyDataOffset()
			);
#endif
	}
	else
	{
		bRet = Real_InternetQueryDataAvailable(hFile, lpdwNumberOfBytesAvailable, dwFlags, dwContext);
		
#ifdef DEBUG_BUILD
		if(logger)
			logger->Log("\treturned %s (%u bytes available to read)\n", bRet == TRUE ? "TRUE" : "FALSE", *lpdwNumberOfBytesAvailable);
#endif
	}
	
Cleanup:

#ifdef DEBUG_BUILD
	if(logger && !bRet)
		logger->Log("\t\tGetLastError() = %u\n", GetLastError());
#endif

	if(pReqRepl)
		pReqRepl->Release();

#ifdef DEBUG_BUILD
	if(logger)
		logger->ReleaseLock();
#endif

	return bRet;
}

BOOL STDAPICALLTYPE WininetHooks::My_InternetReadFile(
  __in   HINTERNET hFile,
  __out  LPVOID lpBuffer,
  __in   DWORD dwNumberOfBytesToRead,
  __out  LPDWORD lpdwNumberOfBytesRead
)
{
	BOOL bRet;
	CConnectionPool *pConnPool = CConnectionPool::GetInstance();
	REQUESTREPLY reqrepl = (REQUESTREPLY)hFile;
	CRequestReply *pReqRepl = NULL;
#ifdef DEBUG_BUILD
	CLogger *logger = NULL;
#endif

	try
	{
#ifdef DEBUG_BUILD
		logger = CLogger::GetInstance("bpcore_wininet");
		logger->ObtainLock();
		logger->Log("My_InternetReadFile: [reqrepl = 0x%x] (dwNumberOfBytesToRead = %u)\n", reqrepl, dwNumberOfBytesToRead);
#endif
	}
	catch(...)
	{
	}

	try
	{
		pReqRepl = pConnPool->GetRequestReply(reqrepl);
		if(!pReqRepl)
		{
			bRet = FALSE;
			goto Cleanup;
		}

		pReqRepl->AddRef();
	}
	catch(...)
	{
		bRet = FALSE;
		goto Cleanup;
	}

	if(pReqRepl->GetReplyConnectionError())
	{
		// should i copy out any data read into stream successfully prior to error?
		bRet = FALSE;
		*lpdwNumberOfBytesRead = 0;
		goto Cleanup;
	}

	if(pReqRepl->GetReplyDataConsumed())
	{
		// we need to copy data out of stream buffer
		PBYTE pbStreamBuffer  = pReqRepl->GetReplyBody()->Buffer();
		size_t cbStreamBuffer = pReqRepl->GetReplyBody()->Length(),
			   cbReadSize = 0,
			   cbReplyDataOffset = pReqRepl->GetReplyDataOffset();

		*lpdwNumberOfBytesRead = 0;

		if(cbReplyDataOffset < cbStreamBuffer)
		{
#ifdef DEBUG_BUILD
			if(logger)
				logger->Log("\tReply data already in stream buffer (%u bytes, now at offset %u)\n", cbStreamBuffer, cbReplyDataOffset);
#endif

			cbReadSize = ((cbStreamBuffer - cbReplyDataOffset) < dwNumberOfBytesToRead) ?
				(cbStreamBuffer - cbReplyDataOffset) : dwNumberOfBytesToRead;

			memcpy(lpBuffer, pbStreamBuffer + cbReplyDataOffset, cbReadSize);
			pReqRepl->SetReplyDataOffset(cbReplyDataOffset + cbReadSize);

			// might have more to read this request, defer to legitimate read
			dwNumberOfBytesToRead -= cbReadSize;
			*lpdwNumberOfBytesRead = cbReadSize;

#ifdef DEBUG_BUILD
			if(logger)
				logger->Log("\tRead %u bytes from stream buffer at offset %u\n\tRemaining bytes to be read %u\n", cbReadSize, cbReplyDataOffset, dwNumberOfBytesToRead);
#endif
			bRet = TRUE;
		}
		
		if(dwNumberOfBytesToRead)
		{
			if(pReqRepl->GetReplyBody()->StreamBufferReplaced())
			{
				// whole buffer replaced. once enough data is read from stream (as it will be by this
				// stage due to the cbReplyDataOffset < cbStreamBuffer check, don't return any more
				// data.

				// will return either cbReadSize or zero, depending on whether there was still data
				// left in the buffer.

				bRet = TRUE;
				goto Cleanup;
			}

			// legitimate read. No need to perform security checking as whole stream has already been
			// deemed to be safe.

			bRet = Real_InternetReadFile(hFile, ((PBYTE)(lpBuffer)) + cbReadSize, dwNumberOfBytesToRead,
				lpdwNumberOfBytesRead);
			
#ifdef DEBUG_BUILD
			if(logger)
			{
				logger->Log("\tInvoked Real_InternetReadFile(lpBuffer[0x%p] + %u, dwNumberOfBytesToRead = %u\n", lpBuffer, cbReadSize, dwNumberOfBytesToRead);
				logger->Log("\tAPI returned %s\n", bRet ? "TRUE" : "FALSE");

				if(!bRet)
				{
					logger->Log("\tGetLastError() = %u\n", GetLastError());
				}
			}
#endif

			if(!bRet)
				goto Cleanup;

			CCommonTypeStreamReader *pStream = pReqRepl->GetReplyBody();

			if(pStream->Write(
					pReqRepl->GetReplyDataOffset(),
					((PBYTE)(lpBuffer)) + cbReadSize,
					*lpdwNumberOfBytesRead,
					TRUE
				) == EDataStreamError::StreamFailure)
			{
				bRet = FALSE;
				*lpdwNumberOfBytesRead = 0;
				goto Cleanup;
			}

			pReqRepl->SetReplyDataOffset(pReqRepl->GetReplyDataOffset() + *lpdwNumberOfBytesRead); // cbReadSize already accounted for
			*lpdwNumberOfBytesRead += cbReadSize;

			if(pReqRepl->GetStreamRequiresChunkProcessing())
			{
				// stream is being processed chunk-by-chunk (i.e. streaming video)

				BOOL bContinue = !bConnMonEnabled || !bValidateResponse || pConnPool->QueryResponseAllowed(reqrepl);
				//BOOL bContinue = pConnPool->QueryResponseAllowed(reqrepl);
				if(!bContinue)
				{
					bRet = FALSE;
					goto Cleanup;
				}
			}
		}
		
		goto Cleanup;
	}

	BOOL bContinue = !bConnMonEnabled || !bValidateResponse || pConnPool->QueryResponseAllowed(reqrepl);
	//BOOL bContinue = pConnPool->QueryResponseAllowed(reqrepl); // don't bother to read if headers fail a check
	if(bContinue)
	{
#ifdef DEBUG_BUILD
		if(logger)
			logger->Log("\tQueryResponseAllowed[1] return true\n");
#endif

		if((bRet = Real_InternetReadFile(hFile, lpBuffer, dwNumberOfBytesToRead, lpdwNumberOfBytesRead)) == FALSE)
		{
			pReqRepl->SetReplyConnectionError(TRUE);
			bRet = FALSE;
			goto Cleanup;
		}
		
#ifdef DEBUG_BUILD
		if(logger)
		{
			logger->Log("\tInvoking Real_InternetReadFile(dwNumberOfBytesToRead = %u)\n", dwNumberOfBytesToRead);
			logger->Log("\t*lpdwNumberOfBytesRead = %u\n", *lpdwNumberOfBytesRead);
			
			if(!bRet)
			{
				logger->Log("\nReturned false; GetLastError() = %u\n", GetLastError());
			}
		}
#endif

		pReqRepl->SetReplyInfo(reqrepl, (PBYTE)lpBuffer, *lpdwNumberOfBytesRead, Callback_InternetReadFile);
		pReqRepl->SetReplyDataOffset(*lpdwNumberOfBytesRead);
		pReqRepl->SetReplyDataConsumed(TRUE);
		
		// this time callbacks can parse the actual response buffer
		BOOL bContinue = !bConnMonEnabled || !bValidateResponse || pConnPool->QueryResponseAllowed(reqrepl);
		//bContinue = pConnPool->QueryResponseAllowed(reqrepl);
		if(!bContinue)
		{
#ifdef DEBUG_BUILD
			if(logger)
				logger->Log("\tQueryResponseAllowed[2] returned false\n");
#endif
			bRet = FALSE;
			goto Cleanup;
		}

#ifdef DEBUG_BUILD
		if(logger)
				logger->Log("\tQueryResponseAllowed[2] returned true\n");
#endif
		// QueryResponseAllowed may have modified the data in the stream buffer

		if(pReqRepl->GetReplyBody())
		{
			PBYTE pbBuffer = pReqRepl->GetReplyBody()->Buffer();
			size_t cbBuffer = pReqRepl->GetReplyBody()->Length(), cbReadable;

			cbReadable = (cbBuffer >= *lpdwNumberOfBytesRead) ? *lpdwNumberOfBytesRead : cbBuffer;

			memcpy(lpBuffer, pbBuffer, cbReadable);
			*lpdwNumberOfBytesRead = cbReadable;

			pReqRepl->SetReplyDataOffset(*lpdwNumberOfBytesRead);
		}

		// after this point, it's likely that more data has been read by the stream class into the
		// pReqRepl->m_pbReply buffer. This data needs to be sent back to the caller on subsequent
		// calls into this function until exhausted, after which more data will be read legitimately.
	}
	else
	{
#ifdef DEBUG_BUILD
		if(logger)
			logger->Log("\tQueryResponseAllowed[1] return false\n");
#endif
		bRet = FALSE;
	}

Cleanup:
	if(!bRet)
	{
#ifdef DEBUG_BUILD
		if(logger)
			logger->Log("\tFailing read attempt\n");
#endif
		//SetLastError(some-socket-error);
		//*lpdwNumberOfBytesRead = 0;
	}
	else
	{
#ifdef DEBUG_BUILD
		if(logger)
		{
			logger->Log("\tData read (*lpdwNumberOfBytesRead = %u)\n", *lpdwNumberOfBytesRead);
			//logger->LogHex((PBYTE)lpBuffer, *lpdwNumberOfBytesRead);
		}
#endif
	}

	if(pReqRepl)
		pReqRepl->Release();

#ifdef DEBUG_BUILD
	if(logger)
		logger->ReleaseLock();
#endif

	return bRet;
}

BOOL STDAPICALLTYPE WininetHooks::My_InternetReadFileExA(
  __in   HINTERNET hFile,
  __out  LPINTERNET_BUFFERSA lpBuffersOut,
  __in   DWORD dwFlags,
  __in   DWORD_PTR dwContext
)
{
	BOOL bRet = FALSE;
	CConnectionPool *pConnPool = CConnectionPool::GetInstance();
	REQUESTREPLY reqrepl = (REQUESTREPLY)hFile;
#ifdef DEBUG_BUILD
	CLogger *logger = NULL;
#endif
	CRequestReply *pReqRepl = NULL;
	
	try
	{
#ifdef DEBUG_BUILD
		logger = CLogger::GetInstance("bpcore_wininet");
		logger->ObtainLock();
		logger->Log("My_InternetReadFileExA: (0x%x)\n", reqrepl);
#endif
	}
	catch(...)
	{
	}
	/*
	DWORD dwB = 0;

	bRet = My_InternetReadFile(hFile, lpBuffersOut->lpvBuffer, lpBuffersOut->dwBufferLength, &dwB);
	if(bRet)
		lpBuffersOut->dwBufferLength = dwB;

	goto Cleanup;
	*/
	
	try
	{
		pReqRepl = pConnPool->GetRequestReply(reqrepl);
		if(!pReqRepl)
		{
			bRet = FALSE;
			goto Cleanup;
		}

		pReqRepl->AddRef();
	}
	catch(...)
	{
		bRet = FALSE;
		goto Cleanup;
	}

	if(pReqRepl->GetReplyConnectionError())
	{
		// should i copy out any data read into stream successfully prior to error?
		bRet = FALSE;
		lpBuffersOut->dwBufferLength = 0;
		goto Cleanup;
	}

	if(pReqRepl->GetReplyDataConsumed())
	{
		// we need to copy data out of stream buffer
		
		PBYTE pbStreamBuffer  = pReqRepl->GetReplyBody()->Buffer();
		size_t cbStreamBuffer = pReqRepl->GetReplyBody()->Length(),
			   cbReadSize = 0,
			   cbReplyDataOffset = pReqRepl->GetReplyDataOffset();
		DWORD dwBufferSize = lpBuffersOut->dwBufferLength;
		PBYTE lpvBufferByteCopy = (PBYTE)lpBuffersOut->lpvBuffer;

		if(cbReplyDataOffset < cbStreamBuffer)
		{
#ifdef DEBUG_BUILD
			if(logger)
				logger->Log("\tReply data already in stream buffer (%u bytes, now at offset %u)\n", cbStreamBuffer, cbReplyDataOffset);
#endif
			cbReadSize = ((cbStreamBuffer - cbReplyDataOffset) < dwBufferSize) ?
				(cbStreamBuffer - cbReplyDataOffset) : dwBufferSize;

			memcpy(lpBuffersOut->lpvBuffer, pbStreamBuffer + cbReplyDataOffset, cbReadSize);
			pReqRepl->SetReplyDataOffset(cbReplyDataOffset + cbReadSize);

			// might have more to read this request, defer to legitimate read
			dwBufferSize -= cbReadSize;
			lpvBufferByteCopy += cbReadSize;

#ifdef DEBUG_BUILD
			if(logger)
				logger->Log("\tRead %u bytes from stream buffer at offset %u\n\tRemaining bytes to be read %u\n", cbReadSize, cbReplyDataOffset, dwBufferSize);
#endif
			bRet = TRUE;
		}
		
		if(dwBufferSize)
		{
			if(pReqRepl->GetReplyBody()->StreamBufferReplaced())
			{
				// whole buffer replaced. once enough data is read from stream (as it will be by this
				// stage due to the cbReplyDataOffset < cbStreamBuffer check, don't return any more
				// data.

				// will return either cbReadSize or zero, depending on whether there was still data
				// left in the buffer.

				lpBuffersOut->dwBufferLength = cbReadSize;

				bRet = TRUE;
				goto Cleanup;
			}

			// legitimate read. No need to perform security checking as whole stream has already been
			// deemed to be safe.

			LPVOID lpvCopy = lpBuffersOut->lpvBuffer;
			
			lpBuffersOut->lpvBuffer = (LPVOID)lpvBufferByteCopy;
			lpBuffersOut->dwBufferLength = dwBufferSize;
			
			bRet = Real_InternetReadFileExA(hFile, lpBuffersOut, dwFlags, dwContext);
			
			if(bRet && (lpBuffersOut->Next || lpBuffersOut->lpcszHeader))
			{
				// should loop through the LPINTERNET_BUFFERS structure copying out the body in
				// incremental chunks, copying out the headers, saving them off. Instead just
				// reject these unusual (per my research) instances. Must implement proper
				// behaviour later.

#ifdef DEBUG_BUILD
				if(logger)
					logger->Log("\tFound INTERNET_BUFFERS structure with Next or lpcszHeader value\n");
				
				CLogger::Break();
#endif
				bRet = FALSE;
				goto Cleanup;
			}

#ifdef DEBUG_BUILD
			if(logger)
			{
				logger->Log("\tInvoked Real_InternetReadFileExA[1]: lpBuffersOut->lpvBuffer[0x%p]; before: dwBufferSize = %u\n"
					"\t\tafter: lpBuffersOut->dwBufferLength = %u\n", lpBuffersOut->lpvBuffer, dwBufferSize, lpBuffersOut->dwBufferLength);

				logger->Log("\tAPI returned %s\n", bRet ? "TRUE" : "FALSE");

				if(!bRet)
				{
					logger->Log("\tGetLastError() = %u\n", GetLastError());
				}
				else
				{
					//logger->Log("\tAsync read returned:\n");
					//logger->LogHex((PBYTE)lpBuffersOut->lpvBuffer, lpBuffersOut->dwBufferLength);
				}
			}
#endif

			if(!bRet)
				goto Cleanup;
			
			CCommonTypeStreamReader *pStream = pReqRepl->GetReplyBody();

			if(pStream->Write(
					pReqRepl->GetReplyDataOffset(),
					((PBYTE)(lpvCopy)) + cbReadSize,
					lpBuffersOut->dwBufferLength,
					TRUE
				) == EDataStreamError::StreamFailure)
			{
				bRet = FALSE;
				lpBuffersOut->dwBufferLength = 0;
				goto Cleanup;
			}

			pReqRepl->SetReplyDataOffset(pReqRepl->GetReplyDataOffset() + lpBuffersOut->dwBufferLength); // cbReadSize already accounted for

			lpBuffersOut->lpvBuffer = lpvCopy;
			lpBuffersOut->dwBufferLength += cbReadSize;
			
			if(pReqRepl->GetStreamRequiresChunkProcessing())
			{
				// stream is being processed chunk-by-chunk (i.e. streaming video)

				BOOL bContinue = !bConnMonEnabled || !bValidateResponse || pConnPool->QueryResponseAllowed(reqrepl);
				//BOOL bContinue = pConnPool->QueryResponseAllowed(reqrepl);
				if(!bContinue)
				{
					bRet = FALSE;
					lpBuffersOut->dwBufferLength = 0;
					goto Cleanup;
				}
			}
		}
		else
		{
			if(!cbReadSize)
			{
				lpBuffersOut->lpvBuffer = 0;
				bRet = TRUE;
			}
		}
		
		goto Cleanup;
	}

	BOOL bContinue = !bConnMonEnabled || !bValidateResponse || pConnPool->QueryResponseAllowed(reqrepl);
	//BOOL bContinue = pConnPool->QueryResponseAllowed(reqrepl);
	if(bContinue)
	{
		*(DWORD *)lpBuffersOut->lpvBuffer = Utils::GetRandomInteger(0, UINT_MAX);
		DWORD dwBufHash = Utils::RSHash((char*)lpBuffersOut->lpvBuffer, lpBuffersOut->dwBufferLength < sizeof(DWORD) ? lpBuffersOut->dwBufferLength : sizeof(DWORD));
		bRet = Real_InternetReadFileExA(hFile, lpBuffersOut, dwFlags, dwContext);
		
#ifdef DEBUG_BUILD
		if(logger)
		{
			logger->Log("\tInvoked Real_InternetReadFileExA[2]: lpBuffersOut->lpvBuffer[0x%p]; after: lpBuffersOut->dwBufferLength = %u\n", lpBuffersOut->lpvBuffer, lpBuffersOut->dwBufferLength);
			logger->Log("\tAPI returned %s\n", bRet ? "TRUE" : "FALSE");

			if(!bRet)
			{
				logger->Log("\tGetLastError() = %u\n", GetLastError());
			}
			else
			{
				//logger->Log("\tAsync read returned:\n");
				//logger->LogHex((PBYTE)lpBuffersOut->lpvBuffer, lpBuffersOut->dwBufferLength);
			}
		}
#endif

		if(bRet && (lpBuffersOut->Next || lpBuffersOut->lpcszHeader))
		{
			// should loop through the LPINTERNET_BUFFERS structure copying out the body in
			// incremental chunks, copying out the headers, saving them off. Instead just
			// reject these unusual (per my research) instances. Must implement proper
			// behaviour later.

#ifdef DEBUG_BUILD
			if(logger)
				logger->Log("\tFound INTERNET_BUFFERS structure with Next or lpcszHeader value\n");

			CLogger::Break();
#endif

			bRet = FALSE;
			lpBuffersOut->dwBufferLength = 0;
			
			goto Cleanup;
		}
		
#ifdef DEBUG_BUILD
		if(logger)
			logger->Log("\tSetReplyInfo(reqrepl=0x%x, lpvBuffer=0x%p, dwBufferLength=%u\n",
			reqrepl, lpBuffersOut->lpvBuffer, lpBuffersOut->dwBufferLength);
#endif

		if(GetLastError() == ERROR_IO_PENDING)
		{
			Sleep(500);

			if(dwBufHash != Utils::RSHash((char*)lpBuffersOut->lpvBuffer, lpBuffersOut->dwBufferLength < sizeof(DWORD) ? lpBuffersOut->dwBufferLength : sizeof(DWORD)))
				bRet = TRUE;
		}

		if(bRet)
		{
			pReqRepl->SetReplyInfo(reqrepl, (PBYTE)lpBuffersOut->lpvBuffer, lpBuffersOut->dwBufferLength, Callback_InternetReadFile);
			pReqRepl->SetReplyDataOffset(lpBuffersOut->dwBufferLength);
			pReqRepl->SetReplyDataConsumed(TRUE);
		}
		else
		{
			// error path
			bRet = FALSE;
			lpBuffersOut->dwBufferLength = 0;
			goto Cleanup;
		}

		// this time callbacks can parse the actual response buffer
		BOOL bContinue = !bConnMonEnabled || !bValidateResponse || pConnPool->QueryResponseAllowed(reqrepl);
		//bContinue = pConnPool->QueryResponseAllowed(reqrepl);
		if(!bContinue)
		{
#ifdef DEBUG_BUILD
			if(logger)
				logger->Log("\tQueryResponseAllowed returned false\n");
#endif

			bRet = FALSE;
			lpBuffersOut->dwBufferLength = 0;
			goto Cleanup;
		}

#ifdef DEBUG_BUILD
		if(logger)
				logger->Log("\tQueryResponseAllowed returned true\n");
#endif
		// QueryResponseAllowed may have modified the data in the stream buffer

		if(pReqRepl->GetReplyBody())
		{
			PBYTE pbBuffer = pReqRepl->GetReplyBody()->Buffer();
			size_t cbBuffer = pReqRepl->GetReplyBody()->Length(), cbReadable;

			cbReadable = (cbBuffer >= lpBuffersOut->dwBufferLength) ? lpBuffersOut->dwBufferLength : cbBuffer;

			memcpy(lpBuffersOut->lpvBuffer, pbBuffer, cbReadable);
			lpBuffersOut->dwBufferLength = cbReadable;

			pReqRepl->SetReplyDataOffset(lpBuffersOut->dwBufferLength);
		}
	}
	else
	{
#ifdef DEBUG_BUILD
		if(logger)
			logger->Log("\tFailing readex attempt (disallowed)\n");
#endif
		bRet = FALSE;
		lpBuffersOut->dwBufferLength = 0;
		goto Cleanup;
	}
	
Cleanup:
	if(!bRet)
	{
		//memset(lpBuffersOut, 0, sizeof(INTERNET_BUFFERS));
		//lpBuffersOut->dwStructSize = sizeof(INTERNET_BUFFERS);
		//lpBuffersOut->dwBufferLength = 0;
	}

	if(pReqRepl)
		pReqRepl->Release();

#ifdef DEBUG_BUILD
	if(logger)
		logger->ReleaseLock();
#endif

	return bRet;
}

BOOL STDAPICALLTYPE WininetHooks::My_InternetReadFileExW(
  __in   HINTERNET hFile,
  __out  LPINTERNET_BUFFERSW lpBuffersOut,
  __in   DWORD dwFlags,
  __in   DWORD_PTR dwContext
)
{
	// Dummy function (not used):
	//   InternetReadFileExW calls InternetReadFileExA directly and does not convert any of the buffers
	//   to/from wide-char/ascii. Furthermore, in XP SP3 unpatched (perhaps patched too?) the W version
	//   of this function does not exist, just the A version.

	return FALSE;
}

BOOL STDAPICALLTYPE WininetHooks::My_InternetWriteFile(
	__in   HINTERNET hFile,
	__in   LPCVOID lpBuffer,
	__in   DWORD dwNumberOfBytesToWrite,
	__out  LPDWORD lpdwNumberOfBytesWritten
)
{
	BOOL bRet = FALSE;
	CConnectionPool *pConnPool = CConnectionPool::GetInstance();
	REQUESTREPLY reqrepl = (REQUESTREPLY)hFile;
	CRequestReply *pReqRepl = NULL;
#ifdef DEBUG_BUILD
	CLogger *logger = NULL;
#endif

	try
	{
#ifdef DEBUG_BUILD
		logger = CLogger::GetInstance("bpcore_wininet");
		logger->ObtainLock();
		logger->Log("My_InternetWriteFile: [reqrepl = 0x%x] (dwNumberOfBytesToWrite = %u)\n", reqrepl, dwNumberOfBytesToWrite);
#endif
	}
	catch(...)
	{
	}

	try
	{
		pReqRepl = pConnPool->GetRequestReply(reqrepl);
		if(!pReqRepl)
		{
			bRet = FALSE;
			goto Cleanup;
		}

		pReqRepl->AddRef();
	}
	catch(...)
	{
		bRet = FALSE;
		goto Cleanup;
	}

	if(lpBuffer)
	{
#ifdef DEBUG_BUILD
		if(logger)
		{
			//logger->Log("\tBuffer data to be written:\n");
			//logger->LogHex((PBYTE)lpBuffer, dwNumberOfBytesToWrite);
		}
#endif

		if(pReqRepl->GetRequestBody() == NULL)
		{
			pReqRepl->SetRequestInfo(reqrepl, (PBYTE)lpBuffer, dwNumberOfBytesToWrite, NULL);
		}
		else
		{
			CCommonTypeStreamReader *pStream = pReqRepl->GetRequestBody();

			if(pStream->Write(
					pStream->Length(),
					(PBYTE)lpBuffer,
					dwNumberOfBytesToWrite,
					TRUE
				) == EDataStreamError::StreamFailure)
			{
				bRet = FALSE;
				*lpdwNumberOfBytesWritten = 0;
				goto Cleanup;
			}
		}
	}

	BOOL bContinue = !bConnMonEnabled || !bValidateRequest || pConnPool->QueryRequestAllowed(reqrepl);
	//BOOL bContinue = pConnPool->QueryRequestAllowed(reqrepl);
	if(bContinue)
	{
#ifdef DEBUG_BUILD
		if(logger)
			logger->Log("\tQueryRequestAllowed returned true\n");
#endif
		// should read back changes to lpBuffer made by request callback subscribers. This isn't supported at present.
		bRet = Real_InternetWriteFile(hFile, lpBuffer, dwNumberOfBytesToWrite, lpdwNumberOfBytesWritten);
#ifdef DEBUG_BUILD
		if(logger)
			logger->Log("\tReal_InternetWriteFile returned %s\n", bRet ? "TRUE" : "FALSE");
#endif
	}
	else
	{
#ifdef DEBUG_BUILD
		if(logger)
			logger->Log("\tQueryRequestAllowed returned false\n");
#endif
		bRet = FALSE;
		*lpdwNumberOfBytesWritten = 0;
		goto Cleanup;
	}

Cleanup:
	if(!bRet)
	{
#ifdef DEBUG_BUILD
		if(logger)
			logger->Log("\tFailing write attempt\n");
#endif
	}
	else
	{
#ifdef DEBUG_BUILD
		if(logger)
		{
			logger->Log("\tData written (*lpdwNumberOfBytesWritten = %u)\n", *lpdwNumberOfBytesWritten);
			//logger->LogHex((PBYTE)lpBuffer, *lpdwNumberOfBytesWritten);
		}
#endif
	}

	if(pReqRepl)
		pReqRepl->Release();

#ifdef DEBUG_BUILD
	if(logger)
		logger->ReleaseLock();
#endif

	return bRet;
}

BOOL STDAPICALLTYPE WininetHooks::My_InternetCloseHandle(
	__in  HINTERNET hInternet
)
{
	BOOL bRet;
	CONNECTION conn = (CONNECTION)hInternet;
	REQUESTREPLY reqrepl = (REQUESTREPLY)hInternet;
	CConnectionPool *pConnPool = CConnectionPool::GetInstance();
#ifdef DEBUG_BUILD
	CLogger *logger = NULL;
#endif

	try
	{
#ifdef DEBUG_BUILD
		logger = CLogger::GetInstance("bpcore_wininet");
		logger->ObtainLock();
		logger->Log("My_InternetCloseHandle(conn = 0x%x)\n", conn);
#endif
	}
	catch(...)
	{
	}

	if(pConnPool->ConnectionExists(conn))
	{
		pConnPool->UnregisterConnection(conn);
	}
	else if(pConnPool->RequestReplyExist(reqrepl))
	{
		pConnPool->UnregisterRequestReply(reqrepl);
	}
	else
	{
#ifdef DEBUG_BUILD
		if(logger)
			logger->Log("\thInternet 0x%x does not exist as conn or reqrepl\n", hInternet);
#endif
	}
	
#ifdef DEBUG_BUILD
	if(logger)
		logger->ReleaseLock();
#endif

	bRet = Real_InternetCloseHandle(hInternet);
	
	return bRet;
}

void WininetHooks::ConfigRefresh(BOOL bRefresh)
{
	IConfigReader *pConfig = GetConfigReader();

	WCHAR *pwszTmp, *pwszToken;

	LPWSTR rgwszConfigName[4] = {
		L"enabled",
		L"validateConnect",
		L"validateRequest",
		L"validateResponse"
	};

	PBOOL rgpbool[4] = {
		&bConnMonEnabled,
		&bValidateConnect,
		&bValidateRequest,
		&bValidateResponse
	};

	for(int i=0; i<4; i++)
	{
		pwszTmp = pConfig->GetConfigElement(L"connectionMonitor", rgwszConfigName[i]);
		if(!pwszTmp)
			continue;

		if(!wcsicmp(pwszTmp, L"true"))
		{
			*(rgpbool[i]) = TRUE;
		}
		else
		{
			*(rgpbool[i]) = FALSE;
		}
	}
}