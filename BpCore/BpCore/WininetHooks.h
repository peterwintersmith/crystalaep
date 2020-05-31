// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#pragma once

#include "ConnectionPool.h"

using namespace DetoursFree;

namespace WininetHooks {

#define DETOUR_TRANS_BEGIN    DetourTransactionBegin(); DetourUpdateThread(GetCurrentThread()); 
#define DETOUR_TRANS_COMMIT   DetourTransactionCommit();

#define ATTACH_DETOUR(func)    {\
	Real_##func = func;\
	DetourAttach((PVOID *)&Real_##func, (PVOID)My_##func);\
}

#define DETACH_DETOUR(func)    {\
	DetourDetach((PVOID *)&Real_##func, (PVOID)My_##func);\
}

#define ATTACH_DETOUR_GPA(mod, name, func)    {\
	PVOID *pvTmp = (PVOID *)&Real_##func;\
	*pvTmp = (PVOID)GetProcAddress((GetModuleHandle(mod) ? GetModuleHandle(mod) : LoadLibrary(mod)), name);\
	DetourAttach((PVOID *)&Real_##func, (PVOID)My_##func);\
}

#define DEFAULT_READ_BUFFER_SIZE	4096
#define MAXIMUM_SINGLE_READ_SIZE	(512 * 1024)

	static HMODULE hmBpcore = NULL, hmWininet = NULL;
	
	static BOOL bConnMonEnabled = TRUE, bValidateConnect = TRUE, bValidateRequest = TRUE,
		bValidateResponse = TRUE;

	static EStreamReadCallbackError Callback_InternetReadFile(
		void *pStateParam,
		PBYTE pbOutputBuffer,
		size_t cbReadSize,
		size_t *pcbBytesRead
	);
	
	typedef HINTERNET (STDAPICALLTYPE *FN_INTERNETCONNECTA_DEF)(
	  __in  HINTERNET hInternet,
	  __in  const char* lpszServerName,
	  __in  INTERNET_PORT nServerPort,
	  __in  const char* lpszUsername,
	  __in  const char* lpszPassword,
	  __in  DWORD dwService,
	  __in  DWORD dwFlags,
	  __in  DWORD_PTR dwContext
	);

	typedef HINTERNET (STDAPICALLTYPE *FN_INTERNETCONNECTW_DEF)(
	  __in  HINTERNET hInternet,
	  __in  LPCTSTR lpszServerName,
	  __in  INTERNET_PORT nServerPort,
	  __in  LPCTSTR lpszUsername,
	  __in  LPCTSTR lpszPassword,
	  __in  DWORD dwService,
	  __in  DWORD dwFlags,
	  __in  DWORD_PTR dwContext
	);

	typedef HINTERNET (STDAPICALLTYPE *FN_HTTPOPENREQUESTA_DEF)(
	  __in  HINTERNET hConnect,
	  __in  const char* lpszVerb,
	  __in  const char* lpszObjectName,
	  __in  const char* lpszVersion,
	  __in  const char* lpszReferer,
	  __in  const char* *lplpszAcceptTypes,
	  __in  DWORD dwFlags,
	  __in  DWORD_PTR dwContext
	);

	typedef HINTERNET (STDAPICALLTYPE *FN_HTTPOPENREQUESTW_DEF)(
	  __in  HINTERNET hConnect,
	  __in  LPCTSTR lpszVerb,
	  __in  LPCTSTR lpszObjectName,
	  __in  LPCTSTR lpszVersion,
	  __in  LPCTSTR lpszReferer,
	  __in  LPCTSTR *lplpszAcceptTypes,
	  __in  DWORD dwFlags,
	  __in  DWORD_PTR dwContext
	);
	
	typedef BOOL (STDAPICALLTYPE *FN_HTTPSENDREQUESTA_DEF)(
	  __in  HINTERNET hRequest,
	  __in  const char* lpszHeaders,
	  __in  DWORD dwHeadersLength,
	  __in  LPVOID lpOptional,
	  __in  DWORD dwOptionalLength
	);

	typedef BOOL (STDAPICALLTYPE *FN_HTTPSENDREQUESTW_DEF)(
	  __in  HINTERNET hRequest,
	  __in  LPCTSTR lpszHeaders,
	  __in  DWORD dwHeadersLength,
	  __in  LPVOID lpOptional,
	  __in  DWORD dwOptionalLength
	);

	typedef BOOL (STDAPICALLTYPE *FN_HTTPSENDREQUESTEXA_DEF)(
	  __in   HINTERNET hRequest,
	  __in   LPINTERNET_BUFFERSA lpBuffersIn,
	  __out  LPINTERNET_BUFFERSA lpBuffersOut,
	  __in   DWORD dwFlags,
	  __in   DWORD_PTR dwContext
	);
	
