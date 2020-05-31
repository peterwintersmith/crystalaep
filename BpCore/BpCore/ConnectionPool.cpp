// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#include "stdafx.h"
#include "WininetHooks.h"
#include "BpCore.h"
#include "Stream.h"
#include "..\..\Shared\Utils\Utils.h"
#include "..\..\AuxCore\AuxCore\IEventWriter.h"
#include "..\..\AuxCore\AuxCore\IRealtimeLog.h"
#include "ConnectionPool.h"

HANDLE CConnectionPool::m_hGcThread = 0;
DWORD CConnectionPool::m_dwGcThreadId = 0;
BOOL CConnectionPool::m_bGcUnload = FALSE;
std::vector<CConnection *> CConnectionPool::m_vecGcConn;
std::vector<CRequestReply *> CConnectionPool::m_vecGcReqRepl;
CRITICAL_SECTION CConnectionPool::m_csGC = {0};
	
CConnectionPool *CConnectionPool::m_Instance = NULL;
CRITICAL_SECTION CConnectionPool::m_csCreateInst = {0};

CConnectionPool *CConnectionPool::GetInstance()
{
	if(m_Instance == NULL)
	{
		EnterCriticalSection(&m_csCreateInst);

		if(m_Instance == NULL)
		{
			m_Instance = new CConnectionPool();
		}

		LeaveCriticalSection(&m_csCreateInst);
	}

	return m_Instance;
}

BOOL CConnectionPool::QueryConnectAllowed(CONNECTION conn)
{
	BOOL bRet = TRUE;
	std::vector<PFN_CP_CALLBACK>::iterator it;
	CConnection *pConn = m_mapConns[conn];
	std::string strUrl;

	if(!pConn)
		goto Cleanup;

	strUrl = pConn->IsSSL() ? "https://" : "http://";
		strUrl += pConn->GetDstHost(); // += www.google.com
		strUrl += pConn->GetObjectName(); // += /foo/image.gif

	IRealtimeLog *pRealtimeLog = GetRealtimeLog();
	
	pRealtimeLog->LogMessage(0, L"Checking connect allowed to %S", strUrl.c_str());

	pConn->AddRef();

	for(it = m_vecOpenCallbacks.begin(); it != m_vecOpenCallbacks.end(); it++)
	{
		if( (*it)(this, pConn) == EAllowableOperation::Disallowed )
		{
			IEventWriter *pEvent = GetEventWriter();
			if(pEvent)
			{
				//std::string strUrl = pConn->IsSSL() ? "https://" : "http://";
				//strUrl += pConn->GetDstHost(); // += www.google.com
				//strUrl += pConn->GetObjectName(); // += /foo/image.gif

				WCHAR *pwszURL = Utils::WcFromMultiByte(const_cast<char *>(strUrl.c_str()));

				pEvent->WriteEvent(
						EEventRepository::ConnectionMonitor,
						EThreatRiskRating::LowRisk,
						L"Outgoing Connection Attempt Blocked",
						L"The Connection Manager has blocked an outgoing connection attempt.\r\n"
						L"The target URL for the connection in question was: %s.\r\n"
						L"Individual policy events can be checked more specific information.",
						pwszURL ? pwszURL : L"unknown site"
					);

				if(pwszURL)
					free(pwszURL);
			}

			pRealtimeLog->LogMessage(1, L"Disallowed %S", strUrl.c_str());

			bRet = FALSE;
			goto Cleanup;
		}
	}
	
	pRealtimeLog->LogMessage(0, L"Permitting %S", strUrl.c_str());

Cleanup:
	if(pConn)
		pConn->Release();

	return bRet;
}

