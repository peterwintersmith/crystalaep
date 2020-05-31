// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#pragma once

#include "..\..\Shared\Utils\Utils.h"

enum EEventWriteStatus {
	StatusSuccess = 0,
	StatusFailure
};

enum EEventRepository {
	AntiSpray,
	ApiFirewall,
	AudioValidator,
	ConnectionMonitor,
	DfnsInDepth,
	HeapMonitor,
	ImageValidator,
	StackMonitor,
	VideoValidator,
	WebAppMonitor
};

enum EThreatRiskRating {
	Information = 0,
	LowRisk,
	MediumRisk,
	HighRisk
};

#define EVENT_XML_FILE_24MB_MAX_SIZE		(24 * 1024 * 1024)
#define EVENT_XML_FILE_TRUNCATE_SIZE		(4 * 1024 * 1024)

class CEventWriterRpc {
	friend int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow);
public:
	static CEventWriterRpc *GetInstance();

	RPC_STATUS StartRPCServer();
	RPC_STATUS StopRPCServer();

	EEventWriteStatus WriteEventInternal(
		HANDLE hProcess,
		LPWSTR pwszProcessName,
		EEventRepository repos,
		EThreatRiskRating risk,
		LPWSTR pwszTitle,
		LPWSTR pwszBody
	);

private:
	CEventWriterRpc() {
#define EVENT_WRITER_MUTEX_NAME	L"EventWriterSyncMutex"
		SECURITY_DESCRIPTOR sd = {0};
		SECURITY_ATTRIBUTES sa = {0};

		if(!Utils::GetNullDacl(&sa, &sd))
		{
			throw new std::exception("Event writer security structure cannot be created");
		}

		m_hMutex = CreateMutex(&sa, FALSE, EVENT_WRITER_MUTEX_NAME);
		if(!m_hMutex)
		{
			throw new std::exception("Event writer synchronisation mutex cannot be created");
		}
	}

	static CRITICAL_SECTION m_csCreateInst;
	static CEventWriterRpc *m_Instance;
	
	static LPWSTR m_rgwszFileNames[10];
	static LPSTR m_rgszRiskRatings[4];
	
	HANDLE m_hMutex;
};