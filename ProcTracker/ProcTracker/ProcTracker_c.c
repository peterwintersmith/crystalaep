// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]


/* this ALWAYS GENERATED file contains the RPC client stubs */


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

#if !defined(_M_IA64) && !defined(_M_AMD64)


#pragma warning( disable: 4049 )  /* more than 64k source lines */
#if _MSC_VER >= 1200
#pragma warning(push)
#endif

#pragma warning( disable: 4211 )  /* redefine extern to static */
#pragma warning( disable: 4232 )  /* dllimport identity*/
#pragma warning( disable: 4024 )  /* array to pointer mapping*/
#pragma warning( disable: 4100 ) /* unreferenced arguments in x86 call */

#pragma optimize("", off ) 

#include <string.h>

#include "ProcTracker_h.h"

#define TYPE_FORMAT_STRING_SIZE   99                                
#define PROC_FORMAT_STRING_SIZE   541                               
#define EXPR_FORMAT_STRING_SIZE   1                                 
#define TRANSMIT_AS_TABLE_SIZE    0            
#define WIRE_MARSHAL_TABLE_SIZE   0            

typedef struct _ProcTracker_MIDL_TYPE_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ TYPE_FORMAT_STRING_SIZE ];
    } ProcTracker_MIDL_TYPE_FORMAT_STRING;

typedef struct _ProcTracker_MIDL_PROC_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ PROC_FORMAT_STRING_SIZE ];
    } ProcTracker_MIDL_PROC_FORMAT_STRING;

typedef struct _ProcTracker_MIDL_EXPR_FORMAT_STRING
    {
    long          Pad;
    unsigned char  Format[ EXPR_FORMAT_STRING_SIZE ];
    } ProcTracker_MIDL_EXPR_FORMAT_STRING;


static const RPC_SYNTAX_IDENTIFIER  _RpcTransferSyntax = 
{{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}};


extern const ProcTracker_MIDL_TYPE_FORMAT_STRING ProcTracker__MIDL_TypeFormatString;
extern const ProcTracker_MIDL_PROC_FORMAT_STRING ProcTracker__MIDL_ProcFormatString;
extern const ProcTracker_MIDL_EXPR_FORMAT_STRING ProcTracker__MIDL_ExprFormatString;

#define GENERIC_BINDING_TABLE_SIZE   0            


/* Standard interface: IProcTracker, ver. 1.0,
   GUID={0x7B54806B,0xFFB4,0x4F9D,{0xB5,0x97,0xCD,0x51,0x0F,0xA0,0x94,0xB0}} */



static const RPC_CLIENT_INTERFACE IProcTracker___RpcClientInterface =
    {
    sizeof(RPC_CLIENT_INTERFACE),
    {{0x7B54806B,0xFFB4,0x4F9D,{0xB5,0x97,0xCD,0x51,0x0F,0xA0,0x94,0xB0}},{1,0}},
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    0,
    0,
    0,
    0,
    0,
    0x00000000
    };
RPC_IF_HANDLE IProcTracker_v1_0_c_ifspec = (RPC_IF_HANDLE)& IProcTracker___RpcClientInterface;

extern const MIDL_STUB_DESC IProcTracker_StubDesc;

static RPC_BINDING_HANDLE IProcTracker__MIDL_AutoBindHandle;


BOOL AddProcess( 
    /* [in] */ handle_t IDL_handle,
    /* [in] */ DWORD dwProcessID)
{

    CLIENT_CALL_RETURN _RetVal;

    _RetVal = NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&IProcTracker_StubDesc,
                  (PFORMAT_STRING) &ProcTracker__MIDL_ProcFormatString.Format[0],
                  ( unsigned char * )&IDL_handle);
    return ( BOOL  )_RetVal.Simple;
    
}


DWORD GetProcessIDs( 
    /* [in] */ handle_t IDL_handle,
    /* [in] */ DWORD cUniqueProcIDs,
    /* [size_is][size_is][unique][out][in] */ unsigned char rg_pszUniqueProcID[  ][ 512 ])
{

    CLIENT_CALL_RETURN _RetVal;

    _RetVal = NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&IProcTracker_StubDesc,
                  (PFORMAT_STRING) &ProcTracker__MIDL_ProcFormatString.Format[40],
                  ( unsigned char * )&IDL_handle);
    return ( DWORD  )_RetVal.Simple;
    
}