BOOL CConnectionPool::QueryRequestAllowed(REQUESTREPLY reqrepl)
{
	BOOL bRet = TRUE;
	std::vector<PFN_CP_CALLBACK>::iterator it;
	CRequestReply *pReqRepl = m_mapRequestReply[reqrepl];
	CConnection *pConn = NULL;
	std::string strUrl;

	if(!pReqRepl)
		goto Cleanup;

	pConn = pReqRepl->GetConnection();

	if(!pConn)
		goto Cleanup;

	strUrl = pConn->IsSSL() ? "https://" : "http://";
		strUrl += pConn->GetDstHost(); // += www.google.com
		strUrl += pConn->GetObjectName(); // += /foo/image.gif

	IRealtimeLog *pRealtimeLog = GetRealtimeLog();

	pRealtimeLog->LogMessage(0, L"Checking request allowed to %S", strUrl.c_str());

	pConn->AddRef();
	pReqRepl->AddRef();

	for(it = m_vecRequestCallbacks.begin(); it != m_vecRequestCallbacks.end(); it++)
	{
		if( (*it)(this, pReqRepl->GetConnection()) == EAllowableOperation::Disallowed )
		{
			IEventWriter *pEvent = GetEventWriter();
			if(pEvent)
			{
				//std::string strUrl = pConn->IsSSL() ? "https://" : "http://";
				//strUrl += pConn->GetDstHost(); // += www.google.com
				//strUrl += pConn->GetObjectName(); // += /foo/image.gif

				WCHAR *pwszURL = Utils::WcFromMultiByte(const_cast<char *>(strUrl.c_str()));

				pEvent->WriteEvent(
						EEventRepository::ConnectionMonitor,
						EThreatRiskRating::LowRisk,
						L"Outgoing Request Blocked",
						L"The Connection Manager has blocked an outgoing request.\r\n"
						L"The target URL for the request in question was: %s.\r\n"
						L"Individual policy events can be checked more specific information.",
						pwszURL ? pwszURL : L"unknown site"
					);

				if(pwszURL)
					free(pwszURL);
			}

			pRealtimeLog->LogMessage(1, L"Disallowed %S", strUrl.c_str());

			bRet = FALSE;
			goto Cleanup;
		}
	}

	pRealtimeLog->LogMessage(0, L"Permitting %S", strUrl.c_str());

Cleanup:
	if(pReqRepl)
		pReqRepl->Release();
	
	if(pConn)
		pConn->Release();
	
	return bRet;
}

BOOL CConnectionPool::QueryResponseAllowed(REQUESTREPLY reqrepl)
{
	BOOL bRet = TRUE;
	std::vector<PFN_CP_CALLBACK>::iterator it;
	CRequestReply *pReqRepl = m_mapRequestReply[reqrepl];
	CConnection *pConn = NULL;
	std::string strUrl;

	if(!pReqRepl)
		goto Cleanup;

	pConn = pReqRepl->GetConnection();
	
	if(!pConn)
		goto Cleanup;

	strUrl = pConn->IsSSL() ? "https://" : "http://";
		strUrl += pConn->GetDstHost(); // += www.google.com
		strUrl += pConn->GetObjectName(); // += /foo/image.gif

	IRealtimeLog *pRealtimeLog = GetRealtimeLog();
	
	pRealtimeLog->LogMessage(0, L"Checking response allowed from %S", strUrl.c_str());

	pConn->AddRef();
	pReqRepl->AddRef();

	for(it = m_vecResponseCallbacks.begin(); it != m_vecResponseCallbacks.end(); it++)
	{
		if( (*it)(this, pReqRepl->GetConnection()) == EAllowableOperation::Disallowed )
		{
			IEventWriter *pEvent = GetEventWriter();
			if(pEvent)
			{
				//std::string strUrl = pConn->IsSSL() ? "https://" : "http://";
				//strUrl += pConn->GetDstHost(); // += www.google.com
				//strUrl += pConn->GetObjectName(); // += /foo/image.gif

				WCHAR *pwszURL = Utils::WcFromMultiByte(const_cast<char *>(strUrl.c_str()));

				pEvent->WriteEvent(
						EEventRepository::ConnectionMonitor,
						EThreatRiskRating::LowRisk,
						L"Incoming Response Blocked",
						L"The Connection Manager has blocked an incoming response.\r\n"
						L"The target URL for the request in question was: %s.\r\n"
						L"Individual policy events can be checked more specific information.",
						pwszURL ? pwszURL : L"unknown site"
					);

				if(pwszURL)
					free(pwszURL);
			}

			pRealtimeLog->LogMessage(1, L"Disallowed %S", strUrl.c_str());

			bRet = FALSE;
			goto Cleanup;
		}
	}

	pRealtimeLog->LogMessage(0, L"Permitting %S", strUrl.c_str());

Cleanup:
	if(pReqRepl)
		pReqRepl->Release();
	
	if(pConn)
		pConn->Release();
	
	return bRet;
}

