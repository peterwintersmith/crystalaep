// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#include "stdafx.h"
#include "StreamWriter.h"

void CCommonTypeStreamWriter::WriteBuffer(PBYTE val, size_t len)
{
	if(m_cbBuffer + len < m_cbBuffer)
	{
		m_bError = TRUE;
		m_cbBuffer = 0;
	}

	if((m_pbBuffer = (PBYTE)realloc(m_pbBuffer, m_cbBuffer + len)) == NULL)
	{
		m_bError = TRUE;
		// should I just return the data in the buffer?
		m_cbBuffer = 0;
	}

	memcpy(m_pbBuffer + m_cbBuffer, val, len);
	m_cbBuffer += len;
}

void CCommonTypeStreamWriter::WriteByte(BYTE val)
{
	WriteBuffer(&val, 1);
}

void CCommonTypeStreamWriter::WriteWord(WORD val)
{
	WriteBuffer((PBYTE)&val, 2);
}

void CCommonTypeStreamWriter::WriteDword(DWORD val)
{
	WriteBuffer((PBYTE)&val, 4);
}

void CCommonTypeStreamWriter::WriteQword(QWORD val)
{
	WriteBuffer((PBYTE)&val, 8);
}

void CCommonTypeStreamWriter::WriteFloat(float val)
{
	WriteBuffer((PBYTE)&val, sizeof(float));
}

void CCommonTypeStreamWriter::WriteDouble(double val)
{
	WriteBuffer((PBYTE)&val, sizeof(double));
}

void CCommonTypeStreamWriter::WriteString(char *val, long cch)
{
	WriteBuffer((PBYTE)val, cch == -1 ? strlen(val) : cch);
}

void CCommonTypeStreamWriter::WriteWString(wchar_t *val, long cch)
{
	WriteBuffer((PBYTE)val, cch == -1 ? (wcslen(val) * sizeof(wchar_t)) : cch);
}

PBYTE CCommonTypeStreamWriter::Buffer()
{
	return m_pbBuffer;
}

size_t CCommonTypeStreamWriter::Length()
{
	return m_cbBuffer;
}

BOOL CCommonTypeStreamWriter::Error()
{
	return m_bError;
}