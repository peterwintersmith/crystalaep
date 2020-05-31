// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

//#include "stdafx.h"
#include "..\..\Shared\Utils\stdafx.h"
#include "..\..\Shared\Utils\Utils.h"

const size_t Utils::ARBITRARY_STRING_SIZE_LIMIT = 0x10000000;

DWORD Utils::dwFirstProtectMs = 10000;
BOOL Utils::bChangeProtect = FALSE;

BOOLEAN (__stdcall *Utils::pfnRtlGenRandom)(
	__out  PVOID RandomBuffer,
	__in   ULONG RandomBufferLength
) = NULL;

HMODULE Utils::hmNtdll = NULL;

NTSTATUS (WINAPI *Utils::pfnNtQueryInformationThread)(
	__in       HANDLE ThreadHandle,
	__in       /*THREADINFOCLASS*/ DWORD ThreadInformationClass,
	__inout    PVOID ThreadInformation,
	__in       ULONG ThreadInformationLength,
	__out_opt  PULONG ReturnLength
) = NULL;

NTSTATUS (NTAPI *Utils::pfnNtReadVirtualMemory)(
	__in       HANDLE ProcessHandle,
	__in       PVOID  BaseAddress,
	__out      PVOID  Buffer,
	__in       ULONG  NumberOfBytesToRead,
	__out_opt  PULONG NumberOfBytesReaded
) = NULL;

BOOL (WINAPI *Utils::pfnIsWow64Process)(
	__in		HANDLE hProcess,
	__out		PBOOL Wow64Process
) = NULL;

UINT (WINAPI *Utils::pfnGetSystemWow64Directory)(
	__out  LPTSTR lpBuffer,
	__in   UINT uSize
) = NULL;

void (WINAPI *Utils::pfnGetNativeSystemInfo)(
	__out  LPSYSTEM_INFO lpSystemInfo
) = NULL;

DWORD (APIENTRY *Utils::pfnGetFileVersionInfoSizeExW)(
	__in DWORD dwFlags,
	__in LPCWSTR lpwstrFilename,
	__out LPDWORD lpdwHandle
) = NULL;
	
BOOL (APIENTRY *Utils::pfnGetFileVersionInfoExW)(__in DWORD dwFlags,
	__in LPCWSTR lpwstrFilename,
	__reserved DWORD dwHandle,
	__in DWORD dwLen,
	__out_bcount(dwLen) LPVOID lpData
) = NULL;
	
BOOL (APIENTRY *Utils::pfnVerQueryValueW)(
	__in LPCVOID pBlock,
	__in LPCWSTR lpSubBlock,
	LPVOID * lplpBuffer,
	__out PUINT puLen
) = NULL;

char *Utils::MbFromWideChar(WCHAR *pwszWideChar, UINT codepage)
{
	char *pszMultiByte = NULL;

	if(pwszWideChar == NULL)
	{
		return NULL;
	}
	
	int length = WideCharToMultiByte(
		codepage,
		0, //WC_ERR_INVALID_CHARS | WC_NO_BEST_FIT_CHARS, // overly conservative
		pwszWideChar,
		-1,
		NULL,
		0, // get length
		NULL,
		NULL);

	if(length < 0 || length > ARBITRARY_STRING_SIZE_LIMIT)
	{
		// yeah that's not suspicious ...
		return NULL;
	}

	if(length == 0)
	{
		DWORD dwLastError = GetLastError();

		switch(dwLastError)
		{
		case ERROR_NO_UNICODE_TRANSLATION:
		case ERROR_INSUFFICIENT_BUFFER:
			return NULL;

		case ERROR_INVALID_FLAGS:
		case ERROR_INVALID_PARAMETER:
			throw new std::exception("Invalid parameter passed to WideCharToMultiByte");
				
		default:
			// must be a zero length string
			break;
		}
	}

	pszMultiByte = (char *)malloc(length + 1);
	if(!pszMultiByte)
		return NULL;

	(void)WideCharToMultiByte(
		codepage,
		0, //WC_ERR_INVALID_CHARS | WC_NO_BEST_FIT_CHARS,
		pwszWideChar,
		-1,
		pszMultiByte,
		length,
		NULL,
		NULL);

	pszMultiByte[length] = 0;
		
	return pszMultiByte;
}

WCHAR *Utils::WcFromMultiByte(char *pszMultiByte, UINT codepage)
{
	WCHAR *pwszWideChar = NULL;

	if(pszMultiByte == NULL)
	{
		return NULL;
	}
	
	int length = MultiByteToWideChar(
		codepage,
		0, //WC_ERR_INVALID_CHARS | WC_NO_BEST_FIT_CHARS, // overly conservative
		pszMultiByte,
		-1,
		NULL,
		0); // get length
		

	if(length < 0 || length > ARBITRARY_STRING_SIZE_LIMIT)
	{
		// yeah that's not suspicious ...
		return NULL;
	}

	if(length == 0)
	{
		DWORD dwLastError = GetLastError();

		switch(dwLastError)
		{
		case ERROR_NO_UNICODE_TRANSLATION:
		case ERROR_INSUFFICIENT_BUFFER:
			return NULL;

		case ERROR_INVALID_FLAGS:
		case ERROR_INVALID_PARAMETER:
			throw new std::exception("Invalid parameter passed to MultiByteToWideChar");
				
		default:
			// must be a zero length string
			break;
		}
	}

	pwszWideChar = (WCHAR *)calloc(length + 1, sizeof(WCHAR));
	if(!pwszWideChar)
		return NULL;

	(void)MultiByteToWideChar(
		codepage,
		0, //WC_ERR_INVALID_CHARS | WC_NO_BEST_FIT_CHARS,
		pszMultiByte,
		-1,
		pwszWideChar,
		length);

	pwszWideChar[length] = 0;
		
	return pwszWideChar;
}

long Utils::Resolve(char *pszDnsHost)
{
	long host = inet_addr(pszDnsHost);
	
	if(host == INADDR_NONE)
	{
		struct hostent *hp = gethostbyname(pszDnsHost);
		if(hp)
		{
			host = *(long *)hp->h_addr;
		}
	}

	return host;
}