void Stop( 
    /* [in] */ handle_t IDL_handle)
{

    NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&IProcTracker_StubDesc,
                  (PFORMAT_STRING) &ProcTracker__MIDL_ProcFormatString.Format[86],
                  ( unsigned char * )&IDL_handle);
    
}


BOOL CreateProtectFileProcess( 
    /* [in] */ handle_t IDL_handle,
    DWORD dwPID)
{

    CLIENT_CALL_RETURN _RetVal;

    _RetVal = NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&IProcTracker_StubDesc,
                  (PFORMAT_STRING) &ProcTracker__MIDL_ProcFormatString.Format[114],
                  ( unsigned char * )&IDL_handle);
    return ( BOOL  )_RetVal.Simple;
    
}


BOOL CheckProtectFileProcess( 
    /* [in] */ handle_t IDL_handle,
    DWORD dwPID)
{

    CLIENT_CALL_RETURN _RetVal;

    _RetVal = NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&IProcTracker_StubDesc,
                  (PFORMAT_STRING) &ProcTracker__MIDL_ProcFormatString.Format[154],
                  ( unsigned char * )&IDL_handle);
    return ( BOOL  )_RetVal.Simple;
    
}


BOOL AddExeToWhitelist( 
    /* [in] */ handle_t IDL_handle,
    /* [string][in] */ WCHAR *pwszExeName)
{

    CLIENT_CALL_RETURN _RetVal;

    _RetVal = NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&IProcTracker_StubDesc,
                  (PFORMAT_STRING) &ProcTracker__MIDL_ProcFormatString.Format[194],
                  ( unsigned char * )&IDL_handle);
    return ( BOOL  )_RetVal.Simple;
    
}


/* Standard interface: IEventWriterRpc, ver. 1.0,
   GUID={0x6FD48920,0x3FA9,0x449B,{0xB5,0x1B,0xFC,0x66,0x79,0x63,0x0C,0x58}} */



static const RPC_CLIENT_INTERFACE IEventWriterRpc___RpcClientInterface =
    {
    sizeof(RPC_CLIENT_INTERFACE),
    {{0x6FD48920,0x3FA9,0x449B,{0xB5,0x1B,0xFC,0x66,0x79,0x63,0x0C,0x58}},{1,0}},
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    0,
    0,
    0,
    0,
    0,
    0x00000000
    };
RPC_IF_HANDLE IEventWriterRpc_v1_0_c_ifspec = (RPC_IF_HANDLE)& IEventWriterRpc___RpcClientInterface;

extern const MIDL_STUB_DESC IEventWriterRpc_StubDesc;

static RPC_BINDING_HANDLE IEventWriterRpc__MIDL_AutoBindHandle;


UINT WriteEventInternal( 
    /* [in] */ handle_t IDL_handle,
    UINT hProcess,
    /* [string][unique][in] */ WCHAR *pwszProcessName,
    UINT repos,
    UINT risk,
    /* [string][in] */ WCHAR *pwszTitle,
    /* [string][in] */ LPWSTR pwszBody)
{

    CLIENT_CALL_RETURN _RetVal;

    _RetVal = NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&IEventWriterRpc_StubDesc,
                  (PFORMAT_STRING) &ProcTracker__MIDL_ProcFormatString.Format[234],
                  ( unsigned char * )&IDL_handle);
    return ( UINT  )_RetVal.Simple;
    
}


/* Standard interface: IUiFeedback, ver. 1.0,
   GUID={0x4B387FFE,0x9E55,0x47CF,{0x87,0x5C,0x1E,0x9F,0xCF,0x1C,0x08,0x3B}} */



static const RPC_CLIENT_INTERFACE IUiFeedback___RpcClientInterface =
    {
    sizeof(RPC_CLIENT_INTERFACE),
    {{0x4B387FFE,0x9E55,0x47CF,{0x87,0x5C,0x1E,0x9F,0xCF,0x1C,0x08,0x3B}},{1,0}},
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    0,
    0,
    0,
    0,
    0,
    0x00000000
    };
RPC_IF_HANDLE IUiFeedback_v1_0_c_ifspec = (RPC_IF_HANDLE)& IUiFeedback___RpcClientInterface;

extern const MIDL_STUB_DESC IUiFeedback_StubDesc;

static RPC_BINDING_HANDLE IUiFeedback__MIDL_AutoBindHandle;


