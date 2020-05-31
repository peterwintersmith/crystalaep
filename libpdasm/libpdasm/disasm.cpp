// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]
// (v) 17/Oct/11, 01/Mar/12

#include "stdafx.h"
#include "disasm.h"

namespace StringUtils
{
	void sprintfcat(char szBuf[], char szFmt[], ...)
	{
		va_list ap;

		va_start(ap, szFmt);

		DWORD len = strlen(szBuf);
		vsprintf(szBuf + len, szFmt, ap);

		va_end(ap);
	}

	void strrepl(char szBuf[], char szFind[], char szReplace[], BOOL bCaseSensitive)
	{
		DWORD cchFind = strlen(szFind),
			  cchReplace = strlen(szReplace),
			  cchBuf = strlen(szBuf);

		for(DWORD i=0; szBuf[i] != '\0';)
		{
			if( (bCaseSensitive  && !strncmp( szBuf + i, szFind, cchFind)) ||
				(!bCaseSensitive && !strnicmp(szBuf + i, szFind, cchFind)) )
			{
				// match
				// AAAAAAAABBBBCCCCCCC      /BBBB/DDDDDD
				// AAAAAAAA      CCCCCCC
				// AAAAAAAADDDDDDCCCCCCC

				memmove(szBuf + i + cchReplace, szBuf + i + cchFind, cchBuf - cchFind - i + 1);
				memcpy(szBuf + i, szReplace, cchReplace);

				i += cchReplace;
				continue;
			}

			i++;
		}
	}
}

namespace Disasm
{
	DWORD FindTwoByteOffsetStart()
	{
		for(DWORD i=0; i < NumberOfInstructions; i++)
		{
			if(TableOfInstructions[i].Prefix0F == TRUE)
				return i;
		}

		return 0;
	}

	void RewriteExactOperand(char szOperand[128], char *pszSeg, char *pszAddrWidth, BOOL bOpndSizePrefix, BOOL bAddrSizePrefix)
	{
		StringUtils::strrepl(szOperand, "(", "");
		StringUtils::strrepl(szOperand, ")", "");

		if(strstr(szOperand, "ST") != NULL)
		{
			StringUtils::strrepl(szOperand, "ST0", "_S_T_0");
			StringUtils::strrepl(szOperand, "ST1", "_S_T_1");
			StringUtils::strrepl(szOperand, "ST2", "_S_T_2");
			StringUtils::strrepl(szOperand, "ST3", "_S_T_3");
			StringUtils::strrepl(szOperand, "ST4", "_S_T_4");
			StringUtils::strrepl(szOperand, "ST5", "_S_T_5");
			StringUtils::strrepl(szOperand, "ST6", "_S_T_6");
			StringUtils::strrepl(szOperand, "ST7", "_S_T_7");
			
			StringUtils::strrepl(szOperand, "ST", "ST(0)");

			StringUtils::strrepl(szOperand, "_S_T_0", "ST(0)");
			StringUtils::strrepl(szOperand, "_S_T_1", "ST(1)");
			StringUtils::strrepl(szOperand, "_S_T_2", "ST(2)");
			StringUtils::strrepl(szOperand, "_S_T_3", "ST(3)");
			StringUtils::strrepl(szOperand, "_S_T_4", "ST(4)");
			StringUtils::strrepl(szOperand, "_S_T_5", "ST(5)");
			StringUtils::strrepl(szOperand, "_S_T_6", "ST(6)");
			StringUtils::strrepl(szOperand, "_S_T_7", "ST(7)");
		}
		
		char *rgszReg16Bit[] = {
			"AX", "CX", "DX", "BX", "SP", "BP", "SI", "DI", NULL
		};

		char szReg[32];

		for(DWORD i=0; rgszReg16Bit[i] != NULL; i++)
		{
			// make all registers 16-bit

			sprintf(szReg, "r%s", rgszReg16Bit[i]);
			StringUtils::strrepl(szOperand, szReg, rgszReg16Bit[i]);
			
			sprintf(szReg, "E%s", rgszReg16Bit[i]);
			StringUtils::strrepl(szOperand, szReg, rgszReg16Bit[i]);
		}

		for(DWORD i=0; rgszReg16Bit[i] != NULL; i++)
		{
			char szRepl[32];

			// if not operand size prefix, update to 32-bit reg

			sprintf(szRepl, "%s%s", bOpndSizePrefix ? "" : "E", rgszReg16Bit[i]);
			StringUtils::strrepl(szOperand, rgszReg16Bit[i], szRepl);
		}

		for(DWORD i=0; rgszReg16Bit[i] != NULL; i++)
		{
			char szRepl[32];

			// if not address size prefix update all dereferences to 32-bit reg

			sprintf(szReg, "[%s]", rgszReg16Bit[i]);
			sprintf(szRepl, "[%s%s]", bAddrSizePrefix ? "" : "E", rgszReg16Bit[i]);

			StringUtils::strrepl(szOperand, szReg, szRepl);
		}

		char *rgszSegs[] = {
			"ES:", "CS:", "DS:", "FS:", "GS:", "SS:", NULL
		};

		BOOL bSeg = FALSE;

		if(pszSeg && pszSeg[0])
		{
			bSeg = TRUE;

			for(DWORD i=0; rgszSegs[i] != NULL; i++)
			{
				// remove segment prefixes
				StringUtils::strrepl(szOperand, rgszSegs[i], "");
			}
		}

		char szFormattedAddr[64];

		for(DWORD i=0; szOperand[i]; i++)
		{
			if(szOperand[i] == '[')
			{
				sprintf(szFormattedAddr, "%s PTR %s", pszAddrWidth ? pszAddrWidth : (bOpndSizePrefix ? "WORD" : "DWORD"), bSeg ? pszSeg : "");
				DWORD cch = strlen(szFormattedAddr);

				memmove(szOperand + i + cch, szOperand + i, strlen(szOperand + i) + 1);
				memcpy(szOperand + i, szFormattedAddr, cch);

				i += cch;
			}
		}
	}

	void GetBits(BYTE in, BOOL bits[8])
	{
		for(int i=0; i < 8; i++)
		{
			bits[7 - i] = (in & (1 << i)) != 0;
		}
	}

	int BitsXtoYAsInt(BYTE in, DWORD x, DWORD y)
	{
		if(x >= y || x > 7 || y > 7)
			return -1;

		BOOL bits[8];
		
		GetBits(in, bits);

		int integer = 0;

		for(int i = (y - x); x <= y; x++)
		{
			integer |= (bits[x] << i--);
		}
		
		return integer;
	}

	int GetModRMMod(BYTE in)
	{
		return BitsXtoYAsInt(in, 0, 1);
	}

	int GetModRMReg(BYTE in)
	{
		return BitsXtoYAsInt(in, 2, 4);
	}

	int GetModRMRegMem(BYTE in)
	{
		return BitsXtoYAsInt(in, 5, 7);
	}

	int IsFPUCommand(BYTE in)
	{
		return BitsXtoYAsInt(in, 0, 4) == 0x1b;
	}

	int GetSIBScale(BYTE in)
	{
		return BitsXtoYAsInt(in, 0, 1);
	}

	int GetSIBIndex(BYTE in)
	{
		return BitsXtoYAsInt(in, 2, 4);
	}

	int GetSIBBase(BYTE in)
	{
		return BitsXtoYAsInt(in, 5, 7);
	}

	BOOL OpndIsValid(DWORD Flags)
	{
		return (Flags & DR_MASK) != EOperandDirection::INVALID_DIRECTION;
	}

	BOOL OpndIsSource(DWORD Flags)
	{
		return (Flags & DR_MASK) == EOperandDirection::Source;
	}

	DWORD GetPrefixCount(PBYTE pInstr, BYTE prefixes[4])
	{
		struct
		{
			ECommandPrefix Prefix;
		} ValidPrefix[] = {
			{ ECommandPrefix::PrefixREPZ },
			{ ECommandPrefix::PrefixREPNZ },
			{ ECommandPrefix::PrefixLOCK },
			{ ECommandPrefix::PrefixAddrSize },
			{ ECommandPrefix::PrefixOpndSize },
			{ ECommandPrefix::PrefixCodeSeg },
			{ ECommandPrefix::PrefixStackSeg },
			{ ECommandPrefix::PrefixDataSeg },
			{ ECommandPrefix::PrefixExtraSeg },
			{ ECommandPrefix::PrefixFileSeg },
			{ ECommandPrefix::PrefixGenSeg },
			{ (ECommandPrefix)0 }
		};

		DWORD dwLength = 0;
		
		for(int i=0; i < 4; i++)
		{
			BOOL bMatch = FALSE;

			for(int j=0; ValidPrefix[j].Prefix != 0; j++)
			{
				if(pInstr[i] == ValidPrefix[j].Prefix)
				{
					prefixes[dwLength] = ValidPrefix[j].Prefix;
					dwLength++;

					bMatch = TRUE;

					break;
				}
			}

			if(!bMatch)
				break;
		}

		return dwLength;
	}

	DWORD SegPrefixCount(BYTE prefix[4], DWORD len)
	{
		struct
		{
			ECommandPrefix Prefix;
			char *Text;
		} ValidPrefix[] = {
			{ ECommandPrefix::PrefixCodeSeg },
			{ ECommandPrefix::PrefixStackSeg },
			{ ECommandPrefix::PrefixDataSeg },
			{ ECommandPrefix::PrefixExtraSeg },
			{ ECommandPrefix::PrefixFileSeg },
			{ ECommandPrefix::PrefixGenSeg },
			{ (ECommandPrefix)0 }
		};

		DWORD dwCount = 0;

		for(DWORD i=0; i < len; i++)
		{
			for(DWORD j=0; ValidPrefix[j].Prefix != 0; j++)
			{
				if(prefix[i] == ValidPrefix[j].Prefix)
				{
					dwCount++;
					break;
				}
			}
		}

		return dwCount;
	}

	DWORD RepPrefixCount(BYTE prefix[4], DWORD len)
	{
		struct
		{
			ECommandPrefix Prefix;
		} ValidPrefix[] = {
			{ ECommandPrefix::PrefixREPZ },
			{ ECommandPrefix::PrefixREPNZ },
			{ (ECommandPrefix)0 }
		};
		
		DWORD dwCount = 0;

		for(DWORD i=0; i < len; i++)
		{
			for(DWORD j=0; ValidPrefix[j].Prefix != 0; j++)
			{
				if(prefix[i] == ValidPrefix[j].Prefix)
				{
					dwCount++;
					break;
				}
			}
		}

		return dwCount;
	}

