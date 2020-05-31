// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#pragma once

#include "EventWriterBase.h"
#include "..\..\Shared\Utils\Utils.h"

#define EVENT_XML_FILE_24MB_MAX_SIZE		(24 * 1024 * 1024)
#define EVENT_XML_FILE_TRUNCATE_SIZE		(4 * 1024 * 1024)

class CEventWriter : public CEventWriterBase {
	friend BOOL AuxCore::Initialize();
public:
	static CEventWriter* GetInstance();
	
	EEventWriteStatus WriteEvent(
			EEventRepository repos,
			EThreatRiskRating risk,
			LPWSTR pwszTitle,
			LPWSTR pwszBodyFormat,
			...
		);
	
	EEventWriteStatus WriteEventProcess(
			HANDLE hProcess,
			LPWSTR pwszProcessName,
			EEventRepository repos,
			EThreatRiskRating risk,
			LPWSTR pwszTitle,
			LPWSTR pwszBodyFormat,
			...
		);

private:
	CEventWriter()
	{
#define EVENT_WRITER_MUTEX_NAME	L"EventWriterSyncMutex"

		// ProcTracker has already created mutex and we should be able to get a handle to it if we're not running in
		// low integrity mode. If we are then we fall back to RPC so it's not a problem.
		m_hMutex = OpenMutex(SYNCHRONIZE, FALSE, EVENT_WRITER_MUTEX_NAME);
	}

	EEventWriteStatus WriteEventInternal(
			HANDLE hProcess,
			LPWSTR pwszProcessName,
			EEventRepository repos,
			EThreatRiskRating risk,
			LPWSTR pwszTitle,
			LPWSTR pwszBody
		);

	EEventWriteStatus WriteEventInProc(
			HANDLE hProcess,
			LPWSTR pwszProcessName,
			EEventRepository repos,
			EThreatRiskRating risk,
			LPWSTR pwszTitle,
			LPWSTR pwszBody
		);

	EEventWriteStatus CEventWriter::WriteEventRPC(
			HANDLE hProcess,
			LPWSTR pwszProcessName,
			EEventRepository repos,
			EThreatRiskRating risk,
			LPWSTR pwszTitle,
			LPWSTR pwszBody
		);

	static CEventWriter *m_Instance;
	static CRITICAL_SECTION m_csCreateInst;

	static LPWSTR m_rgwszFileNames[10];
	static LPSTR m_rgszRiskRatings[4];
	
	HANDLE m_hMutex;
};