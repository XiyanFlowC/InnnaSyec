#include <stdlib.h>

#define LE

#include "codec.h"

static int opcode_buffer_prepared;
static unsigned int opcode_buffer[OPTION_COUNT];

int LookUpSpecial(union uinstr instr);

void PrepareSpecial(union uinstr instr);

int LookUpRegImm(union uinstr instr);

void PrepareRegImm(union uinstr instr);

int LookUpMMI(union uinstr instr);

void PrepareMMI(union uinstr instr);

int LookUpPMFHL(union uinstr instr);

void PreparePMFHL(union uinstr instr);

int LookUpMMI0(union uinstr instr);

void PrepareMMI0(union uinstr instr);

int LookUpMMI1(union uinstr instr);

void PrepareMMI1(union uinstr instr);

int LookUpMMI2(union uinstr instr);

void PrepareMMI2(union uinstr instr);

int LookUpMMI3(union uinstr instr);

void PrepareMMI3(union uinstr instr);

int LookUpCOP0(union uinstr instr);

void PrepareCOP0(union uinstr instr);

int LookUpMF0(union uinstr instr);

void PrepareMF0(union uinstr instr);

int LookUpMT0(union uinstr instr);

void PrepareMT0(union uinstr instr);

int LookUpMF0DBG(union uinstr instr);

void PrepareMF0DBG(union uinstr instr);

int LookUpMT0DBG(union uinstr instr);

void PrepareMT0DBG(union uinstr instr);

int LookUpMF0PREF(union uinstr instr);

void PrepareMF0PREF(union uinstr instr);

int LookUpMT0PREF(union uinstr instr);

void PrepareMT0PREF(union uinstr instr);

int LookUpBC0(union uinstr instr);

void PrepareBC0(union uinstr instr);

int LookUpC0(union uinstr instr);

void PrepareC0(union uinstr instr);

int LookUpCOP1(union uinstr instr);

void PrepareCOP1(union uinstr instr);

int LookUpBC1(union uinstr instr);

void PrepareBC1(union uinstr instr);

int LookUpS(union uinstr instr);

void PrepareS(union uinstr instr);

int LookUpD(union uinstr instr);

void PrepareD(union uinstr instr);

int LookUpW(union uinstr instr);

void PrepareW(union uinstr instr);

int LookUpL(union uinstr instr);

void PrepareL(union uinstr instr);

int LookUpOpcode(union uinstr instr);

/* Opcode Look up tables. */
static int PrimaryOpcodeLookUpTable[] =
{
	SPECIAL, REGIMM,        J,      JAL,      BEQ,      BNE,    BLEZ,  BGTZ,
	ADDI,  ADDIU,     SLTI,    SLTIU,     ANDI,      ORI,     XORI,   LUI,
	COP0,   COP1, RESERVED, RESERVED,     BEQL,     BNEL,    BLEZL, BGTZL,
	DADDI, DADDIU,      LDL,      LDR,      MMI, RESERVED,       LQ,    SQ,
	LB,     LH,      LWL,       LW,      LBU,      LHU,      LWR,   LWU,
	SB,     SH,      SWL,       SW,      SDL,      SDR,      SWR, CACHE,
	RESERVED,   LWC1, RESERVED,     PREF, RESERVED,     LDC1, RESERVED,    LD,
	RESERVED,   SWC1, RESERVED, RESERVED, RESERVED,     SDC1, RESERVED,    SD
};

/* reg: funct */
static int SpecialOpcodeLookUpTable[] =
{
		SLL, RESERVED,  SRL,  SRA,     SLLV, RESERVED,     SRLV,     SRAV,
		JR,     JALR, MOVZ, MOVN,  SYSCALL,    BREAK, RESERVED,     SYNC,
	MFHI,     MTHI, MFLO, MTLO,    DSLLV, RESERVED,    DSRLV,    DSRAV,
	MULT,    MULTU,  DIV, DIVU, RESERVED, RESERVED, RESERVED, RESERVED,
		ADD,     ADDU,  SUB, SUBU,      AND,       OR,      XOR,      NOR,
	MFSA,     MTSA,  SLT, SLTU,     DADD,    DADDU,     DSUB,    DSUBU,
		TGE,     TGEU,  TLT, TLTU,      TEQ, RESERVED,      TNE, RESERVED,
	DSLL, RESERVED, DSRL, DSRA,   DSLL32, RESERVED,   DSRL32,   DSRA32
};

/* imm: rt */
static int RegImmOpcodeLookUpTable[] =
{
		BLTZ,   BGEZ,     BLTZL,   BGEZL, RESERVED, RESERVED, RESERVED, RESERVED,
		TGEI,  TGEIU,     TLTI,    TLTIU,     TEQI, RESERVED,     TNEI, RESERVED,
	BLTZAL, BGEZAL,  BLTZALL,  BGEZALL, RESERVED, RESERVED, RESERVED, RESERVED,
		MTSAB,  MTSAH, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED,
};

/* reg: funct */
static int MMIOpcodeLookUpTable[] =
{
	MADD, MADDU, RESERVED, RESERVED, PLZCW, RESERVED, RESERVED, RESERVED,
	MMI0, MMI2, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED,
	MFHI1, MTHI1, MFLO1, MTLO1, RESERVED, RESERVED, RESERVED, RESERVED,
	MULT1, MULTU1, DIV1, DIVU1, RESERVED, RESERVED, RESERVED, RESERVED,
	MADD1, MADDU1, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED,
	MMI1, MMI3, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED,
	PMFHL, PMTHL_LW, RESERVED, RESERVED, PSLLH, RESERVED, PSRLH, PSRAH,
	RESERVED, RESERVED, RESERVED, RESERVED, PSLLW, RESERVED, PSRLW, PSRAW,
};

/* reg: sa */
static int PMFHLOpcodeLookUpTable[] =
{
	PMFHL_LW, PMFHL_UW, PMFHL_SLW, PMFHL_LH, PMFHL_SH,
};

/* reg: sa */
static int MMI0OpcodeLookUpTable[] =
{
	PADDW, PSUBW, PCGTW, PMAXW,
	PADDH, PSUBH, PCGTH, PMAXH,
	PADDB, PSUBB, PCGTB, RESERVED,
	RESERVED, RESERVED, RESERVED, RESERVED,
	PADDSW, PSUBSW, PEXTLW, PPACW,
	PADDSH, PSUBSH, PEXTLH, PPACH,
	PADDSB, PSUBSB, PEXTLB, PPACB,
	RESERVED, RESERVED, PEXT5, PPAC5,
};