	BOOL ContainsPrefix(BYTE prefix[4], DWORD len, ECommandPrefix item)
	{
		for(DWORD i=0; i < len; i++)
		{
			if(prefix[i] == item)
				return TRUE;
		}

		return FALSE;
	}

	int BinSearchOpcodeRange(BOOL bPrefix0F, BYTE PrimaryOpcode)
	{
		int start = 0;
		int end = bPrefix0F ? (NumberOfInstructions - TwoByteOpcodeOffset) : TwoByteOpcodeOffset;
		int offs = (end - start) / 2;
		int ulimit = (end - start);
		int llimit = 0;

		if(PrimaryOpcode == 0)
			return 0;

		while(true)
		{
			if(TableOfInstructions[(bPrefix0F ? TwoByteOpcodeOffset : 0) + offs].PrimaryOpcode < PrimaryOpcode)
			{
				if(offs <= llimit)
					return -1;

				llimit = offs;
				offs = offs + ((ulimit - llimit) >> 1);
			}
			else if(TableOfInstructions[(bPrefix0F ? TwoByteOpcodeOffset : 0) + offs].PrimaryOpcode > PrimaryOpcode)
			{
				if(offs >= ulimit)
					return -1;

				ulimit = offs;
				offs = offs - ((ulimit - llimit) >> 1);
			}
			else
			{
				while((offs - 1) >= start)
				{
					if(TableOfInstructions[(bPrefix0F ? TwoByteOpcodeOffset : 0) + offs - 1].PrimaryOpcode != PrimaryOpcode)
					{
						return ((bPrefix0F ? TwoByteOpcodeOffset : 0) + offs);
					}

					offs--;
				}
			}
		}

		return -1;
	}

	int GetCommandIndex(PBYTE pInstrBuf)
	{
		BYTE prefixes[4];
		DWORD dwPrefixLen = GetPrefixCount(pInstrBuf, prefixes);

		pInstrBuf += dwPrefixLen;

		BOOL bPrefix0F = pInstrBuf[0] == 0x0f;

		if(bPrefix0F)
			pInstrBuf++;

		int index = -1;

		for(DWORD i = 0 /*BinSearchOpcodeRange(bPrefix0F, pInstrBuf[0])*/; i < NumberOfInstructions; i++)
		{
			PBYTE pInstr = pInstrBuf;

			// if(strcmp(TableOfInstructions[i].Mnemonic, "JMPF") == 0) DebugBreak();
			// if(TableOfInstructions[i].PrimaryOpcode == 0xBF) DebugBreak();

			if(TableOfInstructions[i].Prefix0F != bPrefix0F)
				continue;

			if(TableOfInstructions[i].PrimaryOpcode != pInstr[0])
				continue;

			pInstr++; // primary opcode

			if(TableOfInstructions[i].SecondaryOpcode != 0)
			{
				if(TableOfInstructions[i].SecondaryOpcode != pInstr[0])
					continue;

				// when the second opcode is matched, this supersedes the opcode extension
				index = i;
				break;
			}

			// opcode extension is stored in bits 2-4 (reg) of modr/m (reg portion) if specified

			if(TableOfInstructions[i].OpcodeExtension != -1)
			{
				if(OpndIsValid(TableOfInstructions[i].Operand1.Flags) == FALSE)
				{
					//DEBUGBREAK(-1);
					// HACK FOR SFENCE INSTR. PERHAPS ANOTHER FIX IS IN ORDER.
					// continue; // actually, is this an invalid table entry?!
				}

				EAddressingMode AM = (EAddressingMode)(TableOfInstructions[i].Operand1.Flags & AM_MASK);
				EOperandType OT = (EOperandType)(TableOfInstructions[i].Operand1.Flags & OT_MASK);
				
				if(AM == EAddressingMode::AM_INVALID_MARKER && OpndIsValid(TableOfInstructions[i].Operand2.Flags))
					AM = (EAddressingMode)(TableOfInstructions[i].Operand2.Flags & AM_MASK);

				if(AM == EAddressingMode::AM_INVALID_MARKER && OpndIsValid(TableOfInstructions[i].Operand3.Flags))
					AM = (EAddressingMode)(TableOfInstructions[i].Operand3.Flags & AM_MASK);
				
				if(AM == EAddressingMode::AM_INVALID_MARKER && OpndIsValid(TableOfInstructions[i].Operand4.Flags))
					AM = (EAddressingMode)(TableOfInstructions[i].Operand4.Flags & AM_MASK);
				
				
				BYTE modRM = pInstr[0];
				BOOL bExtnUsed = FALSE;

				if(AM == EAddressingMode::AM_INVALID_MARKER)
				{
					// an invalid entry: opcode extension but no mod r/m byte to check
					// HACK FOR SFENCE INSTR. PERHAPS ANOTHER FIX IS IN ORDER.
					bExtnUsed = TRUE;
					//continue;
				}

				switch(AM)
				{
				case EAddressingMode::AM_A:

					// Direct address. The instruction has no ModR/M byte; the address of the operand is
					// encoded in the instruction; no base register, index register, or scaling factor
					// can be applied (for example, far JMP (EA)).

					DEBUGBREAK(-1); // almost certainly invalid: opcode extn stored in reg field of mod r/m

					break;
				case EAddressingMode::AM_M:
			
					// The ModR/M byte may refer only to memory: mod != 11bin (BOUND, LEA, CALLF, JMPF,
					// LES, LDS, LSS, LFS, LGS, CMPXCHG8B, CMPXCHG16B, F20FF0 LDDQU).

					if(GetModRMMod(modRM) == 3)
						continue;

					bExtnUsed = TRUE;

					break;
				case EAddressingMode::AM_E:
					
					// A ModR/M byte follows the opcode and specifies the operand. The operand is either a
					// general-purpose register or a memory address. If it is a memory address, the address
					// is computed from a segment register and any of the following values: a base
					// register, an index register, a scaling factor, or a displacement.

					bExtnUsed = TRUE;

					break;
				case EAddressingMode::AM_ES:
				
					// (Implies original E). A ModR/M byte follows the opcode and specifies the operand.
					// The operand is either a x87 FPU stack register or a memory address. If it is a memory
					// address, the address is computed from a segment register and any of the following
					// values: a base register, an index register, a scaling factor, or a displacement.
					
					bExtnUsed = TRUE;

					break;
				case EAddressingMode::AM_EST:
					
					// (Implies original E). A ModR/M byte follows the opcode and specifies the x87 FPU
					// stack register.
					
					// i.e. only register modr/m bits valid - 11b?

					if(GetModRMMod(modRM) != 3)
					{
						// check it out: not sure if this precludes the modr/m being other than 11b
						//DebugBreak();
						continue;
					}

					bExtnUsed = TRUE;

					break;
				case EAddressingMode::AM_Z:
					
					// The instruction has no ModR/M byte; the three least-significant bits of the opcode
					// byte selects a general-purpose register

					DEBUGBREAK(-1); // almost certainly invalid: opcode extn stored in reg field of mod r/m

					break;
				case EAddressingMode::AM_G:
				
					// The reg field of the ModR/M byte selects a general register (for example, AX (000)).

					DEBUGBREAK(-1); // should select a reg not an opcode extension
					
					break;
				case EAddressingMode::AM_I:

					// Immediate data. The operand value is encoded in subsequent bytes of the instruction.

					DEBUGBREAK(-1); // almost certainly invalid: opcode extn stored in reg field of mod r/m

					break;
				case EAddressingMode::AM_J:

					// The instruction contains a relative offset to be added to the instruction pointer
					// register (for example, JMP (E9), LOOP)).

					DEBUGBREAK(-1); // almost certainly invalid: opcode extn stored in reg field of mod r/m

					break;
				case EAddressingMode::AM_R:
					
					// The mod field of the ModR/M byte may refer only to a general register (only MOV (0F20-0F24, 0F26)).

					if(GetModRMMod(modRM) != 3)
						continue;

					bExtnUsed = TRUE;

					break;
				case EAddressingMode::AM_S:

					// The reg field of the ModR/M byte selects a segment register (only MOV (8C, 8E)).

					DEBUGBREAK(-1); // should select a segment reg not an extension

					break;
				case EAddressingMode::AM_O:
				
					// The instruction has no ModR/M byte; the offset of the operand is coded as a word,
					// double word or quad word (depending on address size attribute) in the instruction.
					// No base register, index register, or scaling factor can be applied (only MOV 
					// (A0, A1, A2, A3)).
			
					DEBUGBREAK(-1); // almost certainly invalid: opcode extn stored in reg field of mod r/m

					break;
				case EAddressingMode::AM_V:

					// The reg field of the ModR/M byte selects a 128-bit XMM register.
			
					DEBUGBREAK(-1); // should select an xmm reg not an extension

					break;
				case EAddressingMode::AM_W:
				
					// A ModR/M byte follows the opcode and specifies the operand. The operand is either a
					// 128-bit XMM register or a memory address. If it is a memory address, the address is
					// computed from a segment register and any of the following values: a base register,
					// an index register, a scaling factor, and a displacement
			
					DEBUGBREAK(-1); // not sure whether this one is valid or not

					break;
				case EAddressingMode::AM_H:

					// The r/m field of the ModR/M byte always selects a general register, regardless of the
					// mod field (for example, MOV (0F20)).

					bExtnUsed = TRUE;

					break;
				case EAddressingMode::AM_C:

					// The reg field of the ModR/M byte selects a control register (only MOV (0F20, 0F22)).

					DEBUGBREAK(-1); // reg field should select a control reg not an extension

					break;
				case EAddressingMode::AM_D:

					// The reg field of the ModR/M byte selects a debug register (only MOV (0F21, 0F23)).

					DEBUGBREAK(-1); // reg field should select a debug reg not an extension

					break;
				case EAddressingMode::AM_P:

					// The reg field of the ModR/M byte selects a packed quadword MMX technology register.

					DEBUGBREAK(-1); // reg field should select an MMX reg not an extension

					break;
				case EAddressingMode::AM_N:
				
					// The R/M field of the ModR/M byte selects a packed quadword MMX technology register.

					bExtnUsed = TRUE;

					break;
				case EAddressingMode::AM_U:

					// The R/M field of the ModR/M byte selects a 128-bit XMM register.

					bExtnUsed = TRUE;

					break;
				case EAddressingMode::AM_Q:

					// A ModR/M byte follows the opcode and specifies the operand. The operand is either an
					// MMX technology register or a memory address. If it is a memory address, the address
					// is computed from a segment register and any of the following values: a base register,
					// an index register, a scaling factor, and a displacement.

					DEBUGBREAK(-1); // not sure whether this one is valid or not

					break;
				//case 0: // AM_INVALID_MARKER (fix in table generator)
				case EAddressingMode::AM_INVALID_MARKER:
					
					// if(TableOfInstructions[i].Operand1.ExactOpnd == NULL) DebugBreak();
					
					break;
				default:
					
					// this should be all of them
					//printf("AM = %u\n", AM);
					//DebugBreak();

					DEBUGBREAK(-1);

					break;
				}

				//printf(" -- Mnem: %s , extnByte %X\n", TableOfInstructions[i].Mnemonic, extnByte);

				if(bExtnUsed == TRUE)
				{
					if(TableOfInstructions[i].OpcodeExtension != GetModRMReg(modRM))
					{
						// wrong opcode extension bit
						continue;
					}
				}

				// looks like we found the correct table entry
			}
			else
			{
				// extn not used but still may be some exclusions

				if(OpndIsValid(TableOfInstructions[i].Operand1.Flags) == TRUE)
				{
					EAddressingMode AM = (EAddressingMode)(TableOfInstructions[i].Operand1.Flags & AM_MASK);
					EOperandType OT = (EOperandType)(TableOfInstructions[i].Operand1.Flags & OT_MASK);
				
					if(AM == EAddressingMode::AM_INVALID_MARKER && OpndIsValid(TableOfInstructions[i].Operand2.Flags))
						AM = (EAddressingMode)(TableOfInstructions[i].Operand2.Flags & AM_MASK);

					if(AM == EAddressingMode::AM_INVALID_MARKER && OpndIsValid(TableOfInstructions[i].Operand3.Flags))
						AM = (EAddressingMode)(TableOfInstructions[i].Operand3.Flags & AM_MASK);
				
					if(AM == EAddressingMode::AM_INVALID_MARKER && OpndIsValid(TableOfInstructions[i].Operand4.Flags))
						AM = (EAddressingMode)(TableOfInstructions[i].Operand4.Flags & AM_MASK);
				
					if(AM != EAddressingMode::AM_INVALID_MARKER)
					{
						// an invalid entry: opcode extension but no mod r/m byte to check
						BYTE modRM = pInstr[0];
						
						switch(AM)
						{
						case EAddressingMode::AM_M:
			
							// The ModR/M byte may refer only to memory: mod != 11bin (BOUND, LEA, CALLF, JMPF,
							// LES, LDS, LSS, LFS, LGS, CMPXCHG8B, CMPXCHG16B, F20FF0 LDDQU).

							if(GetModRMMod(modRM) == 3)
								continue;
					
							break;
						case EAddressingMode::AM_EST:
					
							// (Implies original E). A ModR/M byte follows the opcode and specifies the x87 FPU
							// stack register.
					
							// i.e. only register modr/m bits valid - 11b?

							if(GetModRMMod(modRM) != 3)
							{
								// check it out: not sure if this precludes the modr/m being other than 11b
								//DebugBreak();
								continue;
							}

							break;
						case AM_R:
					
							// The mod field of the ModR/M byte may refer only to a general register (only MOV (0F20-0F24, 0F26)).

							if(GetModRMMod(modRM) != 3)
								continue;
					
							break;
						}
					}
				}
			}

			if(index != -1)
			{
				//printf("Somehow there are multiple matches '%s' (0x%x) and '%s' (0x%x) at least\n", TableOfInstructions[index].Mnemonic, index, TableOfInstructions[i].Mnemonic, i);

				OpdSrcOrDst operands[4] = {
						TableOfInstructions[index].Operand1,
						TableOfInstructions[index].Operand2,
						TableOfInstructions[index].Operand3,
						TableOfInstructions[index].Operand4,
					};

				BOOL bLgSource = FALSE;

				for(DWORD j=0; j < 4; j++)
				{
					if(OpndIsSource(operands[i].Flags & OT_MASK))
					{
						if(GetOperandTypeSize(i, (EOperandType)(operands[j].Flags & OT_MASK), FALSE) > 6)
						{
							bLgSource = TRUE;
						}
					}
				}

				if(!bLgSource)
				{
					// this instruction doesn't contain any 'large' source operands: prefer it for disassembly
					index = i;
				}
			}
			else
			{
				index = i;
				break;
			}

			//index = i; // best match unless we get a direct match with pri and scnd opcodes
			//return index;
		}

		return index;
	}

