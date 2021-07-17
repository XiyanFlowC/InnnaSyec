#include <stdio.h>
#include "liteopt.h"
#include "simperror.hpp"

FILE *input = NULL, *dout = NULL, *nout = NULL, *xout = NULL, *aout = NULL, *tout = NULL, *sout = NULL;
char *input_name, *dout_name, *nout_name, *xout_name, *aout_name, *tout_name, *sout_name;
#define GEN_MODE_BE 0
#define GEN_MODE_LE 1
int mode = GEN_MODE_LE;

int show_help(const char *pad)
{
	puts("Disasm generator Ver.1.0 Coded by Xiyan");
	puts("-? --help                 show this help.");
	puts("-i --input    [input.txt] set input file name");
	puts("-o --output   [outname]   output name, named as \"outname_xxx.inc\"");
	puts("-a --disasm   [disa.inc]  disasm pat routine output file name");
	puts("-s --asm      [asm.inc]   asm pat routine output file name");
	puts("-d --def      [def.inc]   macro definition output file name");
	puts("-n --ref      [ref.inc]   reflection output file name");
	puts("-x --exec     [exec.inc]  execution output file name");
	puts("-t --template [tmpl.inc]  output template output file name");
	puts("-b --be                   set gen mode to BE machine");
	puts("-l --le                   set gen mode to LE machine(default)");
	puts("");
	return 0;
}

int handle_d(const char *filename)
{
	dout_name = strdup(filename);
	dout = fopen(filename, "w");
	return 0;
}

int handle_n(const char *filename)
{
	nout_name = strdup(filename);
	nout = fopen(filename, "w");
	return 0;
}

int handle_x(const char *filename)
{
	xout_name = strdup(filename);
	xout = fopen(filename, "w");
	return 0;
}

int handle_a(const char *filename)
{
	aout_name = strdup(filename);
	aout = fopen(filename, "w");
	return 0;
}

int handle_t(const char *filename)
{
	tout_name = strdup(filename);
	tout = fopen(filename, "w");
	return 0;
}

int handle_s(const char *filename)
{
	sout_name = strdup(filename);
	sout = fopen(filename, "w");
	return 0;
}

int handle_input(const char *input_str)
{
	input_name = strdup(input_str);
	input = fopen(input_str, "r");
	return 0;
}

int handle_output(const char *output_str)
{
	char buf[256];
	sprintf(buf, "%s_def.inc", output_str);
	dout_name = strdup(buf);
	dout = fopen(buf, "w");
	sprintf(buf, "%s_refl.inc", output_str);
	nout_name = strdup(buf);
	nout = fopen(buf, "w");
	sprintf(buf, "%s_exec.inc", output_str);
	xout_name = strdup(buf);
	xout = fopen(buf, "w");
	sprintf(buf, "%s_disa.inc", output_str);
	aout_name = strdup(buf);
	aout = fopen(buf, "w");
	sprintf(buf, "%s_tmpl.inc", output_str);
	tout_name = strdup(buf);
	tout = fopen(buf, "w");
	sprintf(buf, "%s_asmb.inc", output_str);
	sout_name = strdup(buf);
	sout = fopen(buf, "w");
	return 0;
}

int handle_be(const char *pad)
{
	mode = GEN_MODE_BE;
	return 0;
}

int handle_le(const char *pad)
{
	mode = GEN_MODE_LE;
	return 0;
}

int readbin(const char *str)
{
	int i = 0;
	int ret = 0;
	while (str[i] != '\0')
	{
		if (str[i] == '1')
			ret = (ret << 1) | 1;
		else if (str[i] == '0')
			ret <<= 1;
		else
			error(0x02000000);
		++i;
	}
	return ret;
}

int readoct(const char *str)
{
	int i = 0;
	int ret = 0;
	while (str[i] != '\0')
	{
		if (str[i] > '7' || str[i] < '0')
			error(0x02000001);
		ret = (ret << 3) + (str[i++] - '0');
	}
	return ret;
}

char *get_name(FILE *stream)
{
	static char name_buf[256];
	int idx = 0;
	int rst = fgetc(stream);
	if (rst != EOF)
	{
		if (rst >= 'a' && rst <= 'z' || rst <= 'A' && rst >= 'Z' || rst <= '9' && rst >= '0')
			name_buf[idx++] = (char)rst;
	}
	fseek(stream, -1, SEEK_CUR);
	name_buf[idx] = '\0';
	return name_buf;
}

