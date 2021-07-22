#include "r5900.hpp"
#include "liteopt.h"
#include <errno.h>
#include <string.h>
#include <string>
#include <stack>
#include <stdio.h>

static int mode = 0;
#define MODE_SASM 0
#define MODE_SDISASM 1
// #define MODE_IASM    2
// #define MODE_IDISASM 3
// #define MODE_CASM    4
// #define MODE_CDISASM 5
static int dmode = 2;
#define DM_DB 0
#define DM_DH 1
#define DM_DW 2
#define DM_DD 3
#define DM_DQ 4
static unsigned long long dasm_sta = 0x0, dasm_end = 0xffffffffffffffff;
static FILE *input, *output;
static std::stack<FILE *> scripts;
static bool do_invalid = false/*, do_purge = true*/;

int handle_i(const char *filename);
int handle_o(const char *filename);
int handle_s(const char *filename);
int handle_D(const char *hexstr);
int handle_A(const char *assembly);
int show_help(const char *stub);
void error(int code);
void warn(int code);
void fatal(int code);
void info(int code);

int main(int argc, const char **argv)
{
    lopt_regopt("mode", 'm', LOPT_FLG_CH_VLD | LOPT_FLG_OPT_VLD | LOPT_FLG_STR_VLD,
                [](const char *ans) -> int
                {
                    if(ans == NULL) fatal(1901);
                    mode = *ans == 'a' ? MODE_SASM : MODE_SDISASM;
                    return 0;
                });
    lopt_regopt("auto-conv", 'd', LOPT_FLG_CH_VLD | LOPT_FLG_OPT_VLD | LOPT_FLG_STR_VLD,
                [](const char *md) -> int
                {
                    if(md == NULL) fatal(1901);
                    switch (*md)
                    {
                    case 'b':
                        dmode = DM_DB;
                        break;
                    case 'h':
                        dmode = DM_DH;
                        break;
                    case 'w':
                        dmode = DM_DW;
                        break;
                    case 'd':
                        dmode = DM_DD;
                        break;
                    default:
                        error(1200);
                        break;
                    }
                    return 0;
                });
    lopt_regopt("limits", 'l', LOPT_FLG_CH_VLD | LOPT_FLG_OPT_VLD | LOPT_FLG_STR_VLD,
                [](const char *lmt) -> int
                {
                    sscanf(lmt, "%llx:%llx", &dasm_sta, &dasm_end);
                    return 0;
                });
    lopt_regopt("invalid-ops", 'n', LOPT_FLG_CH_VLD | LOPT_FLG_STR_VLD | LOPT_FLG_OPT_VLD,
                [](const char *stub) -> int
                {
                    do_invalid = true;
                    return 0;
                });
    // lopt_regopt("no-purge", '\0', LOPT_FLG_CH_VLD | LOPT_FLG_OPT_VLD, 
    //             [](const char *stub) -> int
    //             {
    //                 do_purge = false;
    //                 return 0;
    //             });
    lopt_regopt("input", 'i', LOPT_FLG_CH_VLD | LOPT_FLG_OPT_VLD | LOPT_FLG_STR_VLD, handle_i);
    lopt_regopt("output", 'o', LOPT_FLG_CH_VLD | LOPT_FLG_OPT_VLD | LOPT_FLG_STR_VLD, handle_o);
    lopt_regopt("script", 's', LOPT_FLG_CH_VLD | LOPT_FLG_OPT_VLD | LOPT_FLG_STR_VLD, handle_s);
    lopt_regopt("disasmimm", 'D', LOPT_FLG_CH_VLD | LOPT_FLG_OPT_VLD | LOPT_FLG_STR_VLD, handle_D);
    lopt_regopt("asmimm", 'A', LOPT_FLG_CH_VLD | LOPT_FLG_OPT_VLD | LOPT_FLG_STR_VLD, handle_A);
    lopt_regopt("help", 'h', LOPT_FLG_CH_VLD | LOPT_FLG_OPT_VLD | LOPT_FLG_STR_VLD, show_help);

    int ret = lopt_parse(argc, argv);
    if (ret != 0)
    {
        if (ret < 0)
        {
            printf("Unknown switch error, [%s] unrecoginizable.\n", argv[-ret]);
            exit(ret);
        }
        else
        {
            error(1900);
        }
    }

    if(input == NULL)
    {
        fatal(1101);
    }

    fseek(input, 0, SEEK_END);
    int len = ftell(input);
    unsigned char *buf = (unsigned char *)malloc(len);
    rewind(input);
    fread(buf, len, 1, input);
    rewind(input);
    // if(do_purge && mode == MODE_SASM) // copy file to clear
    // {
    //     fwrite(buf, len, 1, output);
    //     rewind(output);
    // }

    if(mode == MODE_SDISASM)
    {
        if(input == NULL) fatal(2001);
        if(output == NULL) fatal(2002);
        static char strbuf[1024];
        for(int i = dasm_sta; i < dasm_end; i += 4)
        {
            unsigned int cmd = *(unsigned int*)(buf + i);
            instr_t result = disasm(cmd);
            if(result.opcode == INVALID)
            {
                if(do_invalid)
                    fputs("INVALID\n", output);
                else
                switch (dmode)
                {
                case DM_DB:
                    fprintf(output, "db 0x%02X\ndb 0x%02X\ndb 0x%02X\ndb 0x%02X\n",
                            (cmd & 0xff), (cmd & 0xff00) >> 8, (cmd & 0xff0000) >> 16, (cmd & 0xff000000) >> 24);
                    break;
                case DM_DH:
                    fprintf(output, "dh 0x%04X\ndh 0x%04X\n",
                            cmd & 0xffff, (cmd & 0xffff0000) >> 16);
                    break;
                case DM_DW:
                    fprintf(output, "dw 0x%08X\n", cmd);
                    break;
                default:
                    error(2000);
                    break;
                }
            }
            else
            {
                printdis(strbuf, result);
                fprintf(output, "%s\n", strbuf);
            }
        }
    }

    
    free(buf);

    return 0;
}

