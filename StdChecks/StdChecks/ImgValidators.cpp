// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#include "stdafx.h"
#include "..\..\BpCore\BpCore\IConnectionPool.h"
#include "..\..\Shared\Utils\Utils.h"
#include "..\..\BpCore\BpCore\IBrwsrInstDetails.h"
#include "..\..\AuxCore\AuxCore\IEventWriter.h"
#include "..\..\BpCore\BpCore\StreamWriter.h"
#include "Policies.h"

EAllowableOperation ResponseFilteringPolicies::IsStreamingProtocolFormat(IConnectionPool *pConnPool, IConnection *pConn)
{
	EAllowableOperation err = EAllowableOperation::Allowed;
	
	IRequestReply *pReqRepl = pConn->GetRequestReply();
	if(pReqRepl)
	{
		ICommonTypeStreamReader *pStream = NULL;
		pStream = pReqRepl->GetReplyBody();
		if(pStream)
		{
			if(pStream->Offset() == 0 && pStream->Length() >= 8)
			{
				// check whether sig is a streaming protocol format: set requires chunked processing
				// pReqRepl->SetStreamRequiresChunkProcessing(TRUE);

				/*
				if(!memcmp(sig, "\x89PNG\x0d\x0a\x1a\x0a", 8))
				{
					// say PNGs are streamable, for example
					//pReqRepl->SetStreamRequiresChunkProcessing(TRUE);
				}
				*/

				// some youtube vids (many of them) are not FLV!

				if(!memcmp(pStream->Buffer(), "FLV", 3))
				{
					pReqRepl->SetStreamRequiresChunkProcessing(TRUE);
				}
			}
		}
	}

	return EAllowableOperation::Allowed;
}

//http://www.w3.org/TR/PNG/
//http://upload.wikimedia.org/wikipedia/commons/9/9a/PNG_transparency_demonstration_2.png

EAllowableOperation ResponseFilteringPolicies::CheckPngFileValidity(IConnectionPool *pConnPool, IConnection *pConn)
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
		logger->Log("ResponseFilteringPolicies::CheckPngFileValidity\n");
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

			BYTE sig[16] = {};
			size_t sigLen = 0;

			pStream->Read(8, sig, &sigLen);

			if(sigLen != 8 || memcmp(sig, "\x89PNG\x0d\x0a\x1a\x0a", 8))
			{
#ifdef DEBUG_BUILD
				if(logger)
					logger->Log("\tNot PNG file\n");
#endif
				goto Cleanup;
			}

#ifdef DEBUG_BUILD
			if(logger)
				logger->Log("\tFound PNG\n");
