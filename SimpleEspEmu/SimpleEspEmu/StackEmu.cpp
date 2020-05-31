// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#include "stdafx.h"
#include "..\..\libpdasm\libpdasm\disasm.h"
#include "StackEmu.h"
#include "..\..\Shared\Utils\Utils.h"

BOOL StackEmu::CalculateStackDelta(CONTEXT *pContext, PBYTE pbInstr, int *piDelta, BOOL *p_bEmuSuccess, BOOL bEmulateRetn, BOOL bUpdateContext)
{
	BOOL bSuccess = FALSE, bBreak = FALSE;
	std::map<LPVOID, BYTE> memoryMap;
	CONTEXT CpuContext = {0};

	if(piDelta == NULL || pContext == NULL)
		goto Cleanup;

	CpuContext = *pContext;
	*piDelta = 0;
	*p_bEmuSuccess = FALSE;

	//debug code
	//printf("--CODE START--\n");
	//end debug

	for(DWORD i = 0; i < STACKEMU_MAX_INSTR; i++)
	{
		DWORD dwInstrLen = Disasm::GetInstructionLength(pbInstr);
		
		if(!dwInstrLen)
			goto Cleanup;

		char szDisasmText[256] = {0};
		
		if(!Disasm::GetDisassemblyText(pbInstr, szDisasmText))
			goto Cleanup;

		//debug code
		//printf("%s\n", szDisasmText);
		//end debug

		std::string strMnem = GetMnemFromDisasm(szDisasmText);
		ECommonInstruction eInstr = InstrFromMnem(strMnem);
		
		std::string strOperand1, strOperand2;

		switch(eInstr)
		{
			case Instr_pop:
				strOperand1 = GetOperandFromDisasm(0, szDisasmText);
				
				if(!EmulatePop(&CpuContext, &memoryMap, strOperand1))
					goto Cleanup;

				break;
			case Instr_push:
				strOperand1 = GetOperandFromDisasm(0, szDisasmText);
				
				if(!EmulatePush(&CpuContext, &memoryMap, strOperand1))
					goto Cleanup;

				break;
			case Instr_mov:
				strOperand1 = GetOperandFromDisasm(0, szDisasmText);
				strOperand2 = GetOperandFromDisasm(1, szDisasmText);
				
				if(!EmulateMov(&CpuContext, &memoryMap, strOperand1, strOperand2))
					goto Cleanup;

				break;
			case Instr_add:
				strOperand1 = GetOperandFromDisasm(0, szDisasmText);
				strOperand2 = GetOperandFromDisasm(1, szDisasmText);
				
				if(!EmulateAdd(&CpuContext, &memoryMap, strOperand1, strOperand2))
					goto Cleanup;

				break;
			case Instr_sub:
				strOperand1 = GetOperandFromDisasm(0, szDisasmText);
				strOperand2 = GetOperandFromDisasm(1, szDisasmText);
				
				if(!EmulateSub(&CpuContext, &memoryMap, strOperand1, strOperand2))
					goto Cleanup;

				break;
			case Instr_xor:
				strOperand1 = GetOperandFromDisasm(0, szDisasmText);
				strOperand2 = GetOperandFromDisasm(1, szDisasmText);
				
				if(!EmulateXor(&CpuContext, &memoryMap, strOperand1, strOperand2))
					goto Cleanup;

				break;
			case Instr_retn:
				
				if(bEmulateRetn)
				{
					strOperand1 = GetOperandFromDisasm(0, szDisasmText);
				
					if(!EmulateRetn(&CpuContext, &memoryMap, strOperand1))
						goto Cleanup;
				}

				*piDelta = (int)CpuContext.Esp - (int)pContext->Esp;
				*p_bEmuSuccess = TRUE;
				
				bBreak = TRUE;

				break;
			default:

				bBreak = TRUE;
				
				break;
		}

		if(bBreak)
			break;

		pbInstr += dwInstrLen;
	}

	
	//debug code
	//printf("--CODE END--\n\n");
	//end debug

	if(bUpdateContext)
		*pContext = CpuContext;

	bSuccess = TRUE;
Cleanup:
	return bSuccess;
}

std::string StackEmu::GetMnemFromDisasm(char *pszDisasmText)
{
	std::string strMnem;
	BOOL bBreak = FALSE;

	for(DWORD i=0; pszDisasmText[i] != '\0'; i++)
	{
		switch(pszDisasmText[i])
		{
		case '(':
		case '<':
			bBreak = TRUE;
			break;
		}

		if(bBreak)
			break;

		strMnem += pszDisasmText[i];
	}

	return strMnem;
}

StackEmu::ECommonInstruction StackEmu::InstrFromMnem(std::string strMnem)
{
	char *rgpszInstr[] = {
		"pop", "push", "mov", "add", "sub", "xor", "retn|ret",
		NULL
	}, szInstr[16] = {0};

	for(DWORD i=0; rgpszInstr[i] != NULL; i++)
	{
		strcpy(szInstr, rgpszInstr[i]);

		char *pszPart = strtok(szInstr, "|");
		if(!pszPart)
			break;

		do
		{
			if(!stricmp(pszPart, strMnem.c_str()))
			{
				return (StackEmu::ECommonInstruction)i;
			}
		}
		while((pszPart = strtok(NULL, "|")) != NULL);
	}

	return ECommonInstruction::Instr_Invalid;
}

