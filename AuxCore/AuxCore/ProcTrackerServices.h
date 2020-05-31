// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#pragma once

#include "AuxCore.h"
#include "ProcTrackerServicesBase.h"

class CProcTrackerServices : public CProcTrackerServicesBase {
	friend BOOL AuxCore::Initialize();
public:
	static CProcTrackerServices* GetInstance();

	// IProcTracker
	BOOL AddProcess(DWORD dwProcessID);

	// IUiFeedback
	BOOL UserBoolFeedback(UINT style, WCHAR *pwszTitle, WCHAR *pwszCaption);

	// IRealtimeLog
	BOOL SetEnabledFeaturesProcess(DWORD dwProcessID, DWORD dwFeaturesMask);
	DWORD GetEnabledFeaturesProcess(DWORD dwProcessID);
	BOOL AddEntryProcess(DWORD dwProcessID, RealtimeLogEntry *pRtLogEnt);
	
private:
	CProcTrackerServices(){
	}

	static CProcTrackerServices *m_Instance;
	static CRITICAL_SECTION m_csCreateInst;
};