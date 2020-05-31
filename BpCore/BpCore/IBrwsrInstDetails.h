// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#pragma once

struct IBrowserInstanceDetails;

#ifdef CRYSTAL_LATE_BINDING
#ifdef DEBUG_BUILD
extern "C" static IBrowserInstanceDetails *(*GetBrowserInstanceDetails)() = (IBrowserInstanceDetails *(*)())GetProcAddress(
		GetModuleHandle(L"C:\\Work\\crystal\\BP\\Impl\\BpCore\\Debug\\bpcore.dll") ?
			GetModuleHandle(L"C:\\Work\\crystal\\BP\\Impl\\BpCore\\Debug\\bpcore.dll") :
			LoadLibrary(L"C:\\Work\\crystal\\BP\\Impl\\BpCore\\Debug\\bpcore.dll"), 
		"GetBrowserInstanceDetails"
	);
#else
extern "C" static IBrowserInstanceDetails *(*GetBrowserInstanceDetails)() = (IBrowserInstanceDetails *(*)())GetProcAddress(
		GetModuleHandle(L"bpcore.dll") ?
			GetModuleHandle(L"bpcore.dll") :
			LoadLibrary(L"bpcore.dll"), 
		"GetBrowserInstanceDetails"
	);
#endif
#else
extern "C" __declspec(dllimport) IBrowserInstanceDetails *GetBrowserInstanceDetails();
#endif

struct IBrowserInstanceDetails {
	virtual LPWSTR GetNavigatedAddress() = 0;
	virtual void SetNavigatedAddress(LPWSTR pwsz) = 0;
	virtual void GetIEAddressBarText() = 0;
};