EConnectionPoolError CConnectionPool::AddOpenCallback(PFN_CP_CALLBACK pfn, DWORD dwLevel)
{
	m_vecOpenCallbacks.push_back(pfn);
	return EConnectionPoolError::Success;
}

EConnectionPoolError CConnectionPool::AddRequestCallback(PFN_CP_CALLBACK pfn, DWORD dwLevel)
{
	m_vecRequestCallbacks.push_back(pfn);
	return EConnectionPoolError::Success;
}

EConnectionPoolError CConnectionPool::AddResponseCallback(PFN_CP_CALLBACK pfn, DWORD dwLevel)
{
	m_vecResponseCallbacks.push_back(pfn);
	return EConnectionPoolError::Success;
}

EConnectionPoolError CConnectionPool::RemoveCallback(PFN_CP_CALLBACK pfn)
{
	EConnectionPoolError err = EConnectionPoolError::Failure;
	std::vector<PFN_CP_CALLBACK>::iterator it;

	it = std::find(m_vecOpenCallbacks.begin(), m_vecOpenCallbacks.end(), pfn);

	// allow removal from multiple callback lists

	if(it !=  m_vecOpenCallbacks.end())
	{
		m_vecOpenCallbacks.erase(it);
		err = EConnectionPoolError::Success;
	}

	it = std::find(m_vecRequestCallbacks.begin(), m_vecRequestCallbacks.end(), pfn);

	if(it != m_vecRequestCallbacks.end())
	{
		m_vecRequestCallbacks.erase(it);
		err = EConnectionPoolError::Success;
	}

	it = std::find(m_vecResponseCallbacks.begin(), m_vecResponseCallbacks.end(), pfn);

	if(it != m_vecResponseCallbacks.end())
	{
		m_vecResponseCallbacks.erase(it);
		err = EConnectionPoolError::Success;
	}

	return err;
}

BOOL CConnectionPool::ConnectionExists(CONNECTION conn)
{
	BOOL bExists;

	EnterCriticalSection(&m_csConnOperation);
	bExists = m_mapConns.find(conn) != m_mapConns.end();
	LeaveCriticalSection(&m_csConnOperation);

	return bExists;
}

EConnectionPoolError CConnectionPool::RegisterConnection(CONNECTION conn)
{
	EConnectionPoolError err = EConnectionPoolError::Failure;
	std::map<CONNECTION, CConnection *>::iterator it;
	CConnection *pConn = NULL;

	EnterCriticalSection(&m_csConnOperation);

	it = m_mapConns.find(conn);
	if(it != m_mapConns.end())
		goto Cleanup;
	
	pConn = new CConnection();
	if(!pConn)
		goto Cleanup;

	// safe, as this should never be registered for GC before it's closed by IE
	pConn->Release();
	
	m_mapConns[conn] = pConn;

	err = EConnectionPoolError::Success;
Cleanup:
	LeaveCriticalSection(&m_csConnOperation);
	return err;
}

CConnection *CConnectionPool::GetConnection(CONNECTION conn)
{
	std::map<CONNECTION, CConnection *>::iterator it;
	CConnection *pConn = NULL;

	EnterCriticalSection(&m_csConnOperation);

	it = m_mapConns.find(conn);
	if(it == m_mapConns.end())
	{
		LeaveCriticalSection(&m_csConnOperation);
		throw new std::exception("Attempt to get handle for CONNECTION which does not exist");
	}

	pConn = m_mapConns[conn];

	LeaveCriticalSection(&m_csConnOperation);
	
	return pConn;
}

