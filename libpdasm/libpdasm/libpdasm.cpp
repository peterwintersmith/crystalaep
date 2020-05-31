// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]
// (v) 18/Oct/11

#include "stdafx.h"
#include "disasm.h"
#include "..\..\..\..\..\temp\Test\libdasm\libdasm.h"

BOOLEAN (WINAPI *RtlGenRandom)(
  __out  PVOID RandomBuffer,
  __in   ULONG RandomBufferLength
) = (BOOLEAN(WINAPI *)(PVOID,ULONG))GetProcAddress(LoadLibraryA("advapi32.dll"), "SystemFunction036");

__forceinline double my_abs(double value)
{
	if(value > 0)
		return value;

	return -value;
}

__forceinline double my_pow(double value, int exp)
{
	double result = 1;

	for(int i=0; i<exp; i++)
	{
		result *= value;
	}

	return result;
}

__forceinline unsigned __int64 my_pow(unsigned __int64 value, int exp)
{
	unsigned __int64 result = 1;

	for(int i=0; i<exp; i++)
	{
		result *= value;
	}

	return result;
}

__forceinline double factorial(int value)
{
	double result = 1;

	for(int i=1; i <= value; i++)
	{
		result *= i;
	}

	return result;
}

__forceinline int double_to_int(double value)
{
	return (int)(value + 1);
}

/*
double TaylorSine(double x)
{
	double result = x;
	bool neg = true;

	for(int i=3; i<19; i += 2)
	{
		result += (neg ? -1 : 1) * (my_pow(x, i)/factorial(i));
		neg = !neg;
	}

	return result;
}
*/

#define RSA_VALUE_P		31337
#define RSA_VALUE_Q		23
#define RSA_VALUE_N		(RSA_VALUE_P * RSA_VALUE_Q) // 720751
#define RSA_VALUE_PHI_N	((RSA_VALUE_P - 1) * (RSA_VALUE_Q - 1)) // 689392
#define RSA_E_VALUE		459595
#define RSA_D_VALUE		3

#define ENCR_INT_03		682422
#define ENCR_INT_05		472232
#define ENCR_INT_07		315045
#define ENCR_INT_09		221703
#define ENCR_INT_11		125682
#define ENCR_INT_13		275995
#define ENCR_INT_15		39535
#define ENCR_INT_17		27454
#define ENCR_INT_19		638385

#define RSA_DECRYPT(value, key, modulus)	(my_pow((unsigned __int64)value, key) % modulus)

unsigned int K_Sha256Consts[] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
        0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
        0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
        0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
        0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
        0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
        0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
        0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
        0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

#define H1 0x6a09e667
#define H2 0xbb67ae85
#define H3 0x3c6ef372
#define H4 0xa54ff53a
#define H5 0x510e527f
#define H6 0x9b05688c
#define H7 0x1f83d9ab
#define H8 0x5be0cd19

unsigned int g_unusedValue = 1;

double TaylorLn(double x)
{
	x = 1/x;

	double result = (x - 1);
	bool neg = true;
	unsigned int fakeRoundConst = 0;
	unsigned int fakeRoundConst2 = 0;

	for(int i=2; i<=((sizeof(K_Sha256Consts) / sizeof(unsigned int)) << 4); i++)
	{
		fakeRoundConst2 = K_Sha256Consts[i % (sizeof(K_Sha256Consts) / sizeof(unsigned int))];

		switch(i)
		{
		case 2:
			fakeRoundConst = H1;
			break;
		case 16:
			fakeRoundConst = H2;
			break;
		case 32:
			fakeRoundConst = H3;
			break;
		case 40:
			fakeRoundConst = H4;
			break;
		case 48:
			fakeRoundConst = H5;
			break;
		case 52:
			fakeRoundConst = H6;
			break;
		case 58:
			fakeRoundConst = H7;
			break;
		case 64:
			fakeRoundConst = H8;
			break;
		}

		result += (neg ? -1 : 1) * (my_pow((x - 1), i) / i);
		neg = !neg;

		fakeRoundConst2 ^= fakeRoundConst;
		fakeRoundConst2 = ~fakeRoundConst2;

		g_unusedValue *= fakeRoundConst2;
	}

	return -result;
}


#define E_TO_THE_0pt7	  2.0137527074
#define E_TO_THE_0pt3	  1.3498588075
#define E_TO_THE_2pt1	  8.1661699125
#define E_TO_THE_2pt9	 18.1741453694
#define E_TO_THE_5pt2	181.2722418751