/* reg: sa */
static int MMI1OpcodeLookUpTable[] =
{
	RESERVED, PABSW, PCEQW, PMINW,
	PADSBH, PABSH, PCEQH, PMINH,
	RESERVED, RESERVED, PCEQB, RESERVED,
	RESERVED, RESERVED, RESERVED, RESERVED,
	PADDUW, PSUBUW, PEXTUW, RESERVED,
	PADDUH, PSUBUH, PEXTUH, RESERVED,
	PADDUB, PSUBUB, PEXTUB, QFSRV,
	RESERVED, RESERVED, RESERVED, RESERVED,
};

/* reg: sa */
static int MMI2OpcodeLookUpTable[] =
{
	PMADDW, RESERVED, PSLLVW, PSRLVW,
	PMSUBW, RESERVED, RESERVED, RESERVED,
	PMFHI, PMFLO, PINTH, RESERVED,
	PMULTW, PDIVW, PCPYLD, RESERVED,
	PMADDH, PHMADH, PAND, PXOR,
	PMSUBH, PHMSBH, RESERVED, RESERVED,
	RESERVED, RESERVED, PEXEH, PREVH,
	PMULTH, PDIVBW, PEXEW, PROT3W,
};

static int MMI3OpcodeLookUpTable[] =
{
	PMADDUW, RESERVED, RESERVED, PSRAVW,
	RESERVED, RESERVED, RESERVED, RESERVED,
	PMTHI, PMTLO, PINTEH, RESERVED,
	PMULTUW, PDIVUW, PCPYUD, RESERVED,
	RESERVED, RESERVED, POR, PNOR,
	RESERVED, RESERVED, RESERVED, RESERVED,
	RESERVED, RESERVED, PEXCH, PCPYH,
	RESERVED, RESERVED, PEXCW, RESERVED,
};

/* reg: rs */
static int COP0OpcodeLookUpTable[] =
{
	MF0, RESERVED, RESERVED, RESERVED, MT0, RESERVED, RESERVED, RESERVED,
	BC0, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED,
	C0, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED,
	RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED,
};

/* REG: RD */
static int MF0OpcodeLookUpTable[] =
{
	MFC0, MFC0, MFC0, MFC0, MFC0, MFC0, MFC0, MFC0,
	MFC0, MFC0, MFC0, MFC0, MFC0, MFC0, MFC0, MFC0,
	MFC0, MFC0, MFC0, MFC0, MFC0, MFC0, MFC0, MFC0,
	MF0DBG, MF0PREF, MFC0, MFC0, MFC0, MFC0, MFC0, MFC0
};

static int MT0OpcodeLookUpTable[] =
{
	MTC0, MTC0, MTC0, MTC0, MTC0, MTC0, MTC0, MTC0,
	MTC0, MTC0, MTC0, MTC0, MTC0, MTC0, MTC0, MTC0,
	MTC0, MTC0, MTC0, MTC0, MTC0, MTC0, MTC0, MTC0,
	MT0DBG, MT0PREF, MTC0, MTC0, MTC0, MTC0, MTC0, MTC0
};

/* reg: rd=Debug(24) funct */
static int MF0DebugOpcodeLookUpTable[] =
{
	MFBPC, RESERVED, MFIAB, MFIABM, MFDAB, MFDABM, MFDVB, MFDVBM,
};

/* reg: rd=Debug(24) funct */
static int MT0DebugOpcodeLookUpTable[] =
{
	MTBPC, RESERVED, MTIAB, MTIABM, MTDAB, MTDABM, MTDVB, MTDVBM,
};

static int MF0PrefOpcodeLookUpTable[] =
{
	MFPS, MFPC,
};

static int MT0PrefOpcodeLookUpTable[] =
{
	MTPS, MTPC,
};

/* rt */
static int BC0OpcodeLookUpTable[] =
{
	BC0F, BC0T, BC0FL, BC0TL,
};

static int C0OpcodeLookUpTable[] =
{
	RESERVED, TLBR, TLBWI, RESERVED, RESERVED, RESERVED, TLBWR, RESERVED,
	TLBP, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED,
	EI, DI, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, /* May EI & DI located here... */
	ERET, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED,
	RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED,
	RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED,
	RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED,
	EI, DI, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, /* EI & DI may not here...*/
	/* Some confusion? */
};

/* reg, rs */
static int COP1OpcodeLookUpTable[] =
{
	MFC1, DMFC1, CFC1, RESERVED, MTC1, DMTC1, CTC1, RESERVED,
	BC1, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED,
	S, D, RESERVED, RESERVED, W, L, RESERVED, RESERVED,
	RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED,
};

/* rt */
static int BC1OpcodeLookUpTable[] =
{
	BC1F, BC1T,
};

/* reg, funct */
static int SOpcodeLookUpTable[] =
{
	ADD_S, SUB_S, MUL_S, DIV_S, SQRT_S, ABS_S, MOV_S, NEG_S,
	ROUND_L_S, TRUNC_L_S, CEIL_L_S, FLOOR_L_S, ROUND_W_S, TRUNC_W_S, CEIL_W_S, FLOOR_W_S,
	RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED,
	ADDA_S, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED,
	RESERVED, CVT_D_S, RESERVED, RESERVED, CVT_W_S, CVT_L_S, RESERVED, RESERVED,
	RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED,
	C_F_S, C_UN_S, C_EQ_S, C_UEQ_S, C_OLT_S, C_ULT_S, C_OLE_S, C_ULE_S,
	C_SF_S, C_NGLE_S, C_SEQ_S, C_NGL_S, C_LT_S, C_NGE_S, C_LE_S, C_NGE_S,
};

/* reg, funct */
static int DOpcodeLookUpTable[] =
{
	ADD_D, SUB_D, MUL_D, DIV_D, SQRT_D, ABS_D, MOV_D, NEG_D,
	ROUND_L_D, TRUNC_L_D, CEIL_L_D, FLOOR_L_D, ROUND_W_D, TRUNC_W_D, CEIL_W_D, FLOOR_W_D,
	RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED,
	RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED,
	CVT_S_D, RESERVED, RESERVED, RESERVED, CVT_W_D, CVT_L_D, RESERVED, RESERVED,
	RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED,
	C_F_D, C_UN_D, C_EQ_D, C_UEQ_D, C_OLT_D, C_ULT_D, C_OLE_D, C_ULE_D,
	C_SF_D, C_NGLE_D, C_SEQ_D, C_NGL_D, C_LT_D, C_NGE_D, C_LE_D, C_NGE_D,
};

static int WOpcodeLookUpTable[] =
{
	RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED,
	RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED,
	RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED,
	RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED,
	CVT_S_W, CVT_D_W, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED,
	RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED,
	RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED,
	RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED,
};

static int LOpcodeLookUpTable[] =
{
	RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED,
	RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED,
	RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED,
	RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED,
	CVT_S_L, CVT_D_L, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED,
	RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED,
	RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED,
	RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED, RESERVED,
};

