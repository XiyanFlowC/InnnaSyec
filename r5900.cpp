#include <cstdio>
#include "r5900.hpp"
#include <cstring>

#ifndef _WIN32
char *strupr(char *str)
{
	char *ret = str;
	while(*str != '\0')
	{
		if(*str >= 'a' && *str <= 'z')
		{
			*str = *str - 'a' + 'A';
		}
		++str;
	}
	return ret;
}
#endif

const char* insts_name[] = {
#include "mips5900_refl.inc"
};

const char* insts_tmpl[] = {
#include "mips5900_tmpl.inc"
};

const int insts_cnt = (int)(sizeof(insts_name)/sizeof(const char *));


const char* gpr_name[] = {
	"zero", "at", "v0", "v1", "a0", "a1", "a2", "a3",
	"t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
	"s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
	"t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra",
	"pc", "hi", "lo",
};

int dechex(const char ch)
{
	if(ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
	if(ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
	if(ch >= '0' && ch <= '9') return ch - '0';
	return -1;
}

int parse_int(int *result, const char *src)
{
	int i = 0, m = 1;
	if(*src == '-')
	{
		++src;
		m = -1;
		++i;
	}
	*result = 0;
	if(*src == '0')
	{
		if(src[1] == 'x') // hex
		{
			src += 2, i += 2;
			int tmp;
			while(-1 != (tmp = dechex(*src++)))
			{
				++i;
				*result = (*result << 4) + tmp;
			}
			*result *= m;
			return i;
		}
		if(src[1] >= '0' && src[1] <= '7') // oct
		{
			src ++;
			i++;
			while(*src <= '7' && *src >= '0')
			{
				*result = (*result << 3) + *src - '0';
				++i;
			}
			*result *= m;
			return i;
		}
		return 1; // 十进制的单个‘0’
	}
	else if (*src >= '0' && *src <= '9')
	{
		while(*src >= '0' && *src <= '9')
		{
			*result = *result * 10 + *src++ - '0';
			++i;
		}
		*result *= m;
		return i;
	}
	return -1;
}

// 获取一个由end_ch终止的项，end_ch本身不含于其中。
int get_term(char *dst, const char *src, const char end_ch)
{
	int i = 0;
	while(src[i] != '\0' && src[i] != end_ch)
	{
		dst[i] = src[i];
		++i;
	}
	dst[i] = '\0';
	return i;
}

// 计算给定要求的项长度
int count_term(const char *src, const char end_ch)
{
	int i = 0;
	while(src[i] != '\0' && src[i] != end_ch)
	{
		++i;
	}
	return i;
}

int get_gprid(const char *name)
{
	for(int i = 0; i < (int)(sizeof(gpr_name)/sizeof(const char *)); ++i)
	{
		if(0 == strcmp(name, gpr_name[i]))
		{
			return i;
		}
	}
	return -1;
}

unsigned long long gpr[34];
unsigned long long gpr_hi[34];

// stubs: to emulate, realize these subroutines
unsigned char lb(unsigned addr)
{
	return 0;
}

unsigned short lh(unsigned addr)
{
	return 0;
}

unsigned long lw(unsigned addr)
{
	return 0;
}

unsigned long long ld(unsigned addr)
{
	return 0;
}

void sb(unsigned addr, unsigned data)
{}

void sh(unsigned addr, unsigned data)
{}

void sw(unsigned addr, unsigned data)
{}

void sd(unsigned addr, unsigned data)
{}

instr_t disasm(unsigned ins)
{
	instr_t ans;
	unsigned long long tmp;
	#include "mips5900_disa.inc"
	ans.opcode = INVALID;
	return ans;
}

unsigned asmble(instr_t ins)
{
	unsigned ans = 0;
	unsigned long long tmp;
	unsigned long long imm = ins.imm;
	unsigned rs = ins.rs, rt = ins.rt, rd = ins.rd;
	switch(ins.opcode)
	{
		#include "mips5900_asmb.inc"
	}
	return ans;
}

int exec_ins(instr_t ins)
{
	int rs = ins.rs;
	int rt = ins.rt;
	int rd = ins.rd;
	int imm = ins.imm;
	long long ans;
	unsigned long long uans;
	switch(ins.opcode)
	{
		#include "mips5900_exec.inc"
		default:
			return UKNOP;
	}
	gpr[pc] += 4;
	return 0;
}

struct instr_t delay_slot;

void nullify_delay_slot()
{
	delay_slot.opcode = NOP;
}

int printdis(char *_buf, instr_t _ins)
{
	if(_ins.opcode == INVALID)
	{
		strcpy(_buf, "INVALID");
		return 7;
	}
	const char* tmpl = insts_tmpl[_ins.opcode];
	int i = 0, j = 0;
	while(tmpl[i] != '\0')
	{
		if(tmpl[i] == '#')
		{
			if(_ins.imm >= 0)
				j += sprintf(_buf + j, "0x%X", _ins.imm);
			else
				j += sprintf(_buf + j, "-0x%X", -_ins.imm);
		}
		else if(tmpl[i] == '$')
		{
			++i;
			if(tmpl[i] == 's')
				j += sprintf(_buf + j, "%s", gpr_name[_ins.rs]);
			if(tmpl[i] == 't')
				j += sprintf(_buf + j, "%s", gpr_name[_ins.rt]);
			if(tmpl[i] == 'd')
				j += sprintf(_buf + j, "%s", gpr_name[_ins.rd]);
		}
		else if(tmpl[i] == '&')
		{
			if(_ins.imm >= 0)
				j += sprintf(_buf + j, "0x%X", _ins.imm << 2);
			else
				j += sprintf(_buf + j, "-0x%X", -_ins.imm << 2);
		}
		else
		{
			_buf[j++] = tmpl[i];
		}
		++i;
	}
	_buf[j] = '\0';
	return j;
}

int inst_id_bynm(char *name)
{
	strupr(name);
	for(int i = 0; i < (int)(sizeof(insts_name)/sizeof(const char *)); ++i)
	{
		if(0 == strcmp(name, insts_name[i]))
		{
			return i;
		}
	}
	return -1;
}

int parse_asm(const char* _buf, instr_t *_ins)
{
	char buf[128];
	sscanf(_buf, "%s", buf);
	
	int inst_id = inst_id_bynm(buf);
	if(inst_id == -1) return -1;
	_ins->opcode = inst_id;

	int q = strlen(buf);

	int ret = parse_param(_buf + q, insts_tmpl[inst_id] + q, _ins);
	if(ret < 0) return ret;

	return q + ret;
}

int parse_param(const char *_buf, const char *src, instr_t *_ins)
{
	char buf[128];
	int p, q = 0; // 模板、缓冲区游标。
	p = q;
	while(src[p] != '\0')
	{
		if(src[p] == ' ') // blank chars
		{
			while(_buf[q] == ' ' || _buf[q] == '\t')
			{
				++q;
			}
			++p;
			continue;
		}
		if(src[p] == '$')
		{
			++p;
			q += get_term(buf, _buf + q, src[p + 1]); // 获取指定的项。
			int reg_id = get_gprid(buf);
			if(reg_id == -1) return -2;
			switch(src[p])
			{
				case 's':
				_ins->rs = reg_id;
				break;
				case 'd':
				_ins->rd = reg_id;
				break;
				case 't':
				_ins->rt = reg_id;
				break;
			}
			++p;
			continue;
		}
		if(src[p] == '#' || src[p] == '&')
		{
			// get_term(buf, _buf + q, src[p + 1]);
			int ans;
			q += parse_int(&ans, _buf + q);
			src[p] == '&' ? _ins->imm = ans >> 2 : _ins->imm = ans;
			++p;
			continue;
		}
		if(src[p] != _buf[q]) {
			return -3;
		}
		++p;
		++q;
	}
	return q;
}