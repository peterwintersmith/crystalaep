// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#pragma once

// CUiFeedback Interface Class

struct IUiFeedback;

#define CRYSTAL_LATE_BINDING

#ifdef CRYSTAL_LATE_BINDING
#ifdef DEBUG_BUILD
extern "C" static IUiFeedback *(*GetUiFeedback)() = (IUiFeedback *(*)())GetProcAddress(
		GetModuleHandle(L"C:\\Work\\crystal\\BP\\Impl\\AuxCore\\Debug\\auxcore.dll") ?
			GetModuleHandle(L"C:\\Work\\crystal\\BP\\Impl\\AuxCore\\Debug\\auxcore.dll") :
			LoadLibrary(L"C:\\Work\\crystal\\BP\\Impl\\AuxCore\\Debug\\auxcore.dll"), 
		"GetUiFeedback"
	);
#else
extern "C" static IUiFeedback *(*GetUiFeedback)() = (IUiFeedback *(*)())GetProcAddress(
		GetModuleHandle(L"auxcore.dll") ?
			GetModuleHandle(L"auxcore.dll") :
			LoadLibrary(L"auxcore.dll"), 
		"GetUiFeedback"
	);
#endif
#else
extern "C" __declspec(dllimport) IUiFeedback *GetUiFeedback();
#endif

enum EUiMessageStyle {
	UiStyle_IconExclamation = MB_ICONEXCLAMATION,
	UiStyle_IconWarning = MB_ICONWARNING,
	UiStyle_IconInformation = MB_ICONINFORMATION,
	UiStyle_IconAsterisx = MB_ICONASTERISK,
	UiStyle_IconQuestion = MB_ICONQUESTION,
	UiStyle_IconStop = MB_ICONSTOP,
	UiStyle_IconError = MB_ICONERROR,
	UiStyle_IconHand = MB_ICONHAND
};

struct IUiFeedback {
	virtual BOOL UserBoolFeedback(EUiMessageStyle style, WCHAR *pwszTitle, WCHAR *pwszCaption) = 0;
};