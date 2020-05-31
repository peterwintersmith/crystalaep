// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#pragma once 

#include "BpCore.h"
#include "Stream.h"
#include "ParseTempStg.h"

class CConnectionPool;
class CConnection;
class CHistoricExchange;
class CRequestReply;
class CDataStreamBase;
class CCommonTypeStreamReader;
class CParseTemporaryStorage;

enum EConnectionPoolError
{
	Failure,
	Success
};

enum EAllowableOperation
{
	Disallowed,
	Allowed
};

typedef EAllowableOperation (*PFN_CP_CALLBACK)(CConnectionPool *pConnPool, CConnection *pConn);
typedef DWORD CONNECTION;
typedef DWORD REQUESTREPLY;

#include "ConnectionPoolBase.h"

class CConnectionPool : public CConnectionPoolBase
{
	friend BOOL BpCore::Initialize();
public:
	static CConnectionPool *GetInstance();

	BOOL QueryConnectAllowed(CONNECTION conn);
	BOOL QueryRequestAllowed(REQUESTREPLY reqrepl);
	BOOL QueryResponseAllowed(REQUESTREPLY reqrepl);

	EConnectionPoolError AddOpenCallback(PFN_CP_CALLBACK pfn, DWORD dwLevel);
	EConnectionPoolError AddCloseCallback(PFN_CP_CALLBACK pfn, DWORD dwLevel) { return EConnectionPoolError::Failure; };
    EConnectionPoolError AddRequestCallback(PFN_CP_CALLBACK pfn, DWORD dwLevel);
    EConnectionPoolError AddResponseCallback(PFN_CP_CALLBACK pfn, DWORD dwLevel);
    EConnectionPoolError RemoveCallback(PFN_CP_CALLBACK pfn);

	BOOL ConnectionExists(CONNECTION conn);
    EConnectionPoolError RegisterConnection(CONNECTION conn);
    CConnection *GetConnection(CONNECTION conn);
    EConnectionPoolError UnregisterConnection(CONNECTION conn);

	BOOL RequestReplyExist(REQUESTREPLY reqrepl);
	EConnectionPoolError RegisterRequestReply(REQUESTREPLY reqrepl);
	CRequestReply *GetRequestReply(REQUESTREPLY reqrepl);
    EConnectionPoolError UnregisterRequestReply(REQUESTREPLY reqrepl);

	void Terminate();

private:
	CConnectionPool() {
		InitializeCriticalSection(&m_csConnOperation);
		InitializeCriticalSection(&m_csReqReplOperation);
		InitializeCriticalSection(&m_csGC);
		
		m_hGcThread = CreateThread(NULL, 0, DeferredGarbageCollector, (LPVOID)'BPGC', 0, &m_dwGcThreadId);
		//if(!m_hGcThread)
		//{
		//	throw new std::exception("Unable to create deferred garbage collection thread\n");
		//}
	}

	static DWORD WINAPI DeferredGarbageCollector(LPVOID lpThreadParameter);
	
	static void RegisterForGC(CConnection *p){
		EnterCriticalSection(&m_csGC);
		m_vecGcConn.push_back(p);
		LeaveCriticalSection(&m_csGC);
	}

	static void RegisterForGC(CRequestReply *p){
		EnterCriticalSection(&m_csGC);
		m_vecGcReqRepl.push_back(p);
		LeaveCriticalSection(&m_csGC);
	}

	static HANDLE m_hGcThread;
	static DWORD m_dwGcThreadId;
	static BOOL m_bGcUnload;
	static std::vector<CConnection *> m_vecGcConn;
	static std::vector<CRequestReply *> m_vecGcReqRepl;
	static CRITICAL_SECTION m_csGC;
	
	static CRITICAL_SECTION m_csCreateInst;
	static CConnectionPool *m_Instance;
	
	std::map<CONNECTION, CConnection *> m_mapConns;
	CRITICAL_SECTION m_csConnOperation;

	std::map<CONNECTION, CRequestReply *> m_mapRequestReply;
	CRITICAL_SECTION m_csReqReplOperation;

	std::vector<PFN_CP_CALLBACK> m_vecOpenCallbacks, m_vecRequestCallbacks, m_vecResponseCallbacks;
};

