// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#pragma once

class CDynamicConfig {
	friend BOOL BpCore::Initialize();
public:
	static CDynamicConfig* GetInstance();

	enum EDynamicRepository {
		AntiSpray,
		ApiFirewall,
		AudioValidator,
		ConnectionMonitor,
		DfnsInDepth,
		HeapMonitor,
		ImageValidator,
		StackMonitor,
		VideoValidator,
		WebAppMonitor
	};

	template<class T>
	void SetProperty(EDynamicRepository repos, std::wstring wstrName, T value)
	{
		std::map<std::wstring, size_t>::iterator it;
		std::wstring wstrNameLower = Utils::wstringlower(wstrName);

		EnterCriticalSection(&m_csPropertyOp);
	
		it = m_vecRepository[repos].find(wstrNameLower);
		if(it != m_vecRepository[repos].end())
		{
			m_vecRepository[repos].erase(wstrNameLower);
		}

		m_vecRepository[repos][wstrNameLower] = (size_t)value;

		LeaveCriticalSection(&m_csPropertyOp);
	}

	template<class T>
	T GetProperty(EDynamicRepository repos, std::wstring wstrName)
	{
		std::map<std::wstring, size_t>::iterator it;
		std::wstring wstrNameLower = Utils::wstringlower(wstrName);
		T value = 0;

		EnterCriticalSection(&m_csPropertyOp);
	
		it = m_vecRepository[repos].find(wstrNameLower);
		if(it != m_vecRepository[repos].end())
		{
			value = (T)m_vecRepository[repos][wstrNameLower];
		}

		LeaveCriticalSection(&m_csPropertyOp);

		return value;
	}

	void Remove(EDynamicRepository repos, std::wstring wstrName)
	{
		std::map<std::wstring, size_t>::iterator it;
		std::wstring wstrNameLower = Utils::wstringlower(wstrName);

		EnterCriticalSection(&m_csPropertyOp);
	
		it = m_vecRepository[repos].find(wstrNameLower);
		if(it != m_vecRepository[repos].end())
		{
			m_vecRepository[repos].erase(wstrNameLower);
		}

		LeaveCriticalSection(&m_csPropertyOp);
	}

private:
	CDynamicConfig()
	{
		InitializeCriticalSection(&m_csPropertyOp);
		m_vecRepository = std::vector<std::map<std::wstring, size_t>>(20);
	}

	static CDynamicConfig *m_Instance;
	static CRITICAL_SECTION m_csCreateInst, m_csPropertyOp;

	std::vector<std::map<std::wstring, size_t>> m_vecRepository;
};