#endif
			DWORD dwLengthAccumulated = 0;
			BOOL bFirstChunk = TRUE, bFoundIDAT = FALSE, bFoundAncillaryChunk = FALSE,
				bFoundTRNS = FALSE, bFoundPLTE = FALSE, bFoundCHRM = FALSE, bFoundGAMA = FALSE;
			
			DWORD dwImgWidth = 0, dwImgHeight = 0;
			BYTE byteDepth = 0, byteColourType = 0, byteCompression = 0, byteFilter = 0, byteInterlace = 0;
			DWORD dwPlteEntries = 0;

			long lWorkingDepth = 0;
			DWORD dwXPPU = 0, dwYPPU = 0;
			BYTE byteUnitSpec = 0;

			std::vector<DWORD> removeChunks;

			while(1)
			{
				DWORD dwChunkLength = Utils::DwordSwapEndian(pStream->ReadDword());
				DWORD dwChunkSignature = pStream->ReadDword();

				if(dwChunkLength >= 0x7fffff00 || pStream->Error())
				{
#ifdef DEBUG_BUILD
					if(logger)
						logger->Log("\t%s\n", pStream->Error() ? "pStream->Error() = TRUE" : "dwChunkLength too large");
#endif
					err = EAllowableOperation::Disallowed;
					goto Cleanup;
				}
				
#ifdef DEBUG_BUILD
				if(logger)
					logger->Log("\tReading chunk of sig %.*s and length 0x%08x\n", 4, (char *)&dwChunkSignature, dwChunkLength);
#endif				
				if(pStream->Seek(dwChunkLength, ESeekPositions::SK_CUR) == EDataStreamError::StreamFailure)
				{
#ifdef DEBUG_BUILD
					if(logger)
						logger->Log("\tSeek(%u, SK_CUR) error\n", dwChunkLength);
#endif
					err = EAllowableOperation::Disallowed;
					goto Cleanup;
				}

				pStream->Seek(-(long)dwChunkLength, ESeekPositions::SK_CUR);

				if(bFirstChunk)
				{
					if(memcmp(&dwChunkSignature, "IHDR", 4))
					{
#ifdef DEBUG_BUILD
						if(logger)
							logger->Log("\tIHDR not first chunk in file!\n");
#endif
						err = EAllowableOperation::Disallowed;
						goto Cleanup;
					}
				}

				switch(Utils::DwordSwapEndian(dwChunkSignature))
				{
				case 'IDAT':
					// multiple permitted, but must have one!

					if(bFoundIDAT && bFoundAncillaryChunk)
					{
						// IDAT chunks appear in succession only
						err = EAllowableOperation::Disallowed;
						goto Cleanup;
					}

					bFoundAncillaryChunk = FALSE;
					bFoundIDAT = TRUE;
					
					pStream->Seek((long)dwChunkLength, ESeekPositions::SK_CUR);

					break;
				case 'IHDR':
					// only permit one per file
					if(bFirstChunk == FALSE)
					{
						err = EAllowableOperation::Disallowed;
						goto Cleanup;
					}

					// read ihdr. validate width, height, depth etc.

					if(dwChunkLength != 4 + 4 + 5)
					{
						err = EAllowableOperation::Disallowed;
						goto Cleanup;
					}

					dwImgWidth = Utils::DwordSwapEndian(pStream->ReadDword()),
					dwImgHeight = Utils::DwordSwapEndian(pStream->ReadDword());

					byteDepth = pStream->ReadByte(),
					byteColourType = pStream->ReadByte(),
					byteCompression = pStream->ReadByte(),
					byteFilter = pStream->ReadByte(),
					byteInterlace = pStream->ReadByte();

					lWorkingDepth = pow((long double)2, (int)byteDepth);

					if(!dwImgWidth)
					{
						err = EAllowableOperation::Disallowed;
						goto Cleanup;
					}

					if(((dwImgWidth * dwImgHeight) / dwImgWidth) != dwImgHeight)
					{
						err = EAllowableOperation::Disallowed;
						goto Cleanup;
					}

					switch(byteColourType)
					{
					case 0: // Greyscale
						
						if(byteDepth != 1 &&  byteDepth != 2 && byteDepth != 4 &&
							byteDepth != 8 && byteDepth != 16)
						{
							err = EAllowableOperation::Disallowed;
							goto Cleanup;
						}

						break;
					case 2: // Truecolour

						if(byteDepth != 8 && byteDepth != 16)
						{
							err = EAllowableOperation::Disallowed;
							goto Cleanup;
						}

						break;
					case 3: // Indexed-colour
						
						if(byteDepth != 1 && byteDepth != 2 && byteDepth != 4 && byteDepth != 8)
						{
							err = EAllowableOperation::Disallowed;
							goto Cleanup;
						}

						break;
					case 4: // Greyscale with alpha
					case 6: // Truecolour with alpha
						
						if(byteDepth != 8 && byteDepth != 16)
						{
							err = EAllowableOperation::Disallowed;
							goto Cleanup;
						}

						break;
					default:
						err = EAllowableOperation::Disallowed;
						goto Cleanup;
					}

					if(byteFilter != 0 || byteCompression != 0)
					{
						err = EAllowableOperation::Disallowed;
						goto Cleanup;
					}

					if(byteInterlace != 0 && byteInterlace != 1)
					{
						err = EAllowableOperation::Disallowed;
						goto Cleanup;
					}

					bFirstChunk = FALSE;

					break;
				case 'tRNS':

					if(bFoundTRNS)
					{
						err = EAllowableOperation::Disallowed;
						goto Cleanup;
					}

					switch(byteColourType)
					{
					case 0:

						if(dwChunkLength != 2)
						{
							err = EAllowableOperation::Disallowed;
							goto Cleanup;
						}

						break;
					case 2:

						if(dwChunkLength != 6)
						{
							err = EAllowableOperation::Disallowed;
							goto Cleanup;
						}

						break;
					case 3:
						
						if(!bFoundPLTE)
						{
							err = EAllowableOperation::Disallowed;
							goto Cleanup;
						}

						if(dwPlteEntries < dwChunkLength)
						{
							err = EAllowableOperation::Disallowed;
							goto Cleanup;
						}

						break;
					default:
						err = EAllowableOperation::Disallowed;
						goto Cleanup;
					}

					bFoundAncillaryChunk = TRUE;
					bFoundTRNS = TRUE;

					pStream->Seek((long)dwChunkLength, ESeekPositions::SK_CUR);

					break;
				case 'PLTE':

					if(bFoundPLTE || (dwChunkLength % 3) || byteColourType == 0 || byteColourType == 4)
					{
						err = EAllowableOperation::Disallowed;
						goto Cleanup;
					}

					dwPlteEntries = dwChunkLength / 3;

					if(dwPlteEntries > lWorkingDepth)
					{
						err = EAllowableOperation::Disallowed;
						goto Cleanup;
					}

					bFoundPLTE = TRUE;
					bFoundAncillaryChunk = TRUE;

					pStream->Seek((long)dwChunkLength, ESeekPositions::SK_CUR);

					break;
				case 'cHRM':

					if(dwChunkLength != (4 * 8))
					{
						err = EAllowableOperation::Disallowed;
						goto Cleanup;
					}

					bFoundCHRM = TRUE;
					bFoundAncillaryChunk = TRUE;

					pStream->Seek((long)dwChunkLength, ESeekPositions::SK_CUR);

					break;
				case 'gAMA':

					if(dwChunkLength != 4)
					{
						err = EAllowableOperation::Disallowed;
						goto Cleanup;
					}

					bFoundGAMA = TRUE;
					bFoundAncillaryChunk = TRUE;

					pStream->Seek((long)dwChunkLength, ESeekPositions::SK_CUR);

					break;
				case 'sBIT':

					BYTE byteSignifBits;

					switch(byteColourType)
					{
					case 0:
						
						if(dwChunkLength != 1)
						{
							err = EAllowableOperation::Disallowed;
							goto Cleanup;
						}

						byteSignifBits = pStream->ReadByte();
						if(byteSignifBits == 0 || byteSignifBits > lWorkingDepth)
						{
							err = EAllowableOperation::Disallowed;
							goto Cleanup;
						}

						break;
					case 2:
						
						if(dwChunkLength != 3)
						{
							err = EAllowableOperation::Disallowed;
							goto Cleanup;
						}

						for(int i=0; i<3; i++)
						{
							byteSignifBits = pStream->ReadByte();
							if(byteSignifBits == 0 || byteSignifBits > lWorkingDepth)
							{
								err = EAllowableOperation::Disallowed;
								goto Cleanup;
							}
						}

						break;
					case 3:
						
						if(dwChunkLength != 3)
						{
							err = EAllowableOperation::Disallowed;
							goto Cleanup;
						}

						for(int i=0; i<3; i++)
						{
							byteSignifBits = pStream->ReadByte();
							if(byteSignifBits == 0 || byteSignifBits > 8 /* 8 is indexed-colours static depth */ )
							{
								err = EAllowableOperation::Disallowed;
								goto Cleanup;
							}
						}

						break;
					case 4:
						
						if(dwChunkLength != 2)
						{
							err = EAllowableOperation::Disallowed;
							goto Cleanup;
						}

						for(int i=0; i<2; i++)
						{
							byteSignifBits = pStream->ReadByte();
							if(byteSignifBits == 0 || byteSignifBits > lWorkingDepth)
							{
								err = EAllowableOperation::Disallowed;
								goto Cleanup;
							}
						}

						break;
					case 6:
						
						if(dwChunkLength != 4)
						{
							err = EAllowableOperation::Disallowed;
							goto Cleanup;
						}
						
						for(int i=0; i<4; i++)
						{
							byteSignifBits = pStream->ReadByte();
							if(byteSignifBits == 0 || byteSignifBits > lWorkingDepth)
							{
								err = EAllowableOperation::Disallowed;
								goto Cleanup;
							}
						}

						break;
					}

					break;
				case 'pHYs':

					if(dwChunkLength != 4 + 4 + 1)
					{
						err = EAllowableOperation::Disallowed;
						goto Cleanup;
					}

					dwXPPU = Utils::DwordSwapEndian(pStream->ReadDword());
					dwYPPU = Utils::DwordSwapEndian(pStream->ReadDword());
					byteUnitSpec = pStream->ReadByte();

					if(!dwYPPU)
					{
						err = EAllowableOperation::Disallowed;
						goto Cleanup;
					}

					if(((dwXPPU * dwYPPU) / dwYPPU) != dwXPPU || (byteUnitSpec != 0 && byteUnitSpec != 1))
					{
						err = EAllowableOperation::Disallowed;
						goto Cleanup;
					}

					break;
				case 'sRGB':
					
					if(dwChunkLength != 1)
					{
						err = EAllowableOperation::Disallowed;
						goto Cleanup;
					}

					if(pStream->ReadByte() > 4)
					{
						err = EAllowableOperation::Disallowed;
						goto Cleanup;
					}

					break;
				case 'IEND':
					// don't remove IEND chunk!

					if(dwChunkLength != 0)
					{
						err = EAllowableOperation::Disallowed;
						goto Cleanup;
					}
					
					break;
				default:
					removeChunks.push_back(dwChunkSignature);
					pStream->Seek((long)dwChunkLength, ESeekPositions::SK_CUR);

					break;
				}

				DWORD dwCRC = Utils::DwordSwapEndian(pStream->ReadDword());
				if(pStream->Error())
				{
#ifdef DEBUG_BUILD
					if(logger)
						logger->Log("\tError reading CRC\n", dwChunkLength);
#endif
					err = EAllowableOperation::Disallowed;
					goto Cleanup;
				}
				
#ifdef DEBUG_BUILD
				if(logger)
					logger->Log("\tCRC is 0x%08x\n", dwCRC);
#endif				
				if(dwLengthAccumulated >= 0x7fffff00)
				{
					err = EAllowableOperation::Disallowed;
					goto Cleanup;
				}

				dwLengthAccumulated += dwChunkLength;

				if(!memcmp(&dwChunkSignature, "IEND", 4))
				{
					pStream->ReadByte();

					if(!pStream->Error())
					{
#ifdef DEBUG_BUILD
						if(logger)
							logger->Log("\tData found after end of file\n");
#endif						
						err = EAllowableOperation::Disallowed;
						goto Cleanup;
					}

					if(!bFoundIDAT)
					{
#ifdef DEBUG_BUILD
						if(logger)
							logger->Log("\tNo IDAT header in file\n");
#endif						
						err = EAllowableOperation::Disallowed;
						goto Cleanup;
					}

#ifdef DEBUG_BUILD
					if(logger)
						logger->Log("\tParsed PNG image successfully. No structural errors found.\n");
#endif					
					break;
				}
			}

			if(removeChunks.size())
			{
				// let's rewrite the PNG and remove all the complex headers
				// note: when we ship, we'll just erase from the existing stream

				CCommonTypeStreamWriter writer;

				pStream->Seek(0, ESeekPositions::SK_SET);

#ifdef DEBUG_BUILD
				if(logger)
					logger->Log("\tFound some headers that need removing. Rewriting image\n");
#endif				
				pStream->Read(8, sig, &sigLen);
				writer.WriteBuffer(sig, 8);

				while(1)
				{
					DWORD dwChunkLength = Utils::DwordSwapEndian(pStream->ReadDword());
					DWORD dwChunkSignature = pStream->ReadDword();

					std::vector<DWORD>::iterator it;

					if((it = std::find(removeChunks.begin(), removeChunks.end(), dwChunkSignature)) != removeChunks.end())
					{
						// do nothing: don't write this chunk

#ifdef DEBUG_BUILD
						if(logger)
							logger->Log("\tRemoving header %.*s\n", 4, (char *)&dwChunkSignature);
#endif
						pStream->Seek(dwChunkLength, ESeekPositions::SK_CUR);
						pStream->ReadDword();
					}
					else
					{
						writer.WriteDword(Utils::DwordSwapEndian(dwChunkLength));
						writer.WriteDword(dwChunkSignature);

#ifdef DEBUG_BUILD
						if(logger)
							logger->Log("\tKeeping header %.*s\n", 4, (char *)&dwChunkSignature);
#endif
						BYTE buf[1024];
						size_t bytesToRead = (dwChunkLength > 1024 ? 1024 : dwChunkLength), bufLen = 0;

						while(bytesToRead)
						{
							pStream->Read(bytesToRead, buf, &bufLen);
							writer.WriteBuffer(buf, bufLen);

							dwChunkLength -= bytesToRead;
							bytesToRead = (dwChunkLength > 1024 ? 1024 : dwChunkLength);
						}

						writer.WriteDword(pStream->ReadDword());
					}

					if(!memcmp(&dwChunkSignature, "IEND", 4))
					{
						// done
						break;
					}
				}

				if(writer.Error())
				{
#ifdef DEBUG_BUILD
					if(logger)
						logger->Log("\tSome unknown writer error (alloc failure most likely)\n");
#endif
					err = EAllowableOperation::Disallowed;
					goto Cleanup;
				}

				if(pStream->ReplaceWhole(writer.Buffer(), writer.Length()) == EDataStreamError::StreamFailure)
				{
#ifdef DEBUG_BUILD
					if(logger)
						logger->Log("\tSome unknown stream replacement error (alloc failure most likely)\n");
#endif
					err = EAllowableOperation::Disallowed;
					goto Cleanup;
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
			strUrl += pConn->GetObjectName(); // += /foo/image.png

			pEvent->WriteEvent(
					EEventRepository::ImageValidator,
					EThreatRiskRating::LowRisk,
					L"Malformed PNG File Blocked",
					L"Response filtering policies have blocked a malformed PNG image.\r\n"
					L"The URL of the image in question was: %hs\r\n."
					L"Certain image parsers are vulnerable to code execution vulnerabilities when attempting to handle malformed image data. "
						L"Blocking the image helps to prevent attacks against these parsers from succeeding through the most common vectors (e.g. "
						L"via malicious or compromised web-sites).",
					strUrl.c_str()
				);
		}
	}

	if(pStream)
		pStream->Seek(0, ESeekPositions::SK_SET);

