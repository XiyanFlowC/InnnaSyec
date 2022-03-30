#ifndef INS_DEF_H
#define INS_DEF_H

#pragma once

enum instr_type {
	IT_reg,
	IT_imm,
	IT_imma,
	IT_immu,
	IT_code,
	IT_jump,
	IT_regf,
	IT_nop
};

extern struct instr_def{
	int type;
	char name[32];
	char para[128];
} instructions[];

#endif