static int LookUpPrimaryOpcode(int opcode)
{
	return RESERVED;
}

unsigned long long sign_extend(unsigned short data)
{
	unsigned long long ans;
	if (data >> 15)
		ans = 0xFFFFFFFFFFFF0000ULL | data;
	else
		ans = data;
	return ans;
}

/// <summary>
/// ����ָ�����ͨ��ָ����ṹ��
/// </summary>
/// <returns>
/// ת����ϵĽṹ�壬ʧ��ʱ return_value.opcode == -1
/// </returns>
/// <param name="instruction">
/// Ҫ����� MIPS R5900 ָ�32 λԪ��
/// </param>
struct instr_t DecodeInstruction(unsigned int instruction)
{
	union uinstr anyl = {0};
	anyl.code = instruction;
	struct instr_t ans = {0};
	if (instruction == 0)
	{
		ans.opcode = NOP;
		return ans;
	}
	int index = LookUpOpcode(anyl);
	ans.opcode = index;
	if (index >= OPTION_COUNT || index < 0) return ans.opcode = -1, ans;
	struct instr_def* def = instructions + index;
	if (def->type == IT_code)
	{
		ans.imm = anyl.dtype.data;
	}
	else if (def->type == IT_imm)
	{
		ans.imm = sign_extend((unsigned short)anyl.itype.imm);
		ans.rt = anyl.itype.rt;
		ans.rs = anyl.itype.rs;
	}
	else if (def->type == IT_immu)
	{
		ans.imm = anyl.itype.imm;
		ans.rt = anyl.itype.rt;
		ans.rs = anyl.itype.rs;
	}
	else if (def->type == IT_imma)
	{
		ans.imm = sign_extend((unsigned short)anyl.itype.imm) << 2;
		ans.rt = anyl.itype.rt;
		ans.rs = anyl.itype.rs;
	}
	else if (def->type == IT_jump)
	{
		ans.imm = (unsigned long long)anyl.jtype.addr << 2;
	}
	else if (def->type == IT_reg)
	{
		ans.rd = anyl.rtype.rd;
		ans.rt = anyl.rtype.rt;
		ans.rs = anyl.rtype.rs;
		ans.sa = anyl.rtype.sa;
	}
	else if (def->type == IT_regf)
	{
		ans.rd = anyl.rtype.sa;
		ans.rt = anyl.rtype.rt;
		ans.rs = anyl.rtype.rd;
	}
	else ans.opcode = -1;
	return ans;
}

/// <summary>
/// ����ָ��ṹ�岢����ָ�32 λԪ��
/// </summary>
/// <returns>
/// ת����ϵĽṹ�壬ʧ��ʱ���� 0 ��NOP��
/// </returns>
/// <param name="instruction">
/// �����˵� MIPS R5900 ָ��
/// </param>
unsigned int EncodeInstruction(struct instr_t instruction)
{
	union uinstr ans = {0};
	ans.code = GetTemplateByIndex((int)instruction.opcode);
	switch (instruction.opcode)
	{
#include "encode.inc"
	}
	return ans.code;
}

/// <summary>
/// ���ݶ���Ĳ�������OpCode.h ֮���壩��ȡָ���
/// </summary>
/// <returns>
/// ָ���ṹ�壬ʧ��ʱ���� NOP ��Ӧ֮�ṹ��
/// </returns>
/// <param name="instruction">
/// Ҫȡ�õĶ�������
/// </param>
struct instr_def GetInstructionDefinitionByIndex(int option)
{
	if (option >= OPTION_COUNT) return instructions[NOP];
	return instructions[option];
}

/// <summary>
/// ���ݶ�������ƣ�InsDef.h ֮���壩��ȡָ���
/// </summary>
/// <returns>
/// ָ���ṹ�壬ʧ��ʱ���� NOP ��Ӧ֮�ṹ��
/// </returns>
/// <param name="instruction">
/// Ҫȡ�õĶ���֮����
/// </param>
struct instr_def GetInstructionDefinitionByName(const char *name)
{
	for (int i = 0; i < OPTION_COUNT; ++i)
	{
		if (strcmp(instructions[i].name, name) == 0)
			return instructions[i];
	}
	return instructions[NOP];
}

int LookUpOpcode(union uinstr instr)
{
	int opcode = PrimaryOpcodeLookUpTable[ instr.base.opcode ];
	if (opcode == SPECIAL)
		return LookUpSpecial(instr);
	if (opcode == REGIMM)
		return LookUpRegImm(instr);
	if (opcode == COP0)
		return LookUpCOP0(instr);
	if (opcode == COP1)
		return LookUpCOP1(instr);
	if (opcode == MMI)
		return LookUpMMI(instr);
	return opcode;
}

unsigned int GetTemplateByIndex(int opcode)
{
	// if (!opcode_buffer_prepared) PrepareOpcodeBuffer();
	return opcode_buffer[opcode];
}

void PrepareOpcodeBuffer()
{
	if (opcode_buffer_prepared) return;
	union uinstr tmp = {0};
	for (int i = 0; i < 64; ++i)
	{
		if (PrimaryOpcodeLookUpTable[i] == RESERVED) continue;
		tmp.base.opcode = i;
		if (PrimaryOpcodeLookUpTable[i] < 0)
		{
			switch (PrimaryOpcodeLookUpTable[i])
			{
			case SPECIAL:
				PrepareSpecial(tmp);
				break;
			case REGIMM:
				PrepareRegImm(tmp);
				break;
			case COP0:
				PrepareCOP0(tmp);
				break;
			case COP1:
				PrepareCOP1(tmp);
				break;
			case MMI:
				PrepareMMI(tmp);
				break;
			}
			continue;
		}
		opcode_buffer[PrimaryOpcodeLookUpTable[i]] = tmp.code;
	}
	opcode_buffer_prepared = 1;
}

void PrepareSpecial(union uinstr instr)
{
	union uinstr tmp = instr;
	for (int i = 0; i < 64; ++i)
	{
		if (SpecialOpcodeLookUpTable[i] < 0) continue;
		tmp.rtype.funct = i;
		opcode_buffer[SpecialOpcodeLookUpTable[i]] = tmp.code;
	}
}

void PrepareRegImm(union uinstr instr)
{
	union uinstr tmp = instr;
	for (int i = 0; i < 32; ++i)
	{
		if (RegImmOpcodeLookUpTable[i] < 0) continue;
		tmp.itype.rt = i;
		opcode_buffer[RegImmOpcodeLookUpTable[i]] = tmp.code;
	}
}

