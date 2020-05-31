// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#pragma once

#include "UiFeedbackBase.h"

class CUiFeedback : public CUiFeedbackBase {
	friend BOOL AuxCore::Initialize();
public:
	static CUiFeedback* GetInstance();
	
	BOOL UserBoolFeedback(EUiMessageStyle style, WCHAR *pwszTitle, WCHAR *pwszCaption);

private:
	CUiFeedback()
	{
		// nothing to do
	}

	static CUiFeedback *m_Instance;
	static CRITICAL_SECTION m_csCreateInst;
	HANDLE m_hMutex;
};