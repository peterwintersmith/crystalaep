// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#pragma once

struct CDataStreamBaseBase
{
	virtual EDataStreamError Read(size_t offs, size_t length, PBYTE buf, size_t *bytesRead) = 0;
    virtual EDataStreamError Write(size_t offs, PBYTE buf, size_t bufLen, BOOL insert) = 0;
    virtual EDataStreamError ReplaceWhole(PBYTE buf, size_t bufLen) = 0;
	virtual EDataStreamError Erase(size_t offs, size_t len) = 0;
	
    virtual BYTE *Buffer() = 0;
	virtual size_t Length() = 0;
	virtual BOOL StreamBufferReplaced() = 0;
};

struct CCommonTypeStreamReaderBase //: public CDataStreamBaseBase
{
	virtual EDataStreamError Read(size_t offs, size_t length, PBYTE buf, size_t *bytesRead) = 0;
    virtual EDataStreamError Write(size_t offs, PBYTE buf, size_t bufLen, BOOL insert) = 0;
    virtual EDataStreamError ReplaceWhole(PBYTE buf, size_t bufLen) = 0;
	virtual EDataStreamError Erase(size_t offs, size_t len) = 0;
	
    virtual BYTE *Buffer() = 0;
	virtual size_t Length() = 0;
	virtual BOOL StreamBufferReplaced() = 0;

	virtual BYTE ReadByte() = 0;
    virtual WORD ReadWord() = 0; // endian option needed
    virtual DWORD ReadDword() = 0;
    virtual QWORD ReadQword() = 0;
    virtual float ReadFloat() = 0;
    virtual double ReadDouble() = 0;
    virtual char *ReadString(long cch = -1) = 0;
    virtual wchar_t *ReadWString(long cch = -1) = 0;
    virtual EDataStreamError Seek(long offs, ESeekPositions from) = 0;
	virtual EDataStreamError Read(size_t length, PBYTE buf, size_t *bytesRead) = 0;

	virtual size_t Offset() = 0;
	virtual BOOL Error() = 0;
};

struct CCommonTypeStreamWriterBase
{
	virtual void WriteBuffer(PBYTE val, size_t len) = 0;
	virtual void WriteByte(BYTE val) = 0;
    virtual void WriteWord(WORD val) = 0;
    virtual void WriteDword(DWORD val) = 0;
    virtual void WriteQword(QWORD val) = 0;
    virtual void WriteFloat(float val) = 0;
    virtual void WriteDouble(double val) = 0;
    virtual void WriteString(char *val, long cch = -1) = 0;
    virtual void WriteWString(wchar_t *val, long cch = -1) = 0;

	virtual PBYTE Buffer() = 0;
	virtual size_t Length() = 0;
	virtual BOOL Error() = 0;
};