// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]
// DetoursFree.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Detours.h"

BOOL (WINAPI *Real_FreeLibrary)(
	__in  HMODULE hModule
) = FreeLibrary;

BOOL WINAPI My_FreeLibrary(
	__in  HMODULE hModule
)
{
	MessageBoxA(0,"My_FreeLibrary called\n","",0);
	return TRUE;
}

int (WINAPI *Real_MessageBoxA)(
    __in_opt HWND hWnd,
    __in_opt LPCSTR lpText,
    __in_opt LPCSTR lpCaption,
    __in UINT uType) = MessageBoxA;//MessageBoxA;


int WINAPI My_MessageBoxA(
    __in_opt HWND hWnd,
    __in_opt LPCSTR lpText,
    __in_opt LPCSTR lpCaption,
    __in UINT uType)
{
	return Real_MessageBoxA(0, "Real_MessageBoxWrapperA", "Called from My_MessageBoxA", 0);
}

int _tmain(int argc, _TCHAR* argv[])
{
	LONG lResult = 0;
	
	printf("Adding detour\n");

	lResult = DetoursFree::DetourTransactionBegin();
	if(lResult)
		printf("DetourTransactionBegin failure %u (0x%x)\n", lResult, lResult);

	lResult = DetoursFree::DetourUpdateThread(GetCurrentThread());
	if(lResult)
		printf("DetourUpdateThread failure %u (0x%x)\n", lResult, lResult);
	
	lResult = DetoursFree::DetourAttach((PVOID *)&Real_MessageBoxA, (PVOID)My_MessageBoxA);
	if(lResult)
		printf("DetourAttach failure %u (0x%x)\n", lResult, lResult);
	
	lResult = DetoursFree::DetourAttach((PVOID *)&Real_FreeLibrary, (PVOID)My_FreeLibrary);
	if(lResult)
		printf("DetourAttach failure %u (0x%x)\n", lResult, lResult);
	
	lResult = DetoursFree::DetourTransactionCommit();
	if(lResult)
		printf("DetourTransactionCommit failure %u (0x%x)\n", lResult, lResult);

	MessageBoxA(0, "hello", "world", 0);
	Real_MessageBoxA(0, "real", "real", 0);

	FreeLibrary((HMODULE)0);
	Real_FreeLibrary((HMODULE)0);

	printf("Removing detour\n");

	lResult = DetoursFree::DetourTransactionBegin();
	if(lResult)
		printf("DetourTransactionBegin failure %u (0x%x)\n", lResult, lResult);

	lResult = DetoursFree::DetourDetach((PVOID *)&Real_FreeLibrary, (PVOID)My_FreeLibrary);
	if(lResult)
		printf("DetourDetach failure %u (0x%x)\n", lResult, lResult);

	lResult = DetoursFree::DetourDetach((PVOID *)&Real_MessageBoxA, (PVOID)My_MessageBoxA);
	if(lResult)
		printf("DetourDetach failure %u (0x%x)\n", lResult, lResult);

	lResult = DetoursFree::DetourTransactionCommit();
	if(lResult)
		printf("DetourTransactionCommit failure %u (0x%x)\n", lResult, lResult);

	MessageBoxA(0, "hello", "world", 0);
	FreeLibrary((HMODULE)0);


	return 0;
}