int StackEmu::GetOperandContainingReg(char *pszRegister, char *pszDisasmText)
{
	int iOperand = -1;
	BOOL bFoundOperands = FALSE;
	char szOperands[6][48] = {0}, *pszOperand = NULL;
	DWORD dwOperandIndex = 0;

	for(DWORD i=0; pszDisasmText[i] != '\0'; i++)
	{
		if(pszDisasmText[i] == '(')
		{
			bFoundOperands = TRUE;
			pszOperand = pszDisasmText + i + 1;
			continue;
		}

		if(!bFoundOperands)
			continue;

		if(pszDisasmText[i] == ',' || pszDisasmText[i] == ')')
		{
			DWORD dwLength = (pszDisasmText + i - pszOperand);
			strncpy(szOperands[dwOperandIndex], pszOperand, (dwLength >= 47) ? 47 : dwLength);
			dwOperandIndex++;
		}

		if(pszDisasmText[i] == ')' || dwOperandIndex >= 6)
			break;
	}

	for(DWORD i=0; i < dwOperandIndex; i++)
	{
		if(strstr(szOperands[i], pszRegister) != NULL)
		{
			iOperand = (int)i;
			break;
		}
	}

	return iOperand;
}

std::string StackEmu::GetOperandFromDisasm(int iOperand, char *pszDisasmText)
{
	std::string strOperand;
	BOOL bFoundOperands = FALSE;
	char szOperands[6][48] = {0}, *pszOperand = NULL;
	DWORD dwOperandIndex = 0;

	for(DWORD i=0; pszDisasmText[i] != '\0'; i++)
	{
		if(pszDisasmText[i] == '(')
		{
			bFoundOperands = TRUE;
			pszOperand = pszDisasmText + i + 1;
			continue;
		}

		if(!bFoundOperands)
			continue;

		if(pszDisasmText[i] == ',' || pszDisasmText[i] == ')')
		{
			DWORD dwLength = (pszDisasmText + i - pszOperand);
			strncpy(szOperands[dwOperandIndex], pszOperand, (dwLength >= 47) ? 47 : dwLength);
			pszOperand = pszDisasmText + i + 1;
			dwOperandIndex++;
		}

		if(pszDisasmText[i] == ')' || dwOperandIndex >= 6)
			break;
	}

	if(iOperand < (int)dwOperandIndex)
	{
		strOperand = szOperands[iOperand];
	}

	return strOperand;
}

BOOL StackEmu::StrToInt(std::string strValue, DWORD *pnChars, DWORD *pdwValue)
{
	char szIntChars[32] = {0}, szValid[] = "1234567890ABCDEF";
	DWORD dwChrPos = 0;

	*pdwValue = 0;

	for(DWORD i=0; i < strValue.size(); i++)
	{
		if(!strchr(szValid, strValue[i]))
		{
			if(!i)
			{
				// the first digit is not an integer - we can't convert this meaningfully
				return FALSE;
			}

			break;
		}

		szIntChars[dwChrPos++] = strValue[i];
		szIntChars[dwChrPos] = '\0';
	}

	*pdwValue = strtoul(szIntChars, NULL, 16);

	if(pnChars)
		*pnChars = strlen(szIntChars);

	return TRUE;
}

StackEmu::ECpuReg8 StackEmu::Reg8FromOperand(std::string strOperand)
{
	char *rgpszReg16[] = { "AL", "CL", "DL", "BL", "AH", "CH", "DH", "BH", NULL };

	for(DWORD i=0; rgpszReg16[i] != NULL; i++)
	{
		if(!strnicmp(strOperand.c_str(), rgpszReg16[i], 2))
		{
			return (ECpuReg8)i;
		}
	}

	return ECpuReg8::Reg8_Invalid;
}

StackEmu::ECpuReg16 StackEmu::Reg16FromOperand(std::string strOperand)
{
	char *rgpszReg16[] = { "AX", "CX", "DX", "BX", "SP", "BP", "SI", "DI", NULL };

	for(DWORD i=0; rgpszReg16[i] != NULL; i++)
	{
		if(!strnicmp(strOperand.c_str(), rgpszReg16[i], 2))
		{
			return (ECpuReg16)i;
		}
	}

	return ECpuReg16::Reg16_Invalid;
}

StackEmu::ECpuReg32 StackEmu::Reg32FromOperand(std::string strOperand)
{
	char *rgpszReg32[] = { "EAX", "ECX", "EDX", "EBX", "ESP", "EBP", "ESI", "EDI", "EIP", NULL };

	for(DWORD i=0; rgpszReg32[i] != NULL; i++)
	{
		if(!strnicmp(strOperand.c_str(), rgpszReg32[i], 3))
		{
			return (ECpuReg32)i;
		}
	}

	return ECpuReg32::Reg32_Invalid;
}

