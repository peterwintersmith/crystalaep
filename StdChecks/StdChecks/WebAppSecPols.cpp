// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#include "stdafx.h"
#include "..\..\BpCore\BpCore\IConnectionPool.h"
#include "..\..\BpCore\BpCore\IStream.h"
#include "..\..\Shared\Utils\Utils.h"
#include "..\..\AuxCore\AuxCore\IEventWriter.h"
#include "Policies.h"

// this function is seriously incomplete and needs to be expanded upon to account for differing code pages, to avoid
// scanning binary data, to factor in POST requests as well as GET - etc.

//http://xss.progphp.com/xss1.html

// BUG: Submit form with param: >

EAllowableOperation UrlFilteringPolicies::CheckInvalidUrlEncoding(IConnectionPool *pConnPool, IConnection *pConn)
{
	EAllowableOperation err = EAllowableOperation::Allowed;
#ifdef DEBUG_BUILD
	CLogger *logger = NULL;
#endif
	char *pszUrlCopy = NULL;

	try
	{
#ifdef DEBUG_BUILD
		logger = CLogger::GetInstance("bpcore_policy");
		logger->ObtainLock();
		logger->Log("UrlFilteringPolicies::CheckInvalidUrlEncoding\n");
#endif
	}
	catch(...)
	{
	}

	std::multimap<std::string, std::string> mmapUrlParamPairs;
		
	pszUrlCopy = strdup(pConn->GetObjectName());
	if(!pszUrlCopy)
		goto Cleanup;

	// pszObject = /foo/bar[.ext][?a=b][&b=c][#xxx]

	char *pszObject = strchr(pszUrlCopy, '?');
	if(!pszObject)
		goto Cleanup;

	pszObject++;
	char *pszEnd = strchr(pszObject, '#');
			
	pszEnd = pszEnd ? pszEnd : (pszObject + strlen(pszObject));
	*pszEnd = '\0';
			
	if(!Utils::GetUrlParams(pszObject, mmapUrlParamPairs))
	{
		// parameter parsing error: suspicious
		err = EAllowableOperation::Disallowed;
		goto Cleanup;
	}
	
#ifdef DEBUG_BUILD
	if(logger)
	{
		logger->Log("\tURL Path: [%s]\n", pConn->GetObjectName());

		for(std::multimap<std::string, std::string>::iterator it = mmapUrlParamPairs.begin(); it != mmapUrlParamPairs.end(); it++)
		{
			BOOL bError = FALSE;
			logger->Log("\tN: [%s] - V: [%s] - U: [%s]\n", it->first.c_str(), it->second.c_str(), Utils::UrlUnescape(it->second, FALSE, bError).c_str());
		}
	}
#endif

	for(std::multimap<std::string, std::string>::iterator it = mmapUrlParamPairs.begin(); it != mmapUrlParamPairs.end(); it++)
	{
		BOOL bError = FALSE;

		for(size_t i=0; i < 2; i++)
		{
			std::string strUnescapedParam = Utils::UrlUnescape(i ? it->first : it->second, FALSE, bError);
			if(bError)
			{
#ifdef DEBUG_BUILD
				if(logger)
					logger->Log("\tBroken URL-encoded parameter in request path: Disallowing!\n");
#endif
				err = EAllowableOperation::Disallowed;
				goto Cleanup;
			}
		}
	}

Cleanup:
	if(err == EAllowableOperation::Disallowed)
	{
		IEventWriter *pEvent = GetEventWriter();
		if(pEvent)
		{
			std::string strUrl = pConn->IsSSL() ? "https://" : "http://";
			strUrl += pConn->GetDstHost(); // += www.google.com
			strUrl += pConn->GetObjectName(); // += /foo/image.gif

			pEvent->WriteEvent(
					EEventRepository::WebAppMonitor,
					EThreatRiskRating::MediumRisk,
					L"Invalid URL Blocked",
					L"The web application security policies have blocked a connection attempt.\r\n"
					L"The target URL in question was: %hs.\r\n"
					L"The URL in question was formed in a highly suspicious fashion, suggesting that the browser was under direct attack.",
					strUrl.c_str()
				);
		}
	}

	if(pszUrlCopy)
		free(pszUrlCopy);

#ifdef DEBUG_BUILD
	if(logger)
		logger->ReleaseLock();
#endif

	return err;
}