class CConnection : public CConnectionBase
{
public:
	CConnection() : m_pszHost(0), m_pszObjectName(0), m_lIpAddress(0), m_nPort(0),
		m_bSsl(FALSE), m_bSslErrors(FALSE), m_pszVerb(0), m_pszVersion(0),
		m_pRequestReply(0), m_nRefCount(1)
	{
		memset(&m_iciCertInfo, 0, sizeof(m_iciCertInfo));
	}

	~CConnection()
	{
		if(m_pszHost) free(m_pszHost);
		if(m_pszObjectName) free(m_pszObjectName);
		if(m_pszVerb) free(m_pszVerb);
		if(m_pszVersion) free(m_pszVersion);
		
		if(m_iciCertInfo.lpszEncryptionAlgName) LocalFree(m_iciCertInfo.lpszEncryptionAlgName);
		if(m_iciCertInfo.lpszIssuerInfo) LocalFree(m_iciCertInfo.lpszIssuerInfo);
		if(m_iciCertInfo.lpszProtocolName) LocalFree(m_iciCertInfo.lpszProtocolName);
		if(m_iciCertInfo.lpszSignatureAlgName) LocalFree(m_iciCertInfo.lpszSignatureAlgName);
		if(m_iciCertInfo.lpszSubjectInfo) LocalFree(m_iciCertInfo.lpszSubjectInfo);

		// m_pRequestReply deleted by CConnectionPool::UnregisterConnection()
	}

    char *GetDstHost(){ return m_pszHost; }
	void SetDstHost(char *host){ m_pszHost = host; }    
	long GetDstIP(){ return m_lIpAddress; }
	void SetDstIP(long ip){ m_lIpAddress = ip; }
	int GetDstPort(){ return m_nPort; }
	void SetDstPort(int port){ m_nPort = port; }
	char *GetObjectName(){ return m_pszObjectName; }
	void SetObjectName(char *object){ m_pszObjectName = object; }
	char *GetHttpVerb(){ return m_pszVerb; }
	void SetHttpVerb(char *verb){ m_pszVerb = verb; }
	char *GetHttpVersion(){ return m_pszVersion; }
	void SetHttpVersion(char *ver){ m_pszVersion = ver; }
	INTERNET_CERTIFICATE_INFO& GetCertificateInfo(){ return m_iciCertInfo; }
	void SetCertificateInfo(INTERNET_CERTIFICATE_INFO ici){ m_iciCertInfo = ici; }
	BOOL IsSSL(){ return m_bSsl; }
	void SetSSL(BOOL bSsl){ m_bSsl = bSsl; }
	BOOL HasSSLErrors(){ return m_bSslErrors; }
	void SetSSLErrors(BOOL bErrors){ m_bSslErrors = bErrors; }

	CRequestReply *GetRequestReply(){ return m_pRequestReply; }
	void SetRequestReply(CRequestReply *pRequestReply){ m_pRequestReply = pRequestReply; }

	void AddRef(){ m_nRefCount++; }
	void Release(){ m_nRefCount--; }
	size_t GetRefCount(){ return m_nRefCount; }

private:
	char *m_pszHost, *m_pszObjectName, *m_pszVerb, *m_pszVersion;
	long m_lIpAddress;
	int m_nPort;
	INTERNET_CERTIFICATE_INFO m_iciCertInfo;
	BOOL m_bSsl, m_bSslErrors;
	CRequestReply *m_pRequestReply;
	size_t m_nRefCount;
};

class CRequestReply : public CRequestReplyBase
{
public:
	CRequestReply() : m_pbRequest(0), m_pbReply(0), m_pRequestStream(0),
		m_pReplyStream(0), m_pfnReadRequest(0), m_pfnReadReply(0), m_pConn(0),
		m_bReplyDataConsumed(FALSE), m_offsReplyDataConsumed(0),
		m_bReplyDataConnectionError(FALSE), m_bReplyDataReadComplete(FALSE),
		m_bRequiresChunkProcessing(FALSE), m_nRefCount(1), m_pParseStorage(0),
		m_cbReplyMax(0), m_cbRequestMax(0)
	{
	}

	~CRequestReply()
	{
		if(m_pbRequest) free(m_pbRequest);
		if(m_pbReply) free(m_pbReply);

		delete m_pRequestStream;
		delete m_pReplyStream;

		DeleteParseStorage();
		// m_pConn deleted by CConnectionPool::UnregisterRequestReply()
	}