EConnectionPoolError CConnectionPool::UnregisterConnection(CONNECTION conn)
{
	EConnectionPoolError err = EConnectionPoolError::Failure;
	std::map<CONNECTION, CConnection *>::iterator it;
	CConnection *pConn = NULL;

	EnterCriticalSection(&m_csConnOperation);

	it = m_mapConns.find(conn);
	if(it == m_mapConns.end())
		goto Cleanup;
	
	pConn = m_mapConns[conn];
	//delete pConn;
	RegisterForGC(pConn);
	m_mapConns.erase(it);

	err = EConnectionPoolError::Success;
Cleanup:
	LeaveCriticalSection(&m_csConnOperation);
	return err;
}

BOOL CConnectionPool::RequestReplyExist(REQUESTREPLY reqrepl)
{
	BOOL bExists;

	EnterCriticalSection(&m_csReqReplOperation);
	bExists = m_mapRequestReply.find(reqrepl) != m_mapRequestReply.end();
	LeaveCriticalSection(&m_csReqReplOperation);
	
	return bExists;
}

EConnectionPoolError CConnectionPool::RegisterRequestReply(REQUESTREPLY reqrepl)
{
	EConnectionPoolError err = EConnectionPoolError::Failure;
	std::map<REQUESTREPLY, CRequestReply *>::iterator it;
	CRequestReply *pReqRepl = NULL;

	EnterCriticalSection(&m_csReqReplOperation);

	it = m_mapRequestReply.find(reqrepl);
	if(it != m_mapRequestReply.end())
		goto Cleanup;
	
	pReqRepl = new CRequestReply();
	if(!pReqRepl)
		goto Cleanup;
	
	// safe, cos this'll never be registered for GC before it's closed by IE
	pReqRepl->Release();

	m_mapRequestReply[reqrepl] = pReqRepl;

	err = EConnectionPoolError::Success;
Cleanup:
	LeaveCriticalSection(&m_csReqReplOperation);
	return err;
}

CRequestReply *CConnectionPool::GetRequestReply(REQUESTREPLY reqrepl)
{
	std::map<REQUESTREPLY, CRequestReply *>::iterator it;
	CRequestReply *pReqRepl = NULL;

	EnterCriticalSection(&m_csReqReplOperation);

	it = m_mapRequestReply.find(reqrepl);
	if(it == m_mapRequestReply.end())
	{
		LeaveCriticalSection(&m_csReqReplOperation);
		throw new std::exception("Attempt to get handle for REQUESTREPLY which does not exist");
	}

	pReqRepl = m_mapRequestReply[reqrepl];

	LeaveCriticalSection(&m_csReqReplOperation);
	
	return pReqRepl;
}

EConnectionPoolError CConnectionPool::UnregisterRequestReply(REQUESTREPLY reqrepl)
{
	EConnectionPoolError err = EConnectionPoolError::Failure;
	std::map<REQUESTREPLY, CRequestReply *>::iterator it;
	CRequestReply *pReqRepl = NULL;

	EnterCriticalSection(&m_csReqReplOperation);

	it = m_mapRequestReply.find(reqrepl);
	if(it == m_mapRequestReply.end())
		goto Cleanup;

	pReqRepl = m_mapRequestReply[reqrepl];
	//delete pReqRepl;
	RegisterForGC(pReqRepl);
	m_mapRequestReply.erase(it);

	err = EConnectionPoolError::Success;
Cleanup:
	LeaveCriticalSection(&m_csReqReplOperation);
	return err;
}

void CConnectionPool::Terminate()
{
	if(!m_hGcThread || !this)
		return;

	m_bGcUnload = TRUE;

	TerminateThread(m_hGcThread, 0);
}

