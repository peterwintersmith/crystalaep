// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#include "stdafx.h"
#include "AuxCore.h"
#include "ConfigReader.h"

CRITICAL_SECTION CConfigReader::m_csCreateInst = {0}, CConfigReader::m_csReloadConfig = {0};
CConfigReader *CConfigReader::m_Instance = NULL;

CConfigReader* CConfigReader::GetInstance()
{
	if(m_Instance == NULL)
	{
		EnterCriticalSection(&m_csCreateInst);

		if(m_Instance == NULL)
		{
			m_Instance = new CConfigReader();
		}

		LeaveCriticalSection(&m_csCreateInst);
	}

	return m_Instance;
}

void CConfigReader::Subscribe(FN_CONFIGREFRESH_CALLBACK pfnConfigRefresh)
{
	EnterCriticalSection(&m_csReloadConfig);

	std::vector<FN_CONFIGREFRESH_CALLBACK>::iterator it = std::find(
			m_vecRefreshSubscribers.begin(),
			m_vecRefreshSubscribers.end(),
			pfnConfigRefresh
		);

	if(it == m_vecRefreshSubscribers.end())
	{
		m_vecRefreshSubscribers.push_back(pfnConfigRefresh);
	}

	LeaveCriticalSection(&m_csReloadConfig);
}

void CConfigReader::UnSubscribe(FN_CONFIGREFRESH_CALLBACK pfnConfigRefresh)
{
	EnterCriticalSection(&m_csReloadConfig);

	std::vector<FN_CONFIGREFRESH_CALLBACK>::iterator it = std::find(
			m_vecRefreshSubscribers.begin(),
			m_vecRefreshSubscribers.end(),
			pfnConfigRefresh
		);

	if(it != m_vecRefreshSubscribers.end())
	{
		m_vecRefreshSubscribers.erase(it);
	}

	LeaveCriticalSection(&m_csReloadConfig);
}

BOOL CConfigReader::LoadFromXmlFile(WCHAR *pwszXmlFile)
{
	BOOL bSuccess = FALSE;
	IStream *pFileStream = NULL;
	IXmlReader *pXmlReader = NULL;
	std::wstring wstrCategory = L"";
	std::vector<FN_CONFIGREFRESH_CALLBACK>::const_iterator it;

	EnterCriticalSection(&m_csReloadConfig);

	DeleteExistingConfig();

	if(FAILED(SHCreateStreamOnFile(pwszXmlFile, STGM_READ, &pFileStream)))
		goto Cleanup;

	if(FAILED(CreateXmlReader(__uuidof(IXmlReader), (void **)&pXmlReader, NULL)))
		goto Cleanup;

	if(FAILED(pXmlReader->SetProperty(XmlReaderProperty_DtdProcessing, DtdProcessing_Prohibit)))
		goto Cleanup;

	if(FAILED(pXmlReader->SetInput(pFileStream)))
		goto Cleanup;

	XmlNodeType nodeType;
		
	LPCWSTR pwszElement, pwszName, pwszValue;
	BOOL bFirstElement = TRUE;
	
	while(pXmlReader->Read(&nodeType) == S_OK)
	{
		switch(nodeType)
		{
		case XmlNodeType_XmlDeclaration:
			// do nothing
			break;
		case XmlNodeType_Element:

			if(FAILED(pXmlReader->GetLocalName(&pwszElement, NULL)))
				goto Cleanup;

			if(!wcsicmp(pwszElement, L"configuration"))
			{
				if(!bFirstElement)
					goto Cleanup;

				bFirstElement = FALSE;
			}
			else
			{
				if(bFirstElement)
					goto Cleanup;

				if(!wcsicmp(pwszElement, L"property"))
				{
					if(FAILED(pXmlReader->MoveToFirstAttribute()))
						goto Cleanup;

					NameValuePair *nvp = new NameValuePair();
					BOOL bPair = FALSE;

					while(1)
					{
						if(!pXmlReader->IsDefault())
						{
							if(FAILED(pXmlReader->GetLocalName(&pwszName, NULL)))
								goto Cleanup;

							if(FAILED(pXmlReader->GetValue(&pwszValue, NULL)))
								goto Cleanup;

							if(!wcsicmp(pwszName, L"name"))
							{
								nvp->wstrName = pwszValue;
							}
							else if(!wcsicmp(pwszName, L"value"))
							{
								nvp->wstrValue = pwszValue;
								bPair = TRUE;
							}

							if(!nvp->wstrName.empty() && bPair == TRUE)
							{
								m_mapConfig[wstrCategory].push_back(nvp);
								nvp = NULL;
							}
						}

						if(pXmlReader->MoveToNextAttribute() != S_OK)
							break;
					}

					if(nvp)
						goto Cleanup;
				}
				else
				{
					wstrCategory = pwszElement;
					goto move_next_element;
				}
			}
				
move_next_element:
			if(FAILED(pXmlReader->MoveToElement()))
				goto Cleanup;

			break;
		case XmlNodeType_EndElement:
			break;
		case XmlNodeType_Text:
			break;
		case XmlNodeType_Whitespace:
			break;
		case XmlNodeType_CDATA:
			break;
		case XmlNodeType_ProcessingInstruction:
			break;
		case XmlNodeType_Comment:
			break;
		case XmlNodeType_DocumentType:
			break;
		}
	}

	// Reload() needs to be called to trigger config update in subscribers

	bSuccess = TRUE;
Cleanup:
	if(pXmlReader)
		pXmlReader->Release();

	if(pFileStream)
		pFileStream->Release();

	LeaveCriticalSection(&m_csReloadConfig);

	return bSuccess;
}