	DWORD GetOperandTypeSize(int instrIdx, EOperandType OT, BOOL bOpndSizePrefix)
	{
		DWORD dwLength = 0;

		if(IsInstructionOperandSizeByte(instrIdx) == TRUE)
		{
			return 1; // sizeof byte
		}

		// these may need to be scaled down according to operand size prefix

		switch(OT)
		{
		case EOperandType::OT_a:
			
			// Two one-word operands in memory or two double-word operands in memory, depending
			// on operand-size attribute (only BOUND).
			
			if(!bOpndSizePrefix)
				dwLength = 8;
			else
				dwLength = 4;

			break;
		case EOperandType::OT_b:
			
			// Byte, regardless of operand-size attribute.

			dwLength = 1;

			break;
		case EOperandType::OT_bcd:
			
			// Packed-BCD. Only x87 FPU instructions (for example, FBLD).

			dwLength = 10;
			
			break;
		case EOperandType::OT_bs:
			
			// Byte, sign-extended to the size of the destination operand.

			dwLength = 1;

			break;
		case EOperandType::OT_bsq:

			// (Byte, sign-extended to 64 bits.)

			// NOT USED
			dwLength = 1;

			break;
		case EOperandType::OT_bss:
			
			// Byte, sign-extended to the size of the stack pointer (for example, PUSH (6A)).

			dwLength = 1;

			break;
		case EOperandType::OT_c:

			// Byte or word, depending on operand-size attribute. (unused even by Intel?)

			// NOT USED
			if(!bOpndSizePrefix)
				dwLength = 2;
			else
				dwLength = 1;

			break;
		case EOperandType::OT_d:
			
			// Doubleword, regardless of operand-size attribute.

			dwLength = 4;
			
			break;
		case EOperandType::OT_di:
			
			// Doubleword-integer. Only x87 FPU instructions (for example, FIADD).

			dwLength = 4;
			
			break;
		case EOperandType::OT_dq:
			
			// Double-quadword, regardless of operand-size attribute (for example, CMPXCHG16B).

			dwLength = 16;
			
			break;
		case EOperandType::OT_dqp:
			
			// Doubleword, or quadword, promoted by REX.W in 64-bit mode (for example, MOVSXD).

			dwLength = 4;
			
			break;
		case EOperandType::OT_dr:
			
			// Double-real. Only x87 FPU instructions (for example, FADD).

			dwLength = 8;

			break;
		case EOperandType::OT_ds:
			
			// Doubleword, sign-extended to 64 bits (for example, CALL (E8).

			// NOT USED
			dwLength = 4;
			
			break;
		case EOperandType::OT_e:
			
			// x87 FPU environment (for example, FSTENV).

			dwLength = 28; // writes 28 bytes to target

			break;
		case EOperandType::OT_er:

			// Extended-real. Only x87 FPU instructions (for example, FLD).

			// TBYTE (80-bit)
			dwLength = 10;

			break;
		case EOperandType::OT_p:
			
			// 32-bit or 48-bit pointer, depending on operand-size attribute (for example, CALLF (9A).
			
			// 9A 90909090 9090     CALL FAR 9090:90909090                   ; Far call

			if(!bOpndSizePrefix)
				dwLength = 6;
			else
				dwLength = 4;

			break;
		case EOperandType::OT_pd:
			
			// Quadword MMX technology data. 

			// 0F1000               MOVUPS XMM0,DQWORD PTR DS:[EAX]
			// 3E:0F110424          MOVUPS DQWORD PTR DS:[ESP],XMM0

			dwLength = 16;
			
			break;
		case EOperandType::OT_pi:
			
			// 128-bit packed double-precision floating-point data.

			// TESTING SEEMS TO INDICATE ONLY 64-bit operand:
			// 0F1000               MOVUPS XMM0,DQWORD PTR DS:[EAX]
			// 0F2A00               CVTPI2PS XMM0,QWORD PTR DS:[EAX] // [*]
			// 3E:0F110424          MOVUPS DQWORD PTR DS:[ESP],XMM0
			
			//dwLength = 8; // [*]
			dwLength = 16;
			
			break;
		case EOperandType::OT_ps:
			
			// 128-bit packed single-precision floating-point data.
			
			dwLength = 16;
			
			break;
		case EOperandType::OT_psq:
			
			// 64-bit packed single-precision floating-point data.

			dwLength = 8;
			
			break;
		case EOperandType::OT_pt:
			
			// (80-bit far pointer.)

			dwLength = 10;
			
			break;
		case EOperandType::OT_ptp:
			
			// 32-bit or 48-bit pointer, depending on operand-size attribute, or 80-bit
			// far pointer, promoted by REX.W in 64-bit mode (for example, CALLF (FF /3)). 
			
			if(!bOpndSizePrefix)
				dwLength = 6;
			else
				dwLength = 4;

			break;
		case EOperandType::OT_q:

			// Quadword, regardless of operand-size attribute (for example, CALL (FF /2)).

			dwLength = 8;

			break;
		case EOperandType::OT_qi:

			// Qword-integer. Only x87 FPU instructions (for example, FILD).

			dwLength = 8;

			break;
		case EOperandType::OT_qp:

			// Quadword, promoted by REX.W (for example, IRETQ).

			dwLength = 8;

			break;
		case EOperandType::OT_s:

			// 6-byte pseudo-descriptor, or 10-byte pseudo-descriptor in 64-bit mode (for example, SGDT).

			dwLength = 6;

			break;
		case EOperandType::OT_sd:
			
			// Scalar element of a 128-bit packed double-precision floating data.

			dwLength = 8;
			//dwLength = 16;
			
			break;
		case EOperandType::OT_si:

			// Doubleword integer register (e. g., eax). (unused even by Intel?)

			dwLength = 4; // ?

			break;
		case EOperandType::OT_sr:
			
			// Single-real. Only x87 FPU instructions (for example, FADD).

			dwLength = 4; // ?
			
			break;
		case EOperandType::OT_ss:

			// Scalar element of a 128-bit packed single-precision floating data.

			dwLength = 4;
			//dwLength = 16;

			break;
		case EOperandType::OT_st:

			// x87 FPU state (for example, FSAVE).

			dwLength = 108;

			break;
		case EOperandType::OT_stx:

			// x87 FPU and SIMD state (FXSAVE and FXRSTOR).

			dwLength = 512;

			break;
		case EOperandType::OT_t:

			// 10-byte far pointer.

			dwLength = 10;

			break;
		case EOperandType::OT_v:

			// Word or doubleword, depending on operand-size attribute (for example, INC (40), PUSH (50)).

			dwLength = 4;

			break;
		case EOperandType::OT_vds:

			// Word or doubleword, depending on operand-size attribute, or doubleword,
			// sign-extended to 64 bits for 64-bit operand size.

			if(!bOpndSizePrefix)
				dwLength = 4;
			else
				dwLength = 2;

			break;
		case EOperandType::OT_vq:

			// Quadword (default) or word if operand-size prefix is used (for example, PUSH (50)).

			if(!bOpndSizePrefix)
				dwLength = 4;
			else
				dwLength = 2;

			break;
		case EOperandType::OT_vqp:

			// Word or doubleword, depending on operand-size attribute, or quadword,
			// promoted by REX.W in 64-bit mode.

			if(!bOpndSizePrefix)
				dwLength = 4;
			else
				dwLength = 2;

			break;
		case EOperandType::OT_vs:

			// Word or doubleword sign extended to the size of the stack pointer
			// (for example, PUSH (68)).

			if(!bOpndSizePrefix)
				dwLength = 4;
			else
				dwLength = 2;

			break;
		case EOperandType::OT_w:

			// Word, regardless of operand-size attribute (for example, ENTER).

			dwLength = 2;

			break;
		case EOperandType::OT_wi:

			// Word-integer. Only x87 FPU instructions (for example, FIADD).

			dwLength = 2;

			break;
		case EOperandType::OT_INVALID_MARKER:

			// let's assume it's a DWORD

			if(!bOpndSizePrefix)
				dwLength = 4;
			else
				dwLength = 2;

			break;
		default:
			//DebugBreak();

			DEBUGBREAK(-1);

			break;
		}

		return dwLength;
	}