int main(int argc, char const *argv[])
{
	lopt_regopt("help", '?', LOPT_FLG_CH_VLD | LOPT_FLG_OPT_VLD | LOPT_FLG_STR_VLD, show_help);
	lopt_regopt("input", 'i', LOPT_FLG_CH_VLD | LOPT_FLG_OPT_VLD | LOPT_FLG_STR_VLD, handle_input);
	lopt_regopt("output", 'o', LOPT_FLG_CH_VLD | LOPT_FLG_OPT_VLD | LOPT_FLG_STR_VLD, handle_output);
	lopt_regopt("be", 'b', LOPT_FLG_CH_VLD | LOPT_FLG_OPT_VLD | LOPT_FLG_STR_VLD, handle_be);
	lopt_regopt("le", 'l', LOPT_FLG_CH_VLD | LOPT_FLG_OPT_VLD | LOPT_FLG_STR_VLD, handle_le);
	lopt_regopt("disasm", 'a', LOPT_FLG_CH_VLD | LOPT_FLG_OPT_VLD | LOPT_FLG_STR_VLD, handle_a);
	lopt_regopt("asm", 's', LOPT_FLG_CH_VLD | LOPT_FLG_OPT_VLD | LOPT_FLG_STR_VLD, handle_s);
	lopt_regopt("def", 'd', LOPT_FLG_CH_VLD | LOPT_FLG_OPT_VLD | LOPT_FLG_STR_VLD, handle_d);
	lopt_regopt("ref", 'n', LOPT_FLG_CH_VLD | LOPT_FLG_OPT_VLD | LOPT_FLG_STR_VLD, handle_n);
	lopt_regopt("exec", 'x', LOPT_FLG_CH_VLD | LOPT_FLG_OPT_VLD | LOPT_FLG_STR_VLD, handle_x);
	lopt_regopt("template", 't', LOPT_FLG_CH_VLD | LOPT_FLG_OPT_VLD | LOPT_FLG_STR_VLD, handle_t);

	int i, ch, stage = 0, ins_id = 0;
	int insc;
	char buffer[128];
	char flg[4096];
	int rst = lopt_parse(argc, argv);
	if (rst != 0)
	{
		printf("Argument parsing error, [%s] error.\n", argv[-rst]);
	}
	if (nout == NULL)
		fatal(0x04000000);
	if (sout == NULL)
		fatal(0x04000001);
	if (tout == NULL)
		fatal(0x04000002);
	if (dout == NULL)
		fatal(0x04000003);
	if (xout == NULL)
		fatal(0x04000004);
	if (aout == NULL)
		fatal(0x04000005);
	if (input == NULL)
		fatal(0x04000100);

	//fputs("{", nout); // reflect array start
	//fputs("{", tout); // i/o template string definition str arr start

	while (EOF != fscanf(input, "%s", buffer))
	{
		fprintf(dout, "#define %s %d\n", buffer, ins_id++); // macro define
		fprintf(nout, "\"%s\", ", buffer);					// relect name out
		fprintf(xout, "case %s:\n", buffer);				// emulation case [per inst]

		// To using these, you should do a first (dis)asm first
		// And set up a switch(opcode) {#include "XXX.inc"}
		// Before the switch, you should set up an "ans"
		// to store the result and a "tmp" for temporary ull store and
		// a "ins" for the generated routine to parsing for disasm.
		// Otherwise, an "ans" to store result and a "tmp" for temporary
		// ull store and a "ins" to store the in-prog inst.
		fprintf(aout, "/*%s*/\n", buffer);	 // disasm comment [debug use]
		fprintf(sout, "case %s:\n", buffer); // asmbler case [per inst]

		fscanf(input, "%d", &insc);
		for (i = 0; i < insc; ++i)
		{
			int s, e, v;
			char val[256];
			fscanf(input, "%d %d %s", &s, &e, val);
			if (val[0] == 'x')
				sscanf(val + 1, "%x", &v);
			else if (val[0] == 'c')
				v = readoct(val + 1);
			else if (val[0] == 'd')
				sscanf(val + 1, "%d", &v);
			else if (val[0] == 'b')
				v = readbin(val + 1);
			else
				error_hint(0x02010000, buffer);
			fprintf(aout, "if(0x%X == ((ins >> %d) & 0x%X))\n", v, s, (1ll << (e - s + 1)) - 1);
			fprintf(sout, "ans |= (0x%X & 0x%X) << %d;\n", v, (1ll << (e - s + 1)) - 1, s);
		}
		fscanf(input, "%d", &insc);
		fputs("{\n", aout);
		for (i = 0; i < insc; ++i)
		{
			int s, e, v;
			char val[256];
			fscanf(input, "%d %d %s", &s, &e, val);
			if (val[0] == '#')
			{
				fprintf(aout, "tmp = (ins >> %d) & 0x%X;\n\
ans.imm = (ins & 1 << %d) ? (signed long long)tmp : tmp;\n",
						s, (1 << (e - s + 1)) - 1, e);
				fprintf(sout, "ans |= (ins.imm & 0x%X) << %d;\n", (1ll << (e - s + 1)) - 1, s);
			}
			else
			{
				fprintf(aout, "ans.%s = (ins >> %d) & 0x%X;\n", val, s, (1ll << (e - s + 1)) - 1);
				fprintf(sout, "ans |= (ins.%s & 0x%X) << %d;\n", val, (1ll << (e - s + 1)) - 1, s);
			}
		}
		fprintf(aout, "ans.opcode = %s;\n", buffer);
		fputs("return ans;\n}\n", aout);
		fscanf(input, "%*[ \n]%*%%[^\n]", flg);
		fgetc(input);
		fprintf(tout, "\"%s\", ", flg);
		fscanf(input, "%[^@]", flg);
		fgetc(input);
		fputs(flg, xout);
		fputs("break;\n", sout);
		fputs("\nbreak;\n", xout);
	}
	//fputs("}", nout);
	//fputs("}", tout);
	fclose(input);
	fclose(aout);
	fclose(xout);
	fclose(dout);
	fclose(nout);
	fclose(tout);

	return 0;
}