void PrepareMMI(union uinstr instr)
{
	union uinstr tmp = instr;
	for (int i = 0; i < 64; ++i)
	{
		if (MMIOpcodeLookUpTable[i] == RESERVED) continue;
		tmp.rtype.funct = i;
		if (MMIOpcodeLookUpTable[i] < 0)
		{
			switch (MMIOpcodeLookUpTable[i])
			{
			case PMFHL:
				PreparePMFHL(tmp);
				break;
			case MMI0:
				PrepareMMI0(tmp);
				break;
			case MMI1:
				PrepareMMI1(tmp);
				break;
			case MMI2:
				PrepareMMI2(tmp);
				break;
			case MMI3:
				PrepareMMI3(tmp);
				break;
			}
			continue;
		}
		opcode_buffer[MMIOpcodeLookUpTable[i]] = tmp.code;
	}
}

void PreparePMFHL(union uinstr instr)
{
	for (int i = 0; i < 5; ++i)
	{
		instr.rtype.sa = i;
		opcode_buffer[PMFHLOpcodeLookUpTable[i]] = instr.code;
	}
}

void PrepareMMI0(union uinstr instr)
{
	for (int i = 0; i < 32; ++i)
	{
		if (MMI0OpcodeLookUpTable[i] == RESERVED) continue;
		instr.rtype.sa = i;
		opcode_buffer[MMI0OpcodeLookUpTable[i]] = instr.code;
	}
}

void PrepareMMI1(union uinstr instr)
{
	for (int i = 0; i < 32; ++i)
	{
		if (MMI1OpcodeLookUpTable[i] == RESERVED) continue;
		instr.rtype.sa = i;
		opcode_buffer[MMI1OpcodeLookUpTable[i]] = instr.code;
	}
}

void PrepareMMI2(union uinstr instr)
{
	for (int i = 0; i < 32; ++i)
	{
		if (MMI2OpcodeLookUpTable[i] == RESERVED) continue;
		instr.rtype.sa = i;
		opcode_buffer[MMI2OpcodeLookUpTable[i]] = instr.code;
	}
}

void PrepareMMI3(union uinstr instr)
{
	for (int i = 0; i < 32; ++i)
	{
		if (MMI3OpcodeLookUpTable[i] == RESERVED) continue;
		instr.rtype.sa = i;
		opcode_buffer[MMI3OpcodeLookUpTable[i]] = instr.code;
	}
}

void PrepareCOP0(union uinstr instr)
{
	for (int i = 0; i < 32; ++i)
	{
		if (COP0OpcodeLookUpTable[i] == RESERVED) continue;
		instr.rtype.rs = i;
		switch (COP0OpcodeLookUpTable[i])
		{
		case MF0:
			PrepareMF0(instr);
			break;
		case MT0:
			PrepareMT0(instr);
			break;
		case BC0:
			PrepareBC0(instr);
			break;
		case C0:
			PrepareC0(instr);
			break;
		}
	}
}

void PrepareMF0(union uinstr instr)
{
	opcode_buffer[MFC0] = instr.code;
	instr.rtype.rd = 24;
	PrepareMF0DBG(instr);
	instr.rtype.rd = 25;
	PrepareMF0PREF(instr);
}

void PrepareMT0(union uinstr instr)
{
	opcode_buffer[MTC0] = instr.code;
	instr.rtype.rd = 24;
	PrepareMT0DBG(instr);
	instr.rtype.rd = 25;
	PrepareMT0PREF(instr);
}

void PrepareMF0DBG(union uinstr instr)
{
	for (int i = 0; i < 8; ++i)
	{
		if (MF0DebugOpcodeLookUpTable[i] == RESERVED) continue;
		instr.rtype.funct = i;
		opcode_buffer[MF0DebugOpcodeLookUpTable[i]] = instr.code;
	}
}

void PrepareMT0DBG(union uinstr instr)
{
	for (int i = 0; i < 8; ++i)
	{
		if (MT0DebugOpcodeLookUpTable[i] == RESERVED) continue;
		instr.rtype.funct = i;
		opcode_buffer[MT0DebugOpcodeLookUpTable[i]] = instr.code;
	}
}

void PrepareMF0PREF(union uinstr instr)
{
	for (int i = 0; i < 2; ++i)
	{
		if (MF0PrefOpcodeLookUpTable[i] == RESERVED) continue;
		instr.rtype.funct = i;
		opcode_buffer[MF0PrefOpcodeLookUpTable[i]] = instr.code;
	}
}

void PrepareMT0PREF(union uinstr instr)
{
	for (int i = 0; i < 2; ++i)
	{
		if (MT0PrefOpcodeLookUpTable[i] == RESERVED) continue;
		instr.rtype.funct = i;
		opcode_buffer[MT0PrefOpcodeLookUpTable[i]] = instr.code;
	}
}

void PrepareBC0(union uinstr instr)
{
	for (int i = 0; i < 4; ++i)
	{
		if (BC0OpcodeLookUpTable[i] == RESERVED) continue;
		instr.rtype.rt = i;
		opcode_buffer[BC0OpcodeLookUpTable[i]] = instr.code;
	}
}

void PrepareC0(union uinstr instr)
{
	for (int i = 0; i < 64; ++i)
	{
		if (C0OpcodeLookUpTable[i] == RESERVED) continue;
		instr.rtype.rt = i;
		opcode_buffer[C0OpcodeLookUpTable[i]] = instr.code;
	}
}

void PrepareCOP1(union uinstr instr)
{
	for (int i = 0; i < 32; ++i)
	{
		if (COP1OpcodeLookUpTable[i] == RESERVED) continue;
		instr.rtype.rs = i;
		if (COP1OpcodeLookUpTable[i] < 0)
		{
			switch (COP1OpcodeLookUpTable[i])
			{
			case BC1:
				PrepareBC1(instr);
				break;
			case S:
				PrepareS(instr);
				break;
			case D:
				PrepareD(instr);
				break;
			case W:
				PrepareW(instr);
				break;
			case L:
				PrepareL(instr);
				break;
			}
			continue;
		}
		opcode_buffer[COP1OpcodeLookUpTable[i]] = instr.code;
	}
}

void PrepareBC1(union uinstr instr)
{
	instr.rtype.rt = 0;
	opcode_buffer[BC1F] = instr.code;
	instr.rtype.rt = 1;
	opcode_buffer[BC1T] = instr.code;
}

void PrepareS(union uinstr instr)
{
	for (int i = 0; i < 64; ++i)
	{
		if (SOpcodeLookUpTable[i] == RESERVED) continue;
		instr.rtype.funct = i;
		opcode_buffer[SOpcodeLookUpTable[i]] = instr.code;
	}
}

void PrepareD(union uinstr instr)
{
	for (int i = 0; i < 64; ++i)
	{
		if (DOpcodeLookUpTable[i] == RESERVED) continue;
		instr.rtype.funct = i;
		opcode_buffer[DOpcodeLookUpTable[i]] = instr.code;
	}
}

void PrepareW(union uinstr instr)
{
	for (int i = 0; i < 64; ++i)
	{
		if (WOpcodeLookUpTable[i] == RESERVED) continue;
		instr.rtype.funct = i;
		opcode_buffer[WOpcodeLookUpTable[i]] = instr.code;
	}
}

