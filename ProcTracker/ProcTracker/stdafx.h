// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <Rpc.h>
#include <Sddl.h>
#include <Psapi.h>
#include <sddl.h>
#include <AccCtrl.h>
#include <Aclapi.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

// STL Header Files
#include <map>
#include <string>
#include <list>
#include <deque>
#include <time.h>

// ProcTracker Specific
#define BPCORE_TERMINATE_FLAG_BASE			0xdeadd0d0
#define BPCORE_TERMINATE_HEAP_CORRUPT		BPCORE_TERMINATE_FLAG_BASE + 1
#define BPCORE_TERMINATE_DOUBLE_FREE		BPCORE_TERMINATE_FLAG_BASE + 2
#define BPCORE_TERMINATE_USE_AFTER_FREE		BPCORE_TERMINATE_FLAG_BASE + 3
#define BPCORE_TERMINATE_ANTISPRAY			BPCORE_TERMINATE_FLAG_BASE + 4
#define BPCORE_TERMINATE_UNTRUSTED_CALLER	BPCORE_TERMINATE_FLAG_BASE + 5
#define BPCORE_TERMINATE_ROP_DETECTED		BPCORE_TERMINATE_FLAG_BASE + 6
#define BPCORE_TERMINATE_SEH_FAULT			BPCORE_TERMINATE_FLAG_BASE + 7

//#define DEBUG_BUILD
#define RELEASE_BUILD