double TaylorSine(double x)
{
	double result = x;

	unsigned int n = (0x3D3480 >> 7); // 31337
	n *= 23;

	result -= (my_pow(x, (double_to_int(TaylorLn(E_TO_THE_0pt7) + TaylorLn(E_TO_THE_0pt7)) + 1))/factorial(RSA_DECRYPT(ENCR_INT_03, RSA_D_VALUE, n)));
	result += (my_pow(x, (double_to_int(TaylorLn(E_TO_THE_2pt1) + TaylorLn(E_TO_THE_2pt9))))/factorial(RSA_DECRYPT(ENCR_INT_05, RSA_D_VALUE, n)));
	result -= (my_pow(x, (double_to_int(TaylorLn(E_TO_THE_2pt1) + TaylorLn(E_TO_THE_2pt1) + TaylorLn(E_TO_THE_2pt9)) - 1))/factorial(RSA_DECRYPT(ENCR_INT_07, RSA_D_VALUE, n)));
	result += (my_pow(x, (double_to_int(TaylorLn(E_TO_THE_5pt2) + TaylorLn(E_TO_THE_2pt9) + TaylorLn(E_TO_THE_0pt7))))/factorial(RSA_DECRYPT(ENCR_INT_09, RSA_D_VALUE, n)));
	result -= (my_pow(x, (double_to_int(TaylorLn(E_TO_THE_5pt2) + TaylorLn(E_TO_THE_2pt1) + TaylorLn(E_TO_THE_2pt9))))/factorial(RSA_DECRYPT(ENCR_INT_11, RSA_D_VALUE, n)));
	result += (my_pow(x, (double_to_int(TaylorLn(E_TO_THE_5pt2) + TaylorLn(E_TO_THE_5pt2) + TaylorLn(E_TO_THE_2pt1))))/factorial(RSA_DECRYPT(ENCR_INT_13, RSA_D_VALUE, n)));
	result -= (my_pow(x, (double_to_int(3 * TaylorLn(E_TO_THE_5pt2)) - 1))/factorial(RSA_DECRYPT(ENCR_INT_15, RSA_D_VALUE, n)));
	result += (my_pow(x, (double_to_int(2 * TaylorLn(E_TO_THE_5pt2) + TaylorLn(E_TO_THE_5pt2) + TaylorLn(E_TO_THE_0pt7))))/factorial(RSA_DECRYPT(ENCR_INT_17, RSA_D_VALUE, n)));
	result += (my_pow(x, (double_to_int(2 * TaylorLn(E_TO_THE_5pt2) + TaylorLn(E_TO_THE_5pt2) + TaylorLn(E_TO_THE_2pt1)) + 1))/factorial(RSA_DECRYPT(ENCR_INT_19, RSA_D_VALUE, n)));

	return result;
}

