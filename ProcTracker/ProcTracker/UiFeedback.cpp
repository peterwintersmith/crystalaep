// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#include "stdafx.h"
#include "ProcTracker_h.h"
#include "UiFeedback.h"

CRITICAL_SECTION CUiFeedback::m_csCreateInst = {0}, CUiFeedback::m_csUiOp = {0};
CUiFeedback *CUiFeedback::m_Instance = NULL;

CUiFeedback *CUiFeedback::GetInstance()
{
	if(m_Instance == NULL)
	{
		EnterCriticalSection(&m_csCreateInst);

		if(m_Instance == NULL)
		{
			m_Instance = new CUiFeedback();
		}

		LeaveCriticalSection(&m_csCreateInst);
	}

	return m_Instance;
}

RPC_STATUS CUiFeedback::StartRPCServer()
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
			(RPC_WSTR)L"UiFeedback",
			pSecDesc
		);
	if(rpcStatus != RPC_S_OK)
		goto Cleanup;

	rpcStatus = RpcServerRegisterIf(IUiFeedback_v1_0_s_ifspec, NULL, NULL);
	if(rpcStatus != RPC_S_OK)
		goto Cleanup;

	rpcStatus = RpcServerListen(1, RPC_C_LISTEN_MAX_CALLS_DEFAULT, FALSE);
	
	// server will be ongoing

Cleanup:
	if(pSecDesc)
		LocalFree(pSecDesc);

	return rpcStatus;
}

RPC_STATUS CUiFeedback::StopRPCServer()
{
	RPC_STATUS rpcStatus;
	
	rpcStatus = RpcMgmtStopServerListening(IUiFeedback_v1_0_s_ifspec);
	if(rpcStatus != RPC_S_OK)
		goto Cleanup;

	rpcStatus = RpcServerUnregisterIf(IUiFeedback_v1_0_s_ifspec, NULL, TRUE);

Cleanup:
	return rpcStatus;
}

BOOL CUiFeedback::UserBoolFeedback(EUiMessageStyle style, WCHAR *pwszCaption, WCHAR *pwszMessage)
{
	BOOL bResult = FALSE;
	int iChoice;
	
	EnterCriticalSection(&m_csUiOp);
	
	/*
		- MB_SYSTEMMODAL flag ensures that the user must respond to the message box before continuing work in the window identified by the hWnd parameter.
		- MB_TOPMOST flag specifies that the message box should be placed above all non-topmost windows and should stay above them, even when the window is deactivated.
		- MB_SETFOREGROUND flag ensures that the message box becomes the foreground window.
	*/

	// weirdly if hWnd = NULL then MessageBox returns zero (error) - use desktop window handle
	iChoice = MessageBox(GetDesktopWindow(), pwszMessage, pwszCaption, MB_SYSTEMMODAL | MB_YESNO | MB_DEFBUTTON2 | (UINT)style);

	if(iChoice == IDYES)
	{
		bResult = TRUE;
	}

	LeaveCriticalSection(&m_csUiOp);

	return bResult;
}

BOOL UserBoolFeedback( 
    /* [in] */ handle_t IDL_handle,
    UINT style,
    WCHAR *pwszTitle,
    WCHAR *pwszCaption)
{
	CUiFeedback *pUiFeedback = CUiFeedback::GetInstance();
	return pUiFeedback->UserBoolFeedback((EUiMessageStyle)style, pwszTitle, pwszCaption);
}