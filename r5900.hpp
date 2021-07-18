#pragma once

#ifndef __XY_DISASM_HXX_
#define __XY_DISASM_HXX_

#include <limits.h>

#include "mips5900_def.inc"
#define INVALID -1

extern const char* insts_name[];

extern const char* insts_tmpl[];

extern const char* gpr_name[]; // mips r5900 regs

enum gpr_nm {
	zero, at, v0, v1, a0, a1, a2, a3, t0,
	t1, t2, t3, t4, t5, t6, t7, s0,
	s1, s2, s3, s4, s5, s6, s7, t8,
	t9, k0, k1, gp, sp, fp, ra, pc,
	hi, lo
};

extern unsigned long long gpr[34];
extern unsigned long long gpr_hi[34];

struct instr_t {
	int opcode;
	int rs, rt, rd;
	int imm;
};

extern struct instr_t delay_slot;

void nullify_delay_slot();

instr_t disasm(unsigned ins);

unsigned asmble(instr_t ins);

#define SIG_INTOVERFLOW 0x1
#define SIG_BREAK       0xE5000000
#define UKNOP           0xFC520000

int exec_ins(instr_t ins);

int printdis(char* _buf, instr_t _ins);

int parse_asm(const char* _buf, instr_t *_ins);

#endif