DWORD CConnectionPool::DeferredGarbageCollector(LPVOID lpThreadParameter)
{
#ifdef DEBUG_BUILD
	CLogger *logger = NULL;
#endif	
	//if(1) return 0;

	struct {
	public:
		bool operator()(CRequestReply *p)
		{
			if(!p->GetRefCount())
			{
				return true;
			}

			return false;
		}

		bool operator()(CConnection *p)
		{
			if(!p->GetRefCount())
			{
				return true;
			}

			return false;
		}
	} RefCountEvaluator;

	while(!m_bGcUnload)
	{
		// temporary fix for race condition. Race condition occurs because InternetCloseHandle can be called to
		// terminate async ops mid-way (so callbacks can suddenly lose their references).
		// Real fix MUST be implemented (or code execution bugs *will* occur).

		Sleep(1000 * 15);
		
		if(m_vecGcConn.size())
		{
			try
			{
#ifdef DEBUG_BUILD
				logger = CLogger::GetInstance("bpcore_garbage");
#endif
			}
			catch(...)
			{
			}
			
#ifdef DEBUG_BUILD
			if(logger)
				logger->Log("CConnectionPool::DeferredGarbageCollector: %u collection objects in GC\n", m_vecGcConn.size());
#endif
			std::vector<CConnection *> vecRemove;

			EnterCriticalSection(&m_csGC);

			for(int i=0; i<m_vecGcConn.size(); i++)
			{
#ifdef DEBUG_BUILD
				if(logger)
					logger->Log("\tObject 0x%p (ref count = %u)\n", m_vecGcConn[i], m_vecGcConn[i]->GetRefCount());
#endif
				if(RefCountEvaluator(m_vecGcConn[i]) == true)
				{
					vecRemove.push_back(m_vecGcConn[i]);
				}
			}

			for(int i=0; i<vecRemove.size(); i++)
			{
				std::vector<CConnection *>::iterator it = std::find(m_vecGcConn.begin(), m_vecGcConn.end(), vecRemove[i]);
				if(it != m_vecGcConn.end())
				{
#ifdef DEBUG_BUILD
					if(logger)
						logger->Log("\tDeleting object 0x%p\n", *it);
#endif					
					delete *it;
					m_vecGcConn.erase(it);
				}
			}

			vecRemove.clear();

			LeaveCriticalSection(&m_csGC);
		}

		if(m_vecGcReqRepl.size())
		{
			try
			{
#ifdef DEBUG_BUILD
				logger = CLogger::GetInstance("bpcore_garbage");
#endif
			}
			catch(...)
			{
			}
			
#ifdef DEBUG_BUILD
			if(logger)
				logger->Log("CConnectionPool::DeferredGarbageCollector: %u request/reply objects in GC\n", m_vecGcReqRepl.size());
#endif
			std::vector<CRequestReply *> vecRemove;

			EnterCriticalSection(&m_csGC);

			for(int i=0; i<m_vecGcReqRepl.size(); i++)
			{
#ifdef DEBUG_BUILD
				if(logger)
					logger->Log("\tObject 0x%p (ref count = %u)\n", m_vecGcReqRepl[i], m_vecGcReqRepl[i]->GetRefCount());
#endif
				if(RefCountEvaluator(m_vecGcReqRepl[i]) == true)
				{
					vecRemove.push_back(m_vecGcReqRepl[i]);
				}
			}

			for(int i=0; i<vecRemove.size(); i++)
			{
				std::vector<CRequestReply *>::iterator it = std::find(m_vecGcReqRepl.begin(), m_vecGcReqRepl.end(), vecRemove[i]);
				if(it != m_vecGcReqRepl.end())
				{
#ifdef DEBUG_BUILD
					if(logger)
						logger->Log("\tDeleting object 0x%p\n", *it);
#endif					
					delete *it;
					m_vecGcReqRepl.erase(it);
				}
			}

			vecRemove.clear();

			LeaveCriticalSection(&m_csGC);
		}
	}

	return 0;
}