	ERegisterType GetRegisterTypeFromOperand(int instrIdx, OpdSrcOrDst operand, BOOL bRefsMem, BOOL bOpndSizePrefix, BOOL bAddrSizePrefix)
	{
		// NOTE: other addressing types may need their indexes checked to ensure they're not FPU commands

		ERegisterType rt = ERegisterType::RegTypeInvalid;
		
		if(IsInstructionOperandSizeByte(instrIdx) == TRUE)
		{
			//return ERegisterType::RegTypeByte; // sizeof byte
		}

		switch(operand.Flags & AM_MASK)
		{
		case EAddressingMode::AM_A:

			// Direct address. The instruction has no ModR/M byte; the address of the operand is
			// encoded in the instruction; no base register, index register, or scaling factor
			// can be applied (for example, far JMP (EA)).

			break;
		case EAddressingMode::AM_M:
			
			// The ModR/M byte may refer only to memory: mod != 11bin (BOUND, LEA, CALLF, JMPF,
			// LES, LDS, LSS, LFS, LGS, CMPXCHG8B, CMPXCHG16B, F20FF0 LDDQU).

			rt = ERegisterType::RegTypeProcessor32;

			if(bRefsMem)
			{
				if(bAddrSizePrefix)
					rt = ERegisterType::RegTypeAddr16;
			}
			else
			{
				if(bOpndSizePrefix)
					rt = ERegisterType::RegTypeProcessor16;
			}

			break;
		case EAddressingMode::AM_E:
			
			// A ModR/M byte follows the opcode and specifies the operand. The operand is either a
			// general-purpose register or a memory address. If it is a memory address, the address
			// is computed from a segment register and any of the following values: a base
			// register, an index register, a scaling factor, or a displacement.
			
			rt = ERegisterType::RegTypeProcessor32;

			if(bRefsMem)
			{
				if(bAddrSizePrefix)
					rt = ERegisterType::RegTypeAddr16;
			}
			else
			{
				if(GetOperandTypeSize(instrIdx, (EOperandType)(operand.Flags & OT_MASK), bOpndSizePrefix) == 1)
				{
					rt = ERegisterType::RegTypeByte;
				}
				else if(bOpndSizePrefix)
				{
					rt = ERegisterType::RegTypeProcessor16;
				}
			}

			break;
		case EAddressingMode::AM_ES:
				
			// (Implies original E). A ModR/M byte follows the opcode and specifies the operand.
			// The operand is either a x87 FPU stack register or a memory address. If it is a memory
			// address, the address is computed from a segment register and any of the following
			// values: a base register, an index register, a scaling factor, or a displacement. 
			
			if(bRefsMem)
			{
				rt = ERegisterType::RegTypeProcessor32;

				if(bAddrSizePrefix)
					rt = ERegisterType::RegTypeAddr16;
			}
			else
			{
				rt = ERegisterType::RegTypeFPU;
			}

			break;
		case EAddressingMode::AM_EST:
					
			// (Implies original E). A ModR/M byte follows the opcode and specifies the x87 FPU
			// stack register.
					
			// i.e. only register modr/m bits valid - 11b?

			rt = ERegisterType::RegTypeFPU;

			break;
		case EAddressingMode::AM_Z:
					
			// The instruction has no ModR/M byte; the three least-significant bits of the opcode
			// byte selects a general-purpose register
			
			rt = ERegisterType::RegTypeProcessor32;

			if(GetOperandTypeSize(instrIdx, (EOperandType)(operand.Flags & OT_MASK), bOpndSizePrefix) == 1)
			{
				rt = ERegisterType::RegTypeByte;
			}
			else if(bOpndSizePrefix)
			{
				rt = ERegisterType::RegTypeProcessor16;
			}

			break;
		case EAddressingMode::AM_G:
				
			// The reg field of the ModR/M byte selects a general register (for example, AX (000)).

			rt = ERegisterType::RegTypeProcessor32;

			if(GetOperandTypeSize(instrIdx, (EOperandType)(operand.Flags & OT_MASK), bOpndSizePrefix) == 1)
			{
				rt = ERegisterType::RegTypeByte;
			}
			else if(bOpndSizePrefix)
			{
				rt = ERegisterType::RegTypeProcessor16;
			}

			break;
		case EAddressingMode::AM_I:

			// Immediate data. The operand value is encoded in subsequent bytes of the instruction.

			break;
		case EAddressingMode::AM_J:

			// The instruction contains a relative offset to be added to the instruction pointer
			// register (for example, JMP (E9), LOOP)).

			break;
		case EAddressingMode::AM_S:

			// The reg field of the ModR/M byte selects a segment register (only MOV (8C, 8E)).

			rt = ERegisterType::RegTypeSegment;

			break;
		case EAddressingMode::AM_R:
					
			// The mod field of the ModR/M byte may refer only to a general register (only MOV (0F20-0F24, 0F26)).

			// MAY NOT BE RIGHT: perhaps 16/32 depending context
			rt = ERegisterType::RegTypeProcessor16;

			break;
		case EAddressingMode::AM_O:
				
			// The instruction has no ModR/M byte; the offset of the operand is coded as a word,
			// double word or quad word (depending on address size attribute) in the instruction.
			// No base register, index register, or scaling factor can be applied (only MOV 
			// (A0, A1, A2, A3)).
			
			break;
		case EAddressingMode::AM_V:

			// The reg field of the ModR/M byte selects a 128-bit XMM register.
			
			rt = ERegisterType::RegTypeXMM;

			break;
		case EAddressingMode::AM_W:
				
			// A ModR/M byte follows the opcode and specifies the operand. The operand is either a
			// 128-bit XMM register or a memory address. If it is a memory address, the address is
			// computed from a segment register and any of the following values: a base register,
			// an index register, a scaling factor, and a displacement
			
			if(bRefsMem)
			{
				rt = ERegisterType::RegTypeProcessor32;

				if(bAddrSizePrefix)
					rt = ERegisterType::RegTypeAddr16;
			}
			else
			{
				rt = ERegisterType::RegTypeXMM;
			}

			break;
		case EAddressingMode::AM_H:

			// The r/m field of the ModR/M byte always selects a general register, regardless of the
			// mod field (for example, MOV (0F20)).

			rt = ERegisterType::RegTypeProcessor32;

			if(GetOperandTypeSize(instrIdx, (EOperandType)(operand.Flags & OT_MASK), bOpndSizePrefix) == 1)
			{
				rt = ERegisterType::RegTypeByte;
			}
			else if(bOpndSizePrefix)
			{
				rt = ERegisterType::RegTypeProcessor16;
			}

			break;
		case EAddressingMode::AM_C:

			// The reg field of the ModR/M byte selects a control register (only MOV (0F20, 0F22)).

			rt = ERegisterType::RegTypeControl;

			break;
		case EAddressingMode::AM_D:

			// The reg field of the ModR/M byte selects a debug register (only MOV (0F21, 0F23)).

			rt = ERegisterType::RegTypeDebug;

			break;
		case EAddressingMode::AM_P:

			// The reg field of the ModR/M byte selects a packed quadword MMX technology register.

			rt = ERegisterType::RegTypeMMX;

			break;
		case EAddressingMode::AM_N:
				
			// The R/M field of the ModR/M byte selects a packed quadword MMX technology register.
			
			rt = ERegisterType::RegTypeMMX;

			break;
		case EAddressingMode::AM_U:

			// The R/M field of the ModR/M byte selects a 128-bit XMM register.

			rt = ERegisterType::RegTypeXMM;

			break;
		case EAddressingMode::AM_Q:

			// A ModR/M byte follows the opcode and specifies the operand. The operand is either an
			// MMX technology register or a memory address. If it is a memory address, the address
			// is computed from a segment register and any of the following values: a base register,
			// an index register, a scaling factor, and a displacement.

			if(bRefsMem)
			{
				rt = ERegisterType::RegTypeProcessor32;

				if(bAddrSizePrefix)
					rt = ERegisterType::RegTypeAddr16;
			}
			else
			{
				rt = ERegisterType::RegTypeMMX;
			}

			break;
		default:
				
			// this should be all of them
			DEBUGBREAK(ERegisterType::RegTypeInvalid);

			break;
		}

		return rt;
	}

