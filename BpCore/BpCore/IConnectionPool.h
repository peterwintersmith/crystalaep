// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#pragma once

#include "..\..\BpCore\BpCore\IStream.h"
#include "..\..\BpCore\BpCore\IParseTempStgStatic.h"

struct IConnectionPool;
struct IConnection;
struct IRequestReply;

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

typedef EAllowableOperation (*PFN_CP_CALLBACK)(IConnectionPool *pConnPool, IConnection *pConn);
typedef DWORD CONNECTION;
typedef DWORD REQUESTREPLY;

#ifdef CRYSTAL_LATE_BINDING
#ifdef DEBUG_BUILD
extern "C" static IConnectionPool *(*GetConnectionPool)() = (IConnectionPool *(*)())GetProcAddress(
		GetModuleHandle(L"C:\\Work\\crystal\\BP\\Impl\\BpCore\\Debug\\bpcore.dll") ?
			GetModuleHandle(L"C:\\Work\\crystal\\BP\\Impl\\BpCore\\Debug\\bpcore.dll") :
			LoadLibrary(L"C:\\Work\\crystal\\BP\\Impl\\BpCore\\Debug\\bpcore.dll"), 
		"GetConnectionPool"
	);
#else
extern "C" static IConnectionPool *(*GetConnectionPool)() = (IConnectionPool *(*)())GetProcAddress(
		GetModuleHandle(L"bpcore.dll") ?
			GetModuleHandle(L"bpcore.dll") :
			LoadLibrary(L"bpcore.dll"), 
		"GetConnectionPool"
	);
#endif
#else
extern "C" __declspec(dllimport) IConnectionPool *GetConnectionPool();
#endif

struct IConnectionPool {
	virtual BOOL QueryConnectAllowed(CONNECTION conn) = 0;
	virtual BOOL QueryRequestAllowed(REQUESTREPLY reqrepl) = 0;
	virtual BOOL QueryResponseAllowed(REQUESTREPLY reqrepl) = 0;

	virtual EConnectionPoolError AddOpenCallback(PFN_CP_CALLBACK pfn, DWORD dwLevel) = 0;
    virtual EConnectionPoolError AddCloseCallback(PFN_CP_CALLBACK pfn, DWORD dwLevel) = 0;
    virtual EConnectionPoolError AddRequestCallback(PFN_CP_CALLBACK pfn, DWORD dwLevel) = 0;
    virtual EConnectionPoolError AddResponseCallback(PFN_CP_CALLBACK pfn, DWORD dwLevel) = 0;
    virtual EConnectionPoolError RemoveCallback(PFN_CP_CALLBACK pfn) = 0;

	virtual BOOL ConnectionExists(CONNECTION conn) = 0;
    virtual EConnectionPoolError RegisterConnection(CONNECTION conn) = 0;
    virtual IConnection *GetConnection(CONNECTION conn) = 0;
    virtual EConnectionPoolError UnregisterConnection(CONNECTION conn) = 0;

	virtual BOOL RequestReplyExist(REQUESTREPLY reqrepl) = 0;
	virtual EConnectionPoolError RegisterRequestReply(REQUESTREPLY reqrepl) = 0;
	virtual IRequestReply *GetRequestReply(REQUESTREPLY reqrepl) = 0;
    virtual EConnectionPoolError UnregisterRequestReply(REQUESTREPLY reqrepl) = 0;
};

struct IConnection {
    virtual char *GetDstHost() = 0;
	virtual void SetDstHost(char *host) = 0;  
	virtual long GetDstIP() = 0;
	virtual void SetDstIP(long ip) = 0;
	virtual int GetDstPort() = 0;
	virtual void SetDstPort(int port) = 0;
	virtual char *GetObjectName() = 0;
	virtual void SetObjectName(char *object) = 0;
	virtual char *GetHttpVerb() = 0;
	virtual void SetHttpVerb(char *verb) = 0;
	virtual char *GetHttpVersion() = 0;
	virtual void SetHttpVersion(char *ver) = 0;
	virtual INTERNET_CERTIFICATE_INFO& GetCertificateInfo() = 0;
	virtual void SetCertificateInfo(INTERNET_CERTIFICATE_INFO ici) = 0;
	virtual BOOL IsSSL() = 0;
	virtual void SetSSL(BOOL bSsl) = 0;
	virtual BOOL HasSSLErrors() = 0;
	virtual void SetSSLErrors(BOOL bErrors) = 0;

	virtual IRequestReply *GetRequestReply() = 0;
	virtual void SetRequestReply(IRequestReply *pRequestReply) = 0;
};

struct IRequestReply {
	virtual IConnection *GetConnection() = 0;
	virtual void SetConnection(IConnection *pConn) = 0;

	virtual char* GetRequestHeader(char *pszHeader) = 0;
	virtual void SetRequestHeader(char *pszHeader, char *pszValue, char chDelim = ';', BOOL bOverwrite = TRUE) = 0;
	virtual const std::map<std::string, std::string>& GetRequestHeaders() = 0;
	virtual ICommonTypeStreamReader *GetRequestBody() = 0;
	virtual void SetRequestInfo(REQUESTREPLY handle, PBYTE pbReadSoFar, size_t cbReadSoFar, PFN_STRMREAD_CALLBACK pfnReadMore) = 0;
	
	virtual std::vector<char *> *GetReplyHeader(char *pszHeader) = 0;
	virtual void SetReplyBanner(char *pszBanner) = 0;
	virtual char *GetReplyBanner() = 0;
	virtual void SetReplyHeader(char *pszHeader, char *pszValue, char chDelim = ';', BOOL bOverwrite = TRUE) = 0;
	virtual const std::multimap<std::string, std::string>& GetReplyHeaders() = 0;
	virtual ICommonTypeStreamReader *GetReplyBody() = 0;
	virtual void SetReplyInfo(REQUESTREPLY handle, PBYTE pbReadSoFar, size_t cbReadSoFar, PFN_STRMREAD_CALLBACK pfnReadMore) = 0;
	
	virtual void SetReplyDataConsumed(BOOL bConsumed) = 0;
	virtual BOOL GetReplyDataConsumed() = 0;
	virtual void SetReplyDataOffset(size_t offs) = 0;
	virtual size_t GetReplyDataOffset() = 0;
	virtual void SetReplyConnectionError(BOOL bErr) = 0;
	virtual BOOL GetReplyConnectionError() = 0;
	virtual void SetReplyDataReadComplete(BOOL bComplete) = 0;
	virtual BOOL GetReplyDataReadComplete() = 0;
	virtual void SetStreamRequiresChunkProcessing(BOOL bComplete) = 0;
	virtual BOOL GetStreamRequiresChunkProcessing() = 0;
	
	virtual IParseTemporaryStorageStatic *GetParseStorage() = 0;
	virtual void DeleteParseStorage() = 0;
};