DWORD *StackEmu::ContextRegFromReg32(CONTEXT *pContext, ECpuReg32 eReg32)
{
	DWORD *pdwRegPtr = NULL;

	switch(eReg32)
	{
		case Reg32_EAX: pdwRegPtr = &pContext->Eax; break;
		case Reg32_ECX: pdwRegPtr = &pContext->Ecx; break;
		case Reg32_EDX: pdwRegPtr = &pContext->Edx; break;
		case Reg32_EBX: pdwRegPtr = &pContext->Ebx; break;
		case Reg32_ESP: pdwRegPtr = &pContext->Esp; break;
		case Reg32_EBP: pdwRegPtr = &pContext->Ebp; break;
		case Reg32_ESI: pdwRegPtr = &pContext->Esi; break;
		case Reg32_EDI: pdwRegPtr = &pContext->Edi; break;
		case Reg32_EIP: pdwRegPtr = &pContext->Eip; break;
	}

	return pdwRegPtr;
}

WORD *StackEmu::ContextRegFromReg16(CONTEXT *pContext, ECpuReg16 eReg16)
{
	WORD *pwRegPtr = NULL;
	
	switch(eReg16)
	{
		case Reg16_AX: pwRegPtr = (WORD *)&pContext->Eax; break;
		case Reg16_CX: pwRegPtr = (WORD *)&pContext->Ecx; break;
		case Reg16_DX: pwRegPtr = (WORD *)&pContext->Edx; break;
		case Reg16_BX: pwRegPtr = (WORD *)&pContext->Ebx; break;
		case Reg16_SP: pwRegPtr = (WORD *)&pContext->Esp; break;
		case Reg16_BP: pwRegPtr = (WORD *)&pContext->Ebp; break;
		case Reg16_SI: pwRegPtr = (WORD *)&pContext->Esi; break;
		case Reg16_DI: pwRegPtr = (WORD *)&pContext->Edi; break;
	}

	return pwRegPtr;
}

BYTE *StackEmu::ContextRegFromReg8(CONTEXT *pContext, ECpuReg8 eReg8)
{
	BYTE *pbRegPtr = NULL;
	
	switch(eReg8)
	{
		case Reg8_AL: pbRegPtr = (BYTE *)&pContext->Eax; break;
		case Reg8_CL: pbRegPtr = (BYTE *)&pContext->Ecx; break;
		case Reg8_DL: pbRegPtr = (BYTE *)&pContext->Edx; break;
		case Reg8_BL: pbRegPtr = (BYTE *)&pContext->Ebx; break;
		case Reg8_AH: pbRegPtr = (BYTE *)&pContext->Eax; pbRegPtr += 1; break;
		case Reg8_CH: pbRegPtr = (BYTE *)&pContext->Ecx; pbRegPtr += 1; break;
		case Reg8_DH: pbRegPtr = (BYTE *)&pContext->Edx; pbRegPtr += 1; break;
		case Reg8_BH: pbRegPtr = (BYTE *)&pContext->Ebx; pbRegPtr += 1; break;
	}

	return pbRegPtr;
}

void StackEmu::Trim(std::string& str)
{
	// trim front
	while(!str.empty() && (str[0] == ' ' || str[0] == '\t'))
	{
		str.erase(0);
	}

	// trim back
	while(!str.empty() && (str[str.size() - 1] == ' ' || str[str.size() - 1] == '\t'))
	{
		str.erase(str.size() - 1);
	}
}

StackEmu::EOperandWidth StackEmu::WidthFromOperand(std::string strOperand)
{
	char *rgpszWidths[] = { "BYTE", "WORD", "DWORD", NULL };
	
	for(DWORD i=0; rgpszWidths[i] != NULL; i++)
	{
		if(!strnicmp(strOperand.c_str(), rgpszWidths[i], strlen(rgpszWidths[i])))
		{
			return (EOperandWidth)i;
		}
	}

	return EOperandWidth::Width_Invalid;
}