int Utils::TrimCount(char *psz)
{
	int trim = 0;
	char white[] = {' ', '\t', 0 };

	while(*psz && strchr(white, *psz) != NULL)
	{
		psz++;
		trim++;
	}

	return trim;
}

DWORD Utils::DwordSwapEndian(DWORD dw)
{
	return (DWORD)(
			((BYTE*)&dw)[0] << 24 |
			((BYTE*)&dw)[1] << 16 |
			((BYTE*)&dw)[2] << 8 |
			((BYTE*)&dw)[3]
		);
}

WORD Utils::WordSwapEndian(WORD w)
{
	return (WORD)(
			((BYTE*)&w)[0] << 8 |
			((BYTE*)&w)[1]
		);
}

BOOL Utils::ValidateSslCertificate(char *pszHost, INTERNET_CERTIFICATE_INFO ici)
{
	// note: MUST validate certificate chain. Can use Cert API to achieve this.

	BOOL bValid = FALSE;
	char *pszReverseHost = NULL, *pszReverseSubject = NULL;
	
#ifdef DEBUG_BUILD
	CLogger *logger = NULL;
#endif

	try
	{
#ifdef DEBUG_BUILD
		logger = CLogger::GetInstance("bpcore_wininet");
		logger->Log("Utils::ValidateSslCertificate\n");
#endif
	}
	catch(...)
	{
	}

#ifdef DEBUG_BUILD
	if(logger)
		logger->Log(
			"\tici.dwKeySize = 0x%x\n"
			"\tici.ftExpiry.dwHighDateTime = 0x%x\n"
			"\tici.ftExpiry.dwLowDateTime = 0x%x\n"
			"\tici.ftExpiry.dwLowDateTime = 0x%x\n"
			"\tici.ftStart.dwHighDateTime = 0x%x\n"
			"\tici.lpszEncryptionAlgName = %s\n"
			"\tici.lpszIssuerInfo = %s\n"
			"\tici.lpszProtocolName = %s\n"
			"\tici.lpszSignatureAlgName = %s\n"
			"\tici.lpszSubjectInfo = %s\n",
			ici.dwKeySize,
			ici.ftExpiry.dwHighDateTime,
			ici.ftExpiry.dwLowDateTime,
			ici.ftStart.dwHighDateTime,
			ici.ftStart.dwLowDateTime,
			ici.lpszEncryptionAlgName ? ici.lpszEncryptionAlgName : L"",
			ici.lpszIssuerInfo ? ici.lpszIssuerInfo : L"",
			ici.lpszProtocolName ? ici.lpszProtocolName : L"",
			ici.lpszSignatureAlgName ? ici.lpszSignatureAlgName : L"",
			ici.lpszSubjectInfo ? ici.lpszSubjectInfo : L""
		);
#endif

	if(!(pszReverseHost = strdup(pszHost)))
		goto Cleanup;
	
	pszReverseHost = strrev(pszReverseHost);

	if(!(pszReverseSubject = strdup(reinterpret_cast<char *>(ici.lpszSubjectInfo))))
		goto Cleanup;

	pszReverseSubject = strrev(pszReverseSubject);

	char *psz1 = pszReverseHost,
		 *psz2 = pszReverseSubject;

	while(*psz2 && *psz2 != '\r' && *psz2 != '\n')
	    psz2++;

	*psz2 = '\0';
	psz2 = pszReverseSubject;

	while((*psz1 && *psz2) && *psz2 != '*' && (*psz1 == *psz2))
	{
		psz1++;
		psz2++;
	}

	if((!*psz1 && !*psz2) || *psz2 == '*')
	{
		// if exact match *psz1 and *psz2 are zero: safe. Do nothing.
		
		if(*psz2 == '*')
		{
			// otherwise (e.g.):
			//     host = "moc.elgoog.dcba" and subject = "moc.elgoog.*"
			
			if((psz1 - pszReverseHost) > 1 && *(psz1 - 1) == '.' && strlen(psz2) == 1)
			{
				// safe: matching wild-card to subdomain. Don't allow complicated wild-cards (e.g.):
				//     subject = "moc.elgoog.dc*"
			}
			else
			{
				goto Cleanup;
			}
		}
	}
	else
	{
		// host doesn't match subject (run out of characters)
		goto Cleanup;
	}

	bValid = TRUE;
Cleanup:

	if(pszReverseHost)
		free(pszReverseHost);
	
	if(pszReverseSubject)
		free(pszReverseSubject);

	return bValid;
}

size_t Utils::GetRandomInteger(size_t min, size_t max)
{
	size_t random;
	
	if(!pfnRtlGenRandom)
	{
		HMODULE hmAdvApi32 = !GetModuleHandle(L"advapi32.dll") ? LoadLibrary(L"advapi32.dll") : GetModuleHandle(L"advapi32.dll");

		if(!hmAdvApi32 || (pfnRtlGenRandom = (BOOLEAN (__stdcall *)(LPVOID, ULONG))GetProcAddress(
				hmAdvApi32,
				"SystemFunction036")) == NULL )
		{
			throw new std::exception("Unable to find RtlGenRandom in advapi32.dll");
		}
	}

	if(pfnRtlGenRandom(&random, sizeof(size_t)) == FALSE)
	{
		throw new std::exception("Unable to generate random integer");
	}

	return min + (random % (max - min));
}

BOOL Utils::GetRandomBytes(BYTE *pBuf, size_t cbBuf)
{
	if(!pfnRtlGenRandom)
	{
		HMODULE hmAdvApi32 = !GetModuleHandle(L"advapi32.dll") ? LoadLibrary(L"advapi32.dll") : GetModuleHandle(L"advapi32.dll");

		if(!hmAdvApi32 || (pfnRtlGenRandom = (BOOLEAN (__stdcall *)(LPVOID, ULONG))GetProcAddress(
				hmAdvApi32,
				"SystemFunction036")) == NULL )
		{
			return FALSE;
			//throw new std::exception("Unable to find RtlGenRandom in advapi32.dll");
		}
	}

	if(pfnRtlGenRandom(pBuf, cbBuf) == FALSE)
	{
		return FALSE;
		//throw new std::exception("Unable to generate random bytes");
	}

	return TRUE;
}