EAllowableOperation ResponseFilteringPolicies::CheckCrossSiteScriptingUrlParams(IConnectionPool *pConnPool, IConnection *pConn)
{
	EAllowableOperation err = EAllowableOperation::Allowed;
#ifdef DEBUG_BUILD
	CLogger *logger = NULL;
#endif
	ICommonTypeStreamReader *pStream = NULL;
	char *pszUrlCopy = NULL;

	try
	{
#ifdef DEBUG_BUILD
		logger = CLogger::GetInstance("bpcore_policy");
		logger->ObtainLock();
		logger->Log("ResponseFilteringPolicies::CheckCrossSiteScriptingUrlParams\n");
#endif
	}
	catch(...)
	{
	}

	IRequestReply *pReqRepl = pConn->GetRequestReply();
	if(!pReqRepl)
	{
		goto Cleanup;
	}
	else
	{
		pStream = pReqRepl->GetReplyBody();
		if(!pStream)
		{
#ifdef DEBUG_BUILD
			if(logger)
				logger->Log("\tNo reply stream attached\n");
#endif
			goto Cleanup;
		}
		else
		{
			if(pReqRepl->GetParseStorage()->GetNamedValue_BOOL(L"parsingFlv"))
				goto Cleanup_no_rewind;

			std::multimap<std::string, std::string> mmapUrlParamPairs;
			BOOL bContentTypeText = FALSE;

			std::vector<char *> *pvecCT = pReqRepl->GetReplyHeader("Content-Type");
			if(pvecCT && pvecCT->size() != 0)
			{
				for(size_t i=0; i<pvecCT->size(); i++)
				{
					if(pvecCT->at(i))
					{
						if(strnicmp(pvecCT->at(i), "text/", 5) == 0)
						{
							bContentTypeText = TRUE;
							break;
						}
					}
				}

				if(!bContentTypeText)
					goto Cleanup;
			}
			
			pszUrlCopy = strdup(pConn->GetObjectName());
			if(!pszUrlCopy)
				goto Cleanup;

			// pszObject = /foo/bar[.ext][?a=b][&b=c][#xxx]

			char *pszObject = strchr(pszUrlCopy, '?');
			if(!pszObject)
				goto Cleanup;

			pszObject++;
			char *pszEnd = strchr(pszObject, '#');
			
			pszEnd = pszEnd ? pszEnd : (pszObject + strlen(pszObject));
			*pszEnd = '\0';
			
			if(!Utils::GetUrlParams(pszObject, mmapUrlParamPairs))
			{
				// parameter parsing error: suspicious
				err = EAllowableOperation::Disallowed;
				goto Cleanup;
			}
			
#ifdef DEBUG_BUILD
			if(logger)
			{
				logger->Log("\tURL Path: [%s]\n", pConn->GetObjectName());

				for(std::multimap<std::string, std::string>::iterator it = mmapUrlParamPairs.begin(); it != mmapUrlParamPairs.end(); it++)
				{
					BOOL bError = FALSE;
					logger->Log("\tN: [%s] - V: [%s] - U:[%s]\n", it->first.c_str(), it->second.c_str(), Utils::UrlUnescape(it->second, FALSE, bError).c_str());
				}
			}
#endif
			for(std::multimap<std::string, std::string>::iterator it = mmapUrlParamPairs.begin(); it != mmapUrlParamPairs.end(); it++)
			{
				BOOL bError = FALSE;

				for(size_t i=0; i < 2; i++)
				{
					std::string strUnescapedParam = Utils::UrlUnescape(i ? it->first : it->second, FALSE, bError);

					if(bError)
					{
						char szUnprotectableXssWarning[] = ""
							"<html>\r\n"
							"\t<head>\r\n"
							"\t\t<title>Page Blocked</title>\r\n"
							"\t</head>\r\n"
							"\t<body>\r\n"
							"\t\t<h1>Page Blocked</h1>\r\n"
							"\t\t<br>\r\n"
							"\t\t<p>The page was blocked as a possible instance of cross-site scripting was identified which could not be otherwise mitigated.</p>\r\n"
							"\t</body>\r\n"
							"<html>";

						pStream->ReplaceWhole((PBYTE)szUnprotectableXssWarning, sizeof(szUnprotectableXssWarning)-1);
						
#ifdef DEBUG_BUILD
						if(logger)
								logger->Log("\tXSS found: Cannot protect: Blocking page!\n", strUnescapedParam.c_str());
#endif
						//err = EAllowableOperation::Disallowed;
						// write a log entry

						IEventWriter *pEvent = GetEventWriter();
						if(pEvent)
						{
							std::string strUrl = pConn->IsSSL() ? "https://" : "http://";
							strUrl += pConn->GetDstHost(); // += www.google.com
							strUrl += pConn->GetObjectName(); // += /foo/image.gif

							pEvent->WriteEvent(
									EEventRepository::WebAppMonitor,
									EThreatRiskRating::MediumRisk,
									L"Cross-site Scripting Attempt Blocked",
									L"The web application security policies have blocked content from a web site.\r\n"
									L"The target URL in question was: %hs.\r\n"
									L"The URL in question was formed in a highly suspicious fashion, suggesting that the browser was under a cross-site scripting attack.",
									strUrl.c_str()
								);
						}

						goto Cleanup;
					}

					if(Utils::ContainsHtmlMetacharacters(strUnescapedParam))
					{
						
						// we really need all the HTML data, in-case our params only match part of the body, effecting a bypass.
						// The later we read the entire stream the fewer pages we impact speed wise.

						BYTE TempBuf[1024];
						size_t cbTempRead = 0;

						for(size_t i=0; ; i += cbTempRead)
						{
							cbTempRead = 0;

							pStream->Read(i, sizeof(TempBuf), TempBuf, &cbTempRead);

							if(pStream->Error() || cbTempRead == 0)
								break;
						}

						// of course we need to figure out the page text encoding, ensure the content-type is text/* (perhaps?)
						// and encode the search parameter to match the appropriate page encoding.

						char *pszXssPos = NULL;
						size_t offs = 0;

						while((pszXssPos = Utils::stristr(
								(char *)pStream->Buffer() + offs,
								const_cast<char *>(strUnescapedParam.c_str()),
								pStream->Length() - offs
							)) != NULL)
						{
							// URL param with HTML metachar found in response: XSS
#ifdef DEBUG_BUILD
							if(logger)
								logger->Log("\tXSS found: param [unescaped(%s)]. Mitigating!\n", strUnescapedParam.c_str());
#endif							
							std::string strSafeParam = "";
						
							for(size_t i=0; i<strUnescapedParam.size(); i++)
							{
								strSafeParam += "<span>";
								strSafeParam += strUnescapedParam.c_str()[i];
								strSafeParam += "</span>";
							}

							offs = (size_t)(pszXssPos - (char *)pStream->Buffer());

							pStream->Write(
									offs,
									(PBYTE)strSafeParam.c_str(),
									strSafeParam.size(),
									TRUE
								);

							// should not overflow, pStream->Length() updated to reflect insertion
							offs += strSafeParam.size();

							// Buffer() may have been reallocated
							pStream->Erase(offs, strUnescapedParam.size());

							IEventWriter *pEvent = GetEventWriter();
							if(pEvent)
							{
								std::string strUrl = pConn->IsSSL() ? "https://" : "http://";
								strUrl += pConn->GetDstHost(); // += www.google.com
								strUrl += pConn->GetObjectName(); // += /foo/image.gif

								pEvent->WriteEvent(
										EEventRepository::WebAppMonitor,
										EThreatRiskRating::MediumRisk,
										L"Cross-site Scripting Attempt Mitigated",
										L"The web application security policies have mitigated a cross-site scripting attempt.\r\n"
										L"The target URL in question was: %hs.\r\n"
										L"The URL in question would have triggered a cross-site scripting attack against the browser, however the attack was safely mitigated.\r\n"
										L"The suspicious parameter in question was (name, value): (%hs, %hs).",
										strUrl.c_str(), it->first.c_str(), it->second.c_str()
									);
							}
						}

						// write a log entry
					}
				}
			}
		}
	}

Cleanup:
	if(err == EAllowableOperation::Disallowed)
	{
		IEventWriter *pEvent = GetEventWriter();
		if(pEvent)
		{
			std::string strUrl = pConn->IsSSL() ? "https://" : "http://";
			strUrl += pConn->GetDstHost(); // += www.google.com
			strUrl += pConn->GetObjectName(); // += /foo/image.gif

			pEvent->WriteEvent(
					EEventRepository::WebAppMonitor,
					EThreatRiskRating::MediumRisk,
					L"Cross-site Scripting Attempt Blocked",
					L"The web application security policies have blocked content from a web site.\r\n"
					L"The target URL in question was: %hs.\r\n"
					L"The URL in question was formed in a highly suspicious fashion, suggesting that the browser was under a cross-site scripting attack.",
					strUrl.c_str()
				);
		}
	}

	if(pStream)
		pStream->Seek(0, ESeekPositions::SK_SET);
	
Cleanup_no_rewind:
	if(pszUrlCopy)
		free(pszUrlCopy);

#ifdef DEBUG_BUILD
	if(logger)
		logger->ReleaseLock();
#endif

	return err;
}

