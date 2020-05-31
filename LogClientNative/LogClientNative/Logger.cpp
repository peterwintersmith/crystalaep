// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#include "stdafx.h"
#include "Logger.h"

int CLogger::m_nInstances = 0;
CLogger::LoggerInstance CLogger::m_Instances[LOGGER_MAX_INSTANCES];

CLogger *CLogger::GetInstance(char *pszPipeName)
{
	for( int i=0; i<m_nInstances; i++ )
	{
		if( stricmp(m_Instances[i].szPipeName, pszPipeName) == 0 )
		{
			if(m_Instances[i].pInst->m_bReopen)
			{
				//std::deque<std::string> copiedQueue = m_Instances[i].pInst->m_queuedMessages;
				
				CLogger *pInst = m_Instances[i].pInst;

				if( (pInst->m_fpPipe = pInst->Open(pszPipeName)) != NULL )
				{
					pInst->m_bReopen = FALSE;
				}

				/*
				if(pInst)
				{
					while(copiedQueue.size() != 0)
					{
						Break();
						std::string str = copiedQueue.at(0);
						copiedQueue.pop_back();
						pInst->Log(str);
					}
				}
				*/

				return pInst;
			}

			return m_Instances[i].pInst;
		}
	}

	if(m_nInstances >= LOGGER_MAX_INSTANCES)
		throw std::exception("Too many logger instances created"); // revise exception type

	CLogger *pInst = new CLogger();

	m_Instances[m_nInstances].pInst = pInst;
	pInst->m_hMutex = CreateMutex(NULL, FALSE, NULL);
	if(pInst->m_hMutex == NULL)
	{
		delete pInst;
		throw new std::exception("Unable to create mutex");
	}

	strncpy(m_Instances[m_nInstances].szPipeName, pszPipeName, 259);
	m_Instances[m_nInstances].szPipeName[259] = 0;
	
	pInst->m_bReopen = FALSE;
	FILE *fp = NULL;

	if( (fp = pInst->Open(pszPipeName)) == NULL )
	{
		delete pInst;
		throw std::exception("Pipe does not exist");
	}

	m_Instances[m_nInstances].fpPipe = fp;

	m_nInstances++;
	return pInst;
}

void CLogger::CloseInstance(char *pszPipeName, CLogger *&pInst)
{
	if( CloseInstance(pszPipeName) == TRUE && pInst )
	{
		delete pInst;
		pInst = NULL;
	}
}

BOOL CLogger::CloseInstance(char *pszPipeName)
{
	CLogger *pInst = NULL;
	int offs = 0;

	for( int i=0; i<m_nInstances; i++ )
	{
		if( stricmp(m_Instances[i].szPipeName, pszPipeName) == 0 )
		{
			pInst = m_Instances[i].pInst;
			offs = i;
			break;
		}
	}

	if(!pInst)
		return FALSE;

	fclose(pInst->m_fpPipe);
	ReleaseMutex(pInst->m_hMutex);
	CloseHandle(pInst->m_hMutex);
	
	memmove(&m_Instances[offs],
		&m_Instances[offs + 1],
		(m_nInstances - offs - 1) * sizeof(LoggerInstance));

	m_nInstances--;

	return TRUE;
}

int CLogger::GetInstCount() {
	return m_nInstances;
}

BOOL CLogger::ObtainLock(DWORD dwTimeout)
{
	switch( WaitForSingleObject(m_hMutex, dwTimeout) )
	{
	case WAIT_TIMEOUT:
		return FALSE;

	case WAIT_OBJECT_0:
		return TRUE;

	case WAIT_FAILED:
	default:
		throw new std::exception("Failure waiting for logger lock");
	}
}

void CLogger::ObtainLock()
{
	//ObtainLock(INFINITE);
}

void CLogger::ReleaseLock()
{
	//ReleaseMutex(m_hMutex);
}

BOOL CLogger::Log(int value, int base)
{
	char szInteger[ 512 ];
	int cch = (int)ceil(
		log10((float)value) / log10((float)base)
		);
	int remainder;

	if( cch < 0 || cch > sizeof(szInteger) )
		return FALSE;

	if(base > 36) // yeah, you can't get '%' in there :D
	{
		strcpy(szInteger + cch + 1, " (base 10)"); 
		base = 10;
	}
	else
		szInteger[cch--] = 0;

	for( int i=cch; i >= 0; i-- )
	{
		remainder = (value % base);
			
		if(remainder <= 9)
			szInteger[i] = '0' + remainder;
		else
			szInteger[i] = 'A' + (remainder - 10);

		value = value / base;
	}

	return Log(szInteger);
}

BOOL CLogger::Log( char *pszFmt, ... )
{
	BOOL bSuccess = FALSE;
	va_list args;
	va_start(args, pszFmt);
	
	//ObtainLock(); // not the best place to use a lock

	if(m_fpPipe == NULL)
		goto Cleanup;

	int cch = vfprintf(m_fpPipe, pszFmt, args);
	if(cch < 0 )
	{
		//CloseInstance(m_szPipeName);
		m_bReopen = TRUE;

		goto Cleanup;
	}

	if(fflush(m_fpPipe) < 0 /* || m_bReopen */)
	{
		m_bReopen = TRUE;

		/*
		char *pszQueue = (char *)malloc(cch + 1);
		if(pszQueue)
		{
			vsnprintf(pszQueue, cch, pszFmt, args);
			pszQueue[cch] = 0;

			m_bReopen = TRUE;
			m_queuedMessages.push_back(pszQueue);
		}
		*/

		goto Cleanup;
	}

	bSuccess = TRUE;
Cleanup:
	va_end(args);
	//ReleaseLock();

	return bSuccess;
}

BOOL CLogger::LogHex(PBYTE pb, size_t cb)
{
	std::string sFmt;
	char tmp[32];
	size_t i, j, n; 
	BYTE ch;

	for( i=0; i<cb; i += n )
	{
		sprintf(tmp, "0x%.08x", i);
		sFmt += tmp;
		sFmt += "   ";

		n = (cb - i < 16) ? cb - i : 16;

		for( j=0; j<n; j++ )
		{
			sprintf(tmp, "%.02x ", pb[i + j]);
			sFmt += tmp;
		}

		for( j=0; j<(16 - n); j++ )
			sFmt += "   ";
			
		sFmt += "   ";

		for( j=0; j<n; j++ )
		{
			ch = pb[i + j];
			if( ch < 0x20 ) ch = '.';
			sprintf(tmp, "%c", ch);
			sFmt += tmp;
		}

		sFmt += "\n";
	}
		
	return Log(sFmt);
}

BOOL CLogger::Log( std::string str )
{
	return Log("%s", str.c_str());
}

void CLogger::Break()
{
	DebugBreak();
}

void CLogger::BreakIf(BOOL bCondition)
{
	if(bCondition)
		Break();
}

FILE *CLogger::Open(char *pszPipeName)
{
	FILE *fp = NULL;
	char szFullName[ 512 ];
	
	int cch = _snprintf(szFullName, sizeof(szFullName)-1, "\\\\.\\pipe\\%s", pszPipeName);
	if(cch >= 0) szFullName[cch > 0 ? cch : 0] = 0;

	if( (fp = fopen(szFullName, "wb")) == NULL )
		goto Cleanup;

	m_fpPipe = fp;

	strncpy(m_szPipeName, pszPipeName, 259);
	m_szPipeName[259] = 0;
		
Cleanup:
	return fp;
}
