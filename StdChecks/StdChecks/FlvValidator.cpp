// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#include "stdafx.h"
#include "..\..\BpCore\BpCore\IConnectionPool.h"
#include "..\..\Shared\Utils\Utils.h"
#include "..\..\AuxCore\AuxCore\IEventWriter.h"
#include "Policies.h"

//http://osflash.org/flv
//http://www.metacafe.com/watch/5695544/limitless_movie_trailer/
//http://www.youtube.com/watch?v=BymeLkZ7GqM

#define FLV_PRINTHEX	0

#define FLV_TAG_TYPE_AUDIO		0x08
#define FLV_TAG_TYPE_VIDEO		0x09
#define FLV_TAG_TYPE_METADATA	0x12

EAllowableOperation ResponseFilteringPolicies::CheckFlvFileValidityChunked(IConnectionPool *pConnPool, IConnection *pConn)
{
	EAllowableOperation err = EAllowableOperation::Allowed;
#ifdef DEBUG_BUILD
	CLogger *logger = NULL;
#endif
	ICommonTypeStreamReader *pStream = NULL;

	try
	{
#ifdef DEBUG_BUILD
		logger = CLogger::GetInstance("bpcore_policy");
		logger->ObtainLock();
		logger->Log("ResponseFilteringPolicies::CheckFlvFileValidityChunked\n");
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
			//if(logger)
			//	logger->Log("\tNo reply stream attached\n");
#endif
			goto Cleanup;
		}
		else
		{
			BOOL bFirstHeader = FALSE;

			if(!pReqRepl->GetParseStorage()->GetNamedValue_BOOL(L"parsingFlv"))
			{
#ifdef DEBUG_BUILD
				//if(logger)
				//	logger->Log("\tparsingFlv not yet set!\n");
#endif
				if(pStream->Offset())
				{
#ifdef DEBUG_BUILD
					if(logger)
						logger->Log("\tStream offset not zero: ignoring parse attempt w/o rewind\n");
#endif
					goto Cleanup_no_rewind;
				}

				BYTE sig[4] = {0};
				size_t cbSig = 0;

				pStream->Read(3, sig, &cbSig);
				if(cbSig != 3 || memcmp(sig, "FLV", 3))
				{
					//if(logger)
					//	logger->Log("\tSignature not FLV: aborting parse\n");

					pReqRepl->SetStreamRequiresChunkProcessing(FALSE);
					goto Cleanup;
				}
				
				pReqRepl->SetStreamRequiresChunkProcessing(TRUE);

#ifdef DEBUG_BUILD
				if(logger)
					logger->Log("\tFLV file detected\n");
#endif
				pReqRepl->GetParseStorage()->SetNamedValue_BOOL(L"parsingFlv", FALSE, TRUE);

				BYTE byteVersion = pStream->ReadByte(),
					 byteFlags   = pStream->ReadByte();

				DWORD dwOffset = Utils::DwordSwapEndian(pStream->ReadDword());
				
				if(pStream->Error() || byteVersion != 0x01 || dwOffset != 9)
				{
#ifdef DEBUG_BUILD
					if(logger)
						logger->Log("\tInvalid version, or invalid offset, or no stream data\n");
#endif
					err = EAllowableOperation::Disallowed;
					goto Cleanup;
				}

				BOOL bAudioInFile = byteFlags & 0x04,
					 bVideoInFile = byteFlags & 0x01;

#ifdef DEBUG_BUILD
				if(logger && bAudioInFile)
					logger->Log("\tAudio stream in FLV\n");

				if(logger && bVideoInFile)
					logger->Log("\tVideo stream in FLV\n");
#endif

				pReqRepl->GetParseStorage()->SetNamedValue_BOOL(L"bAudioInFile", FALSE, bAudioInFile);
				pReqRepl->GetParseStorage()->SetNamedValue_BOOL(L"bVideoInFile", FALSE, bVideoInFile);
				
				bFirstHeader = TRUE;
			}

Parse_next_tag:
#ifdef DEBUG_BUILD
			if(FLV_PRINTHEX && logger)
				logger->LogHex(
					pStream->Buffer() + pStream->Offset(),
					(pStream->Length() - pStream->Offset()) < 512 ? (pStream->Length() - pStream->Offset()) : 512
				);
#endif
			if(pReqRepl->GetParseStorage()->GetNamedValue_BOOL(L"parsingFlv"))
			{
#ifdef DEBUG_BUILD
				if(logger)
					logger->Log("\tparsingFlv set\n");
#endif
				BOOL bParsingTag = pReqRepl->GetParseStorage()->GetNamedValue_BOOL(L"bParsingTag");
				if(!bParsingTag)
				{
#ifdef DEBUG_BUILD
					if(logger)
						logger->Log("\tParsing new FLV tag\n");
#endif					
					DWORD dwPrevTagSize = Utils::DwordSwapEndian(pStream->ReadDword());

					if(pStream->Error() || (bFirstHeader && dwPrevTagSize) || // 0x0b is header size
						(!bFirstHeader && dwPrevTagSize != pReqRepl->GetParseStorage()->GetNamedValue_DWORD(L"prevTagSize") + 0x0b))
					{
#ifdef DEBUG_BUILD
						if(logger)
							logger->Log("\tStream error or invalid previous tag size (stream err = %s, first hdr = %s, prevTagSize + 0x0b = %u, dwPrevTagSize = %u)\n",
								pStream->Error() ? "TRUE" : "FALSE",
								bFirstHeader ? "TRUE" : "FALSE", pReqRepl->GetParseStorage()->GetNamedValue_DWORD(L"prevTagSize") + 0x0b,
								dwPrevTagSize);
#endif
						err = EAllowableOperation::Disallowed;
						goto Cleanup;
					}

					bFirstHeader = FALSE;
						
					BYTE byteFlvType = pStream->ReadByte();

					if(pStream->Error())
					{
#ifdef DEBUG_BUILD
						if(logger)
							logger->Log("\tEnd of FLV stream reached\n");
#endif
						// final tag - perhaps - need extra check?
						pReqRepl->SetStreamRequiresChunkProcessing(FALSE);
						
						goto Cleanup;
					}

#ifdef DEBUG_BUILD
					if(logger)
						logger->Log("\tTag type 0x%.02x\n", byteFlvType);
#endif
					// 24-bit length
					DWORD dwBodyLength = pStream->ReadByte() | pStream->ReadByte() << 8 | pStream->ReadByte() << 16;
#ifdef DEBUG_BUILD
					if(logger)
						logger->Log("\tBody length of %u (0x%x)\n", dwBodyLength, dwBodyLength);
#endif
					// 24-bit timestamp
					pStream->ReadByte();
					pStream->ReadByte();
					pStream->ReadByte();
				
					// 8-bit timestamp-extended
					pStream->ReadByte();

#ifdef DEBUG_BUILD
					if(logger)
						logger->Log("\tSkipped timestamp and extended timestamp\n");
#endif
					DWORD dwStreamId = (pStream->ReadByte() << 16) + Utils::WordSwapEndian(pStream->ReadWord());
					if(pStream->Error() || dwStreamId)
					{
#ifdef DEBUG_BUILD
						if(logger)
							logger->Log("\tSteam error or invalid stream ID\n");
#endif
						err = EAllowableOperation::Disallowed;
						goto Cleanup;
					}

					switch(byteFlvType)
					{
					case FLV_TAG_TYPE_AUDIO:
#ifdef DEBUG_BUILD
						if(logger)
							logger->Log("\tFound AUDIO tag\n");
#endif
						if(!pReqRepl->GetParseStorage()->GetNamedValue_BOOL(L"bAudioInFile"))
						{
							// file should not contain audio
#ifdef DEBUG_BUILD
							if(logger)
								logger->Log("\tAudio tag not expected; disallowing!\n");
#endif							
							err = EAllowableOperation::Disallowed;
							goto Cleanup;
						}

						break;
					case FLV_TAG_TYPE_VIDEO:
#ifdef DEBUG_BUILD
						if(logger)
							logger->Log("\tFound VIDEO tag\n");
#endif
						if(!pReqRepl->GetParseStorage()->GetNamedValue_BOOL(L"bVideoInFile"))
						{
							// file should not contain video
#ifdef DEBUG_BUILD
							if(logger)
								logger->Log("\tVideo tag not expected; disallowing!\n");
#endif							
							err = EAllowableOperation::Disallowed;
							goto Cleanup;
						}

						break;
					case FLV_TAG_TYPE_METADATA:
#ifdef DEBUG_BUILD
						if(logger)
							logger->Log("\tFound METADATA tag\n");
#endif						
						break;
					default:
#ifdef DEBUG_BUILD
						if(logger)
							logger->Log("\tIUnknown FLV tag type (0x%x): Disallowing!\n", byteFlvType);
#endif
						err = EAllowableOperation::Disallowed;
						goto Cleanup;
					}

					pReqRepl->GetParseStorage()->SetNamedValue_BYTE(L"byteFlvType", FALSE, byteFlvType);
					pReqRepl->GetParseStorage()->SetNamedValue_DWORD(L"dwBodyLength", FALSE, dwBodyLength);
					pReqRepl->GetParseStorage()->SetNamedValue_BOOL(L"bParsingTag", FALSE, TRUE);
				}

				// reload these properties
				BYTE byteFlvType =  pReqRepl->GetParseStorage()->GetNamedValue_BYTE(L"byteFlvType");
				
#define FLV_DATABUFFER_SIZE	(1024)
				BYTE buf[FLV_DATABUFFER_SIZE] = {0};
				size_t cbBuf = 0;

				size_t cbBytesReadFromBody = pReqRepl->GetParseStorage()->GetNamedValue_SIZE_T(L"cbBytesReadFromBody");
				DWORD dwBodyLength = pReqRepl->GetParseStorage()->GetNamedValue_DWORD(L"dwBodyLength");

				size_t cbReadCount = (dwBodyLength - cbBytesReadFromBody) < FLV_DATABUFFER_SIZE ? (dwBodyLength - cbBytesReadFromBody) : FLV_DATABUFFER_SIZE;
				pStream->Read(cbReadCount, buf, &cbBuf);
				
#ifdef DEBUG_BUILD
				if(logger)
					logger->Log("\tAttempted to read %u bytes (read %u)\n", cbReadCount, cbBuf);

				if(0 && logger)
					logger->LogHex(buf, cbBuf);
#endif

				if(pStream->Error() || cbBuf != cbReadCount)
				{
#ifdef DEBUG_BUILD
					if(logger)
						logger->Log("\tStream error\n");
#endif
					err = EAllowableOperation::Disallowed;
					goto Cleanup;
				}

				// parse data for tag type
				
				switch(byteFlvType)
				{
				case FLV_TAG_TYPE_AUDIO:
					// process audio tag
					break;
				case FLV_TAG_TYPE_VIDEO:
					// process video tag
					break;
				case FLV_TAG_TYPE_METADATA:
					// process metadata tag
					// structure:
					/*
						Two AMF Packets:
							1. Almost always tag type (byte) 0x02: length (uint16_be) prepended utf-8 string
							2. Almost always tag type (byte) 0x08: mixed array of other AMF types.
						
						Parse each type and perhaps ensure this general structure is maintained.
					*/
					break;
				}

				pReqRepl->GetParseStorage()->SetNamedValue_SIZE_T(L"cbBytesReadFromBody", FALSE, cbBytesReadFromBody + cbBuf);
				if(cbReadCount != FLV_DATABUFFER_SIZE)
				{
					// finished parsing body
#ifdef DEBUG_BUILD
					if(logger)
						logger->Log("\tFinished parsing stream body.. next tag..\n");
#endif
					pReqRepl->GetParseStorage()->SetNamedValue_SIZE_T(L"cbBytesReadFromBody", FALSE, 0);
					pReqRepl->GetParseStorage()->SetNamedValue_BOOL(L"bParsingTag", FALSE, FALSE);
					pReqRepl->GetParseStorage()->SetNamedValue_DWORD(L"prevTagSize", FALSE, dwBodyLength);
					
					goto Parse_next_tag;
				}

				goto Cleanup_no_rewind;
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
			strUrl += pConn->GetObjectName(); // += /foo/image.jpg

			pEvent->WriteEvent(
				EEventRepository::VideoValidator,
					EThreatRiskRating::Information,
					L"Malformed FLV File Blocked",
					L"Response filtering policies have blocked a malformed FLV video stream.\r\n"
					L"The video in question is (%hs).\r\n"
					L"Certain video processors are vulnerable to code execution vulnerabilities when attempting to handle malformed video data.",
					strUrl.c_str()
				);
		}
	}

	// if !flv: rewind...
	if(pStream)
		pStream->Seek(0, ESeekPositions::SK_SET);

Cleanup_no_rewind:
#ifdef DEBUG_BUILD
	if(logger)
		logger->ReleaseLock();
#endif

	return err;
}