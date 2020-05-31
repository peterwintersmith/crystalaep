// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]
// SimpleEspEmu.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "C:\Work\temp\libpdasm\libpdasm\disasm.h"
#include "StackEmu.h"

__declspec(naked) __declspec(dllexport) __declspec(noinline) void StubPopFunc()
{
	// do not call me :P
	
	__asm
	{
		//MOVZX EDX,WORD PTR DS:[esp]
		//LEA EDI,DWORD PTR SS:[ESP-0x4E4]
		//--
		pop eax
		pop eax
		ret
		mov byte ptr [esp], 0x11
		mov dword ptr [esp], 0x11111111
		mov word ptr [esp + 2], 0x2222
		mov byte ptr [esp + 3], 0x41
		ret
		push 5
		pop eax
		push dword ptr [esp + 4*eax]
		pop edi
		push 0x41414141
		mov eax, 0x4243
		mov dl, 0x55
		mov al, 0x41
		mov bl, 0x44
		xor al, bl
		mov edi, 0x50505050
		mov esi, 0x31313131
		xor edi, esi
		mov eax, [esp]
		retn 0x14
	}
}

void DisasmTest(LPVOID lpvFunction)
{
	PBYTE pCode = (PBYTE)lpvFunction;
	if(pCode[0] == 0xe9)
	{
		// being thunked - happens at debug time
		pCode = *(DWORD *)(pCode + 1) + pCode + 5;
	}

	char szDisasmText[128] = {0};

	while(1)
	{
		DWORD dwInstrLen = Disasm::GetInstructionLength(pCode);
		if(!dwInstrLen)
		{
			printf("Disassembly error\n");
			break;
		}

		if(!Disasm::GetDisassemblyText(pCode, szDisasmText))
		{
			printf("Error disassembling at 0x%p\n", pCode);
			break;
		}

		printf("%p    %s\n", pCode, szDisasmText);

		if(pCode[0] == 0xcc || pCode[0] == 0xc9 || pCode[0] == 0xc3)
			break;
		
		pCode += dwInstrLen;
	}

	ExitProcess(0);
}

