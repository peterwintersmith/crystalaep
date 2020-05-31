// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#pragma once

#include "BrwsrInstDetailsBase.h"
#include "..\..\AuxCore\AuxCore\IRealtimeLog.h"

using namespace DetoursFree;

namespace BrowserHooks {
	
#define DETOUR_TRANS_BEGIN    DetourTransactionBegin(); DetourUpdateThread(GetCurrentThread());
#define DETOUR_TRANS_COMMIT   DetourTransactionCommit();

#define ATTACH_DETOUR(func)    {\
	Real_##func = func;\
	DetourAttach((PVOID *)&Real_##func, (PVOID)My_##func);\
}

#define DETACH_DETOUR(func)    {\
	DetourDetach((PVOID *)&Real_##func, (PVOID)My_##func);\
}

#define ATTACH_DETOUR_PTR(mod, offs, func)    {\
	PVOID *pvTmp = (PVOID *)&Real_##func;\
	*pvTmp = (PVOID)((PBYTE)(GetModuleHandle(mod) ? GetModuleHandle(mod) : LoadLibrary(mod)) + (size_t)offs);\
	DetourAttach((PVOID *)&Real_##func, (PVOID)My_##func);\
}

#define ATTACH_DETOUR_GPA(mod, name, func)    {\
	PVOID *pvTmp = (PVOID *)&Real_##func;\
	*pvTmp = (PVOID)GetProcAddress((GetModuleHandle(mod) ? GetModuleHandle(mod) : LoadLibrary(mod)), name);\
	DetourAttach((PVOID *)&Real_##func, (PVOID)My_##func);\
}

#define ATTACH_DETOUR_GPA_ORDINAL(mod, ordinal, func)    {\
	PVOID *pvTmp = (PVOID *)&Real_##func;\
	*pvTmp = (PVOID)GetProcAddress((GetModuleHandle(mod) ? GetModuleHandle(mod) : LoadLibrary(mod)), (char *)ordinal);\
	DetourAttach((PVOID *)&Real_##func, (PVOID)My_##func);\
}

	BOOL AttachAll();
	BOOL DetachAll();

	typedef void CWindow;
	
	typedef DWORD (WINAPI *FN_PARSEURLFROMOUTSIDESOURCEW_DEF)(
		LPCWSTR url,
		LPWSTR out,
		LPDWORD plen,
		LPDWORD unknown
	);

	typedef DWORD (WINAPI *FN_IECREATEFROMPATHCPWITHBCW_DEF)(
		DWORD dwUnknown1,
		LPWSTR lpwszUrl,
		DWORD dwUnknown2,
		LPDWORD lpdwUnknown3
	);

	typedef long (STDAPICALLTYPE *FN_CWINDOW__GETADDRESSBARURL_DEF)(
		__in  CWindow *pThis,
		__out WCHAR **pwszAddressBarUrl
	);

	static FN_CWINDOW__GETADDRESSBARURL_DEF Real_CWindow__GetAddressBarUrl = NULL;	
	static FN_PARSEURLFROMOUTSIDESOURCEW_DEF Real_ParseURLFromOutsideSourceW = NULL;
	static FN_IECREATEFROMPATHCPWITHBCW_DEF Real_IECreateFromPathCPWithBCW = NULL;
	
	
	long STDAPICALLTYPE My_CWindow__GetAddressBarUrl(
		__in  CWindow *pThis,
		__out WCHAR **pwszAddressBarUrl
	);

	DWORD WINAPI My_ParseURLFromOutsideSourceW(
		LPCWSTR url,
		LPWSTR out,
		LPDWORD plen,
		LPDWORD unknown
	);

	DWORD WINAPI My_IECreateFromPathCPWithBCW(
		DWORD dwUnknown1,
		LPWSTR lpwszUrl,
		DWORD dwUnknown2,
		LPDWORD lpdwUnknown3
	);
};

class CBrowserInstanceDetails : public CBrowserInstanceDetailsBase {
	friend BOOL BpCore::Initialize();

public:
	~CBrowserInstanceDetails() {
		if(m_pwszLastNavigatedAddr)
			free(m_pwszLastNavigatedAddr);
		
		m_pwszLastNavigatedAddr = NULL;
	}

	static CBrowserInstanceDetails *GetInstance();
	
	LPWSTR GetNavigatedAddress() {
		return m_pwszLastNavigatedAddr;
	}
	
	void SetNavigatedAddress(LPWSTR pwsz) {
		
		if(pwsz)
		{
			if(wcsnicmp(pwsz, L"http://", 7) && wcsnicmp(pwsz, L"https://", 7) &&
				wcsnicmp(pwsz, L"file://", 7))
			{
				return;
			}
		}

		if(m_pwszLastNavigatedAddr)
		{
			free(m_pwszLastNavigatedAddr);
			m_pwszLastNavigatedAddr = NULL;
		}

#ifdef DEBUG_BUILD
		if(1)
		{
			if(pwsz)
			{
				std::wstring wstrTmp = L"CBrowserInstanceDetails: Url=";
				wstrTmp += pwsz;
				wstrTmp += L"\n";
				OutputDebugString(wstrTmp.c_str());
			}
		}
#endif

		IRealtimeLog *pRealtimeLog = GetRealtimeLog();
		pRealtimeLog->LogMessage(1, L"Address bar change %s", pwsz);

		m_pwszLastNavigatedAddr = pwsz;
	}

	void GetIEAddressBarText();

private:
	CBrowserInstanceDetails() : m_pwszLastNavigatedAddr(0), m_pwszPageBaseUrl(0)
	{
	}

	static CBrowserInstanceDetails *m_Instance;
	static CRITICAL_SECTION m_csCreateInst;
	
	LPWSTR m_pwszLastNavigatedAddr, *m_pwszPageBaseUrl;
};