WCHAR *Utils::GetRegistryString(HKEY hKey, WCHAR *pwszRegPath, WCHAR *pwszKeyName)
{
	BOOL bErr = TRUE;
	HKEY hkResult = 0;
	WCHAR *pwszKeyValue = NULL;
	DWORD cbKeyValue = 0;

	if(RegOpenKeyEx(hKey, pwszRegPath, 0, KEY_QUERY_VALUE,
			&hkResult) != ERROR_SUCCESS)
		goto Cleanup;

	DWORD dwType = 0;

	if(RegQueryValueEx(hkResult, pwszKeyName, NULL, &dwType, NULL,
			&cbKeyValue) != ERROR_SUCCESS)
		goto Cleanup;

	if(dwType != REG_SZ)
		goto Cleanup;

	pwszKeyValue = (WCHAR *)malloc(cbKeyValue + sizeof(WCHAR));
	if(!pwszKeyValue)
		goto Cleanup;

	memset(pwszKeyValue, 0, cbKeyValue + sizeof(WCHAR));

	if(RegQueryValueEx(hkResult, pwszKeyName, NULL, &dwType, (PBYTE)pwszKeyValue,
			&cbKeyValue) != ERROR_SUCCESS)
		goto Cleanup;

	bErr = FALSE;
Cleanup:
	if(hkResult)
		RegCloseKey(hkResult);

	if(bErr && pwszKeyValue)
	{
		free(pwszKeyValue);
		pwszKeyValue = NULL;
	}

	return pwszKeyValue;
}

DWORD Utils::GetPageProtectionMinusExecutionFlag(DWORD flProtect, BOOL bDllMem)
{
	struct
	{
		DWORD flWithExecution;
		DWORD flWithoutExecution;

	} rgdwProtectionPairs[4] = {
		{ PAGE_EXECUTE,			  PAGE_READONLY  },
		{ PAGE_EXECUTE_READ,	  PAGE_READONLY  },
		{ PAGE_EXECUTE_READWRITE, PAGE_READWRITE },
		{ PAGE_EXECUTE_WRITECOPY, PAGE_WRITECOPY }
	};

	for(int i=0; i<4; i++)
	{
		if(flProtect & rgdwProtectionPairs[i].flWithExecution)
		{
			if(bChangeProtect)
			{
				if(bDllMem)
				{
					flProtect ^= rgdwProtectionPairs[i].flWithExecution;
					flProtect |= PAGE_EXECUTE_READ; // but *never* WRITE
				}
				else
				{
					flProtect ^= rgdwProtectionPairs[i].flWithExecution;
					flProtect |= rgdwProtectionPairs[i].flWithoutExecution;
				}
			}
		}
	}

	return flProtect;
}

int Utils::SetChangeProtectThread(BOOL bProtect)
{
#ifdef DEBUG_BUILD
	Sleep(dwFirstProtectMs);
	dwFirstProtectMs = 0;
#endif
	bChangeProtect = bProtect;
	return 0;
}

int Utils::compare(const void *elem1, const void *elem2)
{
	size_t h1 = *(size_t *)elem1,
		   h2 = *(size_t *)elem2;

	if(h1 < h2)
		return -1;

	if(h1 == h2)
		return 0;

	return 1;
}

unsigned int Utils::RSHash(char* str, unsigned int len)
{
	unsigned int b    = 378551;
	unsigned int a    = 63689;
	unsigned int hash = 0;
	unsigned int i    = 0;

	for(i = 0; i < len; str++, i++)
	{
		hash = hash * a + (*str);
		a    = a * b;
	}

	return hash;
}

WCHAR *Utils::wcsistr(WCHAR *pwszString, WCHAR *pwszSearch, size_t cchString)
{
	size_t cchSearch = wcslen(pwszSearch);

	if(cchString != -1)
	{
		if(cchString == cchSearch)
		{
			if(!wcsicmp(pwszString, pwszSearch))
			{
				return pwszString;
			}
		}
		else if(cchString < cchSearch)
		{
			return NULL;
		}
	}

	for(size_t i=0; cchString == -1 ? pwszString[i] != '\0' : i < cchString - cchSearch; i++)
	{
		if(!wcsnicmp(pwszString + i, pwszSearch, cchSearch))
		{
			return pwszString + i;
		}
	}

	return NULL;
}

char *Utils::stristr(char *pszString, char *pszSearch, size_t cchString)
{
	size_t cchSearch = strlen(pszSearch);

	if(cchString != -1)
	{
		if(cchString == cchSearch)
		{
			if(!strnicmp(pszString, pszSearch, cchSearch))
			{
				return pszString;
			}
		}
		else if(cchString < cchSearch)
		{
			return NULL;
		}
	}

	for(size_t i=0; cchString == -1 ? pszString[i] != '\0' : i < cchString - cchSearch; i++)
	{
		if(!strnicmp(pszString + i, pszSearch, cchSearch))
		{
			return pszString + i;
		}
	}

	return NULL;
}

std::wstring Utils::wstringlower(std::wstring wstr)
{
	std::wstring wstrLower = wstr;

	for(size_t i=0; i<wstrLower.size(); i++)
	{
		wstrLower[i] = tolower(wstrLower[i]);
	}

	return wstrLower;
}