BOOL StackEmu::OperandToTypeAndValue(
	CONTEXT *pContext, std::string strOperand, EOperandType *pOperandType,
	DWORD *pOperandValue, ECpuReg8 *pReg8, ECpuReg16 *pReg16, ECpuReg32 *pReg32
	)
{
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

	BOOL bSuccess = FALSE;

	EOperandWidth eWidth = EOperandWidth::Width_DWORD;
	DWORD dwOperandValue = 0, dwTemp = 0, dwScaleFactor = 0;
	BOOL bParsingParens = FALSE, bAddTemp = TRUE, bIsPointer = FALSE, bLiteralOnly = TRUE;

	*pOperandType = EOperandType::OperandType_Invalid;
	*pOperandValue = 0;
	*pReg8 = ECpuReg8::Reg8_Invalid;
	*pReg16 = ECpuReg16::Reg16_Invalid;
	*pReg32 = ECpuReg32::Reg32_Invalid;
	
	const char *pszOperand = strOperand.c_str();

	for(DWORD i=0; i < pszOperand[i] != '\0';)
	{
		if(pszOperand[i] == '[')
		{
			bParsingParens = TRUE;
			bIsPointer = TRUE;
			i++;
			continue;
		}

		if(pszOperand[i] == ']')
		{
			// finish any outstanding math and break
			if(dwScaleFactor)
			{
				dwTemp *= dwScaleFactor;
			}

			if(bAddTemp)
				dwOperandValue += dwTemp;
			else
				dwOperandValue -= dwTemp;

			break;
		}

		if(WidthFromOperand(pszOperand + i) != EOperandWidth::Width_Invalid)
		{
			eWidth = WidthFromOperand(pszOperand + i);
			
			if(eWidth == EOperandWidth::Width_DWORD)
				i += 5;
			else
				i += 4;

			continue;
		}

		ECpuReg8 eReg8 = Reg8FromOperand(pszOperand + i);
		if(eReg8 != ECpuReg8::Reg8_Invalid)
		{
			BYTE *pbRegPtr = ContextRegFromReg8(pContext, eReg8);
			if(!pbRegPtr)
				goto Cleanup;

			if(!bParsingParens)
			{
				eWidth = EOperandWidth::Width_BYTE;
				*pReg8 = eReg8;
				dwOperandValue = (DWORD)*pbRegPtr;
			}
			else
			{
				dwTemp = (DWORD)*pbRegPtr;
			}

			i += 2;
			bLiteralOnly = FALSE;

			continue;
		}

		ECpuReg16 eReg16 = Reg16FromOperand(pszOperand + i);
		if(eReg16 != ECpuReg16::Reg16_Invalid)
		{
			WORD *pwRegPtr = ContextRegFromReg16(pContext, eReg16);
			if(!pwRegPtr)
				goto Cleanup;

			if(!bParsingParens)
			{
				eWidth = EOperandWidth::Width_WORD;
				*pReg16 = eReg16;
				dwOperandValue = (DWORD)*pwRegPtr;
			}
			else
			{
				dwTemp = (DWORD)*pwRegPtr;
			}

			i += 2;
			bLiteralOnly = FALSE;

			continue;
		}

		ECpuReg32 eReg32 = Reg32FromOperand(pszOperand + i);
		if(eReg32 != ECpuReg32::Reg32_Invalid)
		{
			DWORD *pdwRegPtr = ContextRegFromReg32(pContext, eReg32);
			if(!pdwRegPtr)
				goto Cleanup;

			if(!bParsingParens)
			{
				eWidth = EOperandWidth::Width_DWORD;
				*pReg32 = eReg32;
				dwOperandValue = *pdwRegPtr;
			}
			else
			{
				dwTemp = *pdwRegPtr;
			}

			i += 3;
			bLiteralOnly = FALSE;

			continue;
		}

		if(pszOperand[i] == '+')
		{
			if(bParsingParens)
				dwOperandValue = dwTemp;

			bAddTemp = TRUE;
			i++;
			continue;
		}

		if(pszOperand[i] == '-')
		{
			if(bParsingParens)
				dwOperandValue = dwTemp;

			bAddTemp = FALSE;
			i++;
			continue;
		}

		if(pszOperand[i] == '*')
		{
			dwScaleFactor = dwTemp;
			i++;
			continue;
		}

		DWORD dwValue = 0;
		DWORD cch = 0;

		if(StrToInt(pszOperand + i, &cch, &dwValue))
		{
			if(!bParsingParens)
				dwOperandValue = dwValue;
			else
				dwTemp = dwValue;

			i += cch;
			continue;
		}

		i++;
	}

	*pOperandValue = dwOperandValue;
	
	if(bIsPointer)
	{
		switch(eWidth)
		{
		case EOperandWidth::Width_BYTE:
			*pOperandType = EOperandType::OperandType_Mem8;
			break;
		case EOperandWidth::Width_WORD:
			*pOperandType = EOperandType::OperandType_Mem16;
			break;
		case EOperandWidth::Width_DWORD:
			*pOperandType = EOperandType::OperandType_Mem32;
			break;
		}
	}
	else
	{
		if(bLiteralOnly)
		{
			*pOperandType = EOperandType::OperandType_Literal32;
		}
		else
		{
			switch(eWidth)
			{
			case EOperandWidth::Width_BYTE:
				*pOperandType = EOperandType::OperandType_Reg8;
				break;
			case EOperandWidth::Width_WORD:
				*pOperandType = EOperandType::OperandType_Reg16;
				break;
			case EOperandWidth::Width_DWORD:
				*pOperandType = EOperandType::OperandType_Reg32;
				break;
			}
		}
	}

	bSuccess = TRUE;
Cleanup:
	return bSuccess;
}

BOOL StackEmu::WriteMemoryCache(std::map<LPVOID, BYTE> *pMemoryMap, LPVOID lpvPointer, PBYTE pbValuesIn, DWORD dwValuesInLength)
{
	BOOL bSuccess = FALSE;
	PBYTE pbMemPtr = (PBYTE)lpvPointer;

	for(DWORD i=0; i < dwValuesInLength; i++)
	{
		// make sure this is legitimate, were the code to be executing rather than emulating
		if(IsBadWritePtr(pbMemPtr + i, 1))
			goto Cleanup;

		// write to cache
		pMemoryMap->operator[](pbMemPtr + i) = pbValuesIn[i];
	}

	bSuccess = TRUE;
Cleanup:
	return bSuccess;
}

