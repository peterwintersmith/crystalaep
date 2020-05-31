// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#pragma once

// IConfigReader Interface Class

struct IConfigReader;

#ifdef CRYSTAL_LATE_BINDING
#ifdef DEBUG_BUILD
extern "C" static IConfigReader *(*GetConfigReader)() = (IConfigReader *(*)())GetProcAddress(
		GetModuleHandle(L"C:\\Work\\crystal\\BP\\Impl\\AuxCore\\Debug\\auxcore.dll") ?
			GetModuleHandle(L"C:\\Work\\crystal\\BP\\Impl\\AuxCore\\Debug\\auxcore.dll") :
			LoadLibrary(L"C:\\Work\\crystal\\BP\\Impl\\AuxCore\\Debug\\auxcore.dll"), 
		"GetConfigReader"
	);
#else
extern "C" static IConfigReader *(*GetConfigReader)() = (IConfigReader *(*)())GetProcAddress(
		GetModuleHandle(L"auxcore.dll") ?
			GetModuleHandle(L"auxcore.dll") :
			LoadLibrary(L"auxcore.dll"), 
		"GetConfigReader"
	);
#endif
#else
extern "C" __declspec(dllimport) IConfigReader *GetConfigReader();
#endif

struct IConfigReader {
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