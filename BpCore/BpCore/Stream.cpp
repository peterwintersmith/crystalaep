// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#include "stdafx.h"
#include "ConnectionPool.h"
#include "Stream.h"

BYTE *CDataStreamBase::Buffer()
{
	PBYTE pbStreamBuffer = NULL;
	
	EnterCriticalSection(&m_csStreamOperation);
	pbStreamBuffer = *m_ppbBuffer;
	LeaveCriticalSection(&m_csStreamOperation);
	
	return pbStreamBuffer;
}
    
size_t CDataStreamBase::Length()
{
	size_t cbStreamBuffer = 0;
	
	EnterCriticalSection(&m_csStreamOperation);
	cbStreamBuffer = *m_pcbBuffer;
	LeaveCriticalSection(&m_csStreamOperation);
	
	return cbStreamBuffer;
}

BOOL CDataStreamBase::StreamBufferReplaced()
{
	return m_bReplacedWhole;
}

EDataStreamError CDataStreamBase::Read(size_t offs, size_t length, PBYTE buf, size_t *bytesRead)
{
	EDataStreamError err = EDataStreamError::StreamFailure;
	PBYTE  pbStreamBuffer = NULL;
	size_t cbStreamBuffer = 0,
			cbBytesToRead = 0,
			cbBytesActRead = 0;
#ifdef DEBUG_BUILD
	CLogger *logger = NULL;
#endif

	EnterCriticalSection(&m_csStreamOperation);

	pbStreamBuffer = *m_ppbBuffer;
	cbStreamBuffer = *m_pcbBuffer;

	try
	{
#ifdef DEBUG_BUILD
		logger = CLogger::GetInstance("bpcore_wininet");
		logger->ObtainLock();
		logger->Log("Stream->Read(offs = %u, length = %u)\n", offs, length);
#endif
	}
	catch(...)
	{
	}

	*bytesRead = 0;

	if(offs + length < offs)
	{
		// security error
		goto Cleanup;
	}

	if(cbStreamBuffer < length + offs)
	{
		cbBytesToRead = (length + offs) - cbStreamBuffer;
			
#ifdef DEBUG_BUILD
		if(logger)
			logger->Log("\tm_pfnReadAdditional(pbStreamBuffer + cbStreamBuffer = %u, cbBytesToRead = %u)\n", cbStreamBuffer, cbBytesToRead);
#endif

		if(m_pfnReadAdditional(m_handle, NULL, cbBytesToRead, &cbBytesActRead) == EStreamReadCallbackError::ReadCallbackFailure)
		{
			goto Cleanup;
		}

		pbStreamBuffer = *m_ppbBuffer;
		cbStreamBuffer = *m_pcbBuffer;

#ifdef DEBUG_BUILD
		if(logger)
			logger->Log("\tpbStreamBuffer = 0x%p; cbStreamBuffer = %u\n", pbStreamBuffer, cbStreamBuffer);
#endif
	}

	// now we should have sufficient data in our buffer

	if(offs <= cbStreamBuffer)
	{
		cbBytesToRead = ((cbStreamBuffer - offs) < length) ? (cbStreamBuffer - offs) : length;

#ifdef DEBUG_BUILD
		if(logger)
			logger->Log("\tCopying out %u bytes\n", cbBytesToRead);
#endif

		memcpy(buf, pbStreamBuffer + offs, cbBytesToRead);
		*bytesRead = cbBytesToRead;
	}
	else
	{
		// that's messed up
		goto Cleanup;
	}

	err = EDataStreamError::StreamSuccess;
Cleanup:
#ifdef DEBUG_BUILD
	if(logger)
		logger->ReleaseLock();
#endif

	LeaveCriticalSection(&m_csStreamOperation);

	return err;
}