	DWORD OperandSizeInBytes(int instrIdx, OpdSrcOrDst operand, BOOL bRefsMem, BOOL bOpndSizePrefix, BOOL bAddrSizePrefix)
	{
		DWORD dwOperandLen = 0;

		switch(operand.Flags & AM_MASK)
		{
		case EAddressingMode::AM_I:

			// Immediate data. The operand value is encoded in subsequent bytes of the instruction.

			dwOperandLen = GetOperandTypeSize(instrIdx, (EOperandType)(operand.Flags & OT_MASK), bOpndSizePrefix);

			break;
		case EAddressingMode::AM_J:

			// The instruction contains a relative offset to be added to the instruction pointer
			// register (for example, JMP (E9), LOOP)).

			if(GetOperandTypeSize(instrIdx, (EOperandType)(operand.Flags & OT_MASK), bOpndSizePrefix) == 1)
				dwOperandLen = 1;
			else if(bOpndSizePrefix)
				dwOperandLen = 2;
			else
				dwOperandLen = 4;

			break;
		case EAddressingMode::AM_O:
				
			// The instruction has no ModR/M byte; the offset of the operand is coded as a word,
			// double word or quad word (depending on address size attribute) in the instruction.
			// No base register, index register, or scaling factor can be applied (only MOV 
			// (A0, A1, A2, A3)).
			
			if(IsInstructionOperandSizeByte(instrIdx) == TRUE)
			{
				dwOperandLen = 1;
			}
			else
			{
				if(bAddrSizePrefix)
					dwOperandLen = 2;
				else
					dwOperandLen = 4;
			}

			break;
		default:

			// is this all of them?

			break;
		}

		return dwOperandLen;
	}

	BOOL IsModRMUsed(OpdSrcOrDst operand)
	{
		BOOL bUsesModRM = FALSE;

		switch(operand.Flags & AM_MASK)
		{
		case EAddressingMode::AM_A:

			// Direct address. The instruction has no ModR/M byte; the address of the operand is
			// encoded in the instruction; no base register, index register, or scaling factor
			// can be applied (for example, far JMP (EA)).

			break;
		case EAddressingMode::AM_M:
				
			// The ModR/M byte may refer only to memory: mod != 11bin (BOUND, LEA, CALLF, JMPF,
			// LES, LDS, LSS, LFS, LGS, CMPXCHG8B, CMPXCHG16B, F20FF0 LDDQU).

			bUsesModRM = TRUE;

			break;
		case EAddressingMode::AM_E:
					
			// A ModR/M byte follows the opcode and specifies the operand. The operand is either a
			// general-purpose register or a memory address. If it is a memory address, the address
			// is computed from a segment register and any of the following values: a base
			// register, an index register, a scaling factor, or a displacement.
				
			bUsesModRM = TRUE;

			break;
		case EAddressingMode::AM_ES:
				
			// (Implies original E). A ModR/M byte follows the opcode and specifies the operand.
			// The operand is either a x87 FPU stack register or a memory address. If it is a memory
			// address, the address is computed from a segment register and any of the following
			// values: a base register, an index register, a scaling factor, or a displacement. 
			
			bUsesModRM = TRUE;

			break;
		case EAddressingMode::AM_EST:
					
			// (Implies original E). A ModR/M byte follows the opcode and specifies the x87 FPU
			// stack register.
					
			// i.e. only register modr/m bits valid - 11b?

			bUsesModRM = TRUE;

			break;
		case EAddressingMode::AM_Z:
					
			// The instruction has no ModR/M byte; the three least-significant bits of the opcode
			// byte selects a general-purpose register

			break;
		case EAddressingMode::AM_G:
				
			// The reg field of the ModR/M byte selects a general register (for example, AX (000)).

			bUsesModRM = TRUE;

			break;
		case EAddressingMode::AM_I:

			// Immediate data. The operand value is encoded in subsequent bytes of the instruction.

			break;
		case EAddressingMode::AM_J:

			// The instruction contains a relative offset to be added to the instruction pointer
			// register (for example, JMP (E9), LOOP)).

			break;
		case EAddressingMode::AM_R:
					
			// The mod field of the ModR/M byte may refer only to a general register (only MOV (0F20-0F24, 0F26)).

			bUsesModRM = TRUE;

			break;
		case EAddressingMode::AM_S:

			// The reg field of the ModR/M byte selects a segment register (only MOV (8C, 8E)).

			bUsesModRM = TRUE;

			break;
		case EAddressingMode::AM_O:
				
			// The instruction has no ModR/M byte; the offset of the operand is coded as a word,
			// double word or quad word (depending on address size attribute) in the instruction.
			// No base register, index register, or scaling factor can be applied (only MOV 
			// (A0, A1, A2, A3)).
				
			break;
		case EAddressingMode::AM_V:

			// The reg field of the ModR/M byte selects a 128-bit XMM register.

			bUsesModRM = TRUE;

			break;
		case EAddressingMode::AM_W:
				
			// A ModR/M byte follows the opcode and specifies the operand. The operand is either a
			// 128-bit XMM register or a memory address. If it is a memory address, the address is
			// computed from a segment register and any of the following values: a base register,
			// an index register, a scaling factor, and a displacement
				
			bUsesModRM = TRUE;

			break;
		case EAddressingMode::AM_H:

			// The r/m field of the ModR/M byte always selects a general register, regardless of the
			// mod field (for example, MOV (0F20)).

			bUsesModRM = TRUE;

			break;
		case EAddressingMode::AM_C:

			// The reg field of the ModR/M byte selects a control register (only MOV (0F20, 0F22)).

			bUsesModRM = TRUE;

			break;
		case EAddressingMode::AM_D:

			// The reg field of the ModR/M byte selects a debug register (only MOV (0F21, 0F23)).

			bUsesModRM = TRUE;

			break;
		case EAddressingMode::AM_P:

			// The reg field of the ModR/M byte selects a packed quadword MMX technology register.

			bUsesModRM = TRUE;

			break;
		case EAddressingMode::AM_N:
				
			// The R/M field of the ModR/M byte selects a packed quadword MMX technology register.

			bUsesModRM = TRUE;
				
			break;
		case EAddressingMode::AM_U:

			// The R/M field of the ModR/M byte selects a 128-bit XMM register.

			bUsesModRM = TRUE;

			break;
		case EAddressingMode::AM_Q:

			// A ModR/M byte follows the opcode and specifies the operand. The operand is either an
			// MMX technology register or a memory address. If it is a memory address, the address
			// is computed from a segment register and any of the following values: a base register,
			// an index register, a scaling factor, and a displacement.

			bUsesModRM = TRUE;

			break;
		default:
				
			// this should be all of them

			//DebugBreak();

			break;
		}

		return bUsesModRM;
	}

	DWORD GetInstructionOperandLenMax(int index, BOOL bOpndSizePrefix, BOOL bAddrSizePrefix)
	{
		DWORD dwLenMax = 0;

		if(OpndIsValid(TableOfInstructions[index].Operand1.Flags))
		{
			dwLenMax = GetOperandTypeSize(index, (EOperandType)(TableOfInstructions[index].Operand1.Flags & OT_MASK), bOpndSizePrefix);
			
			if(OpndIsValid(TableOfInstructions[index].Operand2.Flags))
			{
				DWORD dwTmp = GetOperandTypeSize(index, (EOperandType)(TableOfInstructions[index].Operand2.Flags & OT_MASK), bOpndSizePrefix);
				if(dwTmp > dwLenMax)
					dwLenMax = dwTmp;

				if(OpndIsValid(TableOfInstructions[index].Operand3.Flags))
				{
					dwTmp = GetOperandTypeSize(index, (EOperandType)(TableOfInstructions[index].Operand3.Flags & OT_MASK), bOpndSizePrefix);
					if(dwTmp > dwLenMax)
						dwLenMax = dwTmp;

					if(OpndIsValid(TableOfInstructions[index].Operand4.Flags))
					{
						dwTmp = GetOperandTypeSize(index, (EOperandType)(TableOfInstructions[index].Operand4.Flags & OT_MASK), bOpndSizePrefix);
						if(dwTmp > dwLenMax)
							dwLenMax = dwTmp;
					}
				}
			}
		}

		return dwLenMax;
	}

	char *GetOperandLenString(DWORD dwLength)
	{
		char *pszWidth = "";
		
		switch(dwLength)
		{
		case 1:
			pszWidth = "BYTE";
			break;
		case 2:
			pszWidth = "WORD";
			break;
		case 4:
			pszWidth = "DWORD";
			break;
		case 8:
			pszWidth = "QWORD";
			break;
		case 10:
			pszWidth = "TBYTE";
			break;
		case 16:
			pszWidth = "DQWORD";
			break;
		case 28:
			pszWidth = "28-BYTE";
			break;
		case 6:
			pszWidth = "6-BYTE";
			break;
		case 108:
			pszWidth = "108-BYTE";
			break;
		case 512:
			pszWidth = "512-BYTE";
			break;
		}
		
		return pszWidth;
	}

	char *GetRegisterName(ERegisterType rt, int reg)
	{
		if((DWORD)rt >= (sizeof(RegisterNameLookupTable) / (8 * 16)) || reg >= 8 || reg < 0)
		{
			return "";
		}

		return RegisterNameLookupTable[reg][(DWORD)rt];
	}