DWORD Utils::GetReturnAddress(int frame)
{
	DWORD dwRetnAddress = 0;
	
	// this is inherently precarious as non-frame based functions will lead to AV
	// exception handler should adequately mitigate

	__try
	{
		__asm
		{
Prologue:
			push eax
			push ebx
			push ecx
			push edx

			lea eax, [dwRetnAddress]
			mov ecx, frame
			mov edx, ebp

			inc ecx // comment out if inlining

NextFrame:
CheckStackTop:
			cmp edx, fs:[4]
			jb CheckStackBase
			xor ebx, ebx
			jmp Done

CheckStackBase:
			cmp edx, fs:[8]
			ja IsWithinStackRegion
			xor ebx, ebx
			jmp Done

IsWithinStackRegion:
			mov ebx, [edx + 4] // saved return address
			mov edx, [edx] // saved base pointer
			loopnz NextFrame

Done:
			mov [eax], ebx
			
Epilogue:
			pop edx
			pop ecx
			pop ebx
			pop eax
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		dwRetnAddress = 0;
	}

	return dwRetnAddress;
}

BOOL Utils::ContainsHtmlMetacharacters(std::string strCheck)
{
	BYTE sz[] = {
		'<',
		'>',
		'\"',
		'\'',
		// take part in illegal UTF encodings
		//     i.e. %3c = %c0 %bc 
		0x80 + '<',
		0x80 + '>',
		0x80 + '\"',
		0x80 + '\'',
	};

	BOOL bContained = FALSE;

	for(size_t i=0; i<sizeof(sz)-1; i++)
	{
		if(strCheck.find(sz[i]) != std::string::npos)
		{
			bContained = TRUE;
			break;
		}
	}

	return bContained;
}

BOOL Utils::GetUrlParams(std::string strUrl, std::multimap<std::string, std::string>& mmapUrlParams)
{
	BOOL bSuccess = TRUE;
	
	char *pszUrlCopy = strdup(strUrl.c_str());
	if(!pszUrlCopy)
		goto Cleanup;

	char *pszArg = strtok(pszUrlCopy, "&");

	while(pszArg)
	{
		char *pszValue = strchr(pszArg, '=');
		
		if(pszValue)
		{
			*pszValue = '\0';
			mmapUrlParams.insert(std::pair<std::string, std::string>(pszArg, pszValue + 1));
			*pszValue = '=';
			
			// move next
		}
		else
		{
			// not name/value pair: move next
			// i.e. /foo/bar.php?delete[&name=value][...]
		}

		pszArg = strtok(NULL, "&");
	}

Cleanup:
	if(pszUrlCopy)
		free(pszUrlCopy);

	if(!bSuccess)
		mmapUrlParams.clear();

	return bSuccess;
}

std::string Utils::UrlUnescape(std::string strEscaped, BOOL bSupportUnicode, BOOL& bError, char chSignifier)
{
	std::string strUnescaped;
	int ch;
	
	strUnescaped.reserve(strEscaped.length());
	bError = FALSE;
	
	for(size_t i=0; i<strEscaped.length();)
	{
		ch = strEscaped[i];

		if(ch == chSignifier && i + 3 <= strEscaped.length())
		{
			i++;
			ch = strEscaped[i];

			if(bSupportUnicode && (ch == 'u' || ch == 'U'))
			{
				i++;

				if(i + 4 <= strEscaped.length())
				{
					if(isxdigit(strEscaped[i]) && isxdigit(strEscaped[i + 1]) && isxdigit(strEscaped[i + 2]) && isxdigit(strEscaped[i + 3]))
					{
						if(sscanf(strEscaped.c_str() + i, "%04x", &ch) > 0)
						{
							i += 4;
						}
					}
					else
					{
						bError = TRUE;
					}
				}
				else
				{
					bError = TRUE;
				}
			}
			else
			{
				if(isxdigit(strEscaped[i]) && isxdigit(strEscaped[i + 1]))
				{
					if(sscanf(strEscaped.c_str() + i, "%02x", &ch) > 0)
					{
						if(!ch)
							bError = TRUE;

						i += 2;
					}
				}
				else
				{
					bError = TRUE;
				}
			}
		}
		else
		{
			if(chSignifier == '%' && ch == '+') // definitely url-encoding
				ch = ' ';

			i++;
		}

		if(ch) // doesn't affect NUL termination: std::string provides this
			strUnescaped += ch;
	}

	return strUnescaped;
}

BOOL Utils::GetProcessName(std::string& strProcName, HANDLE hProcess)
{
	BOOL bSuccess = FALSE;

	if(!hProcess)
		hProcess = GetCurrentProcess();

	char *pszExePath = (char *)malloc(8192);
	if(!pszExePath)
		goto Cleanup;

	DWORD nSize = GetModuleFileNameExA(hProcess, NULL, pszExePath, 8191);
	
	if(!nSize || nSize == 8191)
		goto Cleanup;

	char *psz = strrchr(pszExePath, '\\');
	if(psz)
	{
		memmove(pszExePath, psz + 1, strlen(psz + 1) + 1); // + 1 for NUL
	}

	strProcName.assign(pszExePath);
	
	bSuccess = TRUE;
Cleanup:
	if(pszExePath)
		free(pszExePath);

	return bSuccess;
}

std::string Utils::ClsidToString(CLSID& rclsid)
{
	LPOLESTR lpos = NULL;
	std::string strClsid;

	StringFromCLSID(rclsid, &lpos);
	if(lpos)
	{
		char *pszClsid = Utils::MbFromWideChar(lpos);
		strClsid = pszClsid;
		CoTaskMemFree(lpos);
		free(pszClsid);
	}

	return strClsid;
}

std::wstring Utils::GetRegInstallPath()
{
	std::wstring wstrPath;
	
	LPWSTR pwszPath = GetRegistryString(HKEY_CURRENT_USER, L"Software\\CrystalBP", L"InstallPath");
	if(pwszPath)
	{
		wstrPath = pwszPath;
		free(pwszPath);
	}
	
	return wstrPath;
}

BOOL Utils::CreateEmptyFile(const WCHAR *pwszPath)
{
	HANDLE hFile = CreateFile(
			pwszPath,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			CREATE_ALWAYS,
			0,
			NULL
		);

	if(hFile)
	{
		CloseHandle(hFile);
		return TRUE;
	}

	return FALSE;
}

BOOL Utils::OpenEmptyFile(const WCHAR *pwszPath)
{
	HANDLE hFile = CreateFile(
			pwszPath,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			0,
			NULL
		);

	if(hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
		return TRUE;
	}

	return FALSE;
}

BOOL Utils::DeleteEmptyFile(const WCHAR *pwszPath)
{
	if(OpenEmptyFile(pwszPath))
	{
		return DeleteFile(pwszPath);
	}

	return FALSE;
}

Utils::WinClientVer Utils::GetWinVersion()
{
	DWORD dwVersion = GetVersion(); 
    
    DWORD dwMajorVersion = (DWORD)(LOBYTE(LOWORD(dwVersion))),
		dwMinorVersion = (DWORD)(HIBYTE(LOWORD(dwVersion)));

	if(dwMajorVersion == 5 && dwMinorVersion == 0)
		return Utils::WinClientVer::Windows2000;

	if((dwMajorVersion == 5 && dwMinorVersion == 1) || // XP-32
		(dwMajorVersion == 5 && dwMinorVersion == 2)) // XP-64
		return Utils::WinClientVer::WindowsXP;

	if(dwMajorVersion == 6 && dwMinorVersion == 0)
		return Utils::WinClientVer::WindowsVista;

	if(dwMajorVersion == 6 && dwMinorVersion == 1)
		return Utils::WinClientVer::Windows7;

	return Utils::WinClientVer::Unknown;
}

DWORD Utils::ThreadIdFromHandle(HANDLE hThread)
{
	DWORD dwThreadId = 0;
	THREAD_BASIC_INFORMATION tbi = {0};
	HANDLE hDuplicate = NULL;

	if(!DuplicateHandle(GetCurrentProcess(), hThread, GetCurrentProcess(), &hDuplicate, THREAD_QUERY_INFORMATION, FALSE, 0))
		goto Cleanup;

	if(!pfnNtQueryInformationThread)
	{
		if(!hmNtdll)
			hmNtdll = GetModuleHandle(L"ntdll.dll");
		
		if(!hmNtdll)
			goto Cleanup;

		pfnNtQueryInformationThread = (NTSTATUS (WINAPI *)(__in HANDLE, __in DWORD, __inout PVOID, __in ULONG, __out_opt PULONG))GetProcAddress(hmNtdll, "NtQueryInformationThread");
		if(!pfnNtQueryInformationThread)
			goto Cleanup;
	}

	DWORD dwReturnLength = 0;

	if(!NT_SUCCESS(pfnNtQueryInformationThread(hDuplicate, 0, &tbi, sizeof(tbi), &dwReturnLength)))
		goto Cleanup;
	
	dwThreadId = tbi.UniqueThreadId;

Cleanup:
	if(hDuplicate)
		CloseHandle(hDuplicate);

	return dwThreadId;
}

BOOL Utils::ThreadGetStackRegion(HANDLE hThread, PVOID *pvStackBase, PVOID *pvStackTop)
{
	BOOL bSuccess = FALSE;
	THREAD_BASIC_INFORMATION tbi = {0};
	NT_TIB tib;
	HANDLE hDuplicate = NULL;

	if(!DuplicateHandle(GetCurrentProcess(), hThread, GetCurrentProcess(), &hDuplicate, THREAD_QUERY_INFORMATION, FALSE, 0))
		goto Cleanup;

	if(!pfnNtQueryInformationThread)
	{
		if(!hmNtdll)
			hmNtdll = GetModuleHandle(L"ntdll.dll");
		
		if(!hmNtdll)
			goto Cleanup;

		pfnNtQueryInformationThread = (NTSTATUS (WINAPI *)(__in HANDLE, __in DWORD, __inout PVOID, __in ULONG, __out_opt PULONG))GetProcAddress(hmNtdll, "NtQueryInformationThread");
		if(!pfnNtQueryInformationThread)
			goto Cleanup;
	}

	DWORD dwReturnLength = 0;

	if(!NT_SUCCESS(pfnNtQueryInformationThread(hDuplicate, 0, &tbi, sizeof(tbi), &dwReturnLength)))
		goto Cleanup;
	
	if(!pfnNtReadVirtualMemory)
	{
		if(!hmNtdll)
			hmNtdll = GetModuleHandle(L"ntdll.dll");
		
		if(!hmNtdll)
			goto Cleanup;

		pfnNtReadVirtualMemory = (NTSTATUS (WINAPI *)(__in HANDLE, __in PVOID, __out PVOID, __in ULONG, __out_opt PULONG))GetProcAddress(hmNtdll, "NtReadVirtualMemory");
		if(!pfnNtReadVirtualMemory)
			goto Cleanup;
	}
	
	if(!NT_SUCCESS(pfnNtReadVirtualMemory(GetCurrentProcess(), tbi.TebBaseAddress, &tib, sizeof(tib), &dwReturnLength)))
		goto Cleanup;

	if(pvStackBase)
		*pvStackBase = tib.StackBase;
	
	if(pvStackTop)
		*pvStackTop = tib.StackLimit;

	bSuccess = TRUE;
Cleanup:
	if(hDuplicate)
		CloseHandle(hDuplicate);

	return bSuccess;
}

BOOL Utils::IsBadReadPtr(LPVOID lpvPtr, DWORD cbPtr)
{
	BOOL bSuccess = TRUE; // default IsBad=True
	BYTE rgTemp[32];
	SIZE_T cbRead = 0;
	PBYTE pbPtr = (PBYTE)lpvPtr;

	for(SIZE_T i=0; i < cbPtr;)
	{
		SIZE_T dwReadLength = (cbPtr - i) < 32 ? (cbPtr - i) : 32;
		
		if(!ReadProcessMemory(GetCurrentProcess(), pbPtr + i, rgTemp, dwReadLength, &cbRead))
			goto Cleanup;

		i += cbRead;
	}

	bSuccess = FALSE;
Cleanup:
	return bSuccess;
}

BOOL Utils::Is64BitExecutable(std::string strPath)
{
	/*
		NtHeaders=>FileHeader=>Machine:
		 ALPHA64 = 0x0284
		 AMD64   = 0x8664
		 Intel64 = 0x0200
	*/

#define MACHINE_ALPHA64		0x0284
#define MACHINE_AMD64		0x8664
#define MACHINE_INTEL64		0x0200

	BOOL b64Bit = FALSE;
	PACKED_IMAGE_DOS_HEADER dosHeader = {0};
	PACKED_IMAGE_FILE_HEADER fileHeader = {0};
	static BYTE rgBuffer[4096 + 128] = {0};

	FILE *fp = fopen(strPath.c_str(), "rb");
	if(!fp)
		goto Cleanup;

	if(fread(rgBuffer, 1, sizeof(rgBuffer), fp) != sizeof(rgBuffer))
		goto Cleanup;

	memcpy(&dosHeader, rgBuffer, sizeof(PACKED_IMAGE_DOS_HEADER));

	if(dosHeader.e_lfanew > 4096)
		goto Cleanup;

	if(*(DWORD *)(rgBuffer + dosHeader.e_lfanew) != 'EP')
		goto Cleanup;

	memcpy(&fileHeader, rgBuffer + dosHeader.e_lfanew + 4, sizeof(PACKED_IMAGE_FILE_HEADER));

	switch(fileHeader.Machine)
	{
	case MACHINE_ALPHA64:
	case MACHINE_AMD64:
	case MACHINE_INTEL64:
		
		b64Bit = TRUE;

		break;
	default:
		break;
	}

Cleanup:
	if(fp)
		fclose(fp);

	return b64Bit;
}

BOOL Utils::Is64BitProcess(DWORD dwPID)
{
	BOOL b64Bit = FALSE;

	if(!pfnGetSystemWow64Directory)
	{
		pfnGetSystemWow64Directory = (UINT (WINAPI*)(LPTSTR, UINT))GetProcAddress(GetModuleHandle(L"kernel32.dll"), "IsWow64Process");
		if(!pfnGetSystemWow64Directory) // old 32-bit OS?
			goto Cleanup;
	}

	if(!pfnIsWow64Process)
	{
		pfnIsWow64Process = (BOOL (WINAPI *)(HANDLE, PBOOL))GetProcAddress(GetModuleHandle(L"kernel32.dll"), "IsWow64Process");
		if(!pfnIsWow64Process)
			goto Cleanup; // old OS (pre-64 bit)?
	}

	if(!pfnGetNativeSystemInfo)
	{
		pfnGetNativeSystemInfo = (void (WINAPI *)(LPSYSTEM_INFO))GetProcAddress(GetModuleHandle(L"kernel32.dll"), "GetNativeSystemInfo");
		if(!pfnGetNativeSystemInfo) // old 32-bit OS?
			goto Cleanup;
	}

	SYSTEM_INFO si = {0};

	pfnGetNativeSystemInfo(&si);

	switch(si.wProcessorArchitecture)
	{
	case PROCESSOR_ARCHITECTURE_AMD64:
	case PROCESSOR_ARCHITECTURE_IA64:
		// may still be wow64 process
		break;
	case PROCESSOR_ARCHITECTURE_INTEL:
	case PROCESSOR_ARCHITECTURE_UNKNOWN:
	default:
		goto Cleanup; // assume 32 if not definite 64
	}

	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, dwPID);
	if(!hProcess)
	{
		hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, dwPID);
		if(!hProcess)
			goto Cleanup; // let's assume it's 32-bit since we can't open the process
	}

	BOOL bTmp = FALSE;

	if(!pfnIsWow64Process(hProcess, &bTmp))
		goto Cleanup; // assume 32-bit

	if(bTmp)
		goto Cleanup; // it's a 32-bit WOW64 process

	b64Bit = TRUE;