void PrepareL(union uinstr instr)
{
	for (int i = 0; i < 64; ++i)
	{
		if (LOpcodeLookUpTable[i] == RESERVED) continue;
		instr.rtype.funct = i;
		opcode_buffer[LOpcodeLookUpTable[i]] = instr.code;
	}
}

int LookUpSpecial(union uinstr instr)
{
	return SpecialOpcodeLookUpTable[ instr.rtype.funct ];
}

int LookUpRegImm(union uinstr instr)
{
	return RegImmOpcodeLookUpTable[ instr.itype.rt ];
}

int LookUpMMI(union uinstr instr)
{
	int opcode = MMIOpcodeLookUpTable[instr.rtype.funct];
	if (opcode == MMI0)
		return LookUpMMI0(instr);
	if (opcode == MMI1)
		return LookUpMMI1(instr);
	if (opcode == MMI2)
		return LookUpMMI2(instr);
	if (opcode == MMI3)
		return LookUpMMI3(instr);
	if (opcode == PMFHL)
		return LookUpPMFHL(instr);
	return opcode;
}

int LookUpPMFHL(union uinstr instr)
{
	return PMFHLOpcodeLookUpTable[ instr.rtype.sa ];
}

int LookUpMMI0(union uinstr instr)
{
	return MMI0OpcodeLookUpTable[ instr.rtype.sa ];
}

int LookUpMMI1(union uinstr instr)
{
	return MMI1OpcodeLookUpTable[instr.rtype.sa];
}

int LookUpMMI2(union uinstr instr)
{
	return MMI2OpcodeLookUpTable[instr.rtype.sa];
}

int LookUpMMI3(union uinstr instr)
{
	return MMI3OpcodeLookUpTable[instr.rtype.sa];
}

int LookUpCOP0(union uinstr instr)
{
	switch (COP0OpcodeLookUpTable[instr.rtype.rs])
	{
	case MF0:
		return LookUpMF0(instr);
	case MT0:
		return LookUpMT0(instr);
	case BC0:
		return LookUpBC0(instr);
	case C0:
		return LookUpC0(instr);
	default:
		return COP0OpcodeLookUpTable[instr.rtype.rs];
	}
}

int LookUpMF0(union uinstr instr)
{
	int opcode = instr.rtype.rd;
	if (opcode == MF0DBG)
		return LookUpMF0DBG(instr);
	if (opcode == MF0PREF)
		return LookUpMF0PREF(instr);
	return MF0OpcodeLookUpTable[opcode];
}

int LookUpMT0(union uinstr instr)
{
	int opcode = instr.rtype.rd;
	if (opcode == MT0DBG)
		return LookUpMT0DBG(instr);
	if (opcode == MT0PREF)
		return LookUpMT0PREF(instr);
	return MT0OpcodeLookUpTable[opcode];
}

int LookUpMF0DBG(union uinstr instr)
{
	return MF0DebugOpcodeLookUpTable[ instr.rtype.funct ];
}

int LookUpMT0DBG(union uinstr instr)
{
	return MT0DebugOpcodeLookUpTable[instr.rtype.funct];
}

int LookUpMF0PREF(union uinstr instr)
{
	return MF0PrefOpcodeLookUpTable[instr.rtype.funct];
}

int LookUpMT0PREF(union uinstr instr)
{
	return MT0PrefOpcodeLookUpTable[instr.rtype.funct];
}

int LookUpBC0(union uinstr instr)
{
	return BC0OpcodeLookUpTable[ instr.rtype.rt ];
}

int LookUpC0(union uinstr instr)
{
	return C0OpcodeLookUpTable[instr.rtype.rs];
}

int LookUpCOP1(union uinstr instr)
{
	int opcode = COP1OpcodeLookUpTable[ instr.rtype.rs ];
	switch (opcode)
	{
	case BC1:
		return LookUpBC1(instr);
	case S:
		return LookUpS(instr);
	case D:
		return LookUpD(instr);
	case W:
		return LookUpW(instr);
	case L:
		return LookUpL(instr);
	default:
		return opcode;
	}
}

int LookUpBC1(union uinstr instr)
{
	return BC1OpcodeLookUpTable[ instr.rtype.rt ];
}

int LookUpS(union uinstr instr)
{
	return SOpcodeLookUpTable[instr.rtype.funct];
}

int LookUpD(union uinstr instr)
{
	return DOpcodeLookUpTable[instr.rtype.funct];
}

int LookUpW(union uinstr instr)
{
	return WOpcodeLookUpTable[instr.rtype.funct];
}

int LookUpL(union uinstr instr)
{
	return LOpcodeLookUpTable[instr.rtype.funct];
}