EDataStreamError CDataStreamBase::Write(size_t offs, PBYTE buf, size_t bufLen, BOOL insert)
{
	EDataStreamError err = EDataStreamError::StreamFailure;
	PBYTE  pbStreamBuffer = NULL;
	size_t cbStreamBuffer = 0;
#ifdef DEBUG_BUILD
	CLogger *logger = NULL;
#endif

	EnterCriticalSection(&m_csStreamOperation);

	pbStreamBuffer = *m_ppbBuffer;
	cbStreamBuffer = *m_pcbBuffer;

	try
	{
#ifdef DEBUG_BUILD
		logger = CLogger::GetInstance("bpcore_wininet");
		logger->ObtainLock();
		logger->Log("Stream->Write(offs = %u, bufLen = %u, insert = %s)\n", offs, bufLen, insert ? "true" : "false");
#endif
	}
	catch(...)
	{
	}

	size_t valid = bufLen + offs;
	if(valid < bufLen)
	{
		goto Cleanup;
	}

	valid += cbStreamBuffer;
	if(valid < cbStreamBuffer)
	{
		goto Cleanup;
	}

	if(offs > cbStreamBuffer)
	{
		goto Cleanup;
	}

	if(insert)
	{
		if(*m_pcbBufMax < cbStreamBuffer + bufLen)
		{
			size_t cbPow2 = pow((double)2, (int)(log10((double)cbStreamBuffer + bufLen) / log10((double)2)) + 1);
			if(cbPow2 < cbStreamBuffer + bufLen)
			{
				goto Cleanup;
			}

			if(cbPow2 * 2 < cbPow2)
			{
				goto Cleanup;
			}

			size_t cbMax = cbPow2 * 2; // twice the maximum existing buffer size rounded to nearest power of 2

			if((pbStreamBuffer = (PBYTE)realloc(pbStreamBuffer, cbMax)) == NULL)
			{
				goto Cleanup;
			}

			*m_pcbBufMax = cbMax;
		}
		
		//WCHAR wsz[328];
		//wsprintf(wsz, L"*m_pcbBufMax = 0x%x\npbStreamBuffer = 0x%p\ncbStreamBuffer = 0x%x\noffs = 0x%x\nbuf = 0x%p\nbufLen = 0x%x\n\n", *m_pcbBufMax, pbStreamBuffer, cbStreamBuffer, offs, buf, bufLen);
		//OutputDebugString(wsz);

		memmove(pbStreamBuffer + offs + bufLen, pbStreamBuffer + offs, cbStreamBuffer - offs);
		memcpy(pbStreamBuffer + offs, buf, bufLen);
		cbStreamBuffer += bufLen;
	}
	else
	{
		if(cbStreamBuffer - offs < bufLen)
		{
			goto Cleanup;
		}

		memcpy(pbStreamBuffer + offs, buf, bufLen);
	}

	*m_ppbBuffer = pbStreamBuffer;
	*m_pcbBuffer = cbStreamBuffer;

	err = EDataStreamError::StreamSuccess;
Cleanup:
#ifdef DEBUG_BUILD
	if(logger)
		logger->ReleaseLock();
#endif

	LeaveCriticalSection(&m_csStreamOperation);

	return err;
}

EDataStreamError CDataStreamBase::ReplaceWhole(PBYTE buf, size_t bufLen)
{
	EDataStreamError err = EDataStreamError::StreamFailure;
	PBYTE pbBuffer = NULL;

	EnterCriticalSection(&m_csStreamOperation);
	
	free(*m_ppbBuffer);
	*m_pcbBuffer = 0;
	*m_pcbBufMax = 0;

	if((pbBuffer = (PBYTE)malloc(bufLen)) == NULL)
	{
		goto Cleanup;
	}
	
	memcpy(pbBuffer, buf, bufLen);

	*m_ppbBuffer = pbBuffer;
	*m_pcbBuffer = bufLen;
	*m_pcbBufMax = bufLen;

	m_bReplacedWhole = TRUE;

	err = EDataStreamError::StreamSuccess;
Cleanup:
	LeaveCriticalSection(&m_csStreamOperation);

	return err;
}

EDataStreamError CDataStreamBase::Erase(size_t offs, size_t len)
{
	EDataStreamError err = EDataStreamError::StreamFailure;
	PBYTE pbStreamBuffer = NULL;
	size_t cbStreamBuffer = 0;

	EnterCriticalSection(&m_csStreamOperation);

	pbStreamBuffer = *m_ppbBuffer;
	cbStreamBuffer = *m_pcbBuffer;

	if(offs >= cbStreamBuffer || len > cbStreamBuffer || offs + len > cbStreamBuffer)
	{
		goto Cleanup;
	}

	memmove(pbStreamBuffer + offs, pbStreamBuffer + offs + len, cbStreamBuffer - offs - len);

	pbStreamBuffer = (PBYTE)realloc(pbStreamBuffer, cbStreamBuffer - len);
	if(!pbStreamBuffer)
	{
		goto Cleanup;
	}

	cbStreamBuffer -= len;

	*m_ppbBuffer = pbStreamBuffer;
	*m_pcbBuffer = cbStreamBuffer;
	*m_pcbBufMax = cbStreamBuffer;

	err = EDataStreamError::StreamSuccess;
Cleanup:
	LeaveCriticalSection(&m_csStreamOperation);

	return err;
}

size_t CCommonTypeStreamReader::Offset()
{
	return m_offset;
}

BYTE CCommonTypeStreamReader::ReadByte()
{
	BYTE ret[1] = {0};
	size_t cbRead = 0;

	CDataStreamBase::Read(m_offset, 1, ret, &cbRead);
	if(cbRead != 1)
	{
		m_bError = TRUE;
		return 0;
	}

	m_offset++;

	return ret[0];
}

WORD CCommonTypeStreamReader::ReadWord()
{
	WORD ret[1] = {0};
	size_t cbRead = 0;

	CDataStreamBase::Read(m_offset, 2, (PBYTE)ret, &cbRead);
	if(cbRead != 2)
	{
		m_bError = TRUE;
		return 0;
	}

	m_offset += 2;

	return ret[0];
}

DWORD CCommonTypeStreamReader::ReadDword()
{
	DWORD ret[1] = {0};
	size_t cbRead = 0;

	CDataStreamBase::Read(m_offset, 4, (PBYTE)ret, &cbRead);
	if(cbRead != 4)
	{
		m_bError = TRUE;
		return 0;
	}

	m_offset += 4;

	return ret[0];
}