BOOL StackEmu::ReadMemoryCache(std::map<LPVOID, BYTE> *pMemoryMap, LPVOID lpvPointer, PBYTE pbValuesOut, DWORD dwValuesOutLength)
{
	BOOL bSuccess = FALSE;
	PBYTE pbMemPtr = (PBYTE)lpvPointer;

	for(DWORD i=0; i < dwValuesOutLength; i++)
	{
		// ensure the read is legitimate
		if(Utils::IsBadReadPtr(pbMemPtr + i, 1))
			goto Cleanup;

		if(pMemoryMap->find(pbMemPtr + i) != pMemoryMap->end())
		{
			// we have a cached byte for this address
			pbValuesOut[i] = pMemoryMap->operator[](pbMemPtr + i);
		}
		else
		{
			// we will have to read from system memory
			pbValuesOut[i] = pbMemPtr[i];
		}
	}

	bSuccess = TRUE;
Cleanup:
	return bSuccess;
}


BOOL StackEmu::EmulatePop(CONTEXT *pContext, std::map<LPVOID, BYTE> *pMemoryMap, std::string strDest)
{
	BOOL bSuccess = FALSE;

	EOperandType eType;
	DWORD dwValue;

	DWORD *pdwRegPtr = NULL;
	WORD *pwRegPtr = NULL;

	PBYTE pbMemPtr = NULL;
	
	ECpuReg8 eReg8;
	ECpuReg16 eReg16;
	ECpuReg32 eReg32;

	if(!OperandToTypeAndValue(pContext, strDest, &eType, &dwValue, &eReg8, &eReg16, &eReg32))
		goto Cleanup;

	switch(eType)
	{
	case OperandType_Reg16:
		
		pwRegPtr = ContextRegFromReg16(pContext, eReg16);
		if(!pwRegPtr)
			goto Cleanup;

		pbMemPtr = (PBYTE)pContext->Esp;
		
		if(!ReadMemoryCache(pMemoryMap, pbMemPtr, (PBYTE)pwRegPtr, 2))
			goto Cleanup;

		pContext->Esp += 2;

		break;
	case EOperandType::OperandType_Reg32:
		
		pdwRegPtr = ContextRegFromReg32(pContext, eReg32);
		if(!pdwRegPtr)
			goto Cleanup;

		pbMemPtr = (PBYTE)pContext->Esp;
		
		if(!ReadMemoryCache(pMemoryMap, pbMemPtr, (PBYTE)pdwRegPtr, 4))
			goto Cleanup;

		pContext->Esp += 4;

		break;
	case EOperandType::OperandType_Reg8:
	case EOperandType::OperandType_Mem8:
	case EOperandType::OperandType_Mem16:
	case EOperandType::OperandType_Mem32:
	case EOperandType::OperandType_Literal32:
	case EOperandType::OperandType_Invalid:
		goto Cleanup;
	}

	bSuccess = TRUE;
Cleanup:
	return bSuccess;
}

BOOL StackEmu::EmulatePush(CONTEXT *pContext, std::map<LPVOID, BYTE> *pMemoryMap, std::string strSrc)
{
	BOOL bSuccess = FALSE;

	EOperandType eType;
	DWORD dwValue;

	DWORD *pdwRegPtr = NULL;
	WORD *pwRegPtr = NULL;
	
	PBYTE pbMemPtr = NULL;
	
	ECpuReg8 eReg8;
	ECpuReg16 eReg16;
	ECpuReg32 eReg32;

	DWORD dwTemp = 0;

	if(!OperandToTypeAndValue(pContext, strSrc, &eType, &dwValue, &eReg8, &eReg16, &eReg32))
		goto Cleanup;

	switch(eType)
	{
	case OperandType_Reg16:
		
		pContext->Esp -= 2;
		
		if(!WriteMemoryCache(pMemoryMap, (LPVOID)pContext->Esp, (PBYTE)&dwValue, 2))
			goto Cleanup;

		break;
	case EOperandType::OperandType_Reg32:
	case EOperandType::OperandType_Literal32:
		
		pContext->Esp -= 4;
		
		if(!WriteMemoryCache(pMemoryMap, (LPVOID)pContext->Esp, (PBYTE)&dwValue, 4))
			goto Cleanup;

		break;
	case EOperandType::OperandType_Mem16:
		
		pbMemPtr = (PBYTE)dwValue;
		
		if(!ReadMemoryCache(pMemoryMap, pbMemPtr, (PBYTE)&dwTemp, 4))
			goto Cleanup;

		pContext->Esp -= 4; // this is right, it's pushing a dword from a [word] address
		
		if(!WriteMemoryCache(pMemoryMap, (LPVOID)pContext->Esp, (PBYTE)&dwTemp, 4))
			goto Cleanup;
		
		break;
	case EOperandType::OperandType_Mem32:
		
		pbMemPtr = (PBYTE)dwValue;
		
		if(!ReadMemoryCache(pMemoryMap, pbMemPtr, (PBYTE)&dwTemp, 4))
			goto Cleanup;

		pContext->Esp -= 4; // this is right, it's pushing a dword from a [word] address
		
		if(!WriteMemoryCache(pMemoryMap, (LPVOID)pContext->Esp, (PBYTE)&dwTemp, 4))
			goto Cleanup;
		
		break;
	case EOperandType::OperandType_Reg8:
	case EOperandType::OperandType_Mem8:
	case EOperandType::OperandType_Invalid:
		goto Cleanup;
	}

	bSuccess = TRUE;
Cleanup:
	return bSuccess;
}

