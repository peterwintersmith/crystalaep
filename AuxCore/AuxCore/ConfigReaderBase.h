// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#pragma once

// CConfigReader virtual base class (identical to interface)

struct CConfigReaderBase {
	typedef struct _NameValuePair
	{
		std::wstring wstrName;
		std::wstring wstrValue;

	} NameValuePair;

	typedef void (*FN_CONFIGREFRESH_CALLBACK)(BOOL bRefresh);
	
	virtual void Subscribe(FN_CONFIGREFRESH_CALLBACK pfnConfigRefresh) = 0;
	virtual void UnSubscribe(FN_CONFIGREFRESH_CALLBACK pfnConfigRefresh) = 0;

	virtual BOOL LoadFromXmlFile(WCHAR *pwszXmlFile) = 0;
	virtual WCHAR *GetConfigElement(WCHAR *pwszNamespace, WCHAR *pwszPropertyName) = 0;
	virtual std::map<std::wstring, std::list<NameValuePair *>> *GetConfigRepository() = 0;
	virtual void Reload() = 0;
};