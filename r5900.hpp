#pragma once

#ifndef __XY_DISASM_HXX_
#define __XY_DISASM_HXX_

#include <limits.h>

#include "mips5900_def.inc"
#define INVALID -1

#ifndef _WIN32
char *strupr(char *str);
#endif

// 获取一个以end_ch结尾的项到dst
// 返回值：成功获取的字符个数
int get_term(char *dst, const char *src, const char end_ch);

int count_term(const char *src, const char end_ch);

// 解析整型
// 允许0x、0前缀表示的16进制及8进制解析。允许负号前缀。
// 返回成功处理的字符个数，数字格式错误返回-1
int parse_int(int *result, const char *src);

extern const char* insts_name[];

extern const char* insts_tmpl[];

extern const int insts_cnt;

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

// 打印
int printdis(char* _buf, instr_t _ins);

// 通过指令名获取指令id
// 返回值：指令id，失败返回-1
int inst_id_bynm(char *name);

// 解析字符形式汇编指令（仅真指令，伪指令不可处理）
// 返回值：处理成功的字符数，失败时：
// -1 指令名未知
// -2 寄存器名未知
// -3 格式解析错误
int parse_asm(const char* _buf, instr_t *_ins);

// 解析字符形式汇编参数
// 返回值：处理成功的字符数，失败时：
// -1 指令名未知
// -2 寄存器名未知
// -3 格式解析错误
int parse_param(const char *_buf, const char *src, instr_t *_ins);

#endif
