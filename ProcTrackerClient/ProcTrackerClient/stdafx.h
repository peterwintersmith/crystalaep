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

#define CRYSTAL_LATE_BINDING
#define DEBUG_BUILD

// TODO: reference additional headers your program requires here
#include <cstdio>
#include <cstdlib>
#include <Rpc.h>