WCHAR *CConfigReader::GetConfigElement(WCHAR *pwszNamespace, WCHAR *pwszPropertyName)
{
	WCHAR *pwszPropertyValue = NULL;
	std::list<NameValuePair *>::const_iterator it;

	EnterCriticalSection(&m_csReloadConfig);

	try
	{
		for(it = m_mapConfig[pwszNamespace].begin(); it != m_mapConfig[pwszNamespace].end(); it++)
		{
			if(!wcsicmp((*it)->wstrName.c_str(), pwszPropertyName))
			{
				pwszPropertyValue = const_cast<WCHAR *>((*it)->wstrValue.c_str());
				break;
			}
		}
	}
	catch(...)
	{
	}

	LeaveCriticalSection(&m_csReloadConfig);

	return pwszPropertyValue;
}

void CConfigReader::DeleteExistingConfig()
{
	std::map<std::wstring, std::list<CConfigReader::NameValuePair *>>::const_iterator it;

	if(!m_mapConfig.size())
		return;

	for(it = m_mapConfig.begin(); it != m_mapConfig.end(); it++)
	{
		std::list<CConfigReader::NameValuePair *>::const_iterator it2;

		for(it2 = (*it).second.begin(); it2 != (*it).second.end(); it2++)
		{
			// delete NameValuePair
			if(!(*it2))
				continue;

			delete (*it2);
		}
	}

	m_mapConfig.clear();
}

std::map<std::wstring, std::list<CConfigReader::NameValuePair *>> *CConfigReader::GetConfigRepository()
{
	return &m_mapConfig;
}

void CConfigReader::Reload()
{
	std::vector<FN_CONFIGREFRESH_CALLBACK>::iterator it;

	EnterCriticalSection(&m_csReloadConfig);

	for(it = m_vecRefreshSubscribers.begin(); it != m_vecRefreshSubscribers.end(); it++)
	{
		// prompt subscribers to reload their configuration
		(*it)(TRUE);
	}

	LeaveCriticalSection(&m_csReloadConfig);
}

extern "C" __declspec(dllexport) CConfigReaderBase *GetConfigReader()
{
	return reinterpret_cast<CConfigReaderBase *>(CConfigReader::GetInstance());
}