	typedef BOOL (STDAPICALLTYPE *FN_HTTPSENDREQUESTEXW_DEF)(
	  __in   HINTERNET hRequest,
	  __in   LPINTERNET_BUFFERSW lpBuffersIn,
	  __out  LPINTERNET_BUFFERSW lpBuffersOut,
	  __in   DWORD dwFlags,
	  __in   DWORD_PTR dwContext
	);

	typedef BOOL (STDAPICALLTYPE *FN_HTTPQUERYINFOA_DEF)(
	  __in     HINTERNET hRequest,
	  __in     DWORD dwInfoLevel,
	  __inout  LPVOID lpvBuffer,
	  __inout  LPDWORD lpdwBufferLength,
	  __inout  LPDWORD lpdwIndex
	);
	
	typedef BOOL (STDAPICALLTYPE *FN_HTTPQUERYINFOW_DEF)(
	  __in     HINTERNET hRequest,
	  __in     DWORD dwInfoLevel,
	  __inout  LPVOID lpvBuffer,
	  __inout  LPDWORD lpdwBufferLength,
	  __inout  LPDWORD lpdwIndex
	);
	
	typedef BOOL (STDAPICALLTYPE *FN_INTERNETQUERYDATAAVAILABLE_DEF)(
	  __in   HINTERNET hFile,
	  __out  LPDWORD lpdwNumberOfBytesAvailable,
	  __in   DWORD dwFlags,
	  __in   DWORD_PTR dwContext
	);

	typedef BOOL (STDAPICALLTYPE *FN_INTERNETREADFILE_DEF)(
	  __in   HINTERNET hFile,
	  __out  LPVOID lpBuffer,
	  __in   DWORD dwNumberOfBytesToRead,
	  __out  LPDWORD lpdwNumberOfBytesRead
	);

	typedef BOOL (STDAPICALLTYPE *FN_INTERNETREADFILEEXA_DEF)(
	  __in   HINTERNET hFile,
	  __out  LPINTERNET_BUFFERSA lpBuffersOut,
	  __in   DWORD dwFlags,
	  __in   DWORD_PTR dwContext
	);
	
	typedef BOOL (STDAPICALLTYPE *FN_INTERNETREADFILEEXW_DEF)(
	  __in   HINTERNET hFile,
	  __out  LPINTERNET_BUFFERSW lpBuffersOut,
	  __in   DWORD dwFlags,
	  __in   DWORD_PTR dwContext
	);

	typedef BOOL (STDAPICALLTYPE *FN_INTERNETWRITEFILE_DEF)(
		__in   HINTERNET hFile,
		__in   LPCVOID lpBuffer,
		__in   DWORD dwNumberOfBytesToWrite,
		__out  LPDWORD lpdwNumberOfBytesWritten
	);

	typedef BOOL (STDAPICALLTYPE *FN_INTERNETCLOSEHANDLE_DEF)(
		__in  HINTERNET hInternet
	);

	static FN_INTERNETCONNECTA_DEF			 Real_InternetConnectA = NULL;
	static FN_INTERNETCONNECTW_DEF			 Real_InternetConnectW = NULL;
	static FN_HTTPOPENREQUESTA_DEF			 Real_HttpOpenRequestA = NULL;
	static FN_HTTPOPENREQUESTW_DEF			 Real_HttpOpenRequestW = NULL;
	static FN_HTTPSENDREQUESTA_DEF			 Real_HttpSendRequestA = NULL;
	static FN_HTTPSENDREQUESTW_DEF			 Real_HttpSendRequestW = NULL;
	static FN_HTTPSENDREQUESTEXA_DEF		 Real_HttpSendRequestExA = NULL;
	static FN_HTTPSENDREQUESTEXW_DEF		 Real_HttpSendRequestExW = NULL;
	static FN_HTTPQUERYINFOA_DEF			 Real_HttpQueryInfoA   = NULL;
	static FN_HTTPQUERYINFOW_DEF			 Real_HttpQueryInfoW   = NULL;
	static FN_INTERNETQUERYDATAAVAILABLE_DEF Real_InternetQueryDataAvailable = NULL;
	static FN_INTERNETREADFILE_DEF			 Real_InternetReadFile = NULL;
	static FN_INTERNETREADFILEEXA_DEF		 Real_InternetReadFileExA = NULL;
	static FN_INTERNETREADFILEEXW_DEF		 Real_InternetReadFileExW = NULL;
	static FN_INTERNETWRITEFILE_DEF			 Real_InternetWriteFile = NULL;
	static FN_INTERNETCLOSEHANDLE_DEF		 Real_InternetCloseHandle = NULL;
	
	HINTERNET STDAPICALLTYPE My_InternetConnectA(
	  __in  HINTERNET hInternet,
	  __in  const char* lpszServerName,
	  __in  INTERNET_PORT nServerPort,
	  __in  const char* lpszUsername,
	  __in  const char* lpszPassword,
	  __in  DWORD dwService,
	  __in  DWORD dwFlags,
	  __in  DWORD_PTR dwContext
	);