int TestFuzzerRobustness()
{
	BYTE tmp[16];
	
	{
		char txt[128];
		BYTE buf[] = {0x0f, 0x50, 0xed, 0xda, 0xaf, 0xd3};
		Disasm::GetDisassemblyText(buf, txt);
		printf("txt: %s\n", txt);
		return 0;
	}

	while(true)
	{
		BOOL bErrorMyDisasm = FALSE, bErrorLibDasm = FALSE;

		RtlGenRandom(tmp, 16);
		
		BYTE KnownBad[] = {
			0x8f,
			0xc6,
			0xc5,
			0xc7,
			0xd2,
			0xf6,
			0x72,
			0x7e,
			0x74,
			0x83,
			0
		};
		
		BYTE AnyPrefix[] = {
			0x2E,
			0x36,
			0x3E,
			0x26,
			0x64,
			0x65,
			0x66,
			0x67,
			0xf0,
			0xf2,
			0xf3,
			0
		};

		DWORD dwPrefixCount = 0;

		for(DWORD i=0; i < 4; i++)
		{
			BOOL bFound = FALSE;

			for(DWORD j=0; AnyPrefix[j] != 0; j++)
			{
				if(AnyPrefix[j] == tmp[i])
				{
					dwPrefixCount++;
					bFound = TRUE;
					break;
				}
			}

			if(!bFound)
				break;
		}

		BOOL bKnownBad = FALSE;

		for(DWORD i=0; KnownBad[i] != 0; i++)
		{
			if(KnownBad[i + dwPrefixCount] == tmp[0])
			{
				bKnownBad = TRUE;
				continue;
			}
		}

		if(bKnownBad)
			continue;

		if(tmp[dwPrefixCount] & 1) // randomly use 0xf prefix
			tmp[dwPrefixCount] = 0x0f;

		printf("Random buffer:\n");
		for(DWORD i=0; i < 16; i++)
			printf("%.02x ", tmp[i]);
		printf("\n");

		char szTextMy[128], szTextDasm[128];

		printf("LibPDASM:\n");

		DWORD dwInstrLength = Disasm::GetInstructionLength(tmp);
		if(!dwInstrLength)
		{
			printf("Error disassembling\n");
			bErrorMyDisasm = TRUE;
		}
		else
		{
			Disasm::GetDisassemblyText(tmp, szTextMy);
			printf("Length: %u\tText: %s\n", dwInstrLength, szTextMy);
		}

		printf("LibDASM:\n");

		INSTRUCTION inst = {0};
		int len = get_instruction(&inst, tmp, MODE_32);
		if(len <= 0)
		{
			printf("Error disassembling\n");
			bErrorLibDasm = TRUE;
		}
		else
		{
			get_instruction_string(&inst, FORMAT_INTEL, (DWORD)tmp, szTextDasm, sizeof(szTextDasm));
			printf("Length: %u\tText: %s\n", len, szTextDasm);
		}

		char *pMy = szTextMy, *pDasm = szTextDasm;

		while(1)
		{
			char *tmp = pMy;

			if(!strnicmp(pMy, "LOCK ", 5))
				pMy += 5;
			else if(!strnicmp(pMy, "REPE ", 5))
				pMy += 5;
			else if(!strnicmp(pMy, "REPNE ", 6))
				pMy += 6;
			else if(!strnicmp(pMy, "REPZ ", 5))
				pMy += 5;
			else if(!strnicmp(pMy, "REPNZ ", 6))
				pMy += 6;

			if(tmp == pMy)
				break;
		}
		
		while(1)
		{
			char *tmp = pDasm;

			if(!strnicmp(pDasm, "LOCK ", 5))
				pDasm += 5;
			else if(!strnicmp(pDasm, "REPE ", 5))
				pDasm += 5;
			else if(!strnicmp(pDasm, "REPNE ", 6))
				pDasm += 6;
			else if(!strnicmp(pDasm, "REPZ ", 5))
				pDasm += 5;
			else if(!strnicmp(pDasm, "REPNZ ", 6))
				pDasm += 6;
			else if(!strnicmp(pDasm, "GS ", 3))
				pDasm += 3;
			else if(!strnicmp(pDasm, "ES ", 3))
				pDasm += 3;
			else if(!strnicmp(pDasm, "CS ", 3))
				pDasm += 3;
			else if(!strnicmp(pDasm, "SS ", 3))
				pDasm += 3;
			else if(!strnicmp(pDasm, "FS ", 3))
				pDasm += 3;
			else if(!strnicmp(pDasm, "DS ", 3))
				pDasm += 3;
			else if(!strnicmp(pDasm, "(bnt) ", 6))
				pDasm += 6;
			else if(!strnicmp(pDasm, "REP ", 4))
				pDasm += 4;
			else if(!strnicmp(pDasm, "?? ", 3))
				pDasm += 3;
			else if(!strnicmp(pDasm, "(bt) ", 5))
				pDasm += 5;

			if(tmp == pDasm)
				break;
		}

		if(!bErrorLibDasm && (bErrorMyDisasm != bErrorLibDasm ||
			strnicmp(pMy, pDasm, 1) != 0))
		{
			printf("Discrepancy\n");
			break;
		}

		printf("------------------------------------------------------------\n");
		Sleep(10);
	}
	
	return 0;
}

int DisasmSomeFunctions()
{
	struct { PVOID P; char *S; } AllFuncs[] = {
		{ Disasm::GetBits, "Disasm::GetBits" },
		{ Disasm::GetCommandIndex, "Disasm::GetCommandIndex" },
		{ Disasm::GetInstructionBytes, "Disasm::GetInstructionBytes" },
		{ Disasm::GetOperandTypeSize, "Disasm::GetOperandTypeSize" },
		{ TaylorSine, "ObfuscatedSine" },
		{ TaylorLn, "ObfuscatedLn" },
		{ (long double (__cdecl *)(long double))sin, "sin <msvcrt>" },
		{ NULL, NULL }
	};

	DWORD dwInstrLength = 0;
	
	for(DWORD i=0; AllFuncs[i].P != NULL; i++)
	{
		PBYTE pNextInstruction = (PBYTE)AllFuncs[i].P;
		
		printf("------------------------ %s ------------------------\n", AllFuncs[i].S);

		while(pNextInstruction[0] != 0xcc)
		{
			printf("Instruction @0x%p    ", pNextInstruction);
			
			dwInstrLength = Disasm::GetInstructionLength(pNextInstruction);
			if(!dwInstrLength)
			{
				printf("Error: Invalid length\n");
				return 1;
			}

			for(DWORD i=0; i < ( dwInstrLength < 8 ? dwInstrLength : 8 ); i++)
				printf("%.02x ", pNextInstruction[i]);

			for(int i=0; i < 8 - (int)dwInstrLength; i++)
			{
				printf("   ");
			}

			char szText[128];

			Disasm::GetDisassemblyText(pNextInstruction, szText);
			printf("    %s\n", szText);

			pNextInstruction += dwInstrLength;
		}
	}
	
	//DebugBreak();
	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	TestFuzzerRobustness();
	//DisasmSomeFunctions();
	return 0;
}