BOOL StackEmu::EmulateGenericTransformOperation(CONTEXT *pContext, std::map<LPVOID, BYTE> *pMemoryMap, std::string strDest, std::string strSrc, FN_NUMERIC_FUNCTION_DEF pfnNumericTransform)
{
	BOOL bSuccess = FALSE;

	EOperandType eSrcType, eDstType;

	DWORD dwSrcValue, dwDstValue;

	DWORD *pdwSrcRegPtr = NULL, *pdwDstRegPtr = NULL;
	WORD *pwSrcRegPtr = NULL, *pwDstRegPtr = NULL;
	BYTE *pbSrcRegPtr = NULL, *pbDstRegPtr = NULL;
	
	PBYTE pbMemPtr = NULL;
	
	ECpuReg8 eSrcReg8, eDstReg8;
	ECpuReg16 eSrcReg16, eDstReg16;
	ECpuReg32 eSrcReg32, eDstReg32;

	DWORD dwTemp = 0;

	if(!OperandToTypeAndValue(pContext, strSrc, &eSrcType, &dwSrcValue, &eSrcReg8, &eSrcReg16, &eSrcReg32))
		goto Cleanup;

	if(!OperandToTypeAndValue(pContext, strDest, &eDstType, &dwDstValue, &eDstReg8, &eDstReg16, &eDstReg32))
		goto Cleanup;

	switch(eDstType)
	{
	case EOperandType::OperandType_Reg8:
		
		pbDstRegPtr = ContextRegFromReg8(pContext, eDstReg8);
		if(!pbDstRegPtr)
			goto Cleanup;

		switch(eSrcType)
		{
		case EOperandType::OperandType_Mem8:
		case EOperandType::OperandType_Mem16:
		case EOperandType::OperandType_Mem32:
			
			// all widths are byte regardless of width specifier
			
			pbMemPtr = (PBYTE)dwSrcValue;

			if(!ReadMemoryCache(pMemoryMap, pbMemPtr, (PBYTE)&dwTemp, 1))
				goto Cleanup;

			pfnNumericTransform(pbDstRegPtr, dwTemp, EOperandWidth::Width_BYTE);

			break;
		case EOperandType::OperandType_Reg8:
			
			pbSrcRegPtr = ContextRegFromReg8(pContext, eSrcReg8);
			pfnNumericTransform(pbDstRegPtr, *pbSrcRegPtr, EOperandWidth::Width_BYTE);

			break;
		case EOperandType::OperandType_Literal32:

			pfnNumericTransform(pbDstRegPtr, dwSrcValue, EOperandWidth::Width_BYTE);

			break;
		case EOperandType::OperandType_Reg16:
		case EOperandType::OperandType_Reg32:
		case EOperandType::OperandType_Invalid:
			goto Cleanup;
		}

		break;
	case EOperandType::OperandType_Reg16:

		pwDstRegPtr = ContextRegFromReg16(pContext, eDstReg16);
		if(!pwDstRegPtr)
			goto Cleanup;

		switch(eSrcType)
		{
		case EOperandType::OperandType_Mem8:
		case EOperandType::OperandType_Mem16:
		case EOperandType::OperandType_Mem32:
			
			// all widths are word regardless of width specifier
			
			pbMemPtr = (PBYTE)dwSrcValue;

			if(!ReadMemoryCache(pMemoryMap, pbMemPtr, (PBYTE)&dwTemp, 2))
				goto Cleanup;

			pfnNumericTransform(pwDstRegPtr, dwTemp, EOperandWidth::Width_WORD);

			break;
		case EOperandType::OperandType_Reg16:
			
			pwSrcRegPtr = ContextRegFromReg16(pContext, eSrcReg16);
			pfnNumericTransform(pwDstRegPtr, *pwSrcRegPtr, EOperandWidth::Width_WORD);

			break;
		case EOperandType::OperandType_Literal32:

			pfnNumericTransform(pwDstRegPtr, dwSrcValue, EOperandWidth::Width_WORD);

			break;
		case EOperandType::OperandType_Reg8:
		case EOperandType::OperandType_Reg32:
		case EOperandType::OperandType_Invalid:
			goto Cleanup;
		}

		break;
	case EOperandType::OperandType_Reg32:
		
		pdwDstRegPtr = ContextRegFromReg32(pContext, eDstReg32);
		if(!pdwDstRegPtr)
			goto Cleanup;

		switch(eSrcType)
		{
		case EOperandType::OperandType_Mem8:
		case EOperandType::OperandType_Mem16:
		case EOperandType::OperandType_Mem32:
			
			// all widths are dword regardless of width specifier
			
			pbMemPtr = (PBYTE)dwSrcValue;

			if(!ReadMemoryCache(pMemoryMap, pbMemPtr, (PBYTE)&dwTemp, 4))
				goto Cleanup;

			pfnNumericTransform(pdwDstRegPtr, dwTemp, EOperandWidth::Width_DWORD);

			break;
		case EOperandType::OperandType_Reg32:
			
			pdwSrcRegPtr = ContextRegFromReg32(pContext, eSrcReg32);
			pfnNumericTransform(pdwDstRegPtr, *pdwSrcRegPtr, EOperandWidth::Width_DWORD);
			
			break;
		case EOperandType::OperandType_Literal32:

			pfnNumericTransform(pdwDstRegPtr, dwSrcValue, EOperandWidth::Width_DWORD);
			
			break;
		case EOperandType::OperandType_Reg8:
		case EOperandType::OperandType_Reg16:
		case EOperandType::OperandType_Invalid:
			goto Cleanup;
		}

		break;
		// add mov <byte|word|dword> ptr [mem], val
	case EOperandType::OperandType_Mem8:

		pbMemPtr = (PBYTE)dwDstValue;

		switch(eSrcType)
		{
		case EOperandType::OperandType_Reg8:
			
			pbSrcRegPtr = ContextRegFromReg8(pContext, eSrcReg8);
			if(!pbSrcRegPtr)
				goto Cleanup;

			if(!ReadMemoryCache(pMemoryMap, pbMemPtr, (PBYTE)&dwTemp, 1))
				goto Cleanup;

			pfnNumericTransform(&dwTemp, *pbSrcRegPtr, EOperandWidth::Width_BYTE);
			
			if(!WriteMemoryCache(pMemoryMap, pbMemPtr, (PBYTE)&dwTemp, 1))
				goto Cleanup;

			break;
		case EOperandType::OperandType_Literal32:
			
			if(!ReadMemoryCache(pMemoryMap, pbMemPtr, (PBYTE)&dwTemp, 1))
				goto Cleanup;

			pfnNumericTransform(&dwTemp, dwSrcValue, EOperandWidth::Width_BYTE);
			
			if(!WriteMemoryCache(pMemoryMap, pbMemPtr, (PBYTE)&dwTemp, 1))
				goto Cleanup;

			break;
			// all invalid: mov [mem], [mem] OR mov [mem], <word|dword> reg
		case EOperandType::OperandType_Reg16:
		case EOperandType::OperandType_Reg32:
		case EOperandType::OperandType_Mem8:
		case EOperandType::OperandType_Mem16:
		case EOperandType::OperandType_Mem32:
		case EOperandType::OperandType_Invalid:
			goto Cleanup;
		}

		break;
	case EOperandType::OperandType_Mem16:

		pbMemPtr = (PBYTE)dwDstValue;

		switch(eSrcType)
		{
		case EOperandType::OperandType_Reg16:
			
			pwSrcRegPtr = ContextRegFromReg16(pContext, eSrcReg16);
			if(!pwSrcRegPtr)
				goto Cleanup;

			if(!ReadMemoryCache(pMemoryMap, pbMemPtr, (PBYTE)&dwTemp, 2))
				goto Cleanup;

			pfnNumericTransform(&dwTemp, *pwSrcRegPtr, EOperandWidth::Width_WORD);
			
			if(!WriteMemoryCache(pMemoryMap, pbMemPtr, (PBYTE)&dwTemp, 2))
				goto Cleanup;

			break;
		case EOperandType::OperandType_Literal32:
			
			if(!ReadMemoryCache(pMemoryMap, pbMemPtr, (PBYTE)&dwTemp, 2))
				goto Cleanup;

			pfnNumericTransform(&dwTemp, dwSrcValue, EOperandWidth::Width_WORD);
			
			if(!WriteMemoryCache(pMemoryMap, pbMemPtr, (PBYTE)&dwTemp, 2))
				goto Cleanup;

			break;
			// all invalid: mov [mem], [mem] OR mov [mem], <word|dword> reg
		case EOperandType::OperandType_Reg8:
		case EOperandType::OperandType_Reg32:
		case EOperandType::OperandType_Mem8:
		case EOperandType::OperandType_Mem16:
		case EOperandType::OperandType_Mem32:
		case EOperandType::OperandType_Invalid:
			goto Cleanup;
		}

		break;
	case EOperandType::OperandType_Mem32:

		pbMemPtr = (PBYTE)dwDstValue;

		switch(eSrcType)
		{
		case EOperandType::OperandType_Reg32:
			
			pdwSrcRegPtr = ContextRegFromReg32(pContext, eSrcReg32);
			if(!pdwSrcRegPtr)
				goto Cleanup;

			if(!ReadMemoryCache(pMemoryMap, pbMemPtr, (PBYTE)&dwTemp, 4))
				goto Cleanup;

			pfnNumericTransform(&dwTemp, *pdwSrcRegPtr, EOperandWidth::Width_DWORD);
			
			if(!WriteMemoryCache(pMemoryMap, pbMemPtr, (PBYTE)&dwTemp, 4))
				goto Cleanup;

			break;
		case EOperandType::OperandType_Literal32:
			
			if(!ReadMemoryCache(pMemoryMap, pbMemPtr, (PBYTE)&dwTemp, 4))
				goto Cleanup;

			pfnNumericTransform(&dwTemp, dwSrcValue, EOperandWidth::Width_DWORD);
			
			if(!WriteMemoryCache(pMemoryMap, pbMemPtr, (PBYTE)&dwTemp, 4))
				goto Cleanup;

			break;
			// all invalidL mov [mem], [mem] OR mov [mem], <word|dword> reg
		case EOperandType::OperandType_Reg8:
		case EOperandType::OperandType_Reg16:
		case EOperandType::OperandType_Mem8:
		case EOperandType::OperandType_Mem16:
		case EOperandType::OperandType_Mem32:
		case EOperandType::OperandType_Invalid:
			goto Cleanup;
		}

		break;
	case EOperandType::OperandType_Literal32:
	case EOperandType::OperandType_Invalid:
		goto Cleanup;
	}

	bSuccess = TRUE;
Cleanup:
	return bSuccess;
}

