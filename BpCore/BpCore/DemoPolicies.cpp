// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#include "stdafx.h"
#include "ConnectionPool.h"
#include "Stream.h"
#include "DemoPolicies.h"

EAllowableOperation DemoUrlFilteringPolicies::DisallowSiteVisit(CConnectionPool *pConnPool, CConnection *pConn)
{
	CLogger *logger = NULL;

	try
	{
		logger = CLogger::GetInstance("bpcore_policy");
		logger->Log("Checking DisallowPolicy for site '%s'\n", pConn->GetDstHost());
	}
	catch(...)
	{
	}

	if( stricmp( pConn->GetDstHost(), "www.paypal.com" ) == 0 || strstr(pConn->GetDstHost(), ".cn") != NULL )
	{
		if(logger)
		{
			logger->Log("Disallowed host '%s'\n", pConn->GetDstHost());
		}

		return EAllowableOperation::Disallowed;
	}

	return EAllowableOperation::Allowed;
}

EAllowableOperation DemoSSLFilteringPolicies::DisallowPermissiveSSL(CConnectionPool *pConnPool, CConnection *pConn)
{
	CLogger *logger = NULL;

	try
	{
		logger = CLogger::GetInstance("bpcore_policy");
		logger->Log("Checking permissive SSL DisallowPolicy for site '%s'\n", pConn->GetDstHost());
	}
	catch(...)
	{
	}

	if( pConn->HasSSLErrors() )
	{
		if(logger)
		{
			logger->Log("Disallowed host '%s' due to permissive SSL\n", pConn->GetDstHost());
		}

		return EAllowableOperation::Disallowed;
	}

	return EAllowableOperation::Allowed;
}

EAllowableOperation DemoRequestFilteringPolicies::DisallowRequestToSlashFOO(CConnectionPool *pConnPool, CConnection *pConn)
{
	CLogger *logger = NULL;

	try
	{
		logger = CLogger::GetInstance("bpcore_policy");
		logger->ObtainLock();
		logger->Log("Checking send to /foo DisallowPolicy for site '%s'\n", pConn->GetDstHost());
	}
	catch(...)
	{
	}

	CRequestReply *pReqRepl = pConn->GetRequestReply();
	if(pReqRepl)
	{
		if(logger)
			logger->Log("Contains CRequestReply instance\n");
		
		CDataStreamBase *pStream = pReqRepl->GetRequestBody();
		
		if(pStream)
		{
			if(logger)
				logger->Log("Contains request body stream\n");
		}
		else
		{
			if(logger)
				logger->Log("Does not contain request body stream\n");
		}
	}

	if(pConn->GetObjectName() && stricmp(pConn->GetObjectName(), "/foo") == 0 )
	{
		if(logger)
			logger->Log("Disallowed request to url %s%s%s\n", pConn->IsSSL() ? "https://" : "http://", pConn->GetDstHost(), pConn->GetObjectName());

		if(logger)
			logger->ReleaseLock();

		return EAllowableOperation::Disallowed;
	}
	
	if(logger)
		logger->ReleaseLock();

	return EAllowableOperation::Allowed;
}

EAllowableOperation DemoRequestFilteringPolicies::DisallowRequestContainingPHOOBAR(CConnectionPool *pConnPool, CConnection *pConn)
{
	CLogger *logger = NULL;

	try
	{
		logger = CLogger::GetInstance("bpcore_policy");
		logger->ObtainLock();
		logger->Log("Checking send to /foo DisallowPolicy for site '%s'\n", pConn->GetDstHost());
	}
	catch(...)
	{
	}

	CRequestReply *pReqRepl = pConn->GetRequestReply();
	if(pReqRepl)
	{
		if(logger)
			logger->Log("Contains CRequestReply instance\n");
		
		std::map<std::string, std::string>::const_iterator it;

		for(it = pReqRepl->GetRequestHeaders().begin();
			it != pReqRepl->GetRequestHeaders().end();
			it++)
		{
			if(logger)
				logger->Log("\tHeader: [%s:%s]\n", it->first.c_str(), it->second.c_str());
		}

		if(pReqRepl->GetRequestHeader("User-Agent") != NULL)
		{
			if(logger)
				logger->Log("\tUser-Agent: '%s'\n", pReqRepl->GetRequestHeader("User-Agent"));

			pReqRepl->SetRequestHeader("User-Agent", "Hidden/1.0", ';', TRUE);

			if(logger)
				logger->Log("\tSet to: '%s'\n", pReqRepl->GetRequestHeader("User-Agent"));

			pReqRepl->SetRequestHeader("X-Tagged: ", "True");
		}
		else
		{
			if(logger)
				logger->Log("\tNo 'User-Agent' header\n");
		}

		CDataStreamBase *pStream = pReqRepl->GetRequestBody();
		
		if(pStream)
		{
			if(logger)
				logger->Log("Contains request body stream\n");

			PBYTE buf = pStream->Buffer();
			
			if(buf && pStream->Length() > sizeof("phoobar") )
			{
				for(size_t i=0; i<pStream->Length() - sizeof("phoobar"); i++)
				{
					if(memcmp(buf + i, "phoobar", sizeof("phoobar") - 1) == 0)
					{
						if(logger)
							logger->Log("Disallowing request containing 'phoobar'\n");

						logger->ReleaseLock();

						return EAllowableOperation::Disallowed;
					}
				}
			}

			if(logger)
				logger->Log("Did not find 'phoobar' in request\n");
		}
		else
		{
			if(logger)
				logger->Log("Does not contain request body stream\n");
		}
	}

	if(logger)
		logger->ReleaseLock();

	return EAllowableOperation::Allowed;
}