// ======================
// Parametres handlers
// ======================
int handle_i(const char *filename)
{
    if (input != NULL)
    {
        warn(1000);
        info(1500);
        return 1000;
    }

    if (filename == NULL)
        fatal(1901);

    input = fopen(filename, "rb");
    if (input == NULL)
    {
        int err = errno;
        error(1902);
        error(1100);
        puts(strerror(err));
        return 2;
    }
    return 0;
}

int handle_o(const char *filename)
{
    if (output != NULL)
    {
        warn(1010);
        info(1500);
        return 1010;
    }

    if (filename == NULL)
        fatal(1901);

    output = fopen(filename, mode == MODE_SASM ? "wb" : "w");
    if (output == NULL)
    {
        int err = errno;
        error(1902);
        error(1110);
        puts(strerror(err));
        return 2;
    }
    return 0;
}

int handle_s(const char *filename)
{
    if (filename == NULL)
        fatal(1901);

    FILE *script = fopen(filename, "r");
    if (script == NULL)
    {
        int err = errno;
        error(1902);
        error(1110);
        puts(strerror(err));
        return 2;
    }
    scripts.push(script);
    return 0;
}

int handle_D(const char *hex)
{
    if (hex == NULL)
        fatal(1901);
    char buf[128];
    unsigned long data;
    sscanf(hex, "%lX", &data);
    instr_t inst = disasm(data);
    printdis(buf, inst);
    puts(buf);
    return 0;
}

int handle_A(const char *asmb)
{
    if (asmb == NULL)
        fatal(1901);
    instr_t ans;
    int ret = parse_asm(asmb, &ans);
    if (ret == -1)
    {
        error(8000);
        return 8000;
    }
    if (ret == -2)
    {
        error(8001);
        return 8001;
    }
    if (ret == -3)
    {
        error(8002);
        return 8002;
    }
    printf("%08X\n", asmble(ans));
    return 0;
}