BOOL StackEmu::EmulateMov(CONTEXT *pContext, std::map<LPVOID, BYTE> *pMemoryMap, std::string strDest, std::string strSrc)
{
	return EmulateGenericTransformOperation(pContext, pMemoryMap, strDest, strSrc, MovNumericValues);
}

BOOL StackEmu::EmulateAdd(CONTEXT *pContext, std::map<LPVOID, BYTE> *pMemoryMap, std::string strDest, std::string strSrc)
{
	return EmulateGenericTransformOperation(pContext, pMemoryMap, strDest, strSrc, AddNumericValues);
}

BOOL StackEmu::EmulateSub(CONTEXT *pContext, std::map<LPVOID, BYTE> *pMemoryMap, std::string strDest, std::string strSrc)
{
	return EmulateGenericTransformOperation(pContext, pMemoryMap, strDest, strSrc, SubNumericValues);
}

BOOL StackEmu::EmulateXor(CONTEXT *pContext, std::map<LPVOID, BYTE> *pMemoryMap, std::string strDest, std::string strSrc)
{
	return EmulateGenericTransformOperation(pContext, pMemoryMap, strDest, strSrc, XorNumericValues);
}

BOOL StackEmu::EmulateRetn(CONTEXT *pContext, std::map<LPVOID, BYTE> *pMemoryMap, std::string strSize)
{
	BOOL bSuccess = FALSE;

	EOperandType eType = EOperandType::OperandType_Invalid;
	DWORD dwValue = 0;

	DWORD *pdwRegPtr = NULL;
	WORD *pwRegPtr = NULL;
	PBYTE pbMemPtr = NULL;
	
	ECpuReg8 eReg8;
	ECpuReg16 eReg16;
	ECpuReg32 eReg32;

	if(!EmulatePop(pContext, pMemoryMap, "EIP"))
		goto Cleanup;

	if(!strSize.empty())
	{
		if(!OperandToTypeAndValue(pContext, strSize, &eType, &dwValue, &eReg8, &eReg16, &eReg32))
			goto Cleanup;

		if(eType != EOperandType::OperandType_Literal32)
			goto Cleanup;

		pContext->Esp += dwValue;
	}

	bSuccess = TRUE;
Cleanup:
	return bSuccess;
}