EAllowableOperation DemoResponseFilteringPolicies::DisallowImageContentType(CConnectionPool *pConnPool, CConnection *pConn)
{
	CLogger *logger = NULL;

	try
	{
		logger = CLogger::GetInstance("bpcore_policy");
		logger->ObtainLock();
		logger->Log("Checking DisallowPolicy for Content-Type: image/*\n", pConn->GetDstHost());
	}
	catch(...)
	{
	}

	CRequestReply *pReqRepl = pConn->GetRequestReply();

	if(logger)
	{
		logger->Log("\tHTTP Banner: %s\n", pReqRepl->GetReplyBanner());
		logger->Log("\tHeaders:\n");

		std::multimap<std::string, std::string>::const_iterator it;

		for(it = pReqRepl->GetReplyHeaders().begin(); it != pReqRepl->GetReplyHeaders().end(); it++)
		{
			logger->Log("\t%s:%s\n", it->first.c_str(), it->second.c_str());
		}
	}

	std::vector<char *> *pvecClHdrs = pReqRepl->GetReplyHeader("Content-Type");
	if(pvecClHdrs)
	{
		for(int i=0; i<pvecClHdrs->size(); i++)
		{
			if(pvecClHdrs->at(i) != NULL && strstr(pvecClHdrs->at(i), "image/") != NULL)
			{
				if(logger)
					logger->Log("Disallowing response with Content-Type: image/* header(s)\n");
		
				if(logger)
					logger->ReleaseLock();

				delete pvecClHdrs;

				return EAllowableOperation::Disallowed;
			}
		}

		delete pvecClHdrs;
	}
	
	if(logger)
		logger->ReleaseLock();

	return EAllowableOperation::Allowed;
}

EAllowableOperation DemoResponseFilteringPolicies::JustReadSomeDataFromStream(CConnectionPool *pConnPool, CConnection *pConn)
{
	CLogger *logger = NULL;

	try
	{
		logger = CLogger::GetInstance("bpcore_policy");
		logger->ObtainLock();
		logger->Log("DemoResponseFilteringPolicies::JustReadSomeDataFromStream\n", pConn->GetDstHost());
	}
	catch(...)
	{
	}

	CDataStreamBase *pReplyStream = pConn->GetRequestReply()->GetReplyBody();

	if(!pReplyStream)
	{
		if(logger)
			logger->Log("\tNo reply stream attached yet ...\n");
	}
	else
	{
#define READCOUNT	(1024 * 16)
		if(logger)
			logger->Log("\tReply stream attached. Reading %u bytes\n", READCOUNT);

		size_t cbRead = 0;
		BYTE buf[READCOUNT];
		
		EDataStreamError err = pReplyStream->Read(0, READCOUNT, buf, &cbRead);
		if(err == EDataStreamError::StreamFailure)
		{
			if(logger)
				logger->Log("\nStream Read() resulted in StreamFailure error\n");
		}
		else
		{
			if(logger)
			{
				logger->Log("\nStream Read() succeeded. cbRead = %u\n", cbRead);
				logger->LogHex(buf, cbRead);
			}
		}
	}
	
	logger->ReleaseLock();

	return EAllowableOperation::Allowed;
}

EAllowableOperation DemoResponseFilteringPolicies::RewriteUserAgentHeader(CConnectionPool *pConnPool, CConnection *pConn)
{
	CLogger *logger = NULL;

	try
	{
		logger = CLogger::GetInstance("bpcore_policy");
		logger->ObtainLock();
		logger->Log("DemoResponseFilteringPolicies::RewriteUserAgentHeader\n", pConn->GetDstHost());
	}
	catch(...)
	{
	}

	CRequestReply *pReqRepl = pConn->GetRequestReply();
	if(pReqRepl)
	{
		std::vector<char *> *pvec = pReqRepl->GetReplyHeader("Content-Type");
		if(pvec)
		{
			if(logger)
				logger->Log("\tContent-Type found: '%s'\n", pvec->at(0));
		}
		
		if(logger)
			logger->Log("\tSetting Content-Type to 'text/plain'\n");

		pReqRepl->SetReplyHeader("Content-Type", "text/plain", ';', TRUE);
		delete pvec;

		pvec = pReqRepl->GetReplyHeader("Content-Type");
		if(pvec)
		{
			if(logger)
			logger->Log("\tReading back Content-Type header: '%s'\n", pvec->at(0));
		}

		delete pvec;
	}

	if(logger)
		logger->ReleaseLock();

	return EAllowableOperation::Allowed;
}