Cleanup:
	if(hProcess)
		CloseHandle(hProcess);

	return b64Bit;
}

BOOL Utils::GetNullDacl(SECURITY_ATTRIBUTES *pSA, SECURITY_DESCRIPTOR *pSD)
{
	BOOL bSuccess = FALSE;
	
	if(!pSA || !pSD)
		goto Cleanup;
	
	memset(pSA, 0, sizeof(SECURITY_ATTRIBUTES));
	memset(pSD, 0, sizeof(SECURITY_DESCRIPTOR));

	if(!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION))
		goto Cleanup;

	if(!SetSecurityDescriptorDacl(pSD, TRUE, (PACL)NULL, FALSE))
		goto Cleanup;

	pSA->bInheritHandle = FALSE;
	pSA->lpSecurityDescriptor = pSD;
	pSA->nLength = sizeof(SECURITY_ATTRIBUTES);

	bSuccess = TRUE;
Cleanup:
	return bSuccess;
}

BOOL Utils::IsContainedInPipeSeparatedList(WCHAR *pwszList, WCHAR *pwszItem)
{
	BOOL bFound = FALSE;
	WCHAR *pwszToken = NULL;
	DWORD cch = 0;
	
	if((pwszList = wcsdup(pwszList)) == NULL) // wcstok alters the string, so copy it
		goto Cleanup;

	pwszToken = wcstok(pwszList, L"|");

	while(pwszToken != NULL)
	{
		if(!wcsicmp(pwszToken, pwszItem))
		{
			bFound = TRUE;
			break;
		}

		pwszToken = wcstok(NULL, L"|");
	}

	if(pwszList)
		free(pwszList);

Cleanup:
	return bFound;
}