BOOL StackEmu::AddNumericValues(LPVOID lpvResult, DWORD dwRhs, EOperandWidth eWidth)
{
	switch(eWidth)
	{
	case EOperandWidth::Width_BYTE: *(BYTE *)lpvResult += (BYTE)dwRhs; break;
	case EOperandWidth::Width_WORD: *(BYTE *)lpvResult += (BYTE)dwRhs; break;
	case EOperandWidth::Width_DWORD: *(DWORD *)lpvResult += dwRhs; break;
	}

	return TRUE;
}

BOOL StackEmu::SubNumericValues(LPVOID lpvResult, DWORD dwRhs, EOperandWidth eWidth)
{
	switch(eWidth)
	{
	case EOperandWidth::Width_BYTE: *(BYTE *)lpvResult -= (BYTE)dwRhs; break;
	case EOperandWidth::Width_WORD: *(BYTE *)lpvResult -= (BYTE)dwRhs; break;
	case EOperandWidth::Width_DWORD: *(DWORD *)lpvResult -= dwRhs; break;
	}

	return TRUE;
}

BOOL StackEmu::MovNumericValues(LPVOID lpvResult, DWORD dwRhs, EOperandWidth eWidth)
{
	switch(eWidth)
	{
	case EOperandWidth::Width_BYTE: *(BYTE *)lpvResult = (BYTE)dwRhs; break;
	case EOperandWidth::Width_WORD: *(BYTE *)lpvResult = (BYTE)dwRhs; break;
	case EOperandWidth::Width_DWORD: *(DWORD *)lpvResult = dwRhs; break;
	}

	return TRUE;
}

BOOL StackEmu::XorNumericValues(LPVOID lpvResult, DWORD dwRhs, EOperandWidth eWidth)
{
	switch(eWidth)
	{
	case EOperandWidth::Width_BYTE: *(BYTE *)lpvResult ^= (BYTE)dwRhs; break;
	case EOperandWidth::Width_WORD: *(BYTE *)lpvResult ^= (BYTE)dwRhs; break;
	case EOperandWidth::Width_DWORD: *(DWORD *)lpvResult ^= dwRhs; break;
	}

	return TRUE;
}
