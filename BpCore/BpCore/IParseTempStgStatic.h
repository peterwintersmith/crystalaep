// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#pragma once

struct IParseTemporaryStorageStatic {
	virtual void SetNamedValue_BOOL(WCHAR *pwszName, BOOL bRequiresFree, BOOL value) = 0;
	virtual BOOL GetNamedValue_BOOL(WCHAR *pwszName) = 0;
	
	virtual void SetNamedValue_BYTE(WCHAR *pwszName, BOOL bRequiresFree, BYTE value) = 0;
	virtual BYTE GetNamedValue_BYTE(WCHAR *pwszName) = 0;

	virtual void SetNamedValue_DWORD(WCHAR *pwszName, BOOL bRequiresFree, DWORD value) = 0;
	virtual DWORD GetNamedValue_DWORD(WCHAR *pwszName) = 0;
	
	virtual void SetNamedValue_SIZE_T(WCHAR *pwszName, BOOL bRequiresFree, SIZE_T value) = 0;
	virtual SIZE_T GetNamedValue_SIZE_T(WCHAR *pwszName) = 0;
};