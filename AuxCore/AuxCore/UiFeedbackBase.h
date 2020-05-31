// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#pragma once

// CUiFeedback virtual base class (identical to interface)

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

struct CUiFeedbackBase {
	virtual BOOL UserBoolFeedback(EUiMessageStyle style, WCHAR *pwszTitle, WCHAR *pwszCaption) = 0;
};