BOOL UserBoolFeedback( 
    /* [in] */ handle_t IDL_handle,
    /* [in] */ UINT style,
    /* [string][in] */ WCHAR *pwszTitle,
    /* [string][in] */ WCHAR *pwszCaption)
{

    CLIENT_CALL_RETURN _RetVal;

    _RetVal = NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&IUiFeedback_StubDesc,
                  (PFORMAT_STRING) &ProcTracker__MIDL_ProcFormatString.Format[304],
                  ( unsigned char * )&IDL_handle);
    return ( BOOL  )_RetVal.Simple;
    
}


/* Standard interface: IRealtimeLog, ver. 1.0,
   GUID={0x4E63194F,0xF1B8,0x4E37,{0xA5,0xEC,0xCC,0x88,0x26,0xE5,0x3F,0x2D}} */



static const RPC_CLIENT_INTERFACE IRealtimeLog___RpcClientInterface =
    {
    sizeof(RPC_CLIENT_INTERFACE),
    {{0x4E63194F,0xF1B8,0x4E37,{0xA5,0xEC,0xCC,0x88,0x26,0xE5,0x3F,0x2D}},{1,0}},
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    0,
    0,
    0,
    0,
    0,
    0x00000000
    };
RPC_IF_HANDLE IRealtimeLog_v1_0_c_ifspec = (RPC_IF_HANDLE)& IRealtimeLog___RpcClientInterface;

extern const MIDL_STUB_DESC IRealtimeLog_StubDesc;

static RPC_BINDING_HANDLE IRealtimeLog__MIDL_AutoBindHandle;


BOOL SetEnabledFeaturesProcess( 
    /* [in] */ handle_t IDL_handle,
    /* [in] */ DWORD dwProcessID,
    /* [in] */ DWORD dwFeaturesMask)
{

    CLIENT_CALL_RETURN _RetVal;

    _RetVal = NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&IRealtimeLog_StubDesc,
                  (PFORMAT_STRING) &ProcTracker__MIDL_ProcFormatString.Format[356],
                  ( unsigned char * )&IDL_handle);
    return ( BOOL  )_RetVal.Simple;
    
}


DWORD GetEnabledFeaturesProcess( 
    /* [in] */ handle_t IDL_handle,
    /* [in] */ DWORD dwProcessID)
{

    CLIENT_CALL_RETURN _RetVal;

    _RetVal = NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&IRealtimeLog_StubDesc,
                  (PFORMAT_STRING) &ProcTracker__MIDL_ProcFormatString.Format[402],
                  ( unsigned char * )&IDL_handle);
    return ( DWORD  )_RetVal.Simple;
    
}


BOOL AddEntryProcess( 
    /* [in] */ handle_t IDL_handle,
    /* [in] */ DWORD dwProcessID,
    /* [in] */ RealtimeLogEntry *pRtLogEnt)
{

    CLIENT_CALL_RETURN _RetVal;

    _RetVal = NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&IRealtimeLog_StubDesc,
                  (PFORMAT_STRING) &ProcTracker__MIDL_ProcFormatString.Format[442],
                  ( unsigned char * )&IDL_handle);
    return ( BOOL  )_RetVal.Simple;
    
}


DWORD GetProcessLogs( 
    /* [in] */ handle_t IDL_handle,
    /* [in] */ DWORD dwProcessID,
    /* [in] */ DWORD cMaxLogEntries,
    /* [size_is][out] */ RealtimeLogEntry rg_RtLogEnts[  ])
{

    CLIENT_CALL_RETURN _RetVal;

    _RetVal = NdrClientCall2(
                  ( PMIDL_STUB_DESC  )&IRealtimeLog_StubDesc,
                  (PFORMAT_STRING) &ProcTracker__MIDL_ProcFormatString.Format[488],
                  ( unsigned char * )&IDL_handle);
    return ( DWORD  )_RetVal.Simple;
    
}


#if !defined(__RPC_WIN32__)
#error  Invalid build platform for this stub.
#endif

#if !(TARGET_IS_NT50_OR_LATER)
#error You need Windows 2000 or later to run this stub because it uses these features:
#error   /robust command line switch.
#error However, your C/C++ compilation flags indicate you intend to run this app on earlier systems.
#error This app will fail with the RPC_X_WRONG_STUB_VERSION error.
#endif