// ========================
// Error hints
// ========================
static struct err_t
{
    int code;
    const char *msg;
} errs[] = {
    {0, "操作正常结束。"},
    {1000, "早已打开输入文件。"},
    {1010, "早已打开输出文件。"},
    {1100, "无法打开输入文件。"},
    {1101, "需要输入文件，但未能打开。"},
    {1110, "无法打开输出文件。"},
    {1111, "需要输出文件，但未能打开。"},
    {1200, "未知的输出偏好选项。"},
    {1500, "将忽略新指定的文件。"},
    {1902, "命令行参数解析时发生错误，请留意下方信息。"},
    {1901, "命令行参数解析时出错：需求的参数不完整。"},
    {1900, "命令行参数解析时发生错误，请留意上方信息。"},
    {2000, "反汇编失败。-d 参数错误。"},
    {2001, "输入文件未指定。"},
    {2002, "输出文件未指定。"},
    {8000, "无效汇编指令。"},
    {8001, "未知寄存器名。"},
    {8002, "命令语法不正确。"},
    {9999, "索引未命中。"}};

void error(int code)
{
    for (int i = 0; i < (int)sizeof(err_t); ++i)
    {
        if (errs[i].code == code)
        {
            printf("E%04d %s\n", code, errs[i].msg);
            return;
        }
    }
    fatal(9999);
}

void fatal(int code)
{
    for (int i = 0; i < (int)sizeof(err_t); ++i)
    {
        if (errs[i].code == code)
        {
            printf("F%04d %s\n", code, errs[i].msg);
            exit(i);
        }
    }
    fatal(9999);
}

void warn(int code)
{
    for (int i = 0; i < (int)sizeof(err_t); ++i)
    {
        if (errs[i].code == code)
        {
            printf("W%04d %s\n", code, errs[i].msg);
            return;
        }
    }
    fatal(9999);
}

void info(int code)
{
    for (int i = 0; i < (int)sizeof(err_t); ++i)
    {
        if (errs[i].code == code)
        {
            printf("I%04d %s\n", code, errs[i].msg);
            return;
        }
    }
    fatal(9999);
}

int show_help(const char *stub)
{
    puts("Partically assembler for MIPS R5900.\n    Coded by Xiyan_shan");
    puts("部分汇编补丁应用器，RX79专版。\n    编写者：单希研");
    puts("用法 Usage: prtasm (-m [a/d]) -i [input.bin] -o [output.bin] (-s [script.txt] (Options))");
    puts("选项 Options: ");
    puts("-A [asm code]   [测试]立即转换汇编代码到十六进制数据。");
    puts("-D [hex code]   [测试]立即反汇编一个十六进制数。");
    puts("-i [input.bin]  指定输入文件。");
    puts("-o [output.bin] 指定输出文件。");
    puts("-s [script.txt] 指定脚本文件。");
    puts("-l [range(格式：xxx:xxx，16 进制)] 指定反汇编限定地址。（仅当模式为反汇编时可用）");
    puts("-m [a/d]        指定为汇编或反汇编模式。（反汇编时，选项 -s 被忽略。）");
    puts("-d [b/h/w/d/q]  将无法处理的指令值以db/dh/dw/dd/dq表示。（默认为dw）");
    puts("-I              [未实现]指定为交互模式(忽略“-s”选项)。");
    puts("--invalid-ops   将无法解析的指令值输出为INVALID而非d*。");
    // puts("--no-purge      指定不要复制输入文件到输出文件。");
    puts("例: \n\
    prtasm -i test.elf -o mod.elf -s mod.mips\n\
    prtasm -m d -i test.elf -o dis.mips -l 1050:135c --invalid-ops\n\
    prtasm -m a -i test.elf -o mod.elf -s mod.mips --no-purge");
    return 0;
}