struct instr_def instructions[] = {
	{IT_reg, "ADD", "$rd, $rs, $rt"},
	{IT_imm, "ADDI", "$rt, $rs, #im"},
	{IT_imm, "ADDIU", "$rt, $rs, #im"},
	{IT_reg, "ADDU", "$rd, $rs, $rt"},
	{IT_reg, "AND", "$rd, $rs, $rd"},
	{IT_immu, "ANDI", "$rt, $rs, %im"},
	{IT_imma, "BEQ", "$rs, $rt, &im"},
	{IT_imma, "BEQL", "$rs, $rt, &im"},
	{IT_imma, "BGEZ", "$rs, &im"},
	{IT_imma, "BGEZAL", "$rs, &im"},
	{IT_imma, "BGEZALL", "$rs, &im"},
	{IT_imma, "BGEZL", "$rs, &im"},
	{IT_imma, "BGTZ", "$rs, &im"},
	{IT_imma, "BGTZL", "$rs, &im"},
	{IT_imma, "BLEZ", "$rs, &im"},
	{IT_imma, "BLEZL", "$rs, &im"},
	{IT_imma, "BLTZ", "$rs, &im"},
	{IT_imma, "BLTZAL", "$rs, &im"},
	{IT_imma, "BLTZALL", "$rs, &im"},
	{IT_imma, "BLTZL", "$rs, &im"},
	{IT_imma, "BNE", "$rs, $rt, &im"},
	{IT_imma, "BNEL", "$rs, $rt, &im"},
	{IT_code, "BREAK", "!im"},
	{IT_reg, "DADD", "$rd, $rs, $rt"},
	{IT_imm, "DADDI", "$rt, $rs, #im"},
	{IT_imm, "DADDIU", "$rt, $rs, #im"},
	{IT_reg, "DADDU", "$rd, $rs, $rt"},
	{IT_reg, "DIV", "$rs, $rt"},
	{IT_reg, "DIVU", "$rs, $rt"},
	{IT_reg, "DSLL", "$rd, $rt, %sa"},
	{IT_reg, "DSLL32", "$rd, $rt, %sa"},
	{IT_reg, "DSLLV", "$rd, $rt, $rs"},
	{IT_reg, "DSRA", "$rd, $rt, %sa"},
	{IT_reg, "DSRA32", "$rd, $rt, %sa"},
	{IT_reg, "DSRAV", "$rd, $rt, $rs"},
	{IT_reg, "DSRL", "$rd, $rt, %sa"},
	{IT_reg, "DSRL32", "$rd, $rt, %sa"},
	{IT_reg, "DSRLV", "$rd, $rt, $rs"},
	{IT_reg, "DSUB", "$rd, $rs, $rt"},
	{IT_reg, "DSUBU", "$rd, $rs, $rt"},
	{IT_jump, "J", "*im"},
	{IT_jump, "JAL", "*im"},
	{IT_reg, "JALR", "$rs, $rd"},
	{IT_reg, "JR", "$rs"},
	{IT_imm, "LB", "$rt, #im($rs)"},
	{IT_imm, "LBU", "$rt, #im($rs)"},
	{IT_imm, "LD", "$rt, #im($rs)"},
	{IT_imm, "LDL", "$rt, #im($rs)"},
	{IT_imm, "LDR", "$rt, #im($rs)"},
	{IT_imm, "LH", "$rt, #im($rs)"},
	{IT_imm, "LHU", "$rt, #im($rs)"},
	{IT_immu, "LUI", "$rt, %im"},
	{IT_imm, "LW", "$rt, #im($rs)"},
	{IT_imm, "LWL", "$rt, #im($rs)"},
	{IT_imm, "LWR", "$rt, #im($rs)"},
	{IT_imm, "LWU", "$rt, #im($rs)"},
	{IT_reg, "MFHI", "$rd"},
	{IT_reg, "MFLO", "$rd"},
	{IT_reg, "MOVN", "$rd, $rs, $rt"},
	{IT_reg, "MOVZ", "$rd, $rs, $rt"},
	{IT_reg, "MTHI", "$rs"},
	{IT_reg, "MTLO", "$rs"},
	{IT_reg, "MULT", "$rd, $rs, $rt"},
	{IT_reg, "MULTU", "$rs, $rt"},
	{IT_reg, "NOR", "$rd, $rs, $rt"},
	{IT_reg, "OR", "$rd, $rs, $rt"},
	{IT_immu, "ORI", "$rt, $rs, %im"},
	{IT_imm, "PREF", "$rt, #im($rs)"},
	{IT_imm, "SB", "$rt, #im($rs)"},
	{IT_imm, "SD", "$rt, #im($rs)"},
	{IT_imm, "SDL", "$rt, #im($rs)"},
	{IT_imm, "SDR", "$rt, #im($rs)"},
	{IT_imm, "SH", "$rt, #im($rs)"},
	{IT_reg, "SLL", "$rd, $rt, %sa"},
	{IT_reg, "SLLV", "$rd, $rt, $rs"},
	{IT_reg, "SLT", "$rd, $rt, $rs"},
	{IT_imm, "SLTI", "$rt, $rs, #im"},
	{IT_immu, "SLTIU", "$rt, $rs, %im"},
	{IT_reg, "SLTU", "$rd, $rt, $rs"},
	{IT_reg, "SRA", "$rd, $rt, %sa"},
	{IT_reg, "SRAV", "$rd, $rt, $rs"},
	{IT_reg, "SRL", "$rd, $rt, %sa"},
	{IT_reg, "SRLV", "$rd, $rt, $rs"},
	{IT_reg, "SUB", "$rd, $rt, $rs"},
	{IT_reg, "SUBU", "$rd, $rt, $rs"},
	{IT_imm, "SW", "$rt, #im($rs)"},
	{IT_imm, "SWL", "$rt, #im($rs)"},
	{IT_imm, "SWR", "$rt, #im($rs)"},
	{IT_reg, "SYNC", "!sa"},
	{IT_code, "SYSCALL", "!im"},
	{IT_reg, "TEQ", "$rs, $rt"},
	{IT_imm, "TEQI", "$rs, #im"},
	{IT_reg, "TGE", "$rs, $rt"},
	{IT_imm, "TGEI", "$rs, #im"},
	{IT_imm, "TGEIU", "$rs, #im"},
	{IT_reg, "TGEU", "$rs, $rt"},
	{IT_reg, "TLT", "$rs, $rt"},
	{IT_imm, "TLTI", "$rs, #im"},
	{IT_imm, "TLTIU", "$rs, #im"},
	{IT_reg, "TLTU", "$rs, $rt"},
	{IT_reg, "TNE", "$rs, $rt"},
	{IT_imm, "TNEI", "$rs, #im"},
	{IT_reg, "XOR", "$rd, $rs, $rt"},
	{IT_immu, "XORI", "$rs, $rt, %im"},
	{IT_reg, "DIV1", "$rs, $rt"},
	{IT_reg, "DIVU1", "$rs, $rt"},
	{IT_imm, "LQ", "$rt, #im($rs)"},
	{IT_reg, "MADD", "$rd, $rs, $rt"},
	{IT_reg, "MADD1", "$rd, $rs, $rt"},
	{IT_reg, "MADDU", "$rd, $rs, $rt"},
	{IT_reg, "MADDU1", "$rd, $rs, $rt"},
	{IT_reg, "MFHI1", "$rd"},
	{IT_reg, "MFLO1", "$rd"},
	{IT_reg, "MFSA", "$rd"},
	{IT_reg, "MTHI1", "$rs"},
	{IT_reg, "MTLO1", "$rs"},
	{IT_reg, "MTSA", "$rs"},
	{IT_imm, "MTSAB", "$rs, #im"},
	{IT_imm, "MTSAH", "$rs, #im"},
	{IT_reg, "MULT1", "$rs, $rt"},
	{IT_reg, "MULTU1", "$rs, $rt"},
	{IT_reg, "PABSH", "$rd, $rt"},
	{IT_reg, "PABSW", "$rd, $rt"},
	{IT_reg, "PADDB", "$rd, $rs, $rt"},
	{IT_reg, "PADDH", "$rd, $rs, $rt"},
	{IT_reg, "PADDSB", "$rd, $rs, $rt"},
	{IT_reg, "PADDSH", "$rd, $rs, $rt"},
	{IT_reg, "PADDSW", "$rd, $rs, $rt"},
	{IT_reg, "PADDUB", "$rd, $rs, $rt"},
	{IT_reg, "PADDUH", "$rd, $rs, $rt"},
	{IT_reg, "PADDUW", "$rd, $rs, $rt"},
	{IT_reg, "PADDW", "$rd, $rs, $rt"},
	{IT_reg, "PADSBH", "$rd, $rs, $rt"},
	{IT_reg, "PAND", "$rd, $rs, $rt"},
	{IT_reg, "PCEQB", "$rd, $rs, $rt"},
	{IT_reg, "PCEQH", "$rd, $rs, $rt"},
	{IT_reg, "PCEQW", "$rd, $rs, $rt"},
	{IT_reg, "PCGTB", "$rd, $rs, $rt"},
	{IT_reg, "PCGTH", "$rd, $rs, $rt"},
	{IT_reg, "PCGTW", "$rd, $rs, $rt"},
	{IT_reg, "PCPYH", "$rd, $rs, $rt"},
	{IT_reg, "PCPYLD", "$rd, $rs, $rt"},
	{IT_reg, "PCPYUD", "$rd, $rs, $rt"},
	{IT_reg, "PDIVBW", "$rs, $rt"},
	{IT_reg, "PDIVUW", "$rs, $rt"},
	{IT_reg, "PDIVW", "$rs, $rt"},
	{IT_reg, "PEXCH", "$rd, $rt"},
	{IT_reg, "PEXCW", "$rd, $rt"},
	{IT_reg, "PEXEH", "$rd, $rt"},
	{IT_reg, "PEXEW", "$rd, $rt"},
	{IT_reg, "PEXT5", "$rd, $rt"},
	{IT_reg, "PEXTLB", "$rd, $rs, $rt"},
	{IT_reg, "PEXTLH", "$rd, $rs, $rt"},
	{IT_reg, "PEXTLW", "$rd, $rs, $rt"},
	{IT_reg, "PEXTUB", "$rd, $rs, $rt"},
	{IT_reg, "PEXTUH", "$rd, $rs, $rt"},
	{IT_reg, "PEXTUW", "$rd, $rs, $rt"},
	{IT_reg, "PHMADH", "$rd, $rs, $rt"},
	{IT_reg, "PHMSBH", "$rd, $rs, $rt"},
	{IT_reg, "PINTEH", "$rd, $rs, $rt"},
	{IT_reg, "PINTH", "$rd, $rs, $rt"},
	{IT_reg, "PLZCW", "$rd, $rs"},
	{IT_reg, "PMADDH", "$rd, $rs, $rt"},
	{IT_reg, "PMADDUW", "$rd, $rs, $rt"},
	{IT_reg, "PMADDW", "$rd, $rs, $rt"},
	{IT_reg, "PMAXH", "$rd, $rs, $rt"},
	{IT_reg, "PMAXW", "$rd, $rs, $rt"},
	{IT_reg, "PMFHI", "$rd"},
	{IT_reg, "PMFHL.LW", "$rd"},
	{IT_reg, "PMFHL.UW", "$rd"},
	{IT_reg, "PMFHL.SLW", "$rd"},
	{IT_reg, "PMFHL.LH", "$rd"},
	{IT_reg, "PMFHL.SH", "$rd"},
	{IT_reg, "PMFLO", "$rd"},
	{IT_reg, "PMINH", "$rd, $rs, $rt"},
	{IT_reg, "PMINW", "$rd, $rs, $rt"},
	{IT_reg, "PMSUBH", "$rd, $rs, $rt"},
	{IT_reg, "PMSUBW", "$rd, $rs, $rt"},
	{IT_reg, "PMTHI", "$rs"},
	{IT_reg, "PMTHL.LW", "$rs"},
	{IT_reg, "PMTLO", "$rd, $rs, $rt"},
	{IT_reg, "PMULTH", "$rd, $rs, $rt"},
	{IT_reg, "PMULTUW", "$rd, $rs, $rt"},
	{IT_reg, "PMULTW", "$rd, $rs, $rt"},
	{IT_reg, "PNOR", "$rd, $rs, $rt"},
	{IT_reg, "POR", "$rd, $rs, $rt"},
	{IT_reg, "PPAC5", "$rd, $rt"},
	{IT_reg, "PPACB", "$rd, $rs, $rt"},
	{IT_reg, "PPACH", "$rd, $rs, $rt"},
	{IT_reg, "PPACW", "$rd, $rs, $rt"},
	{IT_reg, "PREVH", "$rd, $rt"},
	{IT_reg, "PROT3W", "$rd, $rt"},
	{IT_reg, "PSLLH", "$rs, $rt, %sa"},
	{IT_reg, "PSLLVW", "$rd, $rs, $rt"},
	{IT_reg, "PSLLW", "$rs, $rt, %sa"},
	{IT_reg, "PSRAH", "$rs, $rt, %sa"},
	{IT_reg, "PSRAVW", "$rd, $rs, $rt"},
	{IT_reg, "PSRAW", "$rs, $rt, %sa"},
	{IT_reg, "PSRLH", "$rs, $rt, %sa"},
	{IT_reg, "PSRLVW", "$rd, $rs, $rt"},
	{IT_reg, "PSRLW", "$rs, $rt, %sa"},
	{IT_reg, "PSUBB", "$rd, $rs, $rt"},
	{IT_reg, "PSUBH", "$rd, $rs, $rt"},
	{IT_reg, "PSUBSB", "$rd, $rs, $rt"},
	{IT_reg, "PSUBSH", "$rd, $rs, $rt"},
	{IT_reg, "PSUBSW", "$rd, $rs, $rt"},
	{IT_reg, "PSUBUB", "$rd, $rs, $rt"},
	{IT_reg, "PSUBUH", "$rd, $rs, $rt"},
	{IT_reg, "PSUBUW", "$rd, $rs, $rt"},
	{IT_reg, "PSUBW", "$rd, $rs, $rt"},
	{IT_reg, "PXOR", "$rd, $rs, $rt"},
	{IT_reg, "QFSRV", "$rd, $rt"},
	{IT_imm, "SQ", "$rt, #im($rs)"},
	{IT_imma, "BC0F", "%im"},
	{IT_imma, "BC0FL", "%im"},
	{IT_imma, "BC0T", "%im"},
	{IT_imma, "BC0TL", "%im"},
	{IT_imma, "CACHE", "$rt, %im($rs)"},
	{IT_reg, "DI", "_"},
	{IT_reg, "EI", "_"},
	{IT_reg, "ERET", "_"},
	{IT_reg, "MFBPC", "$rt"},
	{IT_reg, "MFC0", "$rt, ^rd"},
	{IT_reg, "MFDAB", "$rt"},
	{IT_reg, "MFDABM", "$rt"},
	{IT_reg, "MFDVB", "$rt"},
	{IT_reg, "MFDVBM", "$rt"},
	{IT_reg, "MFIAB", "$rt"},
	{IT_reg, "MFIABM", "$rt"},
	{IT_reg, "MFPC", "$rt"},
	{IT_reg, "MFPS", "$rt"},
	{IT_reg, "MTBPC", "$rt"},
	{IT_reg, "MTC0", "$rt, ^rd"},
	{IT_reg, "MTDAB", "$rt"},
	{IT_reg, "MTDABM", "$rt"},
	{IT_reg, "MTDVB", "$rt"},
	{IT_reg, "MTDVBM", "$rt"},
	{IT_reg, "MTIAB", "$rt"},
	{IT_reg, "MTIABM", "$rt"},
	{IT_reg, "MTPC", "$rt"},
	{IT_reg, "MTPS", "$rt"},
	{IT_reg, "TLBP", "_"},
	{IT_reg, "TLBR", "_"},
	{IT_reg, "TLBWI", "_"},
	{IT_reg, "TLBWR", "_"},
	{IT_regf, "ABS.S", "@fd, @fs"},
	{IT_regf, "ABS.D", "@fd, @fs"},
	{IT_regf, "ADD.S", "@fd, @fs, @ft"},
	{IT_regf, "ADD.D", "@fd, @fs, @ft"},
	{IT_imma, "BC1F", "&im"},
	{IT_imma, "BC1T", "&im"},
	{IT_regf, "C.F.S", "@fs, @ft"},
	{IT_regf, "C.F.D", "@fs, @ft"},
	{IT_regf, "C.UN.S", "@fs, @ft"},
	{IT_regf, "C.UN.D", "@fs, @ft"},
	{IT_regf, "C.EQ.S", "@fs, @ft"},
	{IT_regf, "C.EQ.D", "@fs, @ft"},
	{IT_regf, "C.UEQ.S", "@fs, @ft"},
	{IT_regf, "C.UEQ.D", "@fs, @ft"},
	{IT_regf, "C.OLT.S", "@fs, @ft"},
	{IT_regf, "C.OLT.D", "@fs, @ft"},
	{IT_regf, "C.ULT.S", "@fs, @ft"},
	{IT_regf, "C.ULT.D", "@fs, @ft"},
	{IT_regf, "C.OLE.S", "@fs, @ft"},
	{IT_regf, "C.OLE.D", "@fs, @ft"},
	{IT_regf, "C.ULE.S", "@fs, @ft"},
	{IT_regf, "C.ULE.D", "@fs, @ft"},
	{IT_regf, "C.SF.S", "@fs, @ft"},
	{IT_regf, "C.SF.D", "@fs, @ft"},
	{IT_regf, "C.NGLE.S", "@fs, @ft"},
	{IT_regf, "C.NGLE.D", "@fs, @ft"},
	{IT_regf, "C.SEQ.S", "@fs, @ft"},
	{IT_regf, "C.SEQ.D", "@fs, @ft"},
	{IT_regf, "C.NGL.S", "@fs, @ft"},
	{IT_regf, "C.NGL.D", "@fs, @ft"},
	{IT_regf, "C.LT.S", "@fs, @ft"},
	{IT_regf, "C.LT.D", "@fs, @ft"},
	{IT_regf, "C.NGE.S", "@fs, @ft"},
	{IT_regf, "C.NGE.D", "@fs, @ft"},
	{IT_regf, "C.LE.S", "@fs, @ft"},
	{IT_regf, "C.LE.D", "@fs, @ft"},
	{IT_regf, "C.NGT.S", "@fs, @ft"},
	{IT_regf, "C.NGT.D", "@fs, @ft"},
	{IT_regf, "CEIL.L.S", "@fd, @fs"},
	{IT_regf, "CEIL.L.D", "@fd, @fs"},
	{IT_regf, "CEIL.W.S", "@fd, @fs"},
	{IT_regf, "CEIL.W.D", "@fd, @fs"},
	{IT_reg, "CFC1", "$rt, ^rd"},
	{IT_reg, "CTC1", "$rt, ^rd"},
	{IT_regf, "CVT.D.S", "@fd, @fs"},
	{IT_regf, "CVT.D.W", "@fd, @fs"},
	{IT_regf, "CVT.D.L", "@fd, @fs"},
	{IT_regf, "CVT.L.D", "@fd, @fs"},
	{IT_regf, "CVT.L.S", "@fd, @fs"},
	{IT_regf, "CVT.S.D", "@fd, @fs"},
	{IT_regf, "CVT.S.W", "@fd, @fs"},
	{IT_regf, "CVT.S.L", "@fd, @fs"},
	{IT_regf, "CVT.W.S", "@fd, @fs"},
	{IT_regf, "CVT.W.D", "@fd, @fs"},
	{IT_regf, "DIV.S", "@fd, @fs, @ft"},
	{IT_regf, "DIV.D", "@fd, @fs, @ft"},
	{IT_reg, "DMFC1", "$rt, @rd"},
	{IT_reg, "DMTC1", "$rt, @rd"},
	{IT_regf, "FLOOR.L.S", "@fd, @fs"},
	{IT_regf, "FLOOR.L.D", "@fd, @fs"},
	{IT_regf, "FLOOR.W.S", "@fd, @fs"},
	{IT_regf, "FLOOR.W.D", "@fd, @fs"},
	{IT_imm, "LDC1", "@rt, #im($rs)"},
	{IT_imm, "LWC1", "@rt, #im($rs)"},
	{IT_reg, "MFC1", "$rt, @rd"},
	{IT_regf, "MOV.S", "@fd, @fs"},
	{IT_regf, "MOV.D", "@fd, @fs"},
	{IT_reg, "MTC1", "$rt, @rd"},
	{IT_regf, "MUL.S", "@fd, @fs, @ft"},
	{IT_regf, "MUL.D", "@fd, @fs, @ft"},
	{IT_regf, "NEG.S", "@fd, @fs"},
	{IT_regf, "NEG.D", "@fd, @fs"},
	{IT_regf, "ROUND.L.S", "@fd, @fs"},
	{IT_regf, "ROUND.L.D", "@fd, @fs"},
	{IT_regf, "ROUND.W.S", "@fd, @fs"},
	{IT_regf, "ROUND.W.D", "@fd, @fs"},
	{IT_imm, "SDC1", "@rt, #im($rs)"},
	{IT_regf, "SQRT.S", "@fd, @fs"},
	{IT_regf, "SQRT.D", "@fd, @fs"},
	{IT_regf, "SUB.S", "@fd, @fs, @ft"},
	{IT_regf, "SUB.D", "@fd, @fs, @ft"},
	{IT_imm, "SWC1", "@rt, #im($rs)"},
	{IT_regf, "TRUNC.L.S", "@fd, @fs"},
	{IT_regf, "TRUNC.L.D", "@fd, @fs"},
	{IT_regf, "TRUNC.W.S", "@fd, @fs"},
	{IT_regf, "TRUNC.W.D", "@fd, @fs"},
	{IT_nop, "NOP", "_"},  // Alias of SLL zero, zero, 0
	{IT_regf, "ADDA.S", "@fd, @fs, @ft"},
	//{IT_nop, "INVALID", "_"}, // special mark for invalid op
};
