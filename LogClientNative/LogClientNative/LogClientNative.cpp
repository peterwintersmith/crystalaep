// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]
// LogClientNative.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Logger.h"

int _tmain(int argc, _TCHAR* argv[])
{
	CLogger *logger;
	
	try
	{
		logger = CLogger::GetInstance("peter");
	}
	catch(std::exception ex)
	{
		printf("Exception: %s\n", ex.what());
		return 1;
	}

	printf("GetInstCount() = %u\n", logger->GetInstCount());

	logger->Log("HELLO WORLD\n");
	logger->Log("printf attempt %c %x %u\n", 0x41, 0x41, 0x41);

	logger->CloseInstance("peter", logger);

	Sleep(500);

	//logger->Log("hello");

	//Sleep(2000);

	logger = logger->GetInstance("peter");
	logger->Log("back again!\n");

	std::string str("hello");
	str += "world\n";

	logger->Log(str);

	logger->Log(0xffffff, 2);
	logger->Log("\n");

	logger->LogHex((PBYTE)logger, sizeof(CLogger)); // class inst
	logger->LogHex(*(PBYTE *)logger, 0x80); // vtable

	return 0;
}