BOOL Utils::GetExecutableNameFromParams(std::wstring wstrProcess, std::wstring wstrParameters, std::wstring& wstrExeName)
{
	BOOL bSuccess = FALSE;
	std::wstring wstrExePath = wstrProcess;
	
	wstrExeName = L"";
	
	if(!wstrProcess.empty())
	{
		if(wstrProcess[0] == '\"' && wstrProcess.find('"', 1) != std::wstring::npos)
		{
			wstrExePath = wstrProcess.substr(1, wstrProcess.find('"', 1) - 1);
		}
	}
	else
	{
		if(wstrParameters.empty())
		{
			goto Cleanup;
		}

		WCHAR ch = wstrParameters[0];
		if(ch == '"' && wstrParameters.find('"', 1) != std::wstring::npos)
		{
			wstrExePath = wstrParameters.substr(1, wstrParameters.find('"', 1) - 1);
		}
		else
		{
			if(wstrParameters.find(' ', 1) != std::wstring::npos)
			{
				wstrExePath = wstrParameters.substr(0, wstrParameters.find(' ', 1));
				
				if(!wcsicmp(wstrExePath.c_str() + 1, L":\\Program"))
				{
					wstrExePath = wstrParameters.substr(0, wstrParameters.find(L".exe", 1) + 4);
				}
			}
			else
			{
				wstrExePath = wstrParameters;
			}
		}
	}

	if(wstrExePath.rfind('\\') != std::wstring::npos)
	{
		wstrExeName = wstrExePath.substr(wstrExePath.rfind('\\') + 1);
	}
	else
	{
		wstrExeName = wstrExePath;
	}

	bSuccess = TRUE;
Cleanup:
	return bSuccess;
}

