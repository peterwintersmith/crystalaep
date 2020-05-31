// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#pragma once

struct CProcTrackerServicesBase {
	// IProcTracker
	virtual BOOL AddProcess(DWORD dwProcessID) = 0;

	// IUiFeedback
	virtual BOOL UserBoolFeedback(UINT style, WCHAR *pwszTitle, WCHAR *pwszCaption) = 0;

	// IRealtimeLog
	virtual BOOL SetEnabledFeaturesProcess(DWORD dwProcessID, DWORD dwFeaturesMask) = 0;
	virtual DWORD GetEnabledFeaturesProcess(DWORD dwProcessID) = 0;
	virtual BOOL AddEntryProcess(DWORD dwProcessID, RealtimeLogEntry *pRtLogEnt) = 0;
};