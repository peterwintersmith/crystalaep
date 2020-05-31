// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#include "stdafx.h"
#include "BpCore.h"
#include "BrwsrInstDetails.h"
#include "..\..\AuxCore\AuxCore\IRealtimeLog.h"

CBrowserInstanceDetails *CBrowserInstanceDetails::m_Instance = NULL;
CRITICAL_SECTION CBrowserInstanceDetails::m_csCreateInst = {0};

BOOL BrowserHooks::AttachAll()
{
//#define OFFSET_CWINDOW_GETADDRESSBAR	0x13AB25
//	ATTACH_DETOUR_PTR(L"mshtml.dll", OFFSET_CWINDOW_GETADDRESSBAR, CWindow__GetAddressBarUrl);
//#define OFFSET_CMARKUP_GETBASEURI	0x1EE407
//	ATTACH_DETOUR_PTR(L"mshtml.dll", OFFSET_CMARKUP_GETBASEURI, CMarkup__GetBaseUri);
	
	DETOUR_TRANS_BEGIN
	//ATTACH_DETOUR_GPA_ORDINAL(L"ieframe.dll", 170, ParseURLFromOutsideSourceW);
	// will not work on XP
	ATTACH_DETOUR_GPA_ORDINAL(L"ieframe.dll", 222, IECreateFromPathCPWithBCW);
	LONG lResult = DETOUR_TRANS_COMMIT
	
	IRealtimeLog *pRealtimeLog = GetRealtimeLog();

	switch(lResult)
	{
	case NO_ERROR:
		pRealtimeLog->LogMessage(0, L"Successfully intercepting address bar URL");
		break;
	case ERROR_INVALID_BLOCK:
		pRealtimeLog->LogMessage(1, L"Failed to intercept address bar URLs");
		BrowserHooks::DetachAll(); // shall we unwind?
		break;
	default:
		// fatal error: rewind
		pRealtimeLog->LogMessage(1, L"Unable to intercept URLs from address bar");
		BrowserHooks::DetachAll();
		break;
	}

	return TRUE;
}

BOOL BrowserHooks::DetachAll()
{
	DETOUR_TRANS_BEGIN
	//DETACH_DETOUR(CWindow__GetAddressBarUrl);
	//DETACH_DETOUR(ParseURLFromOutsideSourceW);
	DETACH_DETOUR(IECreateFromPathCPWithBCW);
	DETOUR_TRANS_COMMIT
	
	return TRUE;
}

long STDAPICALLTYPE BrowserHooks::My_CWindow__GetAddressBarUrl(CWindow *pThis, WCHAR **pwszAddressBarUrl)
{
	long lRet = Real_CWindow__GetAddressBarUrl(pThis, pwszAddressBarUrl);
	if(!lRet)
	{
		// success
		CBrowserInstanceDetails *pBrowser = CBrowserInstanceDetails::GetInstance();
		pBrowser->SetNavigatedAddress(wcsdup(*pwszAddressBarUrl)); // assumes ownership of the alloc
	}

	return 0;
}

// this does not log clicked links; only types urls
DWORD WINAPI BrowserHooks::My_ParseURLFromOutsideSourceW(
	LPCWSTR url,
	LPWSTR out,
	LPDWORD plen,
	LPDWORD unknown
)
{
	//DebugBreak();
	CBrowserInstanceDetails *pBrowser = CBrowserInstanceDetails::GetInstance();
	
	if(url)
		pBrowser->SetNavigatedAddress(wcsdup(url));
	
	return Real_ParseURLFromOutsideSourceW(url, out, plen, unknown);
}

// this one catches clicked links too
DWORD WINAPI BrowserHooks::My_IECreateFromPathCPWithBCW(
	DWORD dwUnknown1,
	LPWSTR lpwszUrl,
	DWORD dwUnknown2,
	LPDWORD lpdwUnknown3
)
{
	CBrowserInstanceDetails *pBrowser = CBrowserInstanceDetails::GetInstance();
	
	if(lpwszUrl)
		pBrowser->SetNavigatedAddress(wcsdup(lpwszUrl));
	
	return Real_IECreateFromPathCPWithBCW(dwUnknown1, lpwszUrl, dwUnknown2, lpdwUnknown3);
}

CBrowserInstanceDetails *CBrowserInstanceDetails::GetInstance()
{
	if(m_Instance == NULL)
	{
		EnterCriticalSection(&m_csCreateInst);

		if(m_Instance == NULL)
		{
			m_Instance = new CBrowserInstanceDetails();
		}

		LeaveCriticalSection(&m_csCreateInst);
	}

	return m_Instance;
}

// annoyingly the IE instance rendering content is not the IE instance with the address band root
// window that we need. This will work on IE6 but not 7/8
void CBrowserInstanceDetails::GetIEAddressBarText()
{
	HWND hwAddressBandRoot, hwEdit;
	WCHAR wszAddressBarUrl[4096] = {0};

	for(size_t i=0; i<0x800000; i++)
	{
		hwAddressBandRoot = FindWindowEx((HWND)i, NULL, L"address band root", NULL);
		if(hwAddressBandRoot)
		{
			hwEdit = FindWindowEx(hwAddressBandRoot, NULL, L"edit", NULL);
			if(hwEdit)
			{
				if(GetWindowTextW(hwEdit, wszAddressBarUrl, (sizeof(wszAddressBarUrl)/sizeof(WCHAR)) - 1))
				{
					SetNavigatedAddress(wcsdup(wszAddressBarUrl));
					break;
				}
			}
		}
	}
}

extern "C" __declspec(dllexport) CBrowserInstanceDetailsBase *GetBrowserInstanceDetails()
{
	return reinterpret_cast<CBrowserInstanceDetailsBase *>(CBrowserInstanceDetails::GetInstance());
}

/*
Function Find_Edit_X15_32bit()
'-- Created By Xinon Visual Basic API Spy Code Generator At 05/03/2011 18:28:50 ---
' 32 BIT Version
ieframe& = FindWindow("ieframe", vbNullString)
workerw& = FindWindowEx(ieframe&, 0&, "workerw", vbNullString)
rebarwindow& = FindWindowEx(workerw&, 0&, "rebarwindow32", vbNullString)
addressbandroot& = FindWindowEx(rebarwindow&, 0&, "address band root", vbNullString)
edit& = FindWindowEx(addressbandroot&, 0&, "edit", vbNullString)

Find_Edit_X15_32bit = edit&
End Function
*/