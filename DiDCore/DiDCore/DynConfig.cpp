// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#include "stdafx.h"
#include "DiDCore.h"
#include "..\..\Shared\Utils\Utils.h"
#include "DynConfig.h"

CDynamicConfig *CDynamicConfig::m_Instance = NULL;
CRITICAL_SECTION CDynamicConfig::m_csCreateInst = {0}, CDynamicConfig::m_csPropertyOp = {0};

CDynamicConfig *CDynamicConfig::GetInstance()
{
	if(m_Instance == NULL)
	{
		EnterCriticalSection(&m_csCreateInst);

		if(m_Instance == NULL)
		{
			m_Instance = new CDynamicConfig();
		}

		LeaveCriticalSection(&m_csCreateInst);
	}

	return m_Instance;
}