	CConnection *GetConnection(){ return m_pConn; }
	void SetConnection(CConnection *pConn){ m_pConn = pConn; }

	char* GetRequestHeader(char *pszHeader);
	void SetRequestHeader(char *pszHeader, char *pszValue, char chDelim = ';', BOOL bOverwrite = TRUE);
	const std::map<std::string, std::string>& GetRequestHeaders();
	CCommonTypeStreamReader *GetRequestBody();
	void SetRequestInfo(REQUESTREPLY handle, PBYTE pbReadSoFar, size_t cbReadSoFar, PFN_STRMREAD_CALLBACK pfnReadMore);
	
	std::vector<char *> *GetReplyHeader(char *pszHeader);
	void SetReplyBanner(char *pszBanner);
	char *GetReplyBanner();
	void SetReplyHeader(char *pszHeader, char *pszValue, char chDelim = ';', BOOL bOverwrite = TRUE);
	const std::multimap<std::string, std::string>& GetReplyHeaders();
	CCommonTypeStreamReader *GetReplyBody();
	void SetReplyInfo(REQUESTREPLY handle, PBYTE pbReadSoFar, size_t cbReadSoFar, PFN_STRMREAD_CALLBACK pfnReadMore);
	
	void SetReplyDataConsumed(BOOL bConsumed){ m_bReplyDataConsumed = bConsumed; }
	BOOL GetReplyDataConsumed(){ return m_bReplyDataConsumed; }
	void SetReplyDataOffset(size_t offs){ m_offsReplyDataConsumed = offs; }
	size_t GetReplyDataOffset(){ return m_offsReplyDataConsumed; }
	void SetReplyConnectionError(BOOL bErr){ m_bReplyDataConnectionError = bErr; }
	BOOL GetReplyConnectionError(){ return m_bReplyDataConnectionError; }
	void SetReplyDataReadComplete(BOOL bComplete){ m_bReplyDataReadComplete = bComplete; }
	BOOL GetReplyDataReadComplete(){ return m_bReplyDataReadComplete; }
	void SetStreamRequiresChunkProcessing(BOOL bComplete){ m_bRequiresChunkProcessing = bComplete; }
	BOOL GetStreamRequiresChunkProcessing(){ return m_bRequiresChunkProcessing; }
	
	CParseTemporaryStorageStatic *GetParseStorage()
	{
		if(m_pParseStorage)
			return m_pParseStorage;

		m_pParseStorage = new CParseTemporaryStorageStatic();
		return m_pParseStorage;
	}

	void DeleteParseStorage()
	{
		if(m_pParseStorage)
			delete m_pParseStorage;

		m_pParseStorage = NULL;
	}

	static EStreamReadCallbackError StreamReadRequestProxy(void *handle, PBYTE pbOutputBuffer, size_t cbReadSize, size_t *pcbBytesRead);
	static EStreamReadCallbackError StreamReadReplyProxy(void *handle, PBYTE pbOutputBuffer, size_t cbReadSize, size_t *pcbBytesRead);
	
	void AddRef(){ m_nRefCount++; }
	void Release(){ m_nRefCount--; }
	size_t GetRefCount(){ return m_nRefCount; }

private:
	PBYTE m_pbRequest, m_pbReply;
	size_t m_cbRequest, m_cbRequestMax, m_cbReply, m_cbReplyMax;
	CDataStreamBase *m_pRequestStream, *m_pReplyStream;
	PFN_STRMREAD_CALLBACK m_pfnReadRequest, m_pfnReadReply;
	REQUESTREPLY m_rqrpRequest, m_rqrpReply;
	CConnection *m_pConn;
	std::map<std::string, std::string> m_mapRequestHeaders;
	// use multimap to allow multiple instances of same header (required to detect header injection attempts)
	std::multimap<std::string, std::string> m_multimapReplyHeaders;
	std::string m_replyBanner;
	BOOL m_bReplyDataConsumed, m_bReplyDataConnectionError, m_bReplyDataReadComplete, m_bRequiresChunkProcessing;
	size_t m_offsReplyDataConsumed;
	size_t m_nRefCount;
	CParseTemporaryStorageStatic *m_pParseStorage;
};