char* CRequestReply::GetRequestHeader(char *pszHeader)
{
	std::string header(pszHeader);
	char *pszValue = NULL;
	BOOL bUpperNext = TRUE;

	if(!pszHeader || !pszHeader[0])
	{
		throw new std::exception("Should not query for a NULL or empty header name");
	}

	//std::for_each(header.begin(), header.end(), tolower); // if only c++ supported lamda expressions
	for(size_t i=0; i<header.length(); i++)
	{
		if(bUpperNext)
		{
			header[i] = toupper(header[i]);
			bUpperNext = FALSE;
		}
		else
		{
			header[i] = tolower(header[i]);
		}

		if(!isalpha(header[i]))
			bUpperNext = TRUE;
	}

	if(m_mapRequestHeaders.find(header) != m_mapRequestHeaders.end())
	{
		pszValue = const_cast<char *>(m_mapRequestHeaders[header].c_str());
	}

	return pszValue;
}

void CRequestReply::SetRequestHeader(char *pszHeader, char *pszValue, char chDelim, BOOL bOverwrite)
{
	std::string header(pszHeader);
	std::string value = "";
	std::map<std::string, std::string>::iterator it;
	BOOL bUpperNext = TRUE;

	if(!pszHeader || !pszHeader[0])
	{
		throw new std::exception("Cannot set a NULL or empty header name in request");
	}

	for(size_t i=0; i<header.length(); i++)
	{
		if(bUpperNext)
		{
			header[i] = toupper(header[i]);
			bUpperNext = FALSE;
		}
		else
		{
			header[i] = tolower(header[i]);
		}

		if(!isalpha(header[i]))
			bUpperNext = TRUE;
	}

	if((it = m_mapRequestHeaders.find(header)) != m_mapRequestHeaders.end())
	{
		if(bOverwrite)
		{
			m_mapRequestHeaders.erase(it);
		}
		else
		{
			value += m_mapRequestHeaders[header] + chDelim + " ";
		}
	}

	if(pszValue)
	{
		value += pszValue;
	}

	// if existing header overwrite and NULL value: remove header
	if(bOverwrite && !pszValue)
	{
		return;
	}

	m_mapRequestHeaders[header] = value;
}

const std::map<std::string, std::string>& CRequestReply::GetRequestHeaders()
{
	return m_mapRequestHeaders;
}

CCommonTypeStreamReader *CRequestReply::GetRequestBody()
{
	if(!m_pRequestStream && m_pbRequest)
	{
		m_pRequestStream = new CCommonTypeStreamReader(
			&m_pbRequest,
			&m_cbRequest,
			&m_cbRequestMax,
			this, 
			StreamReadRequestProxy
			);
	}

	return dynamic_cast<CCommonTypeStreamReader *>(m_pRequestStream);
}

void CRequestReply::SetRequestInfo(REQUESTREPLY handle, PBYTE pbReadSoFar, size_t cbReadSoFar, PFN_STRMREAD_CALLBACK pfnReadMore)
{
	m_rqrpRequest = handle;

	m_pbRequest = (PBYTE)malloc(cbReadSoFar);
	if(!m_pbRequest)
	{
		throw new std::exception("Request reply set info out of memory");
	}

	memcpy(m_pbRequest, pbReadSoFar, cbReadSoFar);
	m_cbRequest = cbReadSoFar;

	m_pfnReadRequest = pfnReadMore;
}

std::vector<char *> *CRequestReply::GetReplyHeader(char *pszHeader)
{
	std::string header(pszHeader);
	char *pszValue = NULL;
	BOOL bUpperNext = TRUE;
	
	if(!pszHeader || !pszHeader[0])
	{
		throw new std::exception("Should not query for a NULL or empty header name");
	}

	for(size_t i=0; i<header.length(); i++)
	{
		if(bUpperNext)
		{
			header[i] = toupper(header[i]);
			bUpperNext = FALSE;
		}
		else
		{
			header[i] = tolower(header[i]);
		}

		if(!isalpha(header[i]))
			bUpperNext = TRUE;
	}

	if(m_multimapReplyHeaders.find(header) == m_multimapReplyHeaders.end())
	{
		return NULL;
	}

	std::pair<
			std::multimap<std::string, std::string>::iterator, 
			std::multimap<std::string, std::string>::iterator
		> range;
	
	range = m_multimapReplyHeaders.equal_range(header);

	std::vector<char *> *pvecValues = new std::vector<char *>();
	if(!pvecValues)
	{
		throw new std::exception("Allocation failure allocating reply hdr vector");
	}

	for(; range.first != range.second; range.first++)
	{
		pvecValues->push_back(const_cast<char *>(range.first->second.c_str()));
	}

	return pvecValues; // caller must delete vector pointer
}

