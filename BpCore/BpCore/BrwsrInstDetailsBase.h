// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#pragma once

struct CBrowserInstanceDetailsBase {
	virtual LPWSTR GetNavigatedAddress() = 0;
	virtual void SetNavigatedAddress(LPWSTR pwsz) = 0;
	virtual void GetIEAddressBarText() = 0;
};