	OpdSrcOrDst GetFirstModRMOperand(int index, BOOL& bUsesModRM)
	{
		bUsesModRM = FALSE;

		OpdSrcOrDst operands[4] = {
			TableOfInstructions[index].Operand1,
			TableOfInstructions[index].Operand2,
			TableOfInstructions[index].Operand3,
			TableOfInstructions[index].Operand4
		}, operand = {0};

		for(DWORD i=0; i < 4 && OpndIsValid(operands[i].Flags); i++)
		{
			if(IsModRMUsed(operands[i]))
			{
				switch((EAddressingMode)(operands[i].Flags & AM_MASK))
				{
					case EAddressingMode::AM_G:
				
						// The reg field of the ModR/M byte selects a general register (for example, AX (000)).
						
						if(!bUsesModRM) operand = operands[i];

						break;
					case EAddressingMode::AM_S:

						// The reg field of the ModR/M byte selects a segment register (only MOV (8C, 8E)).
						
						if(!bUsesModRM) operand = operands[i];

						break;
					case EAddressingMode::AM_V:

						// The reg field of the ModR/M byte selects a 128-bit XMM register.
						
						if(!bUsesModRM) operand = operands[i];

						break;
					case EAddressingMode::AM_C:

						// The reg field of the ModR/M byte selects a control register (only MOV (0F20, 0F22)).
						
						if(!bUsesModRM) operand = operands[i];

						break;
					case EAddressingMode::AM_D:

						// The reg field of the ModR/M byte selects a debug register (only MOV (0F21, 0F23)).
						
						if(!bUsesModRM) operand = operands[i];

						break;
					case EAddressingMode::AM_P:

						// The reg field of the ModR/M byte selects a packed quadword MMX technology register.
						
						if(!bUsesModRM) operand = operands[i];

						break;
					default:

						operand = operands[i];

						break;
				}

				bUsesModRM = TRUE;
			}
		}
		
		return operand;
	}

	BOOL IsInstructionOperandSizeByte(int index)
	{
		if(TableOfInstructions[index].OpSize != -1)
		{
			return (TableOfInstructions[index].PrimaryOpcode & 0x1) == 0;
		}

		return FALSE;
	}
	
	BOOL IsInstructionSignExtended(int index)
	{
		if(TableOfInstructions[index].SignExt != -1)
		{
			return (TableOfInstructions[index].PrimaryOpcode & 0x2) == 0;
		}

		return FALSE;
	}

	BOOL ProcessModRMAndSIB(
			int idx,
			PBYTE pModRmBytePointer,
			OpdSrcOrDst& operand,
			BOOL bUsesModRM,
			BYTE& modRM,
			BOOL bAddrSizePrefix,
			BOOL bOpndSizePrefix,
			DWORD& dwIncrement,
			BOOL bText,
			char *szModRM,
			char *pszSegment
		)
	{
		DWORD dwTmp = 0;
		char *pszWidth = "";
		PBYTE pInstr = pModRmBytePointer;

		modRM = pInstr[0];
		
		szModRM[0] = '\0';
		dwIncrement = 0;

		if(bUsesModRM)
		{
			pInstr++;

			// reg = {
			//    eax = 000
			//    ecx = 001
			//    edx = 010
			//    ebx = 011
			//    esp = 100
			//    ebp = 101
			//    esi = 110
			//    edi = 111
			// }

			BOOL bSIBPresent = FALSE;
			BYTE sib = pInstr[0];

			switch(GetModRMMod(modRM))
			{
			case 0: // 00b

				// r/m = [reg] - except ebp, esp.
				//   if r/m = 101: offset32 follows
				//   if r/m = 100: sib follows

				if(!bAddrSizePrefix)
				{
					// regular composition

					if(GetModRMRegMem(modRM) == 5) // 101b
					{
						// 32-bit constant always (!addr size pfx)

						if(bText)
						{
							dwTmp = GetOperandTypeSize(idx, (EOperandType)(operand.Flags & OT_MASK), bOpndSizePrefix);
							//dwTmp = GetInstructionOperandLenMax(idx, bOpndSizePrefix, bAddrSizePrefix);
							pszWidth = GetOperandLenString(dwTmp);
							sprintf(szModRM, "%s PTR %s[%08X]", pszWidth, pszSegment, *(DWORD *)pInstr);
						}

						dwIncrement = 4;
					}
					else if(GetModRMRegMem(modRM) == 4) // 100b
					{
						bSIBPresent = TRUE;
					}
					else
					{
						// nothing to do here
						if(bText)
						{
							dwTmp = GetOperandTypeSize(idx, (EOperandType)(operand.Flags & OT_MASK), bOpndSizePrefix);
							//dwTmp = GetInstructionOperandLenMax(idx, bOpndSizePrefix, bAddrSizePrefix);
							pszWidth = GetOperandLenString(dwTmp);
							sprintf(szModRM, "%s PTR %s[%s]", pszWidth, pszSegment, GetRegisterName(ERegisterType::RegTypeProcessor32, GetModRMRegMem(modRM)));
						}
					}
				}
				else
				{
					// only remains to format the operand text
					if(bText)
					{
						dwTmp = GetOperandTypeSize(idx, (EOperandType)(operand.Flags & OT_MASK), bOpndSizePrefix);
						//dwTmp = GetInstructionOperandLenMax(idx, bOpndSizePrefix, bAddrSizePrefix);
						pszWidth = GetOperandLenString(dwTmp);
						sprintf(szModRM, "%s PTR %s[%s]", pszWidth, pszSegment, GetRegisterName(ERegisterType::RegTypeAddr16, GetModRMRegMem(modRM)));
					}
				}

				break;
			case 1: // 01b

				// r/m = [reg + offset8] except esp
				//   if r/m = 100: sib follows

				// always 8-bit displacement
				dwIncrement = 1;

				if(!bAddrSizePrefix)
				{
					if(GetModRMRegMem(modRM) == 4) // 100b
					{
						bSIBPresent = TRUE;
					}
					else
					{
						// nothing to do here
						if(bText)
						{
							dwTmp = GetOperandTypeSize(idx, (EOperandType)(operand.Flags & OT_MASK), bOpndSizePrefix);
							//dwTmp = GetInstructionOperandLenMax(idx, bOpndSizePrefix, bAddrSizePrefix);
							pszWidth = GetOperandLenString(dwTmp);
							sprintf(szModRM, "%s PTR %s[%s+%02X]", pszWidth, pszSegment, GetRegisterName(ERegisterType::RegTypeProcessor32, GetModRMRegMem(modRM)), *(BYTE *)pInstr);
						}
					}
				}
				else
				{
					// format the operand text
					if(bText)
					{
						dwTmp = GetOperandTypeSize(idx, (EOperandType)(operand.Flags & OT_MASK), bOpndSizePrefix);
						//dwTmp = GetInstructionOperandLenMax(idx, bOpndSizePrefix, bAddrSizePrefix);
						pszWidth = GetOperandLenString(dwTmp);
						sprintf(szModRM, "%s PTR %s[%s+%02X]", pszWidth, pszSegment, GetRegisterName(ERegisterType::RegTypeAddr16, GetModRMRegMem(modRM)), *(BYTE *)pInstr);
					}
				}

				break;
			case 2: // 10b
						
				// r/m = [reg + offset32] except esp
				//   if r/m = 100: sib follows
						
				if(!bAddrSizePrefix)
				{
					// 32-bit displacement
					dwIncrement = 4;

					if(GetModRMRegMem(modRM) == 4) // 100b
					{
						bSIBPresent = TRUE;
					}
					else
					{
						// nothing to do here
						if(bText)
						{
							dwTmp = GetOperandTypeSize(idx, (EOperandType)(operand.Flags & OT_MASK), bOpndSizePrefix);
							//dwTmp = GetInstructionOperandLenMax(idx, bOpndSizePrefix, bAddrSizePrefix);
							pszWidth = GetOperandLenString(dwTmp);
							sprintf(szModRM, "%s PTR %s[%s+%08X]", pszWidth, pszSegment, GetRegisterName(ERegisterType::RegTypeProcessor32, GetModRMRegMem(modRM)), *(DWORD *)pInstr);
						}
					}
				}
				else
				{
					// 16-bit displacement
					dwIncrement = 2;

					if(bText)
					{
						dwTmp = GetOperandTypeSize(idx, (EOperandType)(operand.Flags & OT_MASK), bOpndSizePrefix);
						//dwTmp = GetInstructionOperandLenMax(idx, bOpndSizePrefix, bAddrSizePrefix);
						pszWidth = GetOperandLenString(dwTmp);
						sprintf(szModRM, "%s PTR %s[%s+%04X]", pszWidth, pszSegment, GetRegisterName(ERegisterType::RegTypeAddr16, GetModRMRegMem(modRM)), *(WORD *)pInstr);
					}
				}
						
				break;
			case 3: // 11b
						
				// r/m = reg (any)
						
				if(bText)
				{
					ERegisterType rt = GetRegisterTypeFromOperand(idx, operand, FALSE, bOpndSizePrefix, bAddrSizePrefix);
					char *p = GetRegisterName(rt, GetModRMRegMem(modRM));
					sprintf(szModRM, "%s", GetRegisterName(rt, GetModRMRegMem(modRM)));
				}

				break;
			}

			if(bSIBPresent)
			{
				pInstr++; // skip sib

				// if doing disasm:
				//     parse sib byte
				//     increment by size specified by mod bits

				int sibScale = GetSIBScale(sib),
					sibIndex = GetSIBIndex(sib),
					sibBase  = GetSIBBase(sib);
						
				ERegisterType rt = GetRegisterTypeFromOperand(idx, operand, TRUE, bOpndSizePrefix, bAddrSizePrefix);

				switch(rt)
				{
				case ERegisterType::RegTypeAddr16:
				case ERegisterType::RegTypeProcessor16:
				case ERegisterType::RegTypeProcessor32:
					break;
				default:

					// i don't think the SIB is valid for non-general processor reg types
					// printf("ERegisterType(%u)\n", rt);
							
					// NB: for an instruction like MOVUPS 0f 10 xx xx Operand1.OT could be a dst reg not in the above list.
					// Deal with this later if necessary.

					break;
				}

				if(bText)
				{
					if(GetModRMMod(modRM) == 0 && sibBase == 5)
					{
						dwTmp = GetOperandTypeSize(idx, (EOperandType)(operand.Flags & OT_MASK), bOpndSizePrefix);
						//dwTmp = GetInstructionOperandLenMax(idx, bOpndSizePrefix, bAddrSizePrefix);
						pszWidth = GetOperandLenString(dwTmp);
						sprintf(szModRM, "%s PTR %s[%08X", pszWidth, pszSegment, *(DWORD *)pInstr);
					}
					else
					{
						//DebugBreak();
						dwTmp = GetOperandTypeSize(idx, (EOperandType)(operand.Flags & OT_MASK), bOpndSizePrefix);
						//dwTmp = GetInstructionOperandLenMax(idx, bOpndSizePrefix, bAddrSizePrefix);
						pszWidth = GetOperandLenString(dwTmp);
						sprintf(szModRM, "%s PTR %s[%s", pszWidth, pszSegment, GetRegisterName(rt, sibBase));
					}

					if(sibIndex != 4)
					{
						switch(sibScale)
						{
						case 0:
							StringUtils::sprintfcat(szModRM, "+%s", GetRegisterName(rt, sibIndex));
							break;
						case 1:
							StringUtils::sprintfcat(szModRM, "+2*%s", GetRegisterName(rt, sibIndex));
							break;
						case 2:
							StringUtils::sprintfcat(szModRM, "+4*%s", GetRegisterName(rt, sibIndex));
							break;
						case 3:
							StringUtils::sprintfcat(szModRM, "+8*%s", GetRegisterName(rt, sibIndex));
							break;
						}
					}
				}

				// a bit verbose, but necessary:
				switch(GetModRMMod(modRM))
				{
				case 0: // 00b
							
					// r/m = [reg] - except ebp, esp.
					//   if r/m = 101: offset32 follows
					//   if r/m = 100: sib follows

					if(sibBase == 5) // 101b
					{
						// when mod is 00 and sib-base is 101, a length base follows
						dwIncrement += 4;
					}
							
					break;
				case 1: // 01b
							
					// r/m = [reg + offset8] except esp
					//   if r/m = 100: sib follows

					if(bText)
					{
						StringUtils::sprintfcat(szModRM, "+%02X", *(BYTE *)pInstr);
					}

					break;
				case 2:
							
					// r/m = [reg + offset32] except esp
					//   if r/m = 100: sib follows
							
					if(bText)
					{
						StringUtils::sprintfcat(szModRM, "+%08X", *(DWORD *)pInstr);
					}

					break;
				case 3: // 11b
							
					// r/m = reg (any)

					// illegal: return error
							
					return FALSE;
				}

				if(bText)
				{
					StringUtils::sprintfcat(szModRM, "]");
				}
			}
		}

		dwIncrement += (pInstr - pModRmBytePointer);

		return TRUE;
	}

