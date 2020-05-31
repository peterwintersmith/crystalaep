// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]


/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0555 */
/* at Wed Jul 11 23:01:08 2012
 */
/* Compiler settings for ProcTracker.idl:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 7.00.0555 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__


#ifndef __ProcTracker_h_h__
#define __ProcTracker_h_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __IProcTracker_INTERFACE_DEFINED__
#define __IProcTracker_INTERFACE_DEFINED__

/* interface IProcTracker */
/* [version][uuid] */ 

BOOL AddProcess( 
    /* [in] */ handle_t IDL_handle,
    /* [in] */ DWORD dwProcessID);

DWORD GetProcessIDs( 
    /* [in] */ handle_t IDL_handle,
    /* [in] */ DWORD cUniqueProcIDs,
    /* [size_is][size_is][unique][out][in] */ unsigned char rg_pszUniqueProcID[  ][ 512 ]);

void Stop( 
    /* [in] */ handle_t IDL_handle);

BOOL CreateProtectFileProcess( 
    /* [in] */ handle_t IDL_handle,
    DWORD dwPID);

BOOL CheckProtectFileProcess( 
    /* [in] */ handle_t IDL_handle,
    DWORD dwPID);

BOOL AddExeToWhitelist( 
    /* [in] */ handle_t IDL_handle,
    /* [string][in] */ WCHAR *pwszExeName);



extern RPC_IF_HANDLE IProcTracker_v1_0_c_ifspec;
extern RPC_IF_HANDLE IProcTracker_v1_0_s_ifspec;
#endif /* __IProcTracker_INTERFACE_DEFINED__ */

#ifndef __IEventWriterRpc_INTERFACE_DEFINED__
#define __IEventWriterRpc_INTERFACE_DEFINED__

/* interface IEventWriterRpc */
/* [version][uuid] */ 

UINT WriteEventInternal( 
    /* [in] */ handle_t IDL_handle,
    UINT hProcess,
    /* [string][unique][in] */ WCHAR *pwszProcessName,
    UINT repos,
    UINT risk,
    /* [string][in] */ WCHAR *pwszTitle,
    /* [string][in] */ LPWSTR pwszBody);



extern RPC_IF_HANDLE IEventWriterRpc_v1_0_c_ifspec;
extern RPC_IF_HANDLE IEventWriterRpc_v1_0_s_ifspec;
#endif /* __IEventWriterRpc_INTERFACE_DEFINED__ */

#ifndef __IUiFeedback_INTERFACE_DEFINED__
#define __IUiFeedback_INTERFACE_DEFINED__

/* interface IUiFeedback */
/* [version][uuid] */ 

BOOL UserBoolFeedback( 
    /* [in] */ handle_t IDL_handle,
    /* [in] */ UINT style,
    /* [string][in] */ WCHAR *pwszTitle,
    /* [string][in] */ WCHAR *pwszCaption);



extern RPC_IF_HANDLE IUiFeedback_v1_0_c_ifspec;
extern RPC_IF_HANDLE IUiFeedback_v1_0_s_ifspec;
#endif /* __IUiFeedback_INTERFACE_DEFINED__ */

#ifndef __IRealtimeLog_INTERFACE_DEFINED__
#define __IRealtimeLog_INTERFACE_DEFINED__

/* interface IRealtimeLog */
/* [version][uuid] */ 

typedef /* [public][public][public] */ struct __MIDL_IRealtimeLog_0001
    {
    DWORD dwSeverity;
    WCHAR wszMessage[ 256 ];
    } 	RealtimeLogEntry;


enum EEnabledFeatures
    {	InvalidFeature	= 0,
	ContentFilterFeature	= 1,
	APIMonitorFeature	= 2,
	ROPMonitorFeature	= 4,
	AntiSprayFeature	= 8,
	StackMonitorFeature	= 16,
	HeapMonitorFeature	= 32,
	COMMonitorFeature	= 64
    } ;
BOOL SetEnabledFeaturesProcess( 
    /* [in] */ handle_t IDL_handle,
    /* [in] */ DWORD dwProcessID,
    /* [in] */ DWORD dwFeaturesMask);

DWORD GetEnabledFeaturesProcess( 
    /* [in] */ handle_t IDL_handle,
    /* [in] */ DWORD dwProcessID);

BOOL AddEntryProcess( 
    /* [in] */ handle_t IDL_handle,
    /* [in] */ DWORD dwProcessID,
    /* [in] */ RealtimeLogEntry *pRtLogEnt);

DWORD GetProcessLogs( 
    /* [in] */ handle_t IDL_handle,
    /* [in] */ DWORD dwProcessID,
    /* [in] */ DWORD cMaxLogEntries,
    /* [size_is][out] */ RealtimeLogEntry rg_RtLogEnts[  ]);



extern RPC_IF_HANDLE IRealtimeLog_v1_0_c_ifspec;
extern RPC_IF_HANDLE IRealtimeLog_v1_0_s_ifspec;
#endif /* __IRealtimeLog_INTERFACE_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


