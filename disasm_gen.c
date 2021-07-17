#include <stdio.h>
#include "liteopt.h"

FILE *input = NULL, *cout = NULL, *hout = NULL;
char *input_name, *cout_name, *hout_name;
#define GEN_MODE_BE 0
#define GEN_MODE_LE 1
int mode = GEN_MODE_LE;

int show_help(char *pad)
{
    puts("Disasm generator Ver.1.0 Coded by Xiyan");
    puts("-? --help               show this help.");
    puts("-i --input  [input.txt] set input file name");
    puts("-o --output [outname]   output name, including *.c *.h");
    puts("   --header [out.h]     output header filename");
    puts("   --source [out.c]     output source filename");
    puts("-b --be                 set gen mode to BE machine");
    puts("-l --le                 set gen mode to LE machine(default)");
    puts("");
    return 0;
}

int handle_input(char *input_str)
{
	input_name = strdup(input_str);
    input = fopen(input_str, "rb");
}

int handle_output(char *output_str)
{
    char buf[256];
    sprintf(buf, "%s.c", output_str);
    cout_name = strdup(buf);
    cout = fopen(buf, "wb");
    sprintf(buf, "%s.h", output_str);
    hout_name = strdup(buf);
    hout = fopen(buf, "wb");
    return 0;
}

int handle_be(char *pad)
{
    mode = GEN_MODE_BE;
    return 0;
}

int handle_le(char *pad)
{
    mode = GEN_MODE_LE;
    return 0;
}

__reged_opt = {
    {'?', "help", LOPT_FLG_CH_VLD|LOPT_FLG_OPT_VLD|LOPT_FLG_STR_VLD, show_help},
    {'i', "input", LOPT_FLG_CH_VLD|LOPT_FLG_OPT_VLD|LOPT_FLG_STR_VLD, handle_input},
    {'o', "output", LOPT_FLG_CH_VLD|LOPT_FLG_OPT_VLD|LOPT_FLG_STR_VLD, handle_output},
    {'b', "be", LOPT_FLG_CH_VLD|LOPT_FLG_OPT_VLD|LOPT_FLG_STR_VLD, handle_be},
    {'l', "le", LOPT_FLG_CH_VLD|LOPT_FLG_OPT_VLD|LOPT_FLG_STR_VLD, handle_le}
};

char *get_name(FILE* stream)
{
	static char name_buf[256];
	int idx = 0;
	int rst = fgetchar(stream);
	if(rst != EOF) {
		if(rst >= 'a' && rst <= 'z' || rst <= 'A' && rst >= 'Z' || rst <= '9' && rst >= '0')
			name_buf[idx++] = (char)rst;
	}
	fseek(stream, -1, SEEK_CUR);
	name_buf[idx] = '\0';
	return name_buf;
}

/* register generation */
int cpu_bits;
char* regs[128];
int regc = 0;

void new_reg(char* name);
void mk_regs();

/* instrucment generation */
void def_ins(FILE* stream);
void def_fak_ins(FILE* stream);

int main(int argc, char const *argv[])
{
	int i, ch, stage = 0;
	char buffer[128];
    int rst = lopt_parse(argc, argv);
    if(rst != 0)
    {
        switch (rst)
        {
        case -1:
        case -2:
            puts("Argument parsing error, unknown switch. Use '-?' for help.");
            exit(-1);
            break;
        case -3:
            puts("Unknown error occured.");
        }
    }
    fputs(hout, "#ifndef __DISASMGEN__AUTO_H\n#define __DISASMGEN__AUTO_H\n\n#define<stdio.h>\n\n");
    fputs(hout, "struct instr_t {int opcode; int rs, rt, rd; int imm; long long typ;};");
    fputs(hout, "struct instr_t get_opcode(unsigned instr);\n");
    
    sprintf(buffer, "#include<%s>\n\n", hout_name);
    fputs(cout, buffer);

    while(EOF != (ch = fgetcher(input)))
	{
		char *name;
		switch(stage)
		{
			case 0: /* idle mode */
				if(ch == '#')
				{
					stage = 1; /* remark mode */
					break;
				}
				if(ch == '%')
				{
					stage = 2; /* basic definition area */
				}
				break;
			case 1: /* remark mode */
				if(ch == '.') /* termination */
				{
					stage = 0;
					break;
				}
				break;
			case 2: /* basic definition area */
				if(ch == '#')
				{
					stage = 3; /* basic definition area remark */
					break;
				}
				name = get_name(input);
				if(strcmp("regs", name) == 0)
				{
					scanf("%d", &cpu_bits);
					stage = 4; /* register definition */
					break;
				}
				if(strcmp("instr", name) == 0)
				{
					fputs(cout, "struct instr_t get_opcode(unsigned instr) {\n");
					stage = 5; /* instrucments difinition */
					break;
				}
				/*if(strcmp("struct", name) == 0)
				{
					stage = 6;
					break;
				}*/
				break;
			case 3:
				if(ch == '.')
				{
					stage = 2;
					break;
				}
				break;
			case 4: /* register definition */
				if(ch == '#')
				{
					do
					{
						ch = fgetch(input);
					} while(ch != '.');
					break;
				}
				fscanf("%s", name);
				if(type == -1)
				{
					mk_regs();
					stage = 0;
					break;
				}
				new_reg(name);
				break;
			case 5:
				if(ch == '#')
				{
					do
					{
						ch = fgetch(input);
					} while(ch != '.');
					break;
				}
				if(ch == '*')
				{
					def_ins(input);
					break;
				}
				if(ch == '$')
				{
					def_fak_ins(input);
					break;
				}
				if(ch == '%')
				{
					fputs(cout, "}\n\n");
					stage = 0;
					break;
				}
				break;
		}
	}
	fputs(hout, "\n\n#endif\n"); 
    
    return 0;
}

void new_reg(char* name)
{
	regs[regc++] = strdup(name);
}

void mk_regs()
{
	int i;
	fprintf(hout, "char* reg_name = {\n");
	for(i = 0; i < regc; ++i)
	{
		fprintf(hout, "\"%s\", ", regs[i]);
	}
	fputs(hout, "\n};\n");
	switch(cpu_bits) {
		case 8:
			fputs(hout, "unsigned char ");
			break;
		case 16:
			fputs(hout, "unsigned short ");
			break;
		case 32:
			fputs(hout, "unsigned int ");
			break;
		case 64:
			fputs(hout, "unsigned long long ");
			break;
		case 128:
			fputs(hout, "__type_128 ");
			break;
	}
	for(i = 0; i < regc - 1; ++i)
	{
		fprintf(hout, "%s, ", regs[i]);
	}
	fprintf(hout, "%s;\n", regs[regc - 1]);
	for(i = 0; i < regc; ++i)
	{
		free(regs[i]);
	}
	regc = 0;
}

int inst_id = 0;

void def_ins(FILE* stream)
{
	char name[128];
	int fldc, i;
	int fldl[16];
	fscanf(stream, "%s", &name);
	fprintf(hout, "#define %s %d\n", name, inst_id++);
	fscanf(stream, "%d", &fldc);
	for(i = 0; i < fldc; ++i)
	{
		char condi;
		int sta, end;
		fscanf(stream, "%c,%d,%d ", &condi, &sta, &end);
		if(condi == 'c')
		{
		}
		else if(condi == 'i')
		{
		} 
	}
}


void def_fak_ins(FILE* stream);

