// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#pragma once

enum EUiMessageStyle {
	UiStyle_IconExclamation = MB_ICONEXCLAMATION,
	UiStyle_IconWarning = MB_ICONWARNING,
	UiStyle_IconInformation = MB_ICONINFORMATION,
	UiStyle_IconAsterisx = MB_ICONASTERISK,
	UiStyle_IconQuestion = MB_ICONQUESTION,
	UiStyle_IconStop = MB_ICONSTOP,
	UiStyle_IconError = MB_ICONERROR,
	UiStyle_IconHand = MB_ICONHAND
};

class CUiFeedback {
	friend int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow);
public:
	static CUiFeedback *GetInstance();

	RPC_STATUS StartRPCServer();
	RPC_STATUS StopRPCServer();
	
	BOOL UserBoolFeedback(EUiMessageStyle style, WCHAR *pwszCaption, WCHAR *pwszMessage);

private:
	CUiFeedback() {
		InitializeCriticalSection(&m_csUiOp);
	}

	static CRITICAL_SECTION m_csCreateInst, m_csUiOp;
	static CUiFeedback *m_Instance;
};