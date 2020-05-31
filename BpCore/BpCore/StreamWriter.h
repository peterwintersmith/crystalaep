// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#pragma once

// implementation of this writer is *really* awful. Should maintain a growing buffer, a max and an offset,
// and grow as required. Saves on allocs a hundred times over.

class CCommonTypeStreamWriter {
public:
	CCommonTypeStreamWriter() : m_pbBuffer(0), m_cbBuffer(0), m_offset(0), m_bError(FALSE)
	{
	}

	/*
	CCommonTypeStreamWriter(CDataStreamBase *p)
	{
		throw new std::exception("CCommonTypeStreamWriter and CDataStreamBase are not compatible types");
	}

	CCommonTypeStreamWriter(CDataStreamBase& p)
	{
		throw new std::exception("CCommonTypeStreamWriter and CDataStreamBase are not compatible types");
	}
	*/

	~CCommonTypeStreamWriter()
	{
		if(m_pbBuffer)
		{
			free(m_pbBuffer);
		}
	}

	void WriteBuffer(PBYTE val, size_t len);
	void WriteByte(BYTE val);
    void WriteWord(WORD val);
    void WriteDword(DWORD val);
    void WriteQword(QWORD val);
    void WriteFloat(float val);
    void WriteDouble(double val);
    void WriteString(char *val, long cch = -1);
    void WriteWString(wchar_t *val, long cch = -1);

	PBYTE Buffer();
	size_t Length();
	BOOL Error();

private:
	PBYTE m_pbBuffer;
	size_t m_cbBuffer, m_offset;
	BOOL m_bError;
};