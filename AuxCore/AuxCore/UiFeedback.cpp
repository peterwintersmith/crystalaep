// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#include "stdafx.h"
#include "AuxCore.h"
#include "..\..\ProcTracker\ProcTracker\ProcTracker_h.h"
#include "ProcTrackerServices.h"
#include "UiFeedback.h"

CRITICAL_SECTION CUiFeedback::m_csCreateInst = {0};
CUiFeedback *CUiFeedback::m_Instance = NULL;

CUiFeedback* CUiFeedback::GetInstance()
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

BOOL CUiFeedback::UserBoolFeedback(EUiMessageStyle style, WCHAR *pwszCaption, WCHAR *pwszMessage)
{
	CProcTrackerServices *pProcTracker = CProcTrackerServices::GetInstance();
	return pProcTracker->UserBoolFeedback((UINT)style, pwszCaption, pwszMessage);
}

extern "C" __declspec(dllexport) CUiFeedbackBase *GetUiFeedback()
{
	return reinterpret_cast<CUiFeedbackBase *>(CUiFeedback::GetInstance());
}