Cleanup_no_rewind:
#ifdef DEBUG_BUILD
	if(logger)
		logger->ReleaseLock();
#endif

	return err;
}

//http://www.fileformat.info/format/jpeg/egff.htm
//http://gvsoft.homedns.org/exif/exif-explanation.html

EAllowableOperation ResponseFilteringPolicies::CheckJpegJFIFValidity(IConnectionPool *pConnPool, IConnection *pConn)
{
	EAllowableOperation err = EAllowableOperation::Allowed;
#ifdef DEBUG_BUILD
	CLogger *logger = NULL;
#endif
	ICommonTypeStreamReader *pStream = NULL;
	PBYTE pbAllocation = NULL;

	try
	{
#ifdef DEBUG_BUILD
		logger = CLogger::GetInstance("bpcore_policy");
		logger->ObtainLock();
		logger->Log("ResponseFilteringPolicies::CheckJpegJFIFValidity\n");
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

			CCommonTypeStreamWriter writer;

			// start of image (SOI) marker

			WORD wSOI = Utils::WordSwapEndian(pStream->ReadWord());

			if(pStream->Error() || wSOI != 0xffd8)
			{
#ifdef DEBUG_BUILD
				if(logger)
					logger->Log("\tNot a JPEG image (no SOI marker)\n");
#endif
				goto Cleanup;
			}

			// write SOI marker
			writer.WriteWord(Utils::WordSwapEndian(0xffd8));

			WORD wAPP0 = Utils::WordSwapEndian(pStream->ReadWord());
			WORD wLength = Utils::WordSwapEndian(pStream->ReadWord());

			if(pStream->Error())
			{
#ifdef DEBUG_BUILD
				if(logger)
					logger->Log("\tNot a JPEG (SOI marker, but no APP0/length)\n");
#endif				
				goto Cleanup;
			}
			
#ifdef DEBUG_BUILD
			if(logger)
				logger->Log("\tJPEG image found\n");
#endif			
			if(wAPP0 == 0xffe0)
			{
				BYTE sig[5];
				size_t sigLen;

				pStream->Read(5, sig, &sigLen);
				if(sigLen != 5 || memcmp(sig, "JFIF", 5))
				{
#ifdef DEBUG_BUILD
					if(logger)
						logger->Log("\tAPP0 header but no JFIF sig. Failing.\n");
#endif					
					err = EAllowableOperation::Disallowed;
					goto Cleanup;
				}

				// if all the above has passed this must be an image.
				// process header (first segment):

				WORD wVersion = Utils::WordSwapEndian(pStream->ReadWord());
				BYTE byteUnits = pStream->ReadByte();

				WORD wXDensity = Utils::WordSwapEndian(pStream->ReadWord());
				WORD wYDensity = Utils::WordSwapEndian(pStream->ReadWord());
			
				BYTE byteXThumbnail = pStream->ReadByte();
				BYTE byteYThumbnail = pStream->ReadByte();
			
				DWORD dwThumbnailLength = 3 * byteXThumbnail * byteYThumbnail;
			
				if(pStream->Error() || wLength != 16 + dwThumbnailLength ||
					(dwThumbnailLength == 0 && (byteXThumbnail || byteYThumbnail)))
				{
#ifdef DEBUG_BUILD
					if(logger)
						logger->Log("\tMalformed JPEG [1]\n");
#endif					
					err = EAllowableOperation::Disallowed;
					goto Cleanup;
				}

				if(wXDensity * wYDensity > (0xffffffff / 3)) // arbitrary limit
				{
#ifdef DEBUG_BUILD
					if(logger)
						logger->Log("\tMalformed JPEG [2]\n");
#endif					
					err = EAllowableOperation::Disallowed;
					goto Cleanup;
				}

				if(wVersion < 0x100 || wVersion > 0x102 || byteUnits > 2)
				{
#ifdef DEBUG_BUILD
					if(logger)
						logger->Log("\tMalformed JPEG [3]\n");
#endif					
					err = EAllowableOperation::Disallowed;
					goto Cleanup;
				}

				pStream->Seek(dwThumbnailLength, ESeekPositions::SK_CUR);
			
				if(pStream->Error())
				{
#ifdef DEBUG_BUILD
					if(logger)
						logger->Log("\tMalformed JPEG [5]\n");
#endif					
					err = EAllowableOperation::Disallowed;
					goto Cleanup;
				}
			
				// JFIF header, minus thumbnail
				
				writer.WriteWord(0xffe0);
				writer.WriteWord(Utils::WordSwapEndian(16));
				writer.WriteString("JFIF", 5);
				writer.WriteWord(Utils::WordSwapEndian(wVersion));
				writer.WriteByte(byteUnits);
				writer.WriteWord(Utils::WordSwapEndian(wXDensity));
				writer.WriteWord(Utils::WordSwapEndian(wYDensity));
				writer.WriteByte(0);
				writer.WriteByte(0);

#ifdef DEBUG_BUILD
				if(logger)
					logger->Log("\tWritten sanitised JFIF header\n");
#endif
				wAPP0 = Utils::WordSwapEndian(pStream->ReadWord());
				wLength = Utils::WordSwapEndian(pStream->ReadWord());

				if(pStream->Error())
				{
					// invalid length
#ifdef DEBUG_BUILD
					if(logger)
						logger->Log("\tRan out of stream data reading next APP0\n");
#endif
					err = EAllowableOperation::Disallowed;
					goto Cleanup;
				}
			}

			if((pbAllocation = (PBYTE)malloc(65536)) == NULL)
			{
				// we can't validate the image due to low mem
				err = EAllowableOperation::Disallowed;
				goto Cleanup;
			}

			memset(pbAllocation, 0, 65536);

			size_t bytesRead;
			BOOL bContinue = TRUE;

			while(bContinue)
			{
#ifdef DEBUG_BUILD
				if(logger)
					logger->Log("\tProcessing header 0x%04x length 0x%04x\n", wAPP0, wLength);
#endif
				if(wLength < 2)
				{
					// must contain at least itself
#ifdef DEBUG_BUILD
					if(logger)
						logger->Log("\tUndersized length in APPx header (<2)\n");
#endif					
					err = EAllowableOperation::Disallowed;
					goto Cleanup;
				}

				BOOL bIsSOS = FALSE;

				switch(wAPP0)
				{
				case 0xffda: // SOS - Start of Scan
					bIsSOS = TRUE;

#ifdef DEBUG_BUILD
					if(logger && wAPP0 == 0xffda)
						logger->Log("\tFound start-of-scan header\n");
#endif
				case 0xffdb: // DQT - Define quantization table

#ifdef DEBUG_BUILD
					if(logger && wAPP0 == 0xffdb)
						logger->Log("\tFound quantization table\n");
#endif
				case 0xffc0: // SOF0 - Baseline
					
#ifdef DEBUG_BUILD
					if(logger && wAPP0 == 0xffc0)
						logger->Log("\tFound baseline header\n");
#endif
				case 0xffc2: // SOF2 - Dimension
					// without this header, http://www.jpeg.org/images/blue_large_06.jpg does not
					// process correctly

#ifdef DEBUG_BUILD
					if(logger && wAPP0 == 0xffc2)
						logger->Log("\tFound dimension header\n");
#endif
				case 0xffdd: // DRI - Dynamic Range Increase
					// without this header, many jpegs do not process correctly

#ifdef DEBUG_BUILD
					if(logger && wAPP0 == 0xffdd)
						logger->Log("\tFound dynamic range increase header\n");
#endif
				case 0xffc4: // DHT - Define Huffman table
					
#ifdef DEBUG_BUILD
					if(logger && wAPP0 == 0xffc4)
						logger->Log("\tFound huffman table\n");
#endif
					writer.WriteWord(Utils::WordSwapEndian(wAPP0));
					writer.WriteWord(Utils::WordSwapEndian(wLength));
					
					pStream->Read(wLength - 2, pbAllocation, &bytesRead);
					if(pStream->Error() || bytesRead != (wLength - 2))
					{
						// invalid length
#ifdef DEBUG_BUILD
						if(logger)
							logger->Log("\tNot enough data free to satify read attempt\n");
#endif
						err = EAllowableOperation::Disallowed;
						goto Cleanup;
					}

#ifdef DEBUG_BUILD
					if(logger)
							logger->Log("\tCopied APPx header data\n");
#endif
					writer.WriteBuffer(pbAllocation, bytesRead);

					if(!bIsSOS)
					{
						break;
					}

					// scan data only

					while(1)
					{
						pStream->Read(65536, pbAllocation, &bytesRead);
						writer.WriteBuffer(pbAllocation, bytesRead);

#ifdef DEBUG_BUILD
						if(logger)
							logger->Log("\tWriting %u bytes of image data\n", bytesRead);
#endif
						if(bytesRead != 65536)
						{
							break;
						}
					}

					pStream->Seek(-2, ESeekPositions::SK_CUR);
					wAPP0 = Utils::WordSwapEndian(pStream->ReadWord());
					
					if(wAPP0 != 0xffd9)
					{
#ifdef DEBUG_BUILD
						if(logger)
							logger->Log("\tReached the end of image data but no EOI marker\n");
#endif
						//err = EAllowableOperation::Disallowed;
						//goto Cleanup;
					}
					
#ifdef DEBUG_BUILD
					if(logger)
						logger->Log("\tFound EOI marker at the end of the image\n");
#endif
					bContinue = FALSE;

					break;
				case 0xffd9:
					// unless this is the last marker in the image, it's likely that this terminated a
					// thumbnail. We'll just skip it by doing nothing.
					
#ifdef DEBUG_BUILD
					if(logger)
							logger->Log("\tFound end of image marker (ignoring)\n");
#endif
					break;
				default:
					// remove any other headers
					
#ifdef DEBUG_BUILD
					if(logger)
							logger->Log("\tFound unnecessary header (removing)\n");
#endif
					pStream->Seek(wLength - 2, ESeekPositions::SK_CUR);
					
					if(pStream->Error())
					{
						// broken file
#ifdef DEBUG_BUILD
						if(logger)
							logger->Log("\tError skipping header (broken file). Failing.\n");
#endif
						err = EAllowableOperation::Disallowed;
						goto Cleanup;
					}

					break;
				}

				wAPP0 = Utils::WordSwapEndian(pStream->ReadWord());
				wLength = Utils::WordSwapEndian(pStream->ReadWord());
			}

			if(writer.Error())
			{
#ifdef DEBUG_BUILD
				if(logger)
					logger->Log("\tSome unknown writer error (alloc failure most likely)\n");
#endif
				err = EAllowableOperation::Disallowed;
				goto Cleanup;
			}

			if(pStream->ReplaceWhole(writer.Buffer(), writer.Length()) == EDataStreamError::StreamFailure)
			{
#ifdef DEBUG_BUILD
				if(logger)
					logger->Log("\tSome unknown stream replacement error (alloc failure most likely)\n");
#endif
				err = EAllowableOperation::Disallowed;
				goto Cleanup;
			}

#ifdef DEBUG_BUILD
			if(logger)
				logger->Log("\tRewritten JPEG successfully!\n");
#endif
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
					EEventRepository::ImageValidator,
					EThreatRiskRating::LowRisk,
					L"Malformed JPEG File Blocked",
					L"Response filtering policies have blocked a malformed JPEG image.\r\n"
					L"The URL of the image in question was: %hs\r\n."
					L"Certain image parsers are vulnerable to code execution vulnerabilities when attempting to handle malformed image data. "
						L"Blocking the image helps to prevent attacks against these parsers from succeeding through the most common vectors (e.g. "
						L"via malicious or compromised web-sites).",
					strUrl.c_str()
				);
		}
	}

	if(pStream)
		pStream->Seek(0, ESeekPositions::SK_SET);