void CRequestReply::SetReplyBanner(char *pszBanner)
{
	m_replyBanner = pszBanner;
}

char *CRequestReply::GetReplyBanner()
{
	return const_cast<char *>(m_replyBanner.c_str());
}

void CRequestReply::SetReplyHeader(char *pszHeader, char *pszValue, char chDelim, BOOL bOverwrite)
{
	std::string header(pszHeader);
	std::string value = "";
	std::multimap<std::string, std::string>::iterator it;
	BOOL bUpperNext = TRUE;

	if(!pszHeader || !pszHeader[0])
	{
		throw new std::exception("Cannot set a NULL or empty header name in response");
	}

	for(size_t i=0; i<header.length(); i++)
	{
		if(bUpperNext)
		{
			header[i] = toupper(header[i]);
			bUpperNext = FALSE;
		}
		else
		{
			header[i] = tolower(header[i]);
		}

		if(!isalpha(header[i]))
			bUpperNext = TRUE;
	}

	// just adds to the first matching header
	if((it = m_multimapReplyHeaders.find(header)) != m_multimapReplyHeaders.end())
	{
		if(bOverwrite)
		{
			m_multimapReplyHeaders.erase(it);
		}
		else
		{
			value += it->second + chDelim + " ";
		}
	}

	if(pszValue)
	{
		value += pszValue;
	}

	if(m_multimapReplyHeaders.find(header) != m_multimapReplyHeaders.end())
	{
		m_multimapReplyHeaders.find(header)->second = value;
	}
	else
	{
		m_multimapReplyHeaders.insert(std::pair<std::string, std::string>(header, value));
	}
}

const std::multimap<std::string, std::string>& CRequestReply::GetReplyHeaders()
{
	return m_multimapReplyHeaders;
}

CCommonTypeStreamReader *CRequestReply::GetReplyBody()
{
	if(!m_pReplyStream && m_pbReply)
	{
		m_pReplyStream = new CCommonTypeStreamReader(
			&m_pbReply,
			&m_cbReply,
			&m_cbReplyMax,
			this,
			StreamReadReplyProxy
			);
	}

	return dynamic_cast<CCommonTypeStreamReader*>(m_pReplyStream);
}

void CRequestReply::SetReplyInfo(REQUESTREPLY handle, PBYTE pbReadSoFar, size_t cbReadSoFar, PFN_STRMREAD_CALLBACK pfnReadMore)
{
	m_rqrpReply = handle;

	m_pbReply = (PBYTE)malloc(cbReadSoFar);
	if(!m_pbReply)
	{
		throw new std::exception("Request reply set info out of memory");
	}

	memcpy(m_pbReply, pbReadSoFar, cbReadSoFar);
	m_cbReply = cbReadSoFar;

	m_pfnReadReply = pfnReadMore;
}

EStreamReadCallbackError CRequestReply::StreamReadRequestProxy(void *handle, PBYTE pbOutputBuffer, size_t cbReadSize, size_t *pcbBytesRead)
{
	EStreamReadCallbackError err;
	CRequestReply *pInstance = (CRequestReply *)handle;
		
	if(!pInstance || pInstance->m_pfnReadRequest)
	{
		return EStreamReadCallbackError::ReadCallbackFailure;
	}

	if(pInstance->m_cbRequest + cbReadSize < pInstance->m_cbRequest)
	{
		return EStreamReadCallbackError::ReadCallbackFailure;
	}

	pInstance->m_pbRequest = (PBYTE)realloc(pInstance->m_pbRequest, pInstance->m_cbRequest + cbReadSize);
	if(!pInstance->m_pbRequest)
	{
		return EStreamReadCallbackError::ReadCallbackFailure;
	}

	memset(pInstance->m_pbRequest + pInstance->m_cbRequest, 0, cbReadSize); // just in-case

	err = pInstance->m_pfnReadRequest(
		(void *)pInstance->m_rqrpRequest,
		pInstance->m_pbRequest + pInstance->m_cbRequest,
		cbReadSize,
		pcbBytesRead
		);

	// how can this error? too few bytes in buffer?

	return err;
}

