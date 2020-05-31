// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

//#include "targetver.h"

#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

#define CRYSTAL_LATE_BINDING
//#define DEBUG_BUILD
#define RELEASE_BUILD

#ifndef QWORD
#define QWORD	unsigned __int64
#endif

typedef void* PPEB;
/*
typedef struct _PROCESS_BASIC_INFORMATION {
	PVOID Reserved1;
	PPEB PebBaseAddress;
	PVOID Reserved2[2];
	ULONG_PTR UniqueProcessId;
	PVOID Reserved3;
} PROCESS_BASIC_INFORMATION;

enum PROCESSINFOCLASS {
	ProcessBasicInformation = 0,
	ProcessDebugPort = 7,
	ProcessWow64Information = 26,
	ProcessImageFileName = 27
};
*/

typedef struct _THREAD_BASIC_INFORMATION {
	unsigned long ExitStatus;
	void *TebBaseAddress;
	ULONG UniqueProcessId;
	ULONG UniqueThreadId;
	unsigned long AffinityMask;
	unsigned long BasePriority;
	unsigned long DiffProcessPriority;
} THREAD_BASIC_INFORMATION;

#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)

#define BPCORE_TERMINATE_FLAG_BASE			0xdeadd0d0
#define BPCORE_TERMINATE_HEAP_CORRUPT		BPCORE_TERMINATE_FLAG_BASE + 1
#define BPCORE_TERMINATE_DOUBLE_FREE		BPCORE_TERMINATE_FLAG_BASE + 2
#define BPCORE_TERMINATE_USE_AFTER_FREE		BPCORE_TERMINATE_FLAG_BASE + 3
#define BPCORE_TERMINATE_ANTISPRAY			BPCORE_TERMINATE_FLAG_BASE + 4
#define BPCORE_TERMINATE_UNTRUSTED_CALLER	BPCORE_TERMINATE_FLAG_BASE + 5
#define BPCORE_TERMINATE_ROP_DETECTED		BPCORE_TERMINATE_FLAG_BASE + 6

#define GETESP(var)	__asm\
	{\
		__asm push eax\
		\
		__asm lea eax, [var]\
		__asm mov [eax], esp\
		\
		__asm pop eax\
	}

#define GETRETNADDR(var)	__asm\
	{\
		__asm push eax\
		__asm push ebx\
		\
		__asm lea eax, [var]\
		__asm mov ebx, [ebp + 4]\
		__asm mov [eax], ebx\
		\
		__asm pop ebx\
		__asm pop eax\
	}

// TODO: reference additional headers your program requires here
#include <WinSock2.h>
#include <UrlMon.h>
#include <NTSecAPI.h>
#include <exception>
#include <cstdio>
#include <string>
#include <deque>
//#include <detours.h>
//#include <Logger.h>
#include <WinInet.h>
#include <vector>
#include <algorithm>
#include <map>
#include <Psapi.h>
#include <xmllite.h>
#include <Shlwapi.h>
#include <list>
#include <strsafe.h>
#include <TlHelp32.h>
#include <hash_map>
#include <WinNT.h>
#include <sddl.h>
#include <AccCtrl.h>
#include <Aclapi.h>