BOOL Utils::IsMemMappedOrModule(LPVOID lpvMem)
{
	MEMORY_BASIC_INFORMATION mbi = {0};

	if(VirtualQuery(lpvMem, &mbi, sizeof(mbi)))
	{
		if(mbi.State & MEM_COMMIT)
		{
			if((mbi.Type & MEM_IMAGE) || (mbi.Type & MEM_MAPPED))
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}

BOOL Utils::SetRegistryString(HKEY hKey, WCHAR *pwszRegPath, WCHAR *pwszName, WCHAR *pwszValue)
{
	BOOL bSuccess = FALSE;
	HKEY hkResult = 0;
	
	if(RegOpenKeyEx(hKey, pwszRegPath, 0, KEY_SET_VALUE, &hkResult) != ERROR_SUCCESS)
		goto Cleanup;

	if(RegSetValueEx(hkResult, pwszName, NULL, REG_SZ, (PBYTE)pwszValue,
			sizeof(WCHAR) * (wcslen(pwszValue) + 1)) != ERROR_SUCCESS)
		goto Cleanup;

	bSuccess = TRUE;
Cleanup:
	if(hkResult)
		RegCloseKey(hkResult);

	return bSuccess;
}

BOOL Utils::SetRegistryDword(HKEY hKey, WCHAR *pwszRegPath, WCHAR *pwszName, DWORD dwValue)
{
	BOOL bSuccess = FALSE;
	HKEY hkResult = 0;
	
	if(RegOpenKeyEx(hKey, pwszRegPath, 0, KEY_SET_VALUE, &hkResult) != ERROR_SUCCESS)
		goto Cleanup;

	if(RegSetValueEx(hkResult, pwszName, NULL, REG_DWORD, (PBYTE)&dwValue, sizeof(DWORD)) != ERROR_SUCCESS)
		goto Cleanup;

	bSuccess = TRUE;
Cleanup:
	if(hkResult)
		RegCloseKey(hkResult);

	return bSuccess;
}

BOOL Utils::GetRegistryDword(HKEY hKey, WCHAR *pwszRegPath, WCHAR *pwszKeyName, DWORD *pdwResult)
{
	BOOL bSuccess = FALSE;
	HKEY hkResult = 0;
	DWORD dwType = 0, dwResult = 0, cbResult = sizeof(DWORD);

	if(RegOpenKeyEx(hKey, pwszRegPath, 0, KEY_QUERY_VALUE, &hkResult) != ERROR_SUCCESS)
		goto Cleanup;

	if(RegQueryValueEx(hkResult, pwszKeyName, NULL, &dwType, NULL, NULL) != ERROR_SUCCESS)
		goto Cleanup;

	if(dwType != REG_DWORD || cbResult != sizeof(DWORD))
		goto Cleanup;

	*pdwResult = dwResult;

	bSuccess = TRUE;
Cleanup:
	if(hkResult)
		RegCloseKey(hkResult);

	return bSuccess;
}

BOOL Utils::GetDescriptionStringFile(std::wstring wstrFile, std::wstring& wstrDescription)
{
	BOOL bSuccess = FALSE;
	DWORD dwTemp = 0;
	LPVOID lpvInfo = NULL;
	UINT uiDescLen = 0, uiTranslateLen = 0;
	WCHAR wszFileDesc[260] = {0}, *pwszDescText = NULL;
	wstrDescription = L"";

	struct LANGANDCODEPAGE {
		WORD wLanguage;
		WORD wCodePage;
	} *lpTranslate = NULL;
	
	if(!pfnGetFileVersionInfoSizeExW || !pfnGetFileVersionInfoExW || !pfnVerQueryValueW)
	{
		HMODULE hmVersion = GetModuleHandle(L"version.dll") ? GetModuleHandle(L"version.dll") : LoadLibrary(L"version.dll");
		if(!hmVersion)
			return FALSE;

		pfnGetFileVersionInfoSizeExW = (DWORD (APIENTRY *)(DWORD, LPCWSTR, LPDWORD))GetProcAddress(hmVersion, "GetFileVersionInfoSizeExW");
		pfnGetFileVersionInfoExW = (BOOL (APIENTRY *)(DWORD, LPCWSTR, DWORD, DWORD, LPVOID))GetProcAddress(hmVersion, "GetFileVersionInfoExW");
		pfnVerQueryValueW = (BOOL (APIENTRY *)(LPCVOID, LPCWSTR, LPVOID *, PUINT))GetProcAddress(hmVersion, "VerQueryValueW");
	}

	if(!pfnGetFileVersionInfoSizeExW || !pfnGetFileVersionInfoExW || !pfnVerQueryValueW)
		return FALSE;

	DWORD dwVersionInfoSize = pfnGetFileVersionInfoSizeExW(
			FILE_VER_GET_NEUTRAL,
			wstrFile.c_str(),
			&dwTemp
		);

	if(!dwVersionInfoSize)
		goto Cleanup;

	lpvInfo = (LPVOID)malloc(dwVersionInfoSize);
	if(!lpvInfo)
		goto Cleanup;

	if(!pfnGetFileVersionInfoExW(
				FILE_VER_GET_NEUTRAL,
				wstrFile.c_str(),
				NULL,
				dwVersionInfoSize,
				lpvInfo
			))
		goto Cleanup;


	if(!pfnVerQueryValueW(lpvInfo, L"\\VarFileInfo\\Translation", (LPVOID *)&lpTranslate, &uiTranslateLen))
		goto Cleanup;

	for(DWORD i=0; i < (uiTranslateLen/sizeof(struct LANGANDCODEPAGE)); i++)
	{
		wsprintf(wszFileDesc, L"\\StringFileInfo\\%04x%04x\\FileDescription", lpTranslate->wLanguage, lpTranslate->wCodePage);

		if(!pfnVerQueryValueW(lpvInfo, wszFileDesc, (LPVOID *)&pwszDescText, &uiDescLen))
			goto Cleanup;

		if(uiDescLen > 0 && pwszDescText[0] != '\0')
		{
			if(wstrDescription.size() != 0)
			{
				wstrDescription += L"; ";
			}

			if(pwszDescText[uiDescLen - 1] == '\0')
				uiDescLen--;

			wstrDescription += std::wstring(pwszDescText, uiDescLen);
		}
	}

	bSuccess = TRUE;
Cleanup:
	if(lpvInfo)
		free(lpvInfo);

	return bSuccess;
}

BOOL Utils::GetEnvVar(std::wstring wstrVarName, std::wstring& wstrVarValue)
{
	BOOL bSuccess = FALSE;

	static WCHAR wszEnvVar[16384] = {0};
	const DWORD cchEnvVar = (sizeof(wszEnvVar) / sizeof(WCHAR) - 1);

	DWORD dwLength = GetEnvironmentVariable(wstrVarName.c_str(), wszEnvVar, cchEnvVar);
	if(!dwLength)
		goto Cleanup;
	
	wszEnvVar[dwLength] = '\0';
	wstrVarValue = wszEnvVar;

	bSuccess = TRUE;
Cleanup:
	return bSuccess;
}

BOOL Utils::SetEnvVar(std::wstring wstrVarName, std::wstring wstrVarValue)
{
	return SetEnvironmentVariable(wstrVarName.c_str(), wstrVarValue.c_str());
}

BOOL Utils::GetCurDir(std::wstring& wstrCurDir)
{
	BOOL bSuccess = FALSE;

	static WCHAR wszCurDir[2048] = {0};
	const DWORD cchCurDir = (sizeof(wszCurDir) / sizeof(WCHAR) - 1);

	DWORD dwLength = GetCurrentDirectory(cchCurDir, wszCurDir);
	if(!dwLength)
		goto Cleanup;

	wszCurDir[dwLength] = '\0';
	wstrCurDir = wszCurDir;

	bSuccess = TRUE;
Cleanup:
	return bSuccess;
}

BOOL Utils::SetObjectToLowIntegrity(HANDLE hObject, SE_OBJECT_TYPE type)
{
	BOOL bRet = FALSE;
	DWORD dwErr = ERROR_SUCCESS;
	PSECURITY_DESCRIPTOR pSD = NULL;
	PACL pSacl = NULL;
	BOOL fSaclPresent = FALSE;
	BOOL fSaclDefaulted = FALSE;
 
  if ( ConvertStringSecurityDescriptorToSecurityDescriptorW (
         LOW_INTEGRITY_SDDL_SACL_W, SDDL_REVISION_1, &pSD, NULL ) )
    {
    if ( GetSecurityDescriptorSacl (
           pSD, &fSaclPresent, &pSacl, &fSaclDefaulted ) )
      {
      dwErr = SetSecurityInfo (
                hObject, type, LABEL_SECURITY_INFORMATION,
                NULL, NULL, NULL, pSacl );
 
      bRet = (ERROR_SUCCESS == dwErr);
      }
 
    LocalFree ( pSD );
    }
 
  return bRet;
}

BOOL Utils::IsLowIntegrityProcess(HANDLE hProcess, BOOL *pboolIsLow)
{
	BOOL bSuccess = FALSE;
	HANDLE hToken = 0;
	DWORD dwTokenInfoLength = 0, *pdwSubAuth = NULL, dwIntegLevel = 0;
	PUCHAR pucSubAuthCount = NULL;
	PTOKEN_MANDATORY_LABEL pTIL = NULL;

	if(!OpenProcessToken(hProcess, TOKEN_QUERY, &hToken))
		goto Cleanup;

	GetTokenInformation(hToken, TokenIntegrityLevel, NULL, 0, &dwTokenInfoLength);
	if(dwTokenInfoLength == 0)
		goto Cleanup;
		
	pTIL = (PTOKEN_MANDATORY_LABEL)LocalAlloc(0, dwTokenInfoLength);
	if(!pTIL)
		goto Cleanup;

	if(!GetTokenInformation(hToken, TokenIntegrityLevel, pTIL, dwTokenInfoLength, &dwTokenInfoLength))
		goto Cleanup;

	pucSubAuthCount = GetSidSubAuthorityCount(pTIL->Label.Sid);
	if(!pucSubAuthCount)
		goto Cleanup;

	pdwSubAuth = GetSidSubAuthority(pTIL->Label.Sid, *pucSubAuthCount - 1);
	if(!pdwSubAuth)
		goto Cleanup;

	dwIntegLevel = *pdwSubAuth;

	if(dwIntegLevel == SECURITY_MANDATORY_LOW_RID)
	{
		// low integrity process
		*pboolIsLow = TRUE;
	}
	else
	{
		// medium or high
		*pboolIsLow = FALSE;
	}

	bSuccess = TRUE;
Cleanup:
	if(pTIL)
		LocalFree(pTIL);

	if(hToken)
		CloseHandle(hToken);

	return bSuccess;
}