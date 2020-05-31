// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#pragma once

struct CRealtimeLogBase {
	virtual BOOL Start() = 0;
	virtual BOOL LogMessage(DWORD dwSeverity, LPWSTR lpwszMessage, ...) = 0;
	virtual void Terminate() = 0;
};