EAllowableOperation ResponseFilteringPolicies::CheckCrossSiteScriptingPostBody(IConnectionPool *pConnPool, IConnection *pConn)
{
	EAllowableOperation err = EAllowableOperation::Allowed;
#ifdef DEBUG_BUILD
	CLogger *logger = NULL;
#endif
	ICommonTypeStreamReader *pRequestStream = NULL, *pReplyStream = NULL;
	char *pszHeaders = NULL;

	try
	{
#ifdef DEBUG_BUILD
		logger = CLogger::GetInstance("bpcore_policy");
		logger->ObtainLock();
		logger->Log("ResponseFilteringPolicies::CheckCrossSiteScriptingPostBody\n");
#endif
	}
	catch(...)
	{
	}

	IRequestReply *pReqRepl = pConn->GetRequestReply();
	if(!pReqRepl)
	{
		goto Cleanup;
	}
	else
	{
		pRequestStream = pReqRepl->GetRequestBody();
		pReplyStream = pReqRepl->GetReplyBody();

		if(!pRequestStream || !pReplyStream)
		{
#ifdef DEBUG_BUILD
			if(logger)
				logger->Log("\tNo request stream and no reply stream attached\n");
#endif
			if(pReqRepl->GetParseStorage()->GetNamedValue_BOOL(L"parsingFlv"))
				goto Cleanup_no_rewind;

			goto Cleanup;
		}
		else
		{
			if(pReqRepl->GetParseStorage()->GetNamedValue_BOOL(L"parsingFlv"))
				goto Cleanup_no_rewind;

			std::multimap<std::string, std::string> mmapUrlParamPairs;
			BOOL bUrlEncodedForm = FALSE, bMultipartForm = FALSE;

			char *pszFormContentType = pReqRepl->GetRequestHeader("Content-Type");
			if(pszFormContentType)
			{
#ifdef DEBUG_BUILD
				if(logger)
					logger->Log("\tContent-Type = [%s]\n", pszFormContentType);
#endif
				if(Utils::stristr(pszFormContentType, "/x-www-form-urlencoded"))
					bUrlEncodedForm = TRUE;
				else if(Utils::stristr(pszFormContentType, "multipart/form-data"))
					bMultipartForm = TRUE;
			}

#ifdef DEBUG_BUILD
			if(0 && logger)
			{
				for(std::map<std::string, std::string>::const_iterator it = pReqRepl->GetRequestHeaders().begin();
					it != pReqRepl->GetRequestHeaders().end();
					it++)
				{
					logger->Log("\theader [%s] value[%s]\n", it->first.c_str(), it->second.c_str());
				}
			}
#endif
			// not a recognised form type; searching for XSS may not be a good idea
			if(!bUrlEncodedForm && !bMultipartForm)
				goto Cleanup;

			if(bUrlEncodedForm)
			{
				// entire body is URL-encoded
				std::string strBody((char *)pRequestStream->Buffer(), pRequestStream->Length());

				BOOL bError = FALSE, bQuotedPrintable = FALSE;
				char *pszContTransferEnc = NULL;

				if(pszContTransferEnc = pReqRepl->GetRequestHeader("Content-Transfer-Encoding"))
				{
					if(Utils::stristr(pszContTransferEnc, "quoted-printable"))
					{
						// quoted printable content type
						bQuotedPrintable = TRUE;
					}
				}
				
				// should I support nested encoding i.e. url-encoded(quoted-printable(text))?

				if(bQuotedPrintable)
					strBody = Utils::UrlUnescapeW(strBody, FALSE, bError, '=');

				// if i escape here, i run the risk of double decoding later:
				/*
					Body:
						foo=bar&bar=aaaa%26bbb%3dcccc
						Is 2 params: foo and bar

					If escaped here:
						foo=bar&bar=aaaa&bbb=cccc
						Is three params: foo, bar and aaaa

					Which is incorrect.
				*/


				//else
				//	strBody = Utils::UrlUnescapeW(strBody, FALSE, bError);
				
				if(!Utils::GetUrlParams(strBody.c_str(), mmapUrlParamPairs))
				{
					// parameter parsing error: suspicious
					err = EAllowableOperation::Disallowed;
					goto Cleanup;
				}
			}
			else if(bMultipartForm)
			{
				// body is separated by boundary marker
				/*
					Content-Type: multipart/form-data; boundary=AaB03x

					--AaB03x
					Content-Disposition: form-data; name="submit-name"

					Larry
					--AaB03x
					Content-Disposition: form-data; name="files"; filename="file1.txt"
					Content-Type: text/plain

					... contents of file1.txt ...
					--AaB03x--
				*/

				char *pszBoundary = Utils::stristr(pszFormContentType, "boundary");
				if(!pszBoundary)
				{
					err = EAllowableOperation::Disallowed;
					goto Cleanup;
				}

				// Content-Type: multipart/form-data; [boundary    = AaB03x]

				pszBoundary += sizeof("boundary")-1;
				pszBoundary += Utils::TrimCount(pszBoundary);

				if(*pszBoundary != '=')
				{
					err = EAllowableOperation::Disallowed;
					goto Cleanup;
				}

				pszBoundary++;

				//Content-Type: multipart/form-data; boundary   = [AaB03x]

				pszBoundary += Utils::TrimCount(pszBoundary);

				char *pszStartOfBoundary = pszBoundary;

				while(isalpha(*pszBoundary) || isdigit(*pszBoundary) || *pszBoundary == '-')
					pszBoundary++;

				std::string strBoundary(pszStartOfBoundary, (size_t)(pszBoundary - pszStartOfBoundary));
				if(strBoundary.size() == 0)
				{
					err = EAllowableOperation::Disallowed;
					goto Cleanup;
				}

				// strBoundary == "AaB03x"

				strBoundary = "--" + strBoundary;
				// now properly formatted
				
				char *pszCurrentPart = Utils::stristr(
						(char *)pRequestStream->Buffer(),
						const_cast<char *>(strBoundary.c_str()),
						pRequestStream->Length()
					);

				while(1)
				{
					if(!pszCurrentPart)
					{
						err = EAllowableOperation::Disallowed;
						goto Cleanup;
					}
					
					pszCurrentPart += strBoundary.size();

					if(!strncmp(pszCurrentPart, "--", 2))
					{
						// final marker - end of items [--AaB03x--]
						break;
					}
					else
					{
						pszCurrentPart += Utils::TrimCount(pszCurrentPart);

						if(pszCurrentPart[0] == '\r' && pszCurrentPart[1] == '\n')
						{
							pszCurrentPart += sizeof("\r\n")-1;
						}
						else
						{
							err = EAllowableOperation::Disallowed;
							goto Cleanup;
						}

						// so pointing to start of part data:
						/*
							--AaB03x
				--> here	Content-Disposition: form-data; name="submit-name"

							Larry
							--AaB03x
						*/
						// next: find end of data

						char *pszHeaderEnd = strstr(pszCurrentPart, "\r\n\r\n"); // can "\n\n" also be used?
						if(!pszHeaderEnd)
						{
							err = EAllowableOperation::Disallowed;
							goto Cleanup;
						}

						pszHeaders = strdup(std::string(pszCurrentPart, (size_t)(pszHeaderEnd - pszCurrentPart)).c_str());
						if(!pszHeaders)
						{
							err = EAllowableOperation::Disallowed;
							goto Cleanup;
						}

						BOOL bTextType = TRUE, bQuotedPrintable = FALSE;
						std::string strName;

						char *pszSingleHeader = strtok(pszHeaders, "\n");

						while(pszSingleHeader)
						{
							if(!strnicmp(pszSingleHeader, "Content-Type", sizeof("Content-Type")-1))
							{
								if(!strstr(pszSingleHeader, "text/"))
									bTextType = FALSE;
							}
							else if(!strnicmp(pszSingleHeader, "Content-Transfer-Encoding", sizeof("Content-Transfer-Encoding")-1))
							{
								if(Utils::stristr(pszSingleHeader, "quoted-printable"))
									bQuotedPrintable = FALSE;
							}
							else if(!strnicmp(pszSingleHeader, "Content-Disposition", sizeof("Content-Disposition")-1))
							{
								char *pszName = Utils::stristr(pszSingleHeader, "name");
								if(!pszName)
								{
									err = EAllowableOperation::Disallowed;
									goto Cleanup;
								}

								pszName += sizeof("name")-1;
								pszName += Utils::TrimCount(pszName);
								
								if(*pszName != '=')
								{
									err = EAllowableOperation::Disallowed;
									goto Cleanup;
								}

								pszName++;
								pszName += Utils::TrimCount(pszName);

								char *pszNameEnd = NULL;
								
								if(*pszName != '\"')
								{
									err = EAllowableOperation::Disallowed;
									goto Cleanup;
								}
								
								pszName++;

								pszNameEnd = strchr(pszName, '\"');
								if(!pszNameEnd)
								{
									err = EAllowableOperation::Disallowed;
									goto Cleanup;
								}

								if(strName.size() != 0)
								{
									// two content-disp headers - very suspect
									err = EAllowableOperation::Disallowed;
									goto Cleanup;
								}

								strName = std::string(pszName, (size_t)(pszNameEnd - pszName));
							}

							pszSingleHeader = strtok(NULL, "\n");
						}

						free(pszHeaders);
						pszHeaders = NULL;

						pszHeaderEnd += sizeof("\r\n\r\n")-1;

						pszCurrentPart = Utils::stristr(
								(char *)pszCurrentPart,
								const_cast<char *>(strBoundary.c_str()),
								pRequestStream->Length() - (pszCurrentPart - (char *)pRequestStream->Buffer())
							);

						if(!bTextType)
							continue;

						std::string strValue(pszHeaderEnd, (size_t)(pszCurrentPart - pszHeaderEnd - 2));

						if(bQuotedPrintable)
						{
							BOOL bError = FALSE;
							strValue = Utils::UrlUnescapeW(strValue, FALSE, bError, '=');

							if(bError)
							{
								err = EAllowableOperation::Disallowed;
								goto Cleanup;
							}
						}

						if(strName.empty() || !strName.size())
						{
							err = EAllowableOperation::Disallowed;
							goto Cleanup;
						}
						
						if(strValue.empty())
							strValue = "";

						mmapUrlParamPairs.insert(std::pair<std::string, std::string>(strName, strValue));
					}
				}
			}

#ifdef DEBUG_BUILD
			if(logger)
			{
				for(std::multimap<std::string, std::string>::iterator it = mmapUrlParamPairs.begin(); it != mmapUrlParamPairs.end(); it++)
				{
					logger->Log("\tN: [%s] - V: [%s]\n", it->first.c_str(), it->second.c_str());
				}
			}
#endif
			// mmapUrlParamPairs contains all name/value pairs. If empty: no name/value pairs of text type

			for(std::multimap<std::string, std::string>::iterator it = mmapUrlParamPairs.begin(); it != mmapUrlParamPairs.end(); it++)
			{
				BOOL bError = FALSE;

				for(size_t i=0; i < 2; i++)
				{
					std::string strUnescapedParam = Utils::UrlUnescape(i ? it->first : it->second, FALSE, bError);

					if(bError)
					{
						char szUnprotectableXssWarning[] = ""
							"<html>\r\n"
							"\t<head>\r\n"
							"\t\t<title>Page Blocked</title>\r\n"
							"\t</head>\r\n"
							"\t<body>\r\n"
							"\t\t<h1>Page Blocked</h1>\r\n"
							"\t\t<br>\r\n"
							"\t\t<p>The page was blocked as a possible instance of cross-site scripting was identified which could not be otherwise mitigated.</p>\r\n"
							"\t</body>\r\n"
							"<html>";

						pReplyStream->ReplaceWhole((PBYTE)szUnprotectableXssWarning, sizeof(szUnprotectableXssWarning)-1);
						
#ifdef DEBUG_BUILD
						if(logger)
							logger->Log("\tXSS found: Cannot protect: Blocking page!\n", strUnescapedParam.c_str());
#endif
						//err = EAllowableOperation::Disallowed;
						// write a log entry
						
						IEventWriter *pEvent = GetEventWriter();
						if(pEvent)
						{
							std::string strUrl = pConn->IsSSL() ? "https://" : "http://";
							strUrl += pConn->GetDstHost(); // += www.google.com
							strUrl += pConn->GetObjectName(); // += /foo/image.gif

							pEvent->WriteEvent(
									EEventRepository::WebAppMonitor,
									EThreatRiskRating::MediumRisk,
									L"Cross-site Scripting Attempt Blocked",
									L"The web application security policies have blocked content from a web site.\r\n"
									L"The target URL in question was: %hs.\r\n"
									L"The form submission to the URL in question was formed in a highly suspicious fashion, suggesting that the browser was under a cross-site scripting attack.",
									strUrl.c_str()
								);
						}

						goto Cleanup;
					}

					if(Utils::ContainsHtmlMetacharacters(strUnescapedParam))
					{
						// we really need all the HTML data, in-case our params only match part of the body, effecting a bypass.
						// The later we read the entire stream the fewer pages we impact speed wise.

						BYTE TempBuf[1024];
						size_t cbTempRead = 0;

						for(size_t i=0; ; i += cbTempRead)
						{
							cbTempRead = 0;

							pReplyStream->Read(i, sizeof(TempBuf), TempBuf, &cbTempRead);

							if(pReplyStream->Error() || cbTempRead == 0)
								break;
						}

						// of course we need to figure out the page text encoding, ensure the content-type is text/* (perhaps?)
						// and encode the search parameter to match the appropriate page encoding.

						char *pszXssPos = NULL;
						size_t offs = 0;

						while((pszXssPos = Utils::stristr(
								(char *)pReplyStream->Buffer() + offs,
								const_cast<char *>(strUnescapedParam.c_str()),
								pReplyStream->Length() - offs
							)) != NULL)
						{
							// URL param with HTML metachar found in response: XSS
#ifdef DEBUG_BUILD
							if(logger)
								logger->Log("\tXSS found: param [unescaped(%s)]. Mitigating!\n", strUnescapedParam.c_str());
#endif
							std::string strSafeParam = "";
						
							for(size_t i=0; i<strUnescapedParam.size(); i++)
							{
								strSafeParam += "<span>";
								strSafeParam += strUnescapedParam.c_str()[i];
								strSafeParam += "</span>";
							}

							offs = (size_t)(pszXssPos - (char *)pReplyStream->Buffer());

							pReplyStream->Write(
									offs,
									(PBYTE)strSafeParam.c_str(),
									strSafeParam.size(),
									TRUE
								);

							// should not overflow, pStream->Length() updated to reflect insertion
							offs += strSafeParam.size();

							// Buffer() may have been reallocated
							pReplyStream->Erase(offs, strUnescapedParam.size());

							IEventWriter *pEvent = GetEventWriter();
							if(pEvent)
							{
								std::string strUrl = pConn->IsSSL() ? "https://" : "http://";
								strUrl += pConn->GetDstHost(); // += www.google.com
								strUrl += pConn->GetObjectName(); // += /foo/image.gif

								pEvent->WriteEvent(
										EEventRepository::WebAppMonitor,
										EThreatRiskRating::MediumRisk,
										L"Cross-site Scripting Attempt Mitigated",
										L"The web application security policies have mitigated a cross-site scripting attempt.\r\n"
										L"The target URL in question was: %hs.\r\n"
										L"A form submission to the URL in question would have triggered a cross-site scripting attack against the browser, however the attack was safely mitigated.\r\n"
										L"The suspicious parameter in question was (name, value): (%hs, %hs).",
										strUrl.c_str(), it->first.c_str(), it->second.c_str()
									);
							}
						}
						
						// write a log entry
					}
				}
			}
		}
	}

Cleanup:
	if(err == EAllowableOperation::Disallowed)
	{
		IEventWriter *pEvent = GetEventWriter();
		if(pEvent)
		{
			std::string strUrl = pConn->IsSSL() ? "https://" : "http://";
			strUrl += pConn->GetDstHost(); // += www.google.com
			strUrl += pConn->GetObjectName(); // += /foo/image.gif

			pEvent->WriteEvent(
					EEventRepository::WebAppMonitor,
					EThreatRiskRating::MediumRisk,
					L"Cross-site Scripting Attempt Blocked",
					L"The web application security policies have blocked content from a web site.\r\n"
					L"The target URL in question was: %hs.\r\n"
					L"The a form submission to the URL in question was formed in a highly suspicious fashion, suggesting that the browser was under a cross-site scripting attack.",
					strUrl.c_str()
				);
		}
	}

	if(pReplyStream)
		pReplyStream->Seek(0, ESeekPositions::SK_SET);

Cleanup_no_rewind:
	if(pszHeaders)
		free(pszHeaders);
	
	if(pRequestStream)
		pRequestStream->Seek(0, ESeekPositions::SK_SET);
	
#ifdef DEBUG_BUILD
	if(logger)
		logger->ReleaseLock();
#endif

	return err;
}