EStreamReadCallbackError CRequestReply::StreamReadReplyProxy(void *handle, PBYTE pbOutputBuffer, size_t cbReadSize, size_t *pcbBytesRead) // fixup
{
	EStreamReadCallbackError err;
	CRequestReply *pInstance = (CRequestReply *)handle;
#ifdef DEBUG_BUILD
	CLogger *logger = NULL;
#endif
	
	try
	{
#ifdef DEBUG_BUILD
		logger = CLogger::GetInstance("bpcore_wininet");
		logger->ObtainLock();
		logger->Log("StreamReadReplyProxy(pbOutputBuffer = 0x%p, cbReadSize = %u)\n", pbOutputBuffer, cbReadSize);
#endif
	}
	catch(...)
	{
	}

	if(!pInstance || !pInstance->m_pfnReadReply)
	{
		return EStreamReadCallbackError::ReadCallbackFailure;
	}

	if(pInstance->m_cbReply + cbReadSize < pInstance->m_cbReply)
	{
		return EStreamReadCallbackError::ReadCallbackFailure;
	}

	if(pInstance->m_cbReplyMax < pInstance->m_cbReply + cbReadSize)
	{
		size_t cbPow2 = pow((double)2, (int)(log10((double)pInstance->m_cbReply + cbReadSize) / log10((double)2)) + 1);
		if(cbPow2 < pInstance->m_cbReply + cbReadSize)
		{
			return EStreamReadCallbackError::ReadCallbackFailure;
		}

		if(cbPow2 * 2 < cbPow2)
		{
			return EStreamReadCallbackError::ReadCallbackFailure;
		}

		size_t cbMax = cbPow2 * 2; // twice the maximum existing buffer size rounded to nearest power of 2

		if((pInstance->m_pbReply = (PBYTE)realloc(pInstance->m_pbReply, cbMax)) == NULL)
		{
			return EStreamReadCallbackError::ReadCallbackFailure;
		}

		pInstance->m_cbReplyMax = cbMax;
	}

	memset(pInstance->m_pbReply + pInstance->m_cbReply, 0, cbReadSize); // just in-case

	*pcbBytesRead = 0;

#ifdef DEBUG_BUILD
	if(logger)
		logger->Log("\tpInstance->m_pbReply = 0x%p and pInstance->m_cbReply = %u\n", pInstance->m_pbReply, pInstance->m_cbReply);
#endif

	err = pInstance->m_pfnReadReply(
		(void *)pInstance->m_rqrpReply,
		pInstance->m_pbReply + pInstance->m_cbReply,
		cbReadSize,
		pcbBytesRead
		);
	
#ifdef DEBUG_BUILD
	if(logger)
		logger->Log("\terr = %s. *pcbBytesRead = %u\n", err == EStreamReadCallbackError::ReadCallbackSuccess ? "ReadCallbackSuccess" : "ReadCallbackFailure", *pcbBytesRead);
#endif

	pInstance->m_cbReply += *pcbBytesRead;

	if(err == EStreamReadCallbackError::ReadCallbackFailure)
	{
		pInstance->SetReplyConnectionError(TRUE);
	}
	
#ifdef DEBUG_BUILD
	if(logger)
		logger->ReleaseLock();
#endif

	return err;
}

extern "C" __declspec(dllexport) CConnectionPoolBase *GetConnectionPool()
{
	return reinterpret_cast<CConnectionPoolBase *>(CConnectionPool::GetInstance());
}