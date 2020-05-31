// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#pragma once

#ifndef BUILDING_DIDCORE
//#include "..\..\Shared\Utils\stdafx.h"
#endif

#include <WinInet.h>
#include <AccCtrl.h>

#pragma pack(1)
typedef struct {      // DOS .EXE header
	WORD   e_magic;                     // Magic number
	WORD   e_cblp;                      // Bytes on last page of file
	WORD   e_cp;                        // Pages in file
	WORD   e_crlc;                      // Relocations
	WORD   e_cparhdr;                   // Size of header in paragraphs
	WORD   e_minalloc;                  // Minimum extra paragraphs needed
	WORD   e_maxalloc;                  // Maximum extra paragraphs needed
	WORD   e_ss;                        // Initial (relative) SS value
	WORD   e_sp;                        // Initial SP value
	WORD   e_csum;                      // Checksum
	WORD   e_ip;                        // Initial IP value
	WORD   e_cs;                        // Initial (relative) CS value
	WORD   e_lfarlc;                    // File address of relocation table
	WORD   e_ovno;                      // Overlay number
	WORD   e_res[4];                    // Reserved words
	WORD   e_oemid;                     // OEM identifier (for e_oeminfo)
	WORD   e_oeminfo;                   // OEM information; e_oemid specific
	WORD   e_res2[10];                  // Reserved words
	LONG   e_lfanew;                    // File address of new exe header
} PACKED_IMAGE_DOS_HEADER;

#pragma pack(1)
typedef struct {
	WORD    Machine;
	WORD    NumberOfSections;
	DWORD   TimeDateStamp;
	DWORD   PointerToSymbolTable;
	DWORD   NumberOfSymbols;
	WORD    SizeOfOptionalHeader;
	WORD    Characteristics;
} PACKED_IMAGE_FILE_HEADER;

#pragma pack(1)
typedef struct {
	DWORD   VirtualAddress;
	DWORD   Size;
} PACKED_IMAGE_DATA_DIRECTORY;

#pragma pack(1)
typedef struct {
	//
	// Standard fields.
	//

	WORD    Magic;
	BYTE    MajorLinkerVersion;
	BYTE    MinorLinkerVersion;
	DWORD   SizeOfCode;
	DWORD   SizeOfInitializedData;
	DWORD   SizeOfUninitializedData;
	DWORD   AddressOfEntryPoint;
	DWORD   BaseOfCode;
	DWORD   BaseOfData;

	//
	// NT additional fields.
	//

	DWORD   ImageBase;
	DWORD   SectionAlignment;
	DWORD   FileAlignment;
	WORD    MajorOperatingSystemVersion;
	WORD    MinorOperatingSystemVersion;
	WORD    MajorImageVersion;
	WORD    MinorImageVersion;
	WORD    MajorSubsystemVersion;
	WORD    MinorSubsystemVersion;
	DWORD   Win32VersionValue;
	DWORD   SizeOfImage;
	DWORD   SizeOfHeaders;
	DWORD   CheckSum;
	WORD    Subsystem;
	WORD    DllCharacteristics;
	DWORD   SizeOfStackReserve;
	DWORD   SizeOfStackCommit;
	DWORD   SizeOfHeapReserve;
	DWORD   SizeOfHeapCommit;
	DWORD   LoaderFlags;
	DWORD   NumberOfRvaAndSizes;
	PACKED_IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
} PACKED_IMAGE_OPTIONAL_HEADER;

#define LOW_INTEGRITY_SDDL_SACL_W		L"S:(ML;;NW;;;LW)"

#pragma pack(1)
typedef struct {
	BYTE    Name[IMAGE_SIZEOF_SHORT_NAME];
	union {
			DWORD   PhysicalAddress;
			DWORD   VirtualSize;
	} Misc;
	DWORD   VirtualAddress;
	DWORD   SizeOfRawData;
	DWORD   PointerToRawData;
	DWORD   PointerToRelocations;
	DWORD   PointerToLinenumbers;
	WORD    NumberOfRelocations;
	WORD    NumberOfLinenumbers;
	DWORD   Characteristics;
} PACKED_IMAGE_SECTION_HEADER;

#pragma pack(1)
typedef struct {
	DWORD Signature;
	PACKED_IMAGE_FILE_HEADER FileHeader;
	PACKED_IMAGE_OPTIONAL_HEADER OptionalHeader;
} PACKED_IMAGE_NT_HEADERS;

class Utils
{
public:
	static const size_t ARBITRARY_STRING_SIZE_LIMIT;

	enum WinClientVer
	{
		Unknown = 0,
		Windows2000,
		WindowsXP,
		WindowsVista,
		Windows7
	};