static const ProcTracker_MIDL_PROC_FORMAT_STRING ProcTracker__MIDL_ProcFormatString =
    {
        0,
        {

	/* Procedure AddProcess */

			0x0,		/* 0 */
			0x48,		/* Old Flags:  */
/*  2 */	NdrFcLong( 0x0 ),	/* 0 */
/*  6 */	NdrFcShort( 0x0 ),	/* 0 */
/*  8 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 10 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
/* 12 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 14 */	NdrFcShort( 0x8 ),	/* 8 */
/* 16 */	NdrFcShort( 0x8 ),	/* 8 */
/* 18 */	0x44,		/* Oi2 Flags:  has return, has ext, */
			0x2,		/* 2 */
/* 20 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 22 */	NdrFcShort( 0x0 ),	/* 0 */
/* 24 */	NdrFcShort( 0x0 ),	/* 0 */
/* 26 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter IDL_handle */

/* 28 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 30 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 32 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter dwProcessID */

/* 34 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 36 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 38 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure GetProcessIDs */


	/* Return value */

/* 40 */	0x0,		/* 0 */
			0x48,		/* Old Flags:  */
/* 42 */	NdrFcLong( 0x0 ),	/* 0 */
/* 46 */	NdrFcShort( 0x1 ),	/* 1 */
/* 48 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 50 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
/* 52 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 54 */	NdrFcShort( 0x8 ),	/* 8 */
/* 56 */	NdrFcShort( 0x8 ),	/* 8 */
/* 58 */	0x47,		/* Oi2 Flags:  srv must size, clt must size, has return, has ext, */
			0x3,		/* 3 */
/* 60 */	0x8,		/* 8 */
			0x7,		/* Ext Flags:  new corr desc, clt corr check, srv corr check, */
/* 62 */	NdrFcShort( 0x1 ),	/* 1 */
/* 64 */	NdrFcShort( 0x1 ),	/* 1 */
/* 66 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter IDL_handle */

/* 68 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 70 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 72 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter cUniqueProcIDs */

/* 74 */	NdrFcShort( 0x1b ),	/* Flags:  must size, must free, in, out, */
/* 76 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 78 */	NdrFcShort( 0x14 ),	/* Type Offset=20 */

	/* Parameter rg_pszUniqueProcID */

/* 80 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 82 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 84 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure Stop */


	/* Return value */

/* 86 */	0x0,		/* 0 */
			0x48,		/* Old Flags:  */
/* 88 */	NdrFcLong( 0x0 ),	/* 0 */
/* 92 */	NdrFcShort( 0x2 ),	/* 2 */
/* 94 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 96 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
/* 98 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 100 */	NdrFcShort( 0x0 ),	/* 0 */
/* 102 */	NdrFcShort( 0x0 ),	/* 0 */
/* 104 */	0x40,		/* Oi2 Flags:  has ext, */
			0x0,		/* 0 */
/* 106 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 108 */	NdrFcShort( 0x0 ),	/* 0 */
/* 110 */	NdrFcShort( 0x0 ),	/* 0 */
/* 112 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Procedure CreateProtectFileProcess */


	/* Parameter IDL_handle */

/* 114 */	0x0,		/* 0 */
			0x48,		/* Old Flags:  */
/* 116 */	NdrFcLong( 0x0 ),	/* 0 */
/* 120 */	NdrFcShort( 0x3 ),	/* 3 */
/* 122 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 124 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
/* 126 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 128 */	NdrFcShort( 0x8 ),	/* 8 */
/* 130 */	NdrFcShort( 0x8 ),	/* 8 */
/* 132 */	0x44,		/* Oi2 Flags:  has return, has ext, */
			0x2,		/* 2 */
/* 134 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 136 */	NdrFcShort( 0x0 ),	/* 0 */
/* 138 */	NdrFcShort( 0x0 ),	/* 0 */
/* 140 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter IDL_handle */

/* 142 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 144 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 146 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter dwPID */

/* 148 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 150 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 152 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure CheckProtectFileProcess */


	/* Return value */

/* 154 */	0x0,		/* 0 */
			0x48,		/* Old Flags:  */
/* 156 */	NdrFcLong( 0x0 ),	/* 0 */
/* 160 */	NdrFcShort( 0x4 ),	/* 4 */
/* 162 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 164 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
/* 166 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 168 */	NdrFcShort( 0x8 ),	/* 8 */
/* 170 */	NdrFcShort( 0x8 ),	/* 8 */
/* 172 */	0x44,		/* Oi2 Flags:  has return, has ext, */
			0x2,		/* 2 */
/* 174 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 176 */	NdrFcShort( 0x0 ),	/* 0 */
/* 178 */	NdrFcShort( 0x0 ),	/* 0 */
/* 180 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter IDL_handle */

/* 182 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 184 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 186 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter dwPID */

/* 188 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 190 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 192 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure AddExeToWhitelist */


	/* Return value */

/* 194 */	0x0,		/* 0 */
			0x48,		/* Old Flags:  */
/* 196 */	NdrFcLong( 0x0 ),	/* 0 */
/* 200 */	NdrFcShort( 0x5 ),	/* 5 */
/* 202 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 204 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
/* 206 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 208 */	NdrFcShort( 0x0 ),	/* 0 */
/* 210 */	NdrFcShort( 0x8 ),	/* 8 */
/* 212 */	0x46,		/* Oi2 Flags:  clt must size, has return, has ext, */
			0x2,		/* 2 */
/* 214 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 216 */	NdrFcShort( 0x0 ),	/* 0 */
/* 218 */	NdrFcShort( 0x0 ),	/* 0 */
/* 220 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter IDL_handle */

/* 222 */	NdrFcShort( 0x10b ),	/* Flags:  must size, must free, in, simple ref, */
/* 224 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 226 */	NdrFcShort( 0x30 ),	/* Type Offset=48 */

	/* Parameter pwszExeName */

/* 228 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 230 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 232 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure WriteEventInternal */


	/* Return value */

/* 234 */	0x0,		/* 0 */
			0x48,		/* Old Flags:  */
/* 236 */	NdrFcLong( 0x0 ),	/* 0 */
/* 240 */	NdrFcShort( 0x0 ),	/* 0 */
/* 242 */	NdrFcShort( 0x20 ),	/* x86 Stack size/offset = 32 */
/* 244 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
/* 246 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 248 */	NdrFcShort( 0x18 ),	/* 24 */
/* 250 */	NdrFcShort( 0x8 ),	/* 8 */
/* 252 */	0x46,		/* Oi2 Flags:  clt must size, has return, has ext, */
			0x7,		/* 7 */
/* 254 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 256 */	NdrFcShort( 0x0 ),	/* 0 */
/* 258 */	NdrFcShort( 0x0 ),	/* 0 */
/* 260 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter IDL_handle */

/* 262 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 264 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 266 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter hProcess */

/* 268 */	NdrFcShort( 0xb ),	/* Flags:  must size, must free, in, */
/* 270 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 272 */	NdrFcShort( 0x32 ),	/* Type Offset=50 */

	/* Parameter pwszProcessName */

/* 274 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 276 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 278 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter repos */

/* 280 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 282 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 284 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter risk */

/* 286 */	NdrFcShort( 0x10b ),	/* Flags:  must size, must free, in, simple ref, */
/* 288 */	NdrFcShort( 0x14 ),	/* x86 Stack size/offset = 20 */
/* 290 */	NdrFcShort( 0x30 ),	/* Type Offset=48 */

	/* Parameter pwszTitle */

/* 292 */	NdrFcShort( 0x10b ),	/* Flags:  must size, must free, in, simple ref, */
/* 294 */	NdrFcShort( 0x18 ),	/* x86 Stack size/offset = 24 */
/* 296 */	NdrFcShort( 0x30 ),	/* Type Offset=48 */

	/* Parameter pwszBody */

/* 298 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 300 */	NdrFcShort( 0x1c ),	/* x86 Stack size/offset = 28 */
/* 302 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure UserBoolFeedback */


	/* Return value */

/* 304 */	0x0,		/* 0 */
			0x48,		/* Old Flags:  */
/* 306 */	NdrFcLong( 0x0 ),	/* 0 */
/* 310 */	NdrFcShort( 0x0 ),	/* 0 */
/* 312 */	NdrFcShort( 0x14 ),	/* x86 Stack size/offset = 20 */
/* 314 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
/* 316 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 318 */	NdrFcShort( 0x8 ),	/* 8 */
/* 320 */	NdrFcShort( 0x8 ),	/* 8 */
/* 322 */	0x46,		/* Oi2 Flags:  clt must size, has return, has ext, */
			0x4,		/* 4 */
/* 324 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 326 */	NdrFcShort( 0x0 ),	/* 0 */
/* 328 */	NdrFcShort( 0x0 ),	/* 0 */
/* 330 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter IDL_handle */

/* 332 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 334 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 336 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter style */

/* 338 */	NdrFcShort( 0x10b ),	/* Flags:  must size, must free, in, simple ref, */
/* 340 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 342 */	NdrFcShort( 0x30 ),	/* Type Offset=48 */

	/* Parameter pwszTitle */

/* 344 */	NdrFcShort( 0x10b ),	/* Flags:  must size, must free, in, simple ref, */
/* 346 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 348 */	NdrFcShort( 0x30 ),	/* Type Offset=48 */

	/* Parameter pwszCaption */

/* 350 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 352 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 354 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure SetEnabledFeaturesProcess */


	/* Return value */

/* 356 */	0x0,		/* 0 */
			0x48,		/* Old Flags:  */
/* 358 */	NdrFcLong( 0x0 ),	/* 0 */
/* 362 */	NdrFcShort( 0x0 ),	/* 0 */
/* 364 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 366 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
/* 368 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 370 */	NdrFcShort( 0x10 ),	/* 16 */
/* 372 */	NdrFcShort( 0x8 ),	/* 8 */
/* 374 */	0x44,		/* Oi2 Flags:  has return, has ext, */
			0x3,		/* 3 */
/* 376 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 378 */	NdrFcShort( 0x0 ),	/* 0 */
/* 380 */	NdrFcShort( 0x0 ),	/* 0 */
/* 382 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter IDL_handle */

/* 384 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 386 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 388 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter dwProcessID */

/* 390 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 392 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 394 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter dwFeaturesMask */

/* 396 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 398 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 400 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure GetEnabledFeaturesProcess */


	/* Return value */

/* 402 */	0x0,		/* 0 */
			0x48,		/* Old Flags:  */
/* 404 */	NdrFcLong( 0x0 ),	/* 0 */
/* 408 */	NdrFcShort( 0x1 ),	/* 1 */
/* 410 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 412 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
/* 414 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 416 */	NdrFcShort( 0x8 ),	/* 8 */
/* 418 */	NdrFcShort( 0x8 ),	/* 8 */
/* 420 */	0x44,		/* Oi2 Flags:  has return, has ext, */
			0x2,		/* 2 */
/* 422 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 424 */	NdrFcShort( 0x0 ),	/* 0 */
/* 426 */	NdrFcShort( 0x0 ),	/* 0 */
/* 428 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter IDL_handle */

/* 430 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 432 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 434 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter dwProcessID */

/* 436 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 438 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 440 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure AddEntryProcess */


	/* Return value */

/* 442 */	0x0,		/* 0 */
			0x48,		/* Old Flags:  */
/* 444 */	NdrFcLong( 0x0 ),	/* 0 */
/* 448 */	NdrFcShort( 0x2 ),	/* 2 */
/* 450 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 452 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
/* 454 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 456 */	NdrFcShort( 0x8 ),	/* 8 */
/* 458 */	NdrFcShort( 0x8 ),	/* 8 */
/* 460 */	0x46,		/* Oi2 Flags:  clt must size, has return, has ext, */
			0x3,		/* 3 */
/* 462 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 464 */	NdrFcShort( 0x0 ),	/* 0 */
/* 466 */	NdrFcShort( 0x0 ),	/* 0 */
/* 468 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter IDL_handle */

/* 470 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 472 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 474 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter dwProcessID */

/* 476 */	NdrFcShort( 0x10b ),	/* Flags:  must size, must free, in, simple ref, */
/* 478 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 480 */	NdrFcShort( 0x3e ),	/* Type Offset=62 */

	/* Parameter pRtLogEnt */

/* 482 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 484 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 486 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure GetProcessLogs */


	/* Return value */

/* 488 */	0x0,		/* 0 */
			0x48,		/* Old Flags:  */
/* 490 */	NdrFcLong( 0x0 ),	/* 0 */
/* 494 */	NdrFcShort( 0x3 ),	/* 3 */
/* 496 */	NdrFcShort( 0x14 ),	/* x86 Stack size/offset = 20 */
/* 498 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
/* 500 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 502 */	NdrFcShort( 0x10 ),	/* 16 */
/* 504 */	NdrFcShort( 0x8 ),	/* 8 */
/* 506 */	0x45,		/* Oi2 Flags:  srv must size, has return, has ext, */
			0x4,		/* 4 */
/* 508 */	0x8,		/* 8 */
			0x3,		/* Ext Flags:  new corr desc, clt corr check, */
/* 510 */	NdrFcShort( 0x1 ),	/* 1 */
/* 512 */	NdrFcShort( 0x0 ),	/* 0 */
/* 514 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter IDL_handle */

/* 516 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 518 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 520 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter dwProcessID */

/* 522 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 524 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 526 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter cMaxLogEntries */

/* 528 */	NdrFcShort( 0x13 ),	/* Flags:  must size, must free, out, */
/* 530 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 532 */	NdrFcShort( 0x4c ),	/* Type Offset=76 */

	/* Parameter rg_RtLogEnts */

/* 534 */	NdrFcShort( 0x70 ),	/* Flags:  out, return, base type, */
/* 536 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 538 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

			0x0
        }
    };

static const ProcTracker_MIDL_TYPE_FORMAT_STRING ProcTracker__MIDL_TypeFormatString =
    {
        0,
        {
			NdrFcShort( 0x0 ),	/* 0 */
/*  2 */	
			0x21,		/* FC_BOGUS_ARRAY */
			0x0,		/* 0 */
/*  4 */	NdrFcShort( 0x0 ),	/* 0 */
/*  6 */	0x40,		/* Corr desc:  constant, val=512 */
			0x0,		/* 0 */
/*  8 */	NdrFcShort( 0x200 ),	/* 512 */
/* 10 */	NdrFcShort( 0x1 ),	/* Corr flags:  early, */
/* 12 */	NdrFcLong( 0xffffffff ),	/* -1 */
/* 16 */	NdrFcShort( 0x0 ),	/* Corr flags:  */
/* 18 */	0x2,		/* FC_CHAR */
			0x5b,		/* FC_END */
/* 20 */	
			0x12,		/* FC_UP */
			0x0,		/* 0 */
/* 22 */	NdrFcShort( 0x2 ),	/* Offset= 2 (24) */
/* 24 */	
			0x21,		/* FC_BOGUS_ARRAY */
			0x0,		/* 0 */
/* 26 */	NdrFcShort( 0x0 ),	/* 0 */
/* 28 */	0x29,		/* Corr desc:  parameter, FC_ULONG */
			0x0,		/*  */
/* 30 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 32 */	NdrFcShort( 0x1 ),	/* Corr flags:  early, */
/* 34 */	NdrFcLong( 0xffffffff ),	/* -1 */
/* 38 */	NdrFcShort( 0x0 ),	/* Corr flags:  */
/* 40 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 42 */	NdrFcShort( 0xffd8 ),	/* Offset= -40 (2) */
/* 44 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */
/* 46 */	
			0x11, 0x8,	/* FC_RP [simple_pointer] */
/* 48 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 50 */	
			0x12, 0x8,	/* FC_UP [simple_pointer] */
/* 52 */	
			0x25,		/* FC_C_WSTRING */
			0x5c,		/* FC_PAD */
/* 54 */	
			0x11, 0x0,	/* FC_RP */
/* 56 */	NdrFcShort( 0x6 ),	/* Offset= 6 (62) */
/* 58 */	
			0x29,		/* FC_WSTRING */
			0x5c,		/* FC_PAD */
/* 60 */	NdrFcShort( 0x100 ),	/* 256 */
/* 62 */	
			0x1a,		/* FC_BOGUS_STRUCT */
			0x3,		/* 3 */
/* 64 */	NdrFcShort( 0x204 ),	/* 516 */
/* 66 */	NdrFcShort( 0x0 ),	/* 0 */
/* 68 */	NdrFcShort( 0x0 ),	/* Offset= 0 (68) */
/* 70 */	0x8,		/* FC_LONG */
			0x4c,		/* FC_EMBEDDED_COMPLEX */
/* 72 */	0x0,		/* 0 */
			NdrFcShort( 0xfff1 ),	/* Offset= -15 (58) */
			0x5b,		/* FC_END */
/* 76 */	
			0x21,		/* FC_BOGUS_ARRAY */
			0x3,		/* 3 */
/* 78 */	NdrFcShort( 0x0 ),	/* 0 */
/* 80 */	0x29,		/* Corr desc:  parameter, FC_ULONG */
			0x0,		/*  */
/* 82 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 84 */	NdrFcShort( 0x1 ),	/* Corr flags:  early, */
/* 86 */	NdrFcLong( 0xffffffff ),	/* -1 */
/* 90 */	NdrFcShort( 0x0 ),	/* Corr flags:  */
/* 92 */	0x4c,		/* FC_EMBEDDED_COMPLEX */
			0x0,		/* 0 */
/* 94 */	NdrFcShort( 0xffe0 ),	/* Offset= -32 (62) */
/* 96 */	0x5c,		/* FC_PAD */
			0x5b,		/* FC_END */

			0x0
        }
    };

