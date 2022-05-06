#ifndef _CODEC_H_
#define _CODEC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include "opnm.h"
// #define IT_nop  0
// #define IT_reg  16
// #define IT_jump 32
// #define IT_imm  64
// #define IT_code 128
// #define IT_immu 65
// #define IT_imma 66
// #define IT_regf 17
#include "ins_def.h"

	/* Structures */
	struct instr_t
	{
		unsigned int opcode;
		unsigned int rs;
		unsigned int rt;
		unsigned int rd;
		unsigned int sa;
		unsigned long long imm;
	};

#ifdef LE
	struct instr_base
	{
		unsigned int _data : 26;
		unsigned int opcode : 6;
	};

	struct instr_j
	{
		unsigned int addr : 26;
		unsigned int opcode : 6;
	};

	struct instr_r
	{
		unsigned int funct : 6;
		unsigned int sa : 5;
		unsigned int rd : 5;
		unsigned int rt : 5;
		unsigned int rs : 5;
		unsigned int opcode : 6;
	};

	struct instr_d
	{
		unsigned int funct : 6;
		unsigned int data : 20;
		unsigned int opcode : 6;
	};

	struct instr_i
	{
		unsigned int imm : 16;
		unsigned int rt : 5;
		unsigned int rs : 5;
		unsigned int opcode : 6;
	};
#else
	struct instr_base
	{
		unsigned int opcode : 6;
		unsigned int _data : 26;
	};

	struct instr_j
	{
		unsigned int opcode : 6;
		unsigned int addr : 6;
	};

	struct instr_r
	{
		unsigned int opcode : 6;
		unsigned int rs : 5;
		unsigned int rt : 5;
		unsigned int rd : 5;
		unsigned int sa : 5;
		unsigned int funct : 6;
	};

	struct instr_d
	{
		unsigned int opcode : 6;
		unsigned int data : 20;
		unsigned int funct : 6;
	};

	struct instr_i
	{
		unsigned int opcode : 6;
		unsigned int rs : 5;
		unsigned int rt : 5;
		unsigned int imm : 16;
	};
#endif
	union uinstr
	{
		unsigned int code;
		struct instr_base base;
		struct instr_i itype;
		struct instr_j jtype;
		struct instr_r rtype;
		struct instr_d dtype;
	};

	static const char *gpr_names[] = {
		"zero", "at", "v0", "v1", "a0", "a1", "a2", "a3",
		"t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
		"s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
		"t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra",
	};

	static const char *cop0r_names[] = {
		"Index", "Random", "EntryLo0", "EntryLo1", "Context", "PageMask",
		"Wired", "Reserved0", "BadVAddr", "Count", "EntryHi", "Compare",
		"Status", "Cause", "EPC", "PRId", "Config", "COP0R17", "COP0R18", "COP0R19",
		"COP0R20", "COP0R21", "COP0R22", "COP0R23", "Debug", "Perf",
		"COP0R26", "COP0R27", "TagLo", "TagHi", "ErrorEPC", "COP0R31",
	};

	static const char *cop1r_names[] = {
		"f00", "f01", "f02", "f03", "f04", "f05", "f06", "f07",
		"f08", "f09", "f10", "f11", "f12", "f13", "f14", "f15",
		"f16", "f17", "f18", "f19", "f20", "f21", "f22", "f23",
		"f24", "f25", "f26", "F27", "f28", "f29", "f30", "f31",
	};

	/* Functions */
	int LookUpOpcode(union uinstr instr);

	unsigned int GetTemplateByIndex(int opcode);

	void PrepareOpcodeBuffer();

	struct instr_t DecodeInstruction(unsigned int instruction);

	unsigned int EncodeInstruction(struct instr_t instruction);

	struct instr_def GetInstructionDefinitionByIndex(int option);

	struct instr_def GetInstructionDefinitionByName(const char* name);

#define OPTION_COUNT 333

#ifdef __cplusplus
}
#endif

#endif