Cleanup_no_rewind:
	if(pbAllocation)
		free(pbAllocation);

#ifdef DEBUG_BUILD
	if(logger)
		logger->ReleaseLock();
#endif
	return err;
}

// http://www.fileformat.info/format/gif/corion.htm
// http://www.fileformat.info/format/gif/egff.htm

EAllowableOperation ResponseFilteringPolicies::CheckGifFileValidity(IConnectionPool *pConnPool, IConnection *pConn)
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
		logger->Log("ResponseFilteringPolicies::CheckGifFileValidity\n");
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

			CCommonTypeStreamWriter writer;

			BYTE sig[16] = {};
			size_t sigLen = 0;

			pStream->Read(6, sig, &sigLen);

			if(sigLen != 6 || (memcmp(sig, "GIF87a", 6) && memcmp(sig, "GIF89a", 6)))
			{
#ifdef DEBUG_BUILD
				if(logger)
					logger->Log("\tNot GIF file\n");
#endif
				goto Cleanup;
			}

#ifdef DEBUG_BUILD
			if(logger)
				logger->Log("\tFound GIF\n");
#endif
			WORD wImageWidth = pStream->ReadWord(),
				 wImageHeight = pStream->ReadWord();
			
			BYTE bitMapped = pStream->ReadByte(),
				 colorIndexSB = pStream->ReadByte(),
				 byteReserved = pStream->ReadByte();
			
			if(pStream->Error() || wImageWidth >= 0x7fff || wImageHeight >= 0x7fff)
			{
				// prevent signedness issues, integer wraps
#ifdef DEBUG_BUILD
				if(logger)
					logger->Log("\tGif too short, width or height too large\n");
#endif
				err = EAllowableOperation::Disallowed;
				goto Cleanup;
			}

			DWORD dwBitsPerPixel = bitMapped & 0x7, // bits 0-2
				  dwBitsOfColorResolution = bitMapped & 0x70; // bits 4-6
			
			BOOL bGlobalColorMapFollows = bitMapped & 0x80; // bit 7

			writer.WriteBuffer(sig, sigLen);
			writer.WriteWord(wImageWidth);
			writer.WriteWord(wImageHeight);
			writer.WriteByte(bitMapped);
			writer.WriteByte(colorIndexSB);
			writer.WriteByte(byteReserved);

			BYTE buf[0x400];
			size_t cbRead = 0;

			if(bGlobalColorMapFollows)
			{
#ifdef DEBUG_BUILD
				if(logger)
					logger->Log("\tGif contains global color map\n");
#endif
				DWORD dwNumColorTableEntries = 1 << (dwBitsPerPixel + 1);
				DWORD dwSizeColorTable = 3 * dwNumColorTableEntries; // each entry has 1 byte RGB

				pStream->Read(dwSizeColorTable, buf, &cbRead);
				
				if(dwSizeColorTable != cbRead)
				{
#ifdef DEBUG_BUILD
					if(logger)
						logger->Log("\tGlobal color table size invalid\n");
#endif
					err = EAllowableOperation::Disallowed;
					goto Cleanup;
				}

				writer.WriteBuffer(buf, cbRead);
			}
			else
			{
#ifdef DEBUG_BUILD
				if(logger)
					logger->Log("\tGif doesn't contain global color map\n");
#endif
			}

			BOOL bParseComplete = FALSE;
			BYTE byteNext, byteExtLabel, byteExtLen, paletteDescription, byteLZWMinCodeSize, byteLZWCodeSize;
			WORD wLeftOffset, wUpperOffset, wWidthOfImage, wHeightOfImage;
			DWORD dwRightmostOffset, dwBottommostOffset;
			BOOL bLocalColorTableFollows, bInterlaceFlag, bSortFlag;
			DWORD dwSizeOfLocalColorTableEntry, dwNumLocalColorTableEntries, dwSizeOfLocalColorTable;

			while(1)
			{
				byteNext = pStream->ReadByte();
				
				switch(byteNext)
				{
				case '!': //  extension header
					
#ifdef DEBUG_BUILD
					if(logger)
						logger->Log("\tExtension header found\n");
#endif
					byteExtLabel = pStream->ReadByte();
					byteExtLen = pStream->ReadByte();

					if(pStream->Error())
					{
#ifdef DEBUG_BUILD
						if(logger)
							logger->Log("\tExtension header incomplete\n");
#endif
						err = EAllowableOperation::Disallowed;
						goto Cleanup;
					}

					switch(byteExtLabel)
					{
					case 0xff: // NETSCAPE2.0 header (essential)
						
						// are there versions other than 2.0? Does it matter?

#ifdef DEBUG_BUILD
						if(logger)
							logger->Log("\tFound ext header 0xff (NETSCAPE string): Writing header\n");
#endif
						writer.WriteByte('!');
						writer.WriteByte(0xff);
						writer.WriteByte(0x0b); // length
						writer.WriteBuffer((PBYTE)"NETSCAPE2.0", 11);
						
						// skip original string
						pStream->Seek(byteExtLen, ESeekPositions::SK_CUR);

						break;
					case 0xf9: // graphics control extension

						// manages time between frames, etc

#ifdef DEBUG_BUILD
						if(logger)
							logger->Log("\tFound ext header 0xf9 (GCE header): Writing header\n");
#endif
						writer.WriteByte('!');
						writer.WriteByte(0xf9);
						
						pStream->Read(4, buf, &cbRead);

						if(cbRead != 4 || byteExtLen != 4)
						{
#ifdef DEBUG_BUILD
							if(logger)
								logger->Log("\tIncomplete/invalid GCE extension header\n");
#endif
							err = EAllowableOperation::Disallowed;
							goto Cleanup;
						}
						
						writer.WriteByte(4);
						writer.WriteBuffer(buf, 4);

						break;
					case 0xfe: // comment header
						
#ifdef DEBUG_BUILD
						if(logger)
							logger->Log("\tFound ext header 0xfe (comment string): Writing header\n");
#endif
						writer.WriteByte('!');
						writer.WriteByte(0xfe);
						writer.WriteByte(0x0f); // length
						writer.WriteBuffer((PBYTE)"Comment Removed", 15);
						
						// skip original string
						pStream->Seek(byteExtLen, ESeekPositions::SK_CUR);

						break;
					default:
#ifdef DEBUG_BUILD
						if(logger)
							logger->Log("\tNot writing unrecognised extension header 0x%x (len: 0x%x)\n", byteExtLabel, byteExtLen);
#endif
						pStream->Seek(byteExtLen, ESeekPositions::SK_CUR);

						break;
					}

					if(pStream->Error())
					{
						err = EAllowableOperation::Disallowed;
						goto Cleanup;
					}

					break;
				case ',': // image descriptor
					
#ifdef DEBUG_BUILD
					if(logger)
						logger->Log("\tImage descriptor found\n");
#endif					
					wLeftOffset = pStream->ReadWord();
					wUpperOffset = pStream->ReadWord();
					wWidthOfImage = pStream->ReadWord();
					wHeightOfImage = pStream->ReadWord();

					paletteDescription = pStream->ReadByte();

					if(pStream->Error())
					{
#ifdef DEBUG_BUILD
						if(logger)
								logger->Log("\tIncomplete image descriptor header\n");
#endif
						err = EAllowableOperation::Disallowed;
						goto Cleanup;
					}

					dwRightmostOffset = wLeftOffset + wWidthOfImage;
					dwBottommostOffset = wUpperOffset + wHeightOfImage;

					if(dwRightmostOffset > wImageWidth || dwBottommostOffset > wImageHeight)
					{
#ifdef DEBUG_BUILD
						if(logger)
							logger->Log("\tInvalid top/left values\n");
#endif
						err = EAllowableOperation::Disallowed;
						goto Cleanup;
					}

					writer.WriteByte(',');
					writer.WriteWord(wLeftOffset);
					writer.WriteWord(wUpperOffset);
					writer.WriteWord(wWidthOfImage);
					writer.WriteWord(wHeightOfImage);
					writer.WriteByte(paletteDescription);

					bLocalColorTableFollows = paletteDescription & 1;
					bInterlaceFlag = paletteDescription & 2;
					bSortFlag = paletteDescription & 4;

					dwSizeOfLocalColorTableEntry = paletteDescription & 0xe0;
					dwNumLocalColorTableEntries = 1 << (dwSizeOfLocalColorTableEntry + 1);
					dwSizeOfLocalColorTable = 3 * dwNumLocalColorTableEntries;

					if(bLocalColorTableFollows)
					{
#ifdef DEBUG_BUILD
						if(logger)
							logger->Log("\tContains local color table\n");
#endif
						pStream->Read(dwSizeOfLocalColorTable, buf, &cbRead);

						if(dwSizeOfLocalColorTable != cbRead)
						{
							err = EAllowableOperation::Disallowed;
							goto Cleanup;
						}

						writer.WriteBuffer(buf, cbRead);
					}
					else
					{
#ifdef DEBUG_BUILD
						if(logger)
							logger->Log("\tDoes not contain local color table\n");
#endif
					}

					// image data follows

					byteLZWMinCodeSize = pStream->ReadByte();
					byteLZWCodeSize = pStream->ReadByte();

#ifdef DEBUG_BUILD
					if(logger)
						logger->Log("\tProcessing image data stream (min code size: %x)\n", byteLZWMinCodeSize);
#endif
					if(pStream->Error())
					{
#ifdef DEBUG_BUILD
						if(logger)
							logger->Log("\tInvalid image data stream\n");
#endif
						err = EAllowableOperation::Disallowed;
						goto Cleanup;
					}

					writer.WriteByte(byteLZWMinCodeSize);

					while(1)
					{
#ifdef DEBUG_BUILD
						if(logger)
							logger->Log("\tProcessing image data streamcode size: %x\n", byteLZWCodeSize);
#endif
						writer.WriteByte(byteLZWCodeSize);

						if(!byteLZWCodeSize)
							break;

						if(0 && byteLZWCodeSize < byteLZWMinCodeSize)
						{
#ifdef DEBUG_BUILD
							if(logger)
								logger->Log("\tImage data buffer smaller than minimum code size\n");
#endif
							err = EAllowableOperation::Disallowed;
							goto Cleanup;
						}

						pStream->Read(byteLZWCodeSize, buf, &cbRead);

						if(cbRead != byteLZWCodeSize)
						{
#ifdef DEBUG_BUILD
							if(logger)
								logger->Log("\tInsufficient image data supplied\n");
#endif
							err = EAllowableOperation::Disallowed;
							goto Cleanup;
						}

						writer.WriteBuffer(buf, cbRead);

						byteLZWCodeSize = pStream->ReadByte();
					}

					break;
				case ';':
#ifdef DEBUG_BUILD
					if(logger)
						logger->Log("\tFound end of GIF image marker\n");
#endif
					writer.WriteByte(';');
					bParseComplete = TRUE;
					break;
				default:
					if(pStream->Error())
					{
#ifdef DEBUG_BUILD
						if(logger)
							logger->Log("\tRan out of data prematurely\n");
#endif
						err = EAllowableOperation::Disallowed;
						goto Cleanup;
					}

					// these bytes do not compose any critical data but for some reason the image
					// often will not render without them.

					writer.WriteByte(byteNext);
				}

				if(bParseComplete)
					break;
			}

			if(writer.Error())
			{
#ifdef DEBUG_BUILD
				if(logger)
					logger->Log("\tSome unknown writer error (alloc failure most likely)\n");
#endif
				err = EAllowableOperation::Disallowed;
				goto Cleanup;
			}

			if(pStream->ReplaceWhole(writer.Buffer(), writer.Length()) == EDataStreamError::StreamFailure)
			{
#ifdef DEBUG_BUILD
				if(logger)
					logger->Log("\tSome unknown stream replacement error (alloc failure most likely)\n");
#endif
				err = EAllowableOperation::Disallowed;
				goto Cleanup;
			}

#ifdef DEBUG_BUILD
			if(logger)
				logger->Log("\tFinished processing GIF: Rewritten successfully!\n");
#endif
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
					EEventRepository::ImageValidator,
					EThreatRiskRating::LowRisk,
					L"Malformed GIF File Blocked",
					L"Response filtering policies have blocked a malformed GIF image.\r\n"
					L"The URL of the image in question was: %hs\r\n."
					L"Certain image parsers are vulnerable to code execution vulnerabilities when attempting to handle malformed image data. "
						L"Blocking the image helps to prevent attacks against these parsers from succeeding through the most common vectors (e.g. "
						L"via malicious or compromised web-sites).",
					strUrl.c_str()
				);
		}
	}

	if(pStream)
		pStream->Seek(0, ESeekPositions::SK_SET);

Cleanup_no_rewind:
#ifdef DEBUG_BUILD
	if(logger)
		logger->ReleaseLock();
#endif

	return err;
}