EAllowableOperation DemoResponseFilteringPolicies::ReplacePngWithTemplate(CConnectionPool *pConnPool, CConnection *pConn)
{
	CLogger *logger = NULL;

	try
	{
		logger = CLogger::GetInstance("bpcore_policy");
		logger->ObtainLock();
		logger->Log("DemoResponseFilteringPolicies::ReplacePngWithTemplate\n", pConn->GetDstHost());
	}
	catch(...)
	{
	}

	CRequestReply *pReqRepl = pConn->GetRequestReply();
	if(pReqRepl)
	{
		CDataStreamBase *pStream = pReqRepl->GetReplyBody();
		if(!pStream)
		{
			if(logger)
				logger->Log("\tNo reply stream attached\n");
		}
		else
		{
			BYTE buf[0x100] = {};
			size_t bufLen = 0;

			pStream->Read(0, sizeof(buf), buf, &bufLen);
			
			if(bufLen > 3 && memcmp(buf + 1, "PNG", 3) == 0)
			{
				pStream->ReplaceWhole(ReplacementPngTemplateImage, ReplacementPngTemplateImageSize);
				if(logger)
					logger->Log("\tPNG image detected. Replaced with template image.\n");
			}
			else
			{
				if(logger)
					logger->Log("\tData in buffer is not PNG image\n");
			}
		}
	}

	if(logger)
		logger->ReleaseLock();

	return EAllowableOperation::Allowed;
}

EAllowableOperation DemoResponseFilteringPolicies::SimpleParsePngFile(CConnectionPool *pConnPool, CConnection *pConn)
{
	CLogger *logger = NULL;

	try
	{
		logger = CLogger::GetInstance("bpcore_policy");
		logger->ObtainLock();
		logger->Log("DemoResponseFilteringPolicies::SimpleParsePngFile (%s)\n", pConn->GetDstHost());
	}
	catch(...)
	{
	}

	CRequestReply *pReqRepl = pConn->GetRequestReply();
	if(pReqRepl)
	{
		CCommonTypeStreamReader *pStream = pReqRepl->GetReplyBody();
		if(!pStream)
		{
			if(logger)
				logger->Log("\tNo reply stream attached\n");
		}
		else
		{
			BYTE buf[0x100] = {};
			size_t bufLen = 0;

			pStream->ReadByte();
			buf[0] = pStream->ReadByte();
			buf[1] = pStream->ReadByte();
			buf[2] = pStream->ReadByte();

			if(!pStream->Error() && memcmp(buf, "PNG", 3) == 0)
			{
				pStream->ReplaceWhole(ReplacementPngTemplateImage, ReplacementPngTemplateImageSize);
				if(logger)
					logger->Log("\tPNG image detected. Replaced with template image.\n");
			}
			else
			{
				if(logger)
					logger->Log("\tData in buffer is not PNG image\n");
			}
		}
	}

	if(logger)
		logger->ReleaseLock();

	return EAllowableOperation::Allowed;
}

EAllowableOperation DemoResponseFilteringPolicies::ChunkedReadAttempt(CConnectionPool *pConnPool, CConnection *pConn)
{
	CLogger *logger = NULL;

	try
	{
		logger = CLogger::GetInstance("bpcore_policy");
		logger->ObtainLock();
		logger->Log("DemoResponseFilteringPolicies::ChunkedReadAttempt (%s)\n", pConn->GetDstHost());
	}
	catch(...)
	{
	}

	CRequestReply *pReqRepl = pConn->GetRequestReply();
	if(pReqRepl)
	{
		CCommonTypeStreamReader *pStream = pReqRepl->GetReplyBody();
		if(!pStream)
		{
			if(logger)
				logger->Log("\tNo reply stream attached\n");
		}
		else
		{
			BYTE buf[1024] = {0};
			size_t cbRead = 0;

			size_t cbMax = pStream->Length() - pStream->Offset();
			while(cbMax)
			{
				cbRead = (cbMax < sizeof(buf)) ? cbMax : sizeof(buf);

				pStream->Read(cbRead, buf, &cbRead);
				if(cbRead)
				{
					if(logger)
					{
						logger->Log("Read %u (offs %u) total %u\n", cbRead, pStream->Offset() - cbRead, pStream->Length());
						logger->LogHex(buf, cbRead);
					}
				}

				cbMax -= cbRead;
			}

			pReqRepl->SetStreamRequiresChunkProcessing(TRUE);
		}
	}

	if(logger)
		logger->ReleaseLock();

	return EAllowableOperation::Allowed;
}