int _tmain(int argc, _TCHAR* argv[])
{
	CONTEXT CpuContext = {0};
	CpuContext.ContextFlags = CONTEXT_FULL;

	if(!GetThreadContext(GetCurrentThread(), &CpuContext))
	{
		printf("Error getting thread context\n");
		return 1;
	}
	
	CpuContext.Eax = 0x41414141;
	CpuContext.Ebx = 0x42424242;
	CpuContext.Ecx = 0x43434343;
	CpuContext.Edx = 0x44444444;

	//CpuContext.Esp = 0x45454545;
	CpuContext.Ebp = 0x46464646;
	CpuContext.Esi = 0x47474747;
	CpuContext.Edi = 0x48484848;
	CpuContext.Eip = 0x01010101;

	printf("Register Context (before):\n");
	printf("  EAX = %p  EDX = 0x%p  ECX = %p  EBX = %p\n", CpuContext.Eax, CpuContext.Edx, CpuContext.Ecx, CpuContext.Ebx);
	printf("  ESP = %p  EBP = 0x%p  ESI = %p  EDI = %p\n", CpuContext.Esp, CpuContext.Ebp, CpuContext.Esi, CpuContext.Edi);
	printf("  EIP = %p\n\n", CpuContext.Eip);

	// parse the disasm text (i.e.):
	//   MOV(DWORD [ESP+14], EDX);
	//   CMP(DWORD [ESP+14], BYTE FFFFFFFF);
	//   CALL(FFFFF9E9);
	//   PUSH(ECX);
	//   CMP(EAX, BYTE 03);
	//   CALL(DWORD PTR [00CF50AC]);
	//   RETN(WORD CF36);
	//   JMP(DWORD [00CF3528+4*EDX)]);
	//   JMP(DWORD [00CF3620+4*EAX)]);
	//   LEA(EAX, DWORD [EAX+4*EAX)]);

	/*
	char *rgpszOperandStrings[] = {
		"DWORD [ESP+14]",
		"EDX",
		"FFFFF9E9",
		"ESP",
		"BYTE 03",
		"DWORD PTR [00CF50AC]",
		"WORD CF36",
		"DWORD [00CF3528+4*EDX)]",
		"DWORD [ESP+4*EBP)]",
		"DWORD [EAX+4*EAX)]",
		NULL
	};

	StackEmu::EOperandType eType;
	DWORD dwValue;
	
	StackEmu::ECpuReg8 eReg8;
	StackEmu::ECpuReg16 eReg16;
	StackEmu::ECpuReg32 eReg32;

	for(DWORD i=0; rgpszOperandStrings[i] != NULL; i++)
	{
		printf("Parsing operand: %s\n", rgpszOperandStrings[i]);

		if(!StackEmu::OperandToTypeAndValue(&CpuContext, rgpszOperandStrings[i], &eType, &dwValue, &eReg8, &eReg16, &eReg32))
		{
			printf("Error in StackEmu::OperandToTypeAndValue\n");
			return 1;
		}

		printf("Result:\n");

		switch(eType)
		{
		case StackEmu::EOperandType::OperandType_Mem8:
			printf("  Type: EOperandType::OperandType_Mem8\n");
			break;
		case StackEmu::EOperandType::OperandType_Mem16:
			printf("  Type: EOperandType::OperandType_Mem16\n");
			break;
		case StackEmu::EOperandType::OperandType_Mem32:
			printf("  Type: EOperandType::OperandType_Mem32\n");
			break;
		case StackEmu::EOperandType::OperandType_Reg8:
			printf("  Type: EOperandType::OperandType_Reg8\n");
			break;
		case StackEmu::EOperandType::OperandType_Reg16:
			printf("  Type: EOperandType::OperandType_Reg16\n");
			break;
		case StackEmu::EOperandType::OperandType_Reg32:
			printf("  Type: EOperandType::OperandType_Reg32\n");
			break;
		case StackEmu::EOperandType::OperandType_Literal32:
			printf("  Type: EOperandType::OperandType_Literal32\n");
			break;
		}

		switch(eReg8)
		{
		case StackEmu::ECpuReg8::Reg8_Invalid:
			printf("  Reg8: ECpuReg8::Reg8_Invalid\n");
			break;
		case StackEmu::ECpuReg8::Reg8_AL:
			printf("  Reg8: ECpuReg8::Reg8_AL\n");
			break;
		case StackEmu::ECpuReg8::Reg8_CL:
			printf("  Reg8: ECpuReg8::Reg8_CL\n");
			break;
		case StackEmu::ECpuReg8::Reg8_DL:
			printf("  Reg8: ECpuReg8::Reg8_DL\n");
			break;
		case StackEmu::ECpuReg8::Reg8_BL:
			printf("  Reg8: ECpuReg8::Reg8_BL\n");
			break;
		case StackEmu::ECpuReg8::Reg8_AH:
			printf("  Reg8: ECpuReg8::Reg8_AH\n");
			break;
		case StackEmu::ECpuReg8::Reg8_CH:
			printf("  Reg8: ECpuReg8::Reg8_CH\n");
			break;
		case StackEmu::ECpuReg8::Reg8_DH:
			printf("  Reg8: ECpuReg8::Reg8_DH\n");
			break;
		case StackEmu::ECpuReg8::Reg8_BH:
			printf("  Reg8: ECpuReg8::Reg8_BH\n");
			break;
		}

		switch(eReg16)
		{
		case StackEmu::ECpuReg16::Reg16_Invalid:
			printf("  Reg16: ECpuReg16::Reg16_Invalid\n");
			break;
		case StackEmu::ECpuReg16::Reg16_AX:
			printf("  Reg16: ECpuReg16::Reg16_AX\n");
			break;
		case StackEmu::ECpuReg16::Reg16_CX:
			printf("  Reg16: ECpuReg16::Reg16_CX\n");
			break;
		case StackEmu::ECpuReg16::Reg16_DX:
			printf("  Reg16: ECpuReg16::Reg16_DX\n");
			break;
		case StackEmu::ECpuReg16::Reg16_BX:
			printf("  Reg16: ECpuReg16::Reg16_BX\n");
			break;
		case StackEmu::ECpuReg16::Reg16_SP:
			printf("  Reg16: ECpuReg16::Reg16_SP\n");
			break;
		case StackEmu::ECpuReg16::Reg16_BP:
			printf("  Reg16: ECpuReg16::Reg16_BP\n");
			break;
		case StackEmu::ECpuReg16::Reg16_SI:
			printf("  Reg16: ECpuReg16::Reg16_SI\n");
			break;
		case StackEmu::ECpuReg16::Reg16_DI:
			printf("  Reg16: ECpuReg16::Reg16_DI\n");
			break;
		}

		switch(eReg32)
		{
		case StackEmu::ECpuReg32::Reg32_Invalid:
			printf("  Reg32: ECpuReg32::Reg32_Invalid\n");
			break;
		case StackEmu::ECpuReg32::Reg32_EAX:
			printf("  Reg32: ECpuReg32::Reg32_EAX\n");
			break;
		case StackEmu::ECpuReg32::Reg32_ECX:
			printf("  Reg32: ECpuReg32::Reg32_ECX\n");
			break;
		case StackEmu::ECpuReg32::Reg32_EDX:
			printf("  Reg32: ECpuReg32::Reg32_EDX\n");
			break;
		case StackEmu::ECpuReg32::Reg32_EBX:
			printf("  Reg32: ECpuReg32::Reg32_EBX\n");
			break;
		case StackEmu::ECpuReg32::Reg32_ESP:
			printf("  Reg32: ECpuReg32::Reg32_ESP\n");
			break;
		case StackEmu::ECpuReg32::Reg32_EBP:
			printf("  Reg32: ECpuReg32::Reg32_EBP\n");
			break;
		case StackEmu::ECpuReg32::Reg32_ESI:
			printf("  Reg32: ECpuReg32::Reg32_ESI\n");
			break;
		case StackEmu::ECpuReg32::Reg32_EDI:
			printf("  Reg32: ECpuReg32::Reg32_EDI\n");
			break;
		}

		printf("  Value: %p\n\n", dwValue);
	}
	*/
	
	//DisasmTest(StackEmu::EmulateGenericTransformOperation);
	
	char szDisasmText[128] = {0};
	std::map<LPVOID, DWORD> memoryMap;
	int iStackDelta = 0;

	PBYTE pCode = (PBYTE)StubPopFunc;
	if(pCode[0] == 0xe9)
	{
		// being thunked - happens at debug time
		pCode = *(DWORD *)(pCode + 1) + pCode + 5;
	}

	BOOL bEmuSuccess;

	if(!StackEmu::CalculateStackDelta(&CpuContext, (PBYTE)pCode, &iStackDelta, &bEmuSuccess, FALSE, TRUE))
	{
		printf("StackEmu::CalculateStackDelta failed\n");
		return 1;
	}

	printf("Emulation Success?: %s\tStack Delta: %d\n", bEmuSuccess ? "TRUE" : "FALSE", iStackDelta);

	printf("Register Context (after):\n");
	printf("  EAX = %p  EDX = 0x%p  ECX = %p  EBX = %p\n", CpuContext.Eax, CpuContext.Edx, CpuContext.Ecx, CpuContext.Ebx);
	printf("  ESP = %p  EBP = 0x%p  ESI = %p  EDI = %p\n", CpuContext.Esp, CpuContext.Ebp, CpuContext.Esi, CpuContext.Edi);
	printf("  EIP = %p\n\n", CpuContext.Eip);
	
	return 0;
}