	HINTERNET STDAPICALLTYPE My_InternetConnectW(
	  __in  HINTERNET hInternet,
	  __in  LPCTSTR lpszServerName,
	  __in  INTERNET_PORT nServerPort,
	  __in  LPCTSTR lpszUsername,
	  __in  LPCTSTR lpszPassword,
	  __in  DWORD dwService,
	  __in  DWORD dwFlags,
	  __in  DWORD_PTR dwContext
	);
	
	HINTERNET STDAPICALLTYPE My_HttpOpenRequestA(
	  __in  HINTERNET hConnect,
	  __in  const char* lpszVerb,
	  __in  const char* lpszObjectName,
	  __in  const char* lpszVersion,
	  __in  const char* lpszReferer,
	  __in  const char* *lplpszAcceptTypes,
	  __in  DWORD dwFlags,
	  __in  DWORD_PTR dwContext
	);

	HINTERNET STDAPICALLTYPE My_HttpOpenRequestW(
	  __in  HINTERNET hConnect,
	  __in  LPCTSTR lpszVerb,
	  __in  LPCTSTR lpszObjectName,
	  __in  LPCTSTR lpszVersion,
	  __in  LPCTSTR lpszReferer,
	  __in  LPCTSTR *lplpszAcceptTypes,
	  __in  DWORD dwFlags,
	  __in  DWORD_PTR dwContext
	);

	BOOL STDAPICALLTYPE My_HttpSendRequestA(
	  __in  HINTERNET hRequest,
	  __in  const char* lpszHeaders,
	  __in  DWORD dwHeadersLength,
	  __in  LPVOID lpOptional,
	  __in  DWORD dwOptionalLength
	);

	BOOL STDAPICALLTYPE My_HttpSendRequestW(
	  __in  HINTERNET hRequest,
	  __in  LPCTSTR lpszHeaders,
	  __in  DWORD dwHeadersLength,
	  __in  LPVOID lpOptional,
	  __in  DWORD dwOptionalLength
	);
	
	BOOL STDAPICALLTYPE My_HttpSendRequestExA(
	  __in   HINTERNET hRequest,
	  __in   LPINTERNET_BUFFERSA lpBuffersIn,
	  __out  LPINTERNET_BUFFERSA lpBuffersOut,
	  __in   DWORD dwFlags,
	  __in   DWORD_PTR dwContext
	);

	BOOL STDAPICALLTYPE My_HttpSendRequestExW(
	  __in   HINTERNET hRequest,
	  __in   LPINTERNET_BUFFERSW lpBuffersIn,
	  __out  LPINTERNET_BUFFERSW lpBuffersOut,
	  __in   DWORD dwFlags,
	  __in   DWORD_PTR dwContext
	);

	BOOL STDAPICALLTYPE My_HttpQueryInfoA(
	  __in     HINTERNET hRequest,
	  __in     DWORD dwInfoLevel,
	  __inout  LPVOID lpvBuffer,
	  __inout  LPDWORD lpdwBufferLength,
	  __inout  LPDWORD lpdwIndex
	);

	BOOL STDAPICALLTYPE My_HttpQueryInfoW(
	  __in     HINTERNET hRequest,
	  __in     DWORD dwInfoLevel,
	  __inout  LPVOID lpvBuffer,
	  __inout  LPDWORD lpdwBufferLength,
	  __inout  LPDWORD lpdwIndex
	);
	
	BOOL STDAPICALLTYPE My_InternetReadFile(
	  __in   HINTERNET hFile,
	  __out  LPVOID lpBuffer,
	  __in   DWORD dwNumberOfBytesToRead,
	  __out  LPDWORD lpdwNumberOfBytesRead
	);

	BOOL STDAPICALLTYPE My_InternetReadFileExA(
	  __in   HINTERNET hFile,
	  __out  LPINTERNET_BUFFERSA lpBuffersOut,
	  __in   DWORD dwFlags,
	  __in   DWORD_PTR dwContext
	);

	BOOL STDAPICALLTYPE My_InternetReadFileExW(
	  __in   HINTERNET hFile,
	  __out  LPINTERNET_BUFFERSW lpBuffersOut,
	  __in   DWORD dwFlags,
	  __in   DWORD_PTR dwContext
	);

	BOOL STDAPICALLTYPE My_InternetQueryDataAvailable(
	  __in   HINTERNET hFile,
	  __out  LPDWORD lpdwNumberOfBytesAvailable,
	  __in   DWORD dwFlags,
	  __in   DWORD_PTR dwContext
	);

	BOOL STDAPICALLTYPE My_InternetWriteFile(
		__in   HINTERNET hFile,
		__in   LPCVOID lpBuffer,
		__in   DWORD dwNumberOfBytesToWrite,
		__out  LPDWORD lpdwNumberOfBytesWritten
	);

	BOOL STDAPICALLTYPE My_InternetCloseHandle(
		__in  HINTERNET hInternet
	);

	BOOL AttachAll();
	BOOL DetachAll();

	void ConfigRefresh(BOOL bRefresh);
}