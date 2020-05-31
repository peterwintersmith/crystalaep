// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#pragma once

namespace StackEmu
{
#define STACKEMU_MAX_INSTR		10

	enum ECpuReg8
	{
		Reg8_Invalid = -1,
		Reg8_AL = 0,
		Reg8_CL,
		Reg8_DL,
		Reg8_BL,
		Reg8_AH,
		Reg8_CH,
		Reg8_DH,
		Reg8_BH,
	};

	enum ECpuReg16
	{
		Reg16_Invalid = -1,
		Reg16_AX = 0,
		Reg16_CX,
		Reg16_DX,
		Reg16_BX,
		Reg16_SP,
		Reg16_BP,
		Reg16_SI,
		Reg16_DI
	};

	enum ECpuReg32
	{
		Reg32_Invalid = -1,
		Reg32_EAX = 0,
		Reg32_ECX,
		Reg32_EDX,
		Reg32_EBX,
		Reg32_ESP,
		Reg32_EBP,
		Reg32_ESI,
		Reg32_EDI,
		// inaccessible as no code can compile down to this
		Reg32_EIP
	};

	enum EOperandWidth
	{
		Width_Invalid = -1,
		Width_BYTE = 0,
		Width_WORD,
		Width_DWORD
	};

	enum EOperandType
	{
		OperandType_Invalid = -1,
		OperandType_Reg8 = 0,
		OperandType_Reg16,
		OperandType_Reg32,
		OperandType_Mem8,
		OperandType_Mem16,
		OperandType_Mem32,
		OperandType_Literal32
	};

	enum ECommonInstruction
	{
		Instr_Invalid = -1,
		Instr_pop = 0,
		Instr_push,
		Instr_mov,
		Instr_add,
		Instr_sub,
		Instr_xor,
		Instr_retn,
		// to do:
		Instr_call,
		Instr_leave,
		Instr_enter,
		Instr_lea,
		Instr_jnz,
		Instr_cmp,
		Instr_test,
		Instr_jz,
		Instr_jmp,
		Instr_and,
		Instr_inc,
		Instr_dec,
		Instr_movzx,
		Instr_movsx,
		Instr_or,
		Instr_sbb
	};

	// interface functions
	BOOL CalculateStackDelta(CONTEXT *pContext, PBYTE pbInstr, int *piDelta,  BOOL *p_bEmuSuccess, BOOL bEmulateRetn, BOOL bUpdateContext);

	// helper functions
	std::string GetMnemFromDisasm(char *pszDisasmText);
	ECommonInstruction InstrFromMnem(std::string strMnem);
	int GetOperandContainingReg(char *pszRegister, char *pszDisasmText);
	std::string GetOperandFromDisasm(int iOperand, char *pszDisasmText);
	BOOL StrToInt(std::string strValue, DWORD *pnChars, DWORD *pdwValue);
	ECpuReg8 Reg8FromOperand(std::string strOperand);
	ECpuReg16 Reg16FromOperand(std::string strOperand);
	ECpuReg32 Reg32FromOperand(std::string strOperand);
	BYTE *ContextRegFromReg8(CONTEXT *pContext, ECpuReg8 eReg8);
	WORD *ContextRegFromReg16(CONTEXT *pContext, ECpuReg16 eReg16);
	DWORD *ContextRegFromReg32(CONTEXT *pContext, ECpuReg32 eReg32);
	EOperandWidth WidthFromOperand(std::string strOperand);
	void Trim(std::string& str);
	BOOL OperandToTypeAndValue(
		CONTEXT *pContext, std::string strOperand, EOperandType *pOperandType,
		DWORD *pOperandValue, ECpuReg8 *pReg8, ECpuReg16 *pReg16, ECpuReg32 *pReg32
		);
	BOOL WriteMemoryCache(std::map<LPVOID, BYTE> *pMemoryMap, LPVOID lpvPointer, PBYTE pbValuesIn, DWORD dwValuesInLength);
	BOOL ReadMemoryCache(std::map<LPVOID, BYTE> *pMemoryMap, LPVOID lpvPointer, PBYTE pbValuesOut, DWORD dwValuesOutLength);

	typedef BOOL (*FN_NUMERIC_FUNCTION_DEF)(LPVOID lpvResult, DWORD dwRhs, EOperandWidth eWidth);
	
	// emulator functions
	BOOL EmulatePop(CONTEXT *pContext, std::map<LPVOID, BYTE> *pMemoryMap, std::string strDest);
	BOOL EmulatePush(CONTEXT *pContext, std::map<LPVOID, BYTE> *pMemoryMap, std::string strSrc);
	BOOL EmulateGenericTransformOperation(CONTEXT *pContext, std::map<LPVOID, BYTE> *pMemoryMap, std::string strDest, std::string strSrc, FN_NUMERIC_FUNCTION_DEF pfnNumericTransform);
	/**/BOOL EmulateMov(CONTEXT *pContext, std::map<LPVOID, BYTE> *pMemoryMap, std::string strDest, std::string strSrc);
	/**/BOOL EmulateAdd(CONTEXT *pContext, std::map<LPVOID, BYTE> *pMemoryMap, std::string strDest, std::string strSrc);
	/**/BOOL EmulateSub(CONTEXT *pContext, std::map<LPVOID, BYTE> *pMemoryMap, std::string strDest, std::string strSrc);
	/**/BOOL EmulateXor(CONTEXT *pContext, std::map<LPVOID, BYTE> *pMemoryMap, std::string strDest, std::string strSrc);
	/**/BOOL EmulateRetn(CONTEXT *pContext, std::map<LPVOID, BYTE> *pMemoryMap, std::string strSize);
	
	// auxilliary functions
	BOOL AddNumericValues(LPVOID lpvResult, DWORD dwRhs, EOperandWidth eWidth);
	BOOL SubNumericValues(LPVOID lpvResult, DWORD dwRhs, EOperandWidth eWidth);
	BOOL MovNumericValues(LPVOID lpvResult, DWORD dwRhs, EOperandWidth eWidth);
	BOOL XorNumericValues(LPVOID lpvResult, DWORD dwRhs, EOperandWidth eWidth);
}