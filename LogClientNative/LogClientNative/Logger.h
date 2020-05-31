// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#pragma once

//#include <deque>

#define LOGGER_MAX_INSTANCES    10

class CLogger {
public:

	static CLogger *GetInstance(char *pszPipeName);
	static void CloseInstance(char *pszPipeName, CLogger *&pInst);
	static BOOL CloseInstance(char *pszPipeName);
	static int GetInstCount();
	BOOL CLogger::ObtainLock(DWORD dwTimeout);
	void ObtainLock();
	void ReleaseLock();
	BOOL Log(int value, int base = 10);
	BOOL Log( char *pszFmt, ... );
	BOOL Log( std::string str );
	BOOL LogHex(PBYTE pb, size_t cb);
	static void Break();
	static void BreakIf(BOOL bCondition);

private:
	
	FILE *Open(char *pszPipeName);
	
	CLogger() : m_fpPipe(0) {
		m_szPipeName[0] = 0;
	}

	typedef struct
	{
		char szPipeName[260];
		CLogger *pInst;
		FILE *fpPipe;

	} LoggerInstance;

	static LoggerInstance m_Instances[LOGGER_MAX_INSTANCES];
	static int m_nInstances;

	FILE *m_fpPipe;
	char m_szPipeName[260];
	HANDLE m_hMutex;
	
	//std::deque<std::string> m_queuedMessages;
	BOOL m_bReopen;
};