QWORD CCommonTypeStreamReader::ReadQword()
{
	QWORD ret[1] = {0};
	size_t cbRead = 0;

	CDataStreamBase::Read(m_offset, 8, (PBYTE)ret, &cbRead);
	if(cbRead != 8)
	{
		m_bError = TRUE;
		return 0;
	}

	m_offset += 8;

	return ret[0];
}

float CCommonTypeStreamReader::ReadFloat()
{
	float ret[1] = {0};
	size_t cbRead = 0;

	CDataStreamBase::Read(m_offset, sizeof(float), (PBYTE)ret, &cbRead);
	if(cbRead != sizeof(float))
	{
		m_bError = TRUE;
		return 0.0;
	}

	m_offset += sizeof(float);

	return ret[0];
}


double CCommonTypeStreamReader::ReadDouble()
{
	double ret[1] = {0};
	size_t cbRead = 0;

	CDataStreamBase::Read(m_offset, sizeof(double), (PBYTE)ret, &cbRead);
	if(cbRead != sizeof(double))
	{
		m_bError = TRUE;
		return 0.0;
	}

	m_offset += sizeof(double);

	return ret[0];
}

char *CCommonTypeStreamReader::ReadString(long cch)
{
	char *pszValue = NULL, ch[1] = {0};
	size_t cchValue = 0, cchMax = 0, cbRead = 0;

	if(cch == 0)
	{
		// consistent with the case where cch == -1 and byte read is zero
		return strdup("");
	}

	while(1)
	{
		CDataStreamBase::Read(m_offset, 1, (PBYTE)ch, &cbRead);
		if(cbRead != 1)
		{
			m_bError = TRUE;
			break;
		}

		if(cchValue == cchMax)
		{
			cchMax += 512; // make define

			if((pszValue = (char *)realloc(pszValue, cchMax)) == NULL)
			{
				m_bError = TRUE;
				return NULL;
			}
		}

		pszValue[cchValue] = ch[0];
		cchValue++;

		if(cchValue == cch || ((cch == -1) ? ch[0] == '\0' : 0))
			break;
	}
	
	m_offset += cchValue;

	return pszValue;
}

wchar_t *CCommonTypeStreamReader::ReadWString(long cch)
{
	wchar_t *pwszValue = NULL, wch[1] = {0};
	size_t cchValue = 0, cchMax = 0, cbRead = 0;

	if(cch == 0)
	{
		// consistent with the case where cch == -1 and byte read is zero
		return wcsdup(L"");
	}

	while(1)
	{
		CDataStreamBase::Read(m_offset, 1, (PBYTE)wch, &cbRead);
		if(cbRead != 1)
		{
			m_bError = TRUE;
			break;
		}

		if(cchValue == cchMax)
		{
			cchMax += 512; // make define

			if((pwszValue = (wchar_t *)realloc(pwszValue, cchMax * sizeof(wchar_t))) == NULL)
			{
				m_bError = TRUE;
				return NULL;
			}
		}

		pwszValue[cchValue] = wch[0];
		cchValue++;

		if(cchValue == cch || ((cch == -1) ? wch[0] == '\0' : 0))
			break;
	}
	
	m_offset += cchValue;

	return pwszValue;
}

EDataStreamError CCommonTypeStreamReader::Seek(long offs, ESeekPositions from)
{
#define SINGLE_SEEK_READ_SIZE	1024

	EDataStreamError err = EDataStreamError::StreamFailure;
	size_t cbTotalRead = 0, cbRead = 0;
	BYTE buf[SINGLE_SEEK_READ_SIZE];

	switch(from)
	{
	case ESeekPositions::SK_SET:
		m_offset = offs;
		
		break;
	case ESeekPositions::SK_CUR:
		m_offset += offs;
		
		break;
	case ESeekPositions::SK_END:
		
		if(offs > 0)
		{
			m_bError = TRUE;
			goto Cleanup;
		}

		while(!cbTotalRead || (cbRead == SINGLE_SEEK_READ_SIZE))
		{
			if(CDataStreamBase::Read(cbTotalRead, SINGLE_SEEK_READ_SIZE, buf, &cbRead) == EDataStreamError::StreamFailure)
			{
				m_bError = TRUE;
				goto Cleanup;
			}

			cbTotalRead += cbRead;
		}

		m_offset = (long)cbTotalRead + offs;

		break;
	}

	err = EDataStreamError::StreamSuccess;
Cleanup:
	return err;
}

EDataStreamError CCommonTypeStreamReader::Read(size_t length, PBYTE buf, size_t *bytesRead)
{
	if(CDataStreamBase::Read(m_offset, length, buf, bytesRead) == EDataStreamError::StreamFailure)
	{
		return EDataStreamError::StreamFailure;
	}

	if(*bytesRead != length)
	{
		m_bError = TRUE;
	}

	m_offset += *bytesRead;

	return EDataStreamError::StreamSuccess;
}

BOOL CCommonTypeStreamReader::Error()
{
	return m_bError;
}