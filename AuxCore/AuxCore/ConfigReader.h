// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#pragma once

#include "ConfigReaderBase.h"

class CConfigReader : public CConfigReaderBase {
	friend BOOL AuxCore::Initialize();
public:
	static CConfigReader* GetInstance();
	
	typedef void (*FN_CONFIGREFRESH_CALLBACK)(BOOL bRefresh);
	
	void Subscribe(FN_CONFIGREFRESH_CALLBACK pfnConfigRefresh);
	void UnSubscribe(FN_CONFIGREFRESH_CALLBACK pfnConfigRefresh);

	BOOL LoadFromXmlFile(WCHAR *pwszXmlFile);
	WCHAR *GetConfigElement(WCHAR *pwszNamespace, WCHAR *pwszPropertyName);
	std::map<std::wstring, std::list<NameValuePair *>> *GetConfigRepository();
	void Reload();
	
private:
	CConfigReader()
	{
	}

	void DeleteExistingConfig();

	static CConfigReader *m_Instance;
	static CRITICAL_SECTION m_csCreateInst, m_csReloadConfig;

	std::vector<FN_CONFIGREFRESH_CALLBACK> m_vecRefreshSubscribers;

	std::map<std::wstring, std::list<NameValuePair *>> m_mapConfig;
};