static const unsigned short IProcTracker_FormatStringOffsetTable[] =
    {
    0,
    40,
    86,
    114,
    154,
    194
    };


static const MIDL_STUB_DESC IProcTracker_StubDesc = 
    {
    (void *)& IProcTracker___RpcClientInterface,
    MIDL_user_allocate,
    MIDL_user_free,
    &IProcTracker__MIDL_AutoBindHandle,
    0,
    0,
    0,
    0,
    ProcTracker__MIDL_TypeFormatString.Format,
    1, /* -error bounds_check flag */
    0x50002, /* Ndr library version */
    0,
    0x700022b, /* MIDL Version 7.0.555 */
    0,
    0,
    0,  /* notify & notify_flag routine table */
    0x1, /* MIDL flag */
    0, /* cs routines */
    0,   /* proxy/server info */
    0
    };

static const unsigned short IEventWriterRpc_FormatStringOffsetTable[] =
    {
    234
    };


static const MIDL_STUB_DESC IEventWriterRpc_StubDesc = 
    {
    (void *)& IEventWriterRpc___RpcClientInterface,
    MIDL_user_allocate,
    MIDL_user_free,
    &IEventWriterRpc__MIDL_AutoBindHandle,
    0,
    0,
    0,
    0,
    ProcTracker__MIDL_TypeFormatString.Format,
    1, /* -error bounds_check flag */
    0x50002, /* Ndr library version */
    0,
    0x700022b, /* MIDL Version 7.0.555 */
    0,
    0,
    0,  /* notify & notify_flag routine table */
    0x1, /* MIDL flag */
    0, /* cs routines */
    0,   /* proxy/server info */
    0
    };