	DWORD GetInstructionBytes(PBYTE pInstr, BYTE bytes[16], BOOL bText, char szText[128])
	{
		PBYTE pInstrCopy = pInstr;
		// should get bytes for only the indicated instruction, plus prefix
		
		BOOL bFPUCommand = FALSE;

		//printf(" ** pInstr[0-1] = %x %x\n", pInstr[0], pInstr[1]);

		if(bText)
			szText[0] = '\0';

		char *pszSegment = "";

		int idx = GetCommandIndex(pInstr);
		if(idx != -1)
		{
			BYTE prefix[4];
			DWORD dwPrefixLen = GetPrefixCount(pInstr, prefix);

			BOOL bAddrSizePrefix = ContainsPrefix(prefix, dwPrefixLen, ECommandPrefix::PrefixAddrSize),
				 bOpndSizePrefix = ContainsPrefix(prefix, dwPrefixLen, ECommandPrefix::PrefixOpndSize); // updated on per-operand basis
			
			BOOL bSignedIns = TableOfInstructions[idx].SignExt != -1;

			if(RepPrefixCount(prefix, dwPrefixLen) > 1 || SegPrefixCount(prefix, dwPrefixLen) > 1)
			{
				// only one prefix of any given type is allowed (doesn't seem to apply to size replacement)
				return 0;
			}

			if(bText)
			{
				StringUtils::sprintfcat(szText, "%s", TableOfInstructions[idx].Mnemonic);

				for(DWORD i=0; i < dwPrefixLen; i++)
				{
					switch(prefix[i])
					{
					case ECommandPrefix::PrefixREPZ:
						StringUtils::sprintfcat(szText, "<REPZ>");
						break;
					case ECommandPrefix::PrefixREPNZ:
						StringUtils::sprintfcat(szText, "<REPNZ>");
						break;
					case ECommandPrefix::PrefixLOCK:
						StringUtils::sprintfcat(szText, "<LOCK>");
						break;
					case ECommandPrefix::PrefixCodeSeg:
						pszSegment = "CS:";
						break;
					case ECommandPrefix::PrefixStackSeg:
						pszSegment = "SS:";
						break;
					case ECommandPrefix::PrefixDataSeg:
						pszSegment = "DS:";
						break;
					case ECommandPrefix::PrefixExtraSeg:
						pszSegment = "ES:";
						break;
					case ECommandPrefix::PrefixFileSeg:
						pszSegment = "FS:";
						break;
					case ECommandPrefix::PrefixGenSeg:
						pszSegment = "GS:";
						break;
					}
				}

				StringUtils::sprintfcat(szText, "(");
				//StringUtils::sprintfcat(szText, "%s", TableOfInstructions[idx].Mnemonic);
			}

			pInstr += dwPrefixLen;
			
			BOOL bPrefix0F = pInstr[0] == 0x0f;
			
			if(bPrefix0F)
			{
				pInstr++;
			}

			pInstr++; // primary opcode

			if(TableOfInstructions[idx].SecondaryOpcode != 0)
			{
				pInstr++;
			}

			if(IsFPUCommand(TableOfInstructions[idx].PrimaryOpcode))
			{
				// all FPU commands are two bytes (in total, or at least?)
				bFPUCommand = TRUE;
			}
			
			// we may have to find the first operand with a IsModRMUsed = true entry (skipping ExactOpnd)

			if(OpndIsValid(TableOfInstructions[idx].Operand1.Flags) == TRUE)
			{
				BOOL bUsesModRM = FALSE;
				DWORD dwTmp = 0;
				char szModRM[128] = {0};

				BYTE modRM = 0;
				PBYTE pModRmBytePointer = pInstr;
				DWORD dwIncrement = 0;

				OpdSrcOrDst operand = GetFirstModRMOperand(idx, bUsesModRM);
				
				if(!ProcessModRMAndSIB(idx, pModRmBytePointer, operand, bUsesModRM, modRM, bAddrSizePrefix, bOpndSizePrefix, dwIncrement, bText, szModRM, pszSegment))
					return 0;

				// process operands

				OpdSrcOrDst operands[4] = {
					TableOfInstructions[idx].Operand1,
					TableOfInstructions[idx].Operand2,
					TableOfInstructions[idx].Operand3,
					TableOfInstructions[idx].Operand4,
				};

				// mod r/m immediate data comes before operand immediate data: increment here
				pInstr += dwIncrement;

				BOOL bReEnableText = FALSE;

				for(DWORD i=0; i < 4 && OpndIsValid(operands[i].Flags) != FALSE; i++)
				{
					DWORD dwOperandSize = OperandSizeInBytes(idx, operands[i], bUsesModRM && GetModRMMod(modRM) != 3, bOpndSizePrefix, bAddrSizePrefix);
					
					if(bText)
					{
						if(i == 0)
						{
							StringUtils::sprintfcat(szText, "");
						}
					}

					if(bText)
					{
						if(operands[i].ExactOpnd != NULL)
						{
							bReEnableText = TRUE;
							bText = FALSE;

							// qualify with width/seg/sizepfx
							char szExact[128];
							strcpy(szExact, operands[i].ExactOpnd);
							RewriteExactOperand(szExact, pszSegment, IsInstructionOperandSizeByte(idx) ? "BYTE" : NULL, bOpndSizePrefix, bAddrSizePrefix);
							StringUtils::sprintfcat(szText, "%s", szExact);
						}
					}

					switch(operands[i].Flags & AM_MASK)
					{
					case EAddressingMode::AM_A:

						// Direct address. The instruction has no ModR/M byte; the address of the operand is
						// encoded in the instruction; no base register, index register, or scaling factor
						// can be applied (for example, far JMP (EA)).

						if(bText)
						{
							ERegisterType rt = GetRegisterTypeFromOperand(idx, operands[i], TRUE, bOpndSizePrefix, bAddrSizePrefix);
							
							if((operands[i].Flags & OT_MASK) == EOperandType::OT_p)
							{
								dwTmp = GetOperandTypeSize(idx, (EOperandType)(operands[i].Flags & OT_MASK), bOpndSizePrefix);

								if(dwTmp == 4)
									StringUtils::sprintfcat(szText, "FAR %04X:%04X", *(WORD *)pInstr, *(WORD *)(pInstr + 2));
								else if(dwTmp == 6)
									StringUtils::sprintfcat(szText, "FAR %04X:%08X", *(WORD *)pInstr, *(DWORD *)(pInstr + 2));
								else
									; // not valid
								
								break;
							}

							StringUtils::sprintfcat(szText, "%08X", *(DWORD *)pInstr);
						}

						break;
					case EAddressingMode::AM_M:
				
						// The ModR/M byte may refer only to memory: mod != 11bin (BOUND, LEA, CALLF, JMPF,
						// LES, LDS, LSS, LFS, LGS, CMPXCHG8B, CMPXCHG16B, F20FF0 LDDQU).
						
						if(bText)
						{
							if(!ProcessModRMAndSIB(idx, pModRmBytePointer, operands[i], bUsesModRM, modRM, bAddrSizePrefix, bOpndSizePrefix, dwIncrement, bText, szModRM, pszSegment))
								return 0;

							StringUtils::sprintfcat(szText, szModRM);
						}

						break;
					case EAddressingMode::AM_E:
					
						// A ModR/M byte follows the opcode and specifies the operand. The operand is either a
						// general-purpose register or a memory address. If it is a memory address, the address
						// is computed from a segment register and any of the following values: a base
						// register, an index register, a scaling factor, or a displacement.
						
						if(bText)
						{
							if(!ProcessModRMAndSIB(idx, pModRmBytePointer, operands[i], bUsesModRM, modRM, bAddrSizePrefix, bOpndSizePrefix, dwIncrement, bText, szModRM, pszSegment))
								return 0;

							StringUtils::sprintfcat(szText, szModRM);
						}

						break;
					case EAddressingMode::AM_ES:
				
						// (Implies original E). A ModR/M byte follows the opcode and specifies the operand.
						// The operand is either a x87 FPU stack register or a memory address. If it is a memory
						// address, the address is computed from a segment register and any of the following
						// values: a base register, an index register, a scaling factor, or a displacement. 
						
						if(bText)
						{
							if(!ProcessModRMAndSIB(idx, pModRmBytePointer, operands[i], bUsesModRM, modRM, bAddrSizePrefix, bOpndSizePrefix, dwIncrement, bText, szModRM, pszSegment))
								return 0;

							StringUtils::sprintfcat(szText, szModRM);
						}

						break;
					case EAddressingMode::AM_EST:
					
						// (Implies original E). A ModR/M byte follows the opcode and specifies the x87 FPU
						// stack register.
						
						if(bText)
						{
							if(!ProcessModRMAndSIB(idx, pModRmBytePointer, operands[i], bUsesModRM, modRM, bAddrSizePrefix, bOpndSizePrefix, dwIncrement, bText, szModRM, pszSegment))
								return 0;

							StringUtils::sprintfcat(szText, szModRM);
						}

						break;
					case EAddressingMode::AM_Z:
					
						// The instruction has no ModR/M byte; the three least-significant bits of the opcode
						// byte selects a general-purpose register

						if(bText)
						{
							ERegisterType rt = GetRegisterTypeFromOperand(idx, operands[i], FALSE, bOpndSizePrefix, bAddrSizePrefix);
							BYTE OpcLsb = TableOfInstructions[idx].SecondaryOpcode ? TableOfInstructions[idx].SecondaryOpcode : TableOfInstructions[idx].PrimaryOpcode;
							StringUtils::sprintfcat(szText, GetRegisterName(rt, GetModRMRegMem(OpcLsb)));
						}

						break;
					case EAddressingMode::AM_G:
				
						// The reg field of the ModR/M byte selects a general register (for example, AX (000)).
						
						if(bText)
						{
							ERegisterType rt = GetRegisterTypeFromOperand(idx, operands[i], FALSE, bOpndSizePrefix, bAddrSizePrefix);
							StringUtils::sprintfcat(szText, GetRegisterName(rt, GetModRMReg(modRM)));
						}

						break;
					case EAddressingMode::AM_I:

						// Immediate data. The operand value is encoded in subsequent bytes of the instruction.

						if(bText)
						{
							// entirely optional, we needn't prefix constants with their size in stored form
							
							switch(dwOperandSize)
							{
							case 1:
								StringUtils::sprintfcat(szText, /*"BYTE %02X"*/ "%X", bSignedIns ? *(char *)pInstr : *(BYTE *)pInstr);
								break;
							case 2:
								StringUtils::sprintfcat(szText, /*"WORD %04X"*/ "%X", *(WORD *)pInstr);
								break;
							case 4:
								StringUtils::sprintfcat(szText, /*"%08X"*/ "%X", *(DWORD *)pInstr);
								break;
							}
						}

						break;
					case EAddressingMode::AM_J:

						// The instruction contains a relative offset to be added to the instruction pointer
						// register (for example, JMP (E9), LOOP)).

						if(bText)
						{
							switch(dwOperandSize)
							{
							case 1:
								StringUtils::sprintfcat(szText, "SHORT %02X", *(BYTE *)pInstr);
								break;
							case 2:
								StringUtils::sprintfcat(szText, "%04X", *(WORD *)pInstr);
								break;
							case 4:
								StringUtils::sprintfcat(szText, "%08X", *(DWORD *)pInstr);
								break;
							}
						}

						break;
					case EAddressingMode::AM_R:
						
						// The mod field of the ModR/M byte may refer only to a general register (only MOV (0F20-0F24, 0F26)).
						
						if(bText)
						{
							if(!ProcessModRMAndSIB(idx, pModRmBytePointer, operands[i], bUsesModRM, modRM, bAddrSizePrefix, bOpndSizePrefix, dwIncrement, bText, szModRM, pszSegment))
								return 0;

							StringUtils::sprintfcat(szText, szModRM);
						}
						
						break;
					case EAddressingMode::AM_S:

						// The reg field of the ModR/M byte selects a segment register (only MOV (8C, 8E)).

						if(bText)
						{
							StringUtils::sprintfcat(szText, GetRegisterName(ERegisterType::RegTypeSegment, GetModRMReg(modRM)));
						}

						break;
					case EAddressingMode::AM_O:
				
						// The instruction has no ModR/M byte; the offset of the operand is coded as a word,
						// double word or quad word (depending on address size attribute) in the instruction.
						// No base register, index register, or scaling factor can be applied (only MOV 
						// (A0, A1, A2, A3)).

						if(bText)
						{
							switch(dwOperandSize)
							{
							case 1:
								StringUtils::sprintfcat(szText, "BYTE PTR %s[%08X]", pszSegment, *(DWORD *)pInstr);
								break;
							case 2:
								StringUtils::sprintfcat(szText, "WORD PTR %s[%04X]", pszSegment, *(WORD *)pInstr);
								break;
							case 4:
								StringUtils::sprintfcat(szText, "DWORD PTR %s[%08X]", pszSegment, *(DWORD *)pInstr);
								break;
							}
						}

						break;
					case EAddressingMode::AM_V:

						// The reg field of the ModR/M byte selects a 128-bit XMM register.
						
						if(bText)
						{
							StringUtils::sprintfcat(szText, GetRegisterName(ERegisterType::RegTypeXMM, GetModRMReg(modRM)));
						}

						break;
					case EAddressingMode::AM_W:
				
						// A ModR/M byte follows the opcode and specifies the operand. The operand is either a
						// 128-bit XMM register or a memory address. If it is a memory address, the address is
						// computed from a segment register and any of the following values: a base register,
						// an index register, a scaling factor, and a displacement
						
						if(bText)
						{
							if(!ProcessModRMAndSIB(idx, pModRmBytePointer, operands[i], bUsesModRM, modRM, bAddrSizePrefix, bOpndSizePrefix, dwIncrement, bText, szModRM, pszSegment))
								return 0;

							StringUtils::sprintfcat(szText, szModRM);
						}

						break;
					case EAddressingMode::AM_H:

						// The r/m field of the ModR/M byte always selects a general register, regardless of the
						// mod field (for example, MOV (0F20)).
						
						if(bText)
						{
							if(!ProcessModRMAndSIB(idx, pModRmBytePointer, operands[i], bUsesModRM, modRM, bAddrSizePrefix, bOpndSizePrefix, dwIncrement, bText, szModRM, pszSegment))
								return 0;

							StringUtils::sprintfcat(szText, szModRM);
						}

						break;
					case EAddressingMode::AM_C:

						// The reg field of the ModR/M byte selects a control register (only MOV (0F20, 0F22)).
						
						if(bText)
						{
							StringUtils::sprintfcat(szText, GetRegisterName(ERegisterType::RegTypeControl, GetModRMReg(modRM)));
						}

						break;
					case EAddressingMode::AM_D:

						// The reg field of the ModR/M byte selects a debug register (only MOV (0F21, 0F23)).
						
						if(bText)
						{
							StringUtils::sprintfcat(szText, GetRegisterName(ERegisterType::RegTypeDebug, GetModRMReg(modRM)));
						}

						break;
					case EAddressingMode::AM_P:

						// The reg field of the ModR/M byte selects a packed quadword MMX technology register.
						
						if(bText)
						{
							StringUtils::sprintfcat(szText, GetRegisterName(ERegisterType::RegTypeMMX, GetModRMReg(modRM)));
						}

						break;
					case EAddressingMode::AM_N:
				
						// The R/M field of the ModR/M byte selects a packed quadword MMX technology register.
						
						if(bText)
						{
							if(!ProcessModRMAndSIB(idx, pModRmBytePointer, operands[i], bUsesModRM, modRM, bAddrSizePrefix, bOpndSizePrefix, dwIncrement, bText, szModRM, pszSegment))
								return 0;

							StringUtils::sprintfcat(szText, szModRM);
						}

						break;
					case EAddressingMode::AM_U:

						// The R/M field of the ModR/M byte selects a 128-bit XMM register.
						
						if(bText)
						{
							if(!ProcessModRMAndSIB(idx, pModRmBytePointer, operands[i], bUsesModRM, modRM, bAddrSizePrefix, bOpndSizePrefix, dwIncrement, bText, szModRM, pszSegment))
								return 0;

							StringUtils::sprintfcat(szText, szModRM);
						}

						break;
					case EAddressingMode::AM_Q:

						// A ModR/M byte follows the opcode and specifies the operand. The operand is either an
						// MMX technology register or a memory address. If it is a memory address, the address
						// is computed from a segment register and any of the following values: a base register,
						// an index register, a scaling factor, and a displacement.
						
						if(bText)
						{
							if(!ProcessModRMAndSIB(idx, pModRmBytePointer, operands[i], bUsesModRM, modRM, bAddrSizePrefix, bOpndSizePrefix, dwIncrement, bText, szModRM, pszSegment))
								return 0;

							StringUtils::sprintfcat(szText, szModRM);
						}

						break;
					default:
				
						// this should be all of them
						// DebugBreak();

						break;
					}


					if(!(i == 3 || !OpndIsValid(operands[i+1].Flags)))
					{
						if(bText || bReEnableText)
						{
							StringUtils::sprintfcat(szText, ", ");
						}
					}

					if(bReEnableText)
					{
						bText = TRUE;
						bReEnableText = FALSE;
					}

					pInstr += dwOperandSize;
				}
			}

			if(bText)
			{
				StringUtils::sprintfcat(szText, ");");
			}
		}

		DWORD dwLength = (pInstr - pInstrCopy);

		if(bFPUCommand && dwLength < 2)
			return 0;

		return dwLength;
	}

	DWORD GetInstructionLength(PBYTE pInstr)
	{
		// return instruction length
		BYTE tmp[16];
		return GetInstructionBytes(pInstr, tmp);
	}

	BOOL GetInstructionMnemonic(PBYTE pInstr, char text[128], BOOL bHint)
	{
		int idx = GetCommandIndex(pInstr);
		if(idx == -1)
			return FALSE;

		if(bHint)
		{
			_snprintf(text, 128, "%s (%s)", TableOfInstructions[idx].Mnemonic, TableOfInstructions[idx].Hint);
		}
		else
		{
			_snprintf(text, 128, "%s", TableOfInstructions[idx].Mnemonic);
		}

		return TRUE;
	}

	BOOL GetDisassemblyText(PBYTE pInstr, char szText[128])
	{
		BYTE tmp[16];
 		return GetInstructionBytes(pInstr, tmp, TRUE, szText);
	}
}