	static char *MbFromWideChar(WCHAR *pwszWideChar, UINT codepage = CP_UTF8);
	static WCHAR *WcFromMultiByte(char *pszMultiByte, UINT codepage = CP_UTF8);
	static long Resolve(char *pszDnsHost);
	static int TrimCount(char *psz);
	static DWORD DwordSwapEndian(DWORD dw);
	static WORD WordSwapEndian(WORD w);
	static BOOL ValidateSslCertificate(char *pszHost, INTERNET_CERTIFICATE_INFO ici);
	static size_t GetRandomInteger(size_t min, size_t max);
	static BOOL GetRandomBytes(BYTE *pBuf, size_t cbBuf);
	static WCHAR *GetRegistryString(HKEY hKey, WCHAR *pwszRegPath, WCHAR *pwszKeyName);
	__declspec(noinline) static DWORD GetPageProtectionMinusExecutionFlag(DWORD flProtect, BOOL bDllMem = FALSE);
	static int SetChangeProtectThread(BOOL bProtect);
	static int compare(const void *elem1, const void *elem2);
	static unsigned int RSHash(char* str, unsigned int len);
	static WCHAR *wcsistr(WCHAR *pwszString, WCHAR *pwszSearch, size_t cchString = -1);
	static char *stristr(char *pszString, char *pszSearch, size_t cchString = -1);
	static std::wstring wstringlower(std::wstring wstr);
	/*__forceinline*/ __declspec(noinline) static DWORD GetReturnAddress(int frame);
	static BOOL ContainsHtmlMetacharacters(std::string strCheck);
	static BOOL GetUrlParams(std::string strUrl, std::multimap<std::string, std::string>& mmapUrlParams);
	static std::string UrlUnescape(std::string strUrl, BOOL bSupportUnicode, BOOL& bError, char chSignifier = '%');
	static BOOL GetProcessName(std::string& strProcName, HANDLE hProcess = NULL);
	static std::string ClsidToString(CLSID& rclsid);
	static std::wstring GetRegInstallPath();
	static BOOL CreateEmptyFile(const WCHAR *pwszPath);
	static BOOL OpenEmptyFile(const WCHAR *pwszPath);
	static BOOL DeleteEmptyFile(const WCHAR *pwszPath);
	static WinClientVer GetWinVersion();
	static DWORD ThreadIdFromHandle(HANDLE hThread);
	static BOOL ThreadGetStackRegion(HANDLE hThread, PVOID *pvStackBase, PVOID *pvStackTop);
	static BOOL IsBadReadPtr(LPVOID lpvPtr, DWORD cbPtr);
	//static BOOL IsBadWritePtr(LPVOID lpvPtr, DWORD cbPtr);
	static BOOL Is64BitExecutable(std::string strPath);
	static BOOL Is64BitProcess(DWORD dwPID);
	static BOOL GetNullDacl(SECURITY_ATTRIBUTES *pSA, SECURITY_DESCRIPTOR *pSD);
	static BOOL IsContainedInPipeSeparatedList(WCHAR *pwszList, WCHAR *pwszItem);
	static BOOL GetExecutableNameFromParams(std::wstring wstrProcess, std::wstring wstrParameters, std::wstring& wstrExeName);
	static BOOL IsMemMappedOrModule(LPVOID lpvMem);
	static BOOL SetRegistryString(HKEY hKey, WCHAR *pwszRegPath, WCHAR *pwszName, WCHAR *pwszValue);
	static BOOL SetRegistryDword(HKEY hKey, WCHAR *pwszRegPath, WCHAR *pwszName, DWORD dwValue);
	static BOOL GetRegistryDword(HKEY hKey, WCHAR *pwszRegPath, WCHAR *pwszKeyName, DWORD *pdwResult);
	static BOOL GetDescriptionStringFile(std::wstring wstrFile, std::wstring& wstrDescription);
	static BOOL GetEnvVar(std::wstring wstrVarName, std::wstring& wstrVarValue);
	static BOOL SetEnvVar(std::wstring wstrVarName, std::wstring wstrVarValue);
	static BOOL GetCurDir(std::wstring& wstrCurDir);
	static BOOL SetObjectToLowIntegrity(HANDLE hObject, SE_OBJECT_TYPE type);
	static BOOL IsLowIntegrityProcess(HANDLE hProcess, BOOL *pboolIsLow);

	static DWORD dwFirstProtectMs;
	static BOOL bChangeProtect;

	static BOOLEAN (__stdcall *pfnRtlGenRandom)(
		__out  PVOID RandomBuffer,
		__in   ULONG RandomBufferLength
	);

	static HMODULE hmNtdll;
	
	static NTSTATUS (WINAPI *pfnNtQueryInformationThread)(
		__in       HANDLE ThreadHandle,
		__in       /*THREADINFOCLASS*/ DWORD ThreadInformationClass,
		__inout    PVOID ThreadInformation,
		__in       ULONG ThreadInformationLength,
		__out_opt  PULONG ReturnLength
	);
	
	static NTSTATUS (NTAPI *pfnNtReadVirtualMemory)(
		__in       HANDLE ProcessHandle,
		__in       PVOID  BaseAddress,
		__out      PVOID  Buffer,
		__in       ULONG  NumberOfBytesToRead,
		__out_opt  PULONG NumberOfBytesReaded
	);

	static UINT (WINAPI *pfnGetSystemWow64Directory)(
	  __out  LPTSTR lpBuffer,
	  __in   UINT uSize
	);

	static BOOL (WINAPI *pfnIsWow64Process)(
	  __in		HANDLE hProcess,
	  __out		PBOOL Wow64Process
	);

	static void (WINAPI *pfnGetNativeSystemInfo)(
		__out  LPSYSTEM_INFO lpSystemInfo
	);

	static DWORD (APIENTRY *pfnGetFileVersionInfoSizeExW)(__in DWORD dwFlags, __in LPCWSTR lpwstrFilename, __out LPDWORD lpdwHandle);
	
	static BOOL (APIENTRY *pfnGetFileVersionInfoExW)(__in DWORD dwFlags,
                                    __in LPCWSTR lpwstrFilename,
                                    __reserved DWORD dwHandle,
                                    __in DWORD dwLen,
                                    __out_bcount(dwLen) LPVOID lpData);
	
	static BOOL (APIENTRY *pfnVerQueryValueW)(
			__in LPCVOID pBlock,
			__in LPCWSTR lpSubBlock,
			LPVOID * lplpBuffer,
			__out PUINT puLen
			);
};