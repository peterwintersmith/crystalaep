// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#pragma once

#include "stdafx.h"

class CParseTemporaryStorage {
public:
	~CParseTemporaryStorage()
	{
		std::hash_map<std::wstring, StoredItem>::iterator it;

		for(it = m_hmapStorage.begin(); it != m_hmapStorage.end(); it++)
		{
			if(it->second.bRequiresFree)
			{
				if(it->second.Item)
				{
					free((void *)it->second.Item);
				}
			}
		}
	}

	template<class T>
	void SetNamedValue(WCHAR *pwszName, BOOL bRequiresFree, T value)
	{
		m_hmapStorage[pwszName].bRequiresFree = bRequiresFree;
		m_hmapStorage[pwszName].Item = (size_t)value;
	}
	
	template<class T>
	T GetNamedValue(WCHAR *pwszName)
	{
		if(m_hmapStorage.find(pwszName) != m_hmapStorage.end())
		{
			return (T)m_hmapStorage[pwszName].Item;
		}

		return (T)0;
	}

private:
	typedef struct {
		BOOL bRequiresFree;
		size_t Item;
	} StoredItem;

	std::hash_map<std::wstring, StoredItem> m_hmapStorage;
};

#include "ParseTempStgStaticBase.h"

class CParseTemporaryStorageStatic : public CParseTemporaryStorageStaticBase {
public:
	void SetNamedValue_BOOL(WCHAR *pwszName, BOOL bRequiresFree, BOOL value)
	{
		m_stg.SetNamedValue<BOOL>(pwszName, FALSE, value);
	}

	BOOL GetNamedValue_BOOL(WCHAR *pwszName)
	{
		return m_stg.GetNamedValue<BOOL>(pwszName);
	}

	void SetNamedValue_BYTE(WCHAR *pwszName, BOOL bRequiresFree, BYTE value)
	{
		m_stg.SetNamedValue<BYTE>(pwszName, FALSE, value);
	}

	BYTE GetNamedValue_BYTE(WCHAR *pwszName)
	{
		return m_stg.GetNamedValue<BYTE>(pwszName);
	}

	void SetNamedValue_DWORD(WCHAR *pwszName, BOOL bRequiresFree, DWORD value)
	{
		m_stg.SetNamedValue<DWORD>(pwszName, FALSE, value);
	}

	DWORD GetNamedValue_DWORD(WCHAR *pwszName)
	{
		return m_stg.GetNamedValue<DWORD>(pwszName);
	}

	void SetNamedValue_SIZE_T(WCHAR *pwszName, BOOL bRequiresFree, SIZE_T value)
	{
		m_stg.SetNamedValue<SIZE_T>(pwszName, FALSE, value);
	}

	SIZE_T GetNamedValue_SIZE_T(WCHAR *pwszName)
	{
		return m_stg.GetNamedValue<SIZE_T>(pwszName);
	}

private:
	CParseTemporaryStorage m_stg;
};