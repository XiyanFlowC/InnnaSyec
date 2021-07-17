#include "r5900.hpp"
#include "liteopt.h"
#include <errno.h>
#include <string.h>
#include <string>
#include <stack>
#include <stdio.h>

static int mode = 0;
#define MODE_SASM    0
#define MODE_SDISASM 1
#define MODE_IASM    2
#define MODE_IDISASM 3
#define MODE_CASM    4
#define MODE_CDISASM 5
static FILE *input, *output;
static std::stack<FILE *> scripts;

int handle_i(const char *filename);
int handle_o(const char *filename);
int handle_s(const char *filename);
int handle_D(const char *hexstr);
void error(int code);
void warn(int code);
void fatal(int code);
void info(int code);

int main(int argc, const char **argv)
{
    lopt_regopt("input", 'i', LOPT_FLG_CH_VLD | LOPT_FLG_OPT_VLD | LOPT_FLG_STR_VLD, handle_i);
    lopt_regopt("output", 'o', LOPT_FLG_CH_VLD | LOPT_FLG_OPT_VLD | LOPT_FLG_STR_VLD, handle_o);
    lopt_regopt("script", 's', LOPT_FLG_CH_VLD | LOPT_FLG_OPT_VLD | LOPT_FLG_STR_VLD, handle_s);
    lopt_regopt("disasmimm", 'D', LOPT_FLG_CH_VLD | LOPT_FLG_OPT_VLD | LOPT_FLG_STR_VLD, handle_D);

    int ret = lopt_parse(argc, argv);
    if (ret != 0)
    {
        if (ret < 0)
        {
            printf("Unknown switch error, [%s] unrecoginizable.", argv[-ret]);
            exit(ret);
        }
        else
        {
            error(1900);
        }
    }

    return 0;
}

// ======================
// Parametres handlers
// ======================
int handle_i(const char *filename)
{
    if(input != NULL)
    {
        warn(1000);
        info(1500);
        return 1000;
    }

    if(filename == NULL) fatal(1901);

    input = fopen(filename, "rb");
    if(input == NULL)
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
    if(output != NULL)
    {
        warn(1010);
        info(1500);
        return 1010;
    }

    if(filename == NULL) fatal(1901);

    output = fopen(filename, "wb");
    if(output == NULL)
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
    if(filename == NULL) fatal(1901);

    FILE *script = fopen(filename, "r");
    if(script == NULL)
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
    char buf[128];
    unsigned long data;
    sscanf(hex, "%X", &data);
    instr_t inst = disasm(data);
    printdis(buf, inst);
    puts(buf);
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
    {1110, "无法打开输出文件。"},
    {1500, "将忽略新指定的文件。"},
    {1902, "命令行参数解析时发生错误，请留意下方信息。"},
    {1901, "命令行参数解析时出错：需求的参数不完整。"},
    {1900, "命令行参数解析时发生错误，请留意上方信息。"},
    {9999, "索引未命中。"}
};

void error(int code)
{
    for (int i = 0; i < sizeof(err_t); ++i)
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
    for (int i = 0; i < sizeof(err_t); ++i)
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
    for (int i = 0; i < sizeof(err_t); ++i)
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
    for (int i = 0; i < sizeof(err_t); ++i)
    {
        if (errs[i].code == code)
        {
            printf("I%04d %s\n", code, errs[i].msg);
            return;
        }
    }
    fatal(9999);
}