static const unsigned short IUiFeedback_FormatStringOffsetTable[] =
    {
    304
    };


static const MIDL_STUB_DESC IUiFeedback_StubDesc = 
    {
    (void *)& IUiFeedback___RpcClientInterface,
    MIDL_user_allocate,
    MIDL_user_free,
    &IUiFeedback__MIDL_AutoBindHandle,
    0,
    0,
    0,
    0,
    ProcTracker__MIDL_TypeFormatString.Format,
    1, /* -error bounds_check flag */
    0x50002, /* Ndr library version */
    0,
    0x700022b, /* MIDL Version 7.0.555 */
    0,
    0,
    0,  /* notify & notify_flag routine table */
    0x1, /* MIDL flag */
    0, /* cs routines */
    0,   /* proxy/server info */
    0
    };

static const unsigned short IRealtimeLog_FormatStringOffsetTable[] =
    {
    356,
    402,
    442,
    488
    };


static const MIDL_STUB_DESC IRealtimeLog_StubDesc = 
    {
    (void *)& IRealtimeLog___RpcClientInterface,
    MIDL_user_allocate,
    MIDL_user_free,
    &IRealtimeLog__MIDL_AutoBindHandle,
    0,
    0,
    0,
    0,
    ProcTracker__MIDL_TypeFormatString.Format,
    1, /* -error bounds_check flag */
    0x50002, /* Ndr library version */
    0,
    0x700022b, /* MIDL Version 7.0.555 */
    0,
    0,
    0,  /* notify & notify_flag routine table */
    0x1, /* MIDL flag */
    0, /* cs routines */
    0,   /* proxy/server info */
    0
    };
#pragma optimize("", on )
#if _MSC_VER >= 1200
#pragma warning(pop)
#endif


#endif /* !defined(_M_IA64) && !defined(_M_AMD64)*/

