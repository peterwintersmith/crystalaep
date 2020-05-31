// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#pragma once

enum EDataStreamError {
	StreamFailure,
	StreamSuccess
};

enum EStreamReadCallbackError {
	ReadCallbackFailure,
	ReadCallbackSuccess
};

typedef EStreamReadCallbackError (*PFN_STRMREAD_CALLBACK)(void *pStateParam, PBYTE pbOutputBuffer, size_t cbReadSize, size_t *pcbBytesRead);
enum ESeekPositions { SK_SET = 0, SK_CUR, SK_END };

#include "StreamBase.h"

class CDataStreamBase : public CCommonTypeStreamReaderBase {
public:
	CDataStreamBase(PBYTE *ppbBuffer, size_t *pcbBuffer, size_t *pcbBufMax, void *handle = NULL, PFN_STRMREAD_CALLBACK pfn = NULL) :
		m_ppbBuffer(ppbBuffer), m_pcbBuffer(pcbBuffer), m_pcbBufMax(pcbBufMax), m_handle(handle),
		m_pfnReadAdditional(pfn), m_bReplacedWhole(FALSE)
	{
		InitializeCriticalSection(&m_csStreamOperation);
	}
	
	virtual ~CDataStreamBase() { }
    
	EDataStreamError Read(size_t offs, size_t length, PBYTE buf, size_t *bytesRead);
    EDataStreamError Write(size_t offs, PBYTE buf, size_t bufLen, BOOL insert);
    EDataStreamError ReplaceWhole(PBYTE buf, size_t bufLen);
	EDataStreamError Erase(size_t offs, size_t len);
	
    BYTE *Buffer();
	size_t Length();
	BOOL StreamBufferReplaced();
	
private:
	CDataStreamBase(){}

	PBYTE *m_ppbBuffer;
	size_t *m_pcbBuffer, *m_pcbBufMax;
	PFN_STRMREAD_CALLBACK m_pfnReadAdditional;
	void *m_handle;
	BOOL m_bReplacedWhole;

	CRITICAL_SECTION m_csStreamOperation;
};

class CCommonTypeStreamReader : public CDataStreamBase {
public:
	CCommonTypeStreamReader(PBYTE *ppbBuffer, size_t *pcbBuffer, size_t *pcbBufMax, void *handle = NULL, PFN_STRMREAD_CALLBACK pfn = NULL) :
		m_bError(FALSE), m_offset(0), CDataStreamBase(ppbBuffer, pcbBuffer, pcbBufMax, handle, pfn)
	{
	}
	
    BYTE ReadByte();
    WORD ReadWord(); // endian option needed
    DWORD ReadDword();
    QWORD ReadQword();
    float ReadFloat();
    double ReadDouble();
    char *ReadString(long cch = -1);
    wchar_t *ReadWString(long cch = -1);
    EDataStreamError Seek(long offs, ESeekPositions from);
	EDataStreamError Read(size_t length, PBYTE buf, size_t *bytesRead);

	size_t Offset();
	BOOL Error();

private:
	BOOL m_bError;
	size_t m_offset;
};