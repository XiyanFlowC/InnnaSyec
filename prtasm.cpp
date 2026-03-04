//#include "r5900.hpp"
#include "liteopt.h"
#include "inscodec/xyutils.h"
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include "inscodec/disas.h"
#include "inscodec/ins_def.h"
// #include <string>
#include <stack>
#include <vector>
#include <string>
// #include <map>
#include <stdio.h>

#ifdef DEBUG
#define debug(...) printf("[DEBUG]" __VA_ARGS__)
#else
#define debug(...)
#endif

// 运行模式
static int mode = 0;
#define MODE_SASM 0
#define MODE_SDISASM 1
// #define MODE_IASM    2
// #define MODE_IDISASM 3
// #define MODE_CASM    4
// #define MODE_CDISASM 5
#define MODE_EXPORT 6

// 反汇编时，不可认识的代码的输出方式（0.byte, 1.half, 2.word）
static int dmode = 2;
#define DM_DB 0
#define DM_DH 1
#define DM_DW 2
#define DM_DD 3
#define DM_DQ 4
// 反汇编操作起讫地址
static unsigned long long dasm_sta = 0x0, dasm_end = 0xffffffffffffffff;
static FILE *input, *output, *script; // TODO: include 支持，script相关全部重写
static char /* *scriptnm,*/ *outputnm;
static char scriptnm[2048];
static bool do_invalid = false /*, do_purge = true*/, stop_if_overline = false, stop_firsterr = false, move_only_word = false;
int cerror = 0, cwarn = 0;
static std::vector<int> suppressed_warn_codes;

// 存储脚本行以支持stdin和多遍扫描
static std::vector<std::string> script_lines;

/******************************
 * 结构体定义存储            *
 ******************************/
struct struct_field_t {
    char name[128];
    int size;        // 字段大小：1, 2, 4, 8
    int offset;      // 字段在结构体中的偏移量
};

struct struct_def_t {
    char name[128];
    std::vector<struct_field_t> fields;
    int total_size;
};

#ifndef STRUCT_MAX_COUNT
#define STRUCT_MAX_COUNT 256
#endif
static std::vector<struct_def_t> struct_defs;

// 根据类型名获取字段大小
static int get_field_size_by_typename(const char *typename_str)
{
    if (0 == strcmp(typename_str, "byte") || 0 == strcmp(typename_str, "BYTE"))
        return 1;
    if (0 == strcmp(typename_str, "half") || 0 == strcmp(typename_str, "HALF"))
        return 2;
    if (0 == strcmp(typename_str, "word") || 0 == strcmp(typename_str, "WORD"))
        return 4;
    if (0 == strcmp(typename_str, "dword") || 0 == strcmp(typename_str, "DWORD"))
        return 8;
    return -1;
}

// 查找结构体定义
static struct_def_t* find_struct_def(const char *name)
{
    for (size_t i = 0; i < struct_defs.size(); ++i)
    {
        if (0 == strcmp(struct_defs[i].name, name))
            return &struct_defs[i];
    }
    return NULL;
}


int handle_i(const char *filename);
int handle_o(const char *filename);
int handle_s(const char *filename);
int handle_W(const char *warnspec);
int handle_D(const char *hexstr);
int handle_A(const char *assembly);
int show_help(const char *stub);
// 获取并输出错误信息
void error(int code);
// 获取并输出警告信息
void warn(int code);
// 获取并输出致命信息，并中止程序
void fatal(int code);
void info(int code);
int load_script_lines();
//int check();
int genasm(unsigned char *buffer, long long buffer_size, bool check_only = false, int *line_idx = nullptr);

static int is_warning_suppressed(int code)
{
    for (size_t i = 0; i < suppressed_warn_codes.size(); ++i)
    {
        if (suppressed_warn_codes[i] == code)
            return 1;
    }
    return 0;
}

int main(int argc, const char **argv)
{
    PrepareOpcodeBuffer();
    
    lopt_regopt("stop-if-out-of-range", 'r', LOPT_FLG_CH_VLD | LOPT_FLG_OPT_VLD | LOPT_FLG_STR_VLD,
                [](const char *ans) -> int
                {
                    stop_if_overline = true;
                    return 0;
                });
    lopt_regopt("move-only-word", '\0', LOPT_FLG_OPT_VLD | LOPT_FLG_STR_VLD,
                [](const char *ans) -> int
                {
                    move_only_word = true;
                    return 0;
                });
    lopt_regopt("stop-at-first-error", 'e', LOPT_FLG_CH_VLD | LOPT_FLG_OPT_VLD | LOPT_FLG_STR_VLD,
                [](const char *ans) -> int
                {
                    stop_firsterr = true;
                    return 0;
                });
    lopt_regopt("mode", 'm', LOPT_FLG_CH_VLD | LOPT_FLG_OPT_VLD | LOPT_FLG_STR_VLD,
                [](const char *ans) -> int
                {
                    if (ans == NULL)
                        fatal(1901);
                    mode = *ans == 'a' ? MODE_SASM : MODE_SDISASM;
                    return 0;
                });
    lopt_regopt("auto-conv", 'd', LOPT_FLG_CH_VLD | LOPT_FLG_OPT_VLD | LOPT_FLG_STR_VLD,
                [](const char *md) -> int
                {
                    if (md == NULL)
                        fatal(1901);
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
                    if(lmt == NULL) fatal(1901);
                    sscanf(lmt, "%llx:%llx", &dasm_sta, &dasm_end);
                    return 0;
                });
    lopt_regopt("invalid-ops", 'n', LOPT_FLG_CH_VLD | LOPT_FLG_STR_VLD | LOPT_FLG_OPT_VLD,
                [](const char *stub) -> int
                {
                    do_invalid = true;
                    return 0;
                });
    lopt_regopt("suppress-warn", 'W', LOPT_FLG_CH_VLD | LOPT_FLG_OPT_VLD | LOPT_FLG_STR_VLD, handle_W);
    // lopt_regopt("no-purge", '\0', LOPT_FLG_CH_VLD | LOPT_FLG_OPT_VLD,
    //             [](const char *stub) -> int
    //             {
    //                 do_purge = false;
    //                 return 0;
    //             });
    lopt_regopt("input", 'i', LOPT_FLG_CH_VLD | LOPT_FLG_OPT_VLD | LOPT_FLG_STR_VLD, handle_i);
    lopt_regopt("output", 'o', LOPT_FLG_CH_VLD | LOPT_FLG_OPT_VLD | LOPT_FLG_STR_VLD, handle_o);
    lopt_regopt("script", 's', LOPT_FLG_CH_VLD | LOPT_FLG_OPT_VLD | LOPT_FLG_STR_VLD, handle_s);
    lopt_regopt("scriptstdin", 'S', LOPT_FLG_CH_VLD | LOPT_FLG_OPT_VLD | LOPT_FLG_STR_VLD,
                [](const char *stub) -> int
                {
                    script = stdin;
                    //scriptnm = strdup("<stdin>");
                    strcpy(scriptnm, "<stdin>");
                    return 0;
                });
    lopt_regopt("disasmimm", 'D', LOPT_FLG_CH_VLD | LOPT_FLG_OPT_VLD | LOPT_FLG_STR_VLD, handle_D);
    lopt_regopt("asmimm", 'A', LOPT_FLG_CH_VLD | LOPT_FLG_OPT_VLD | LOPT_FLG_STR_VLD, handle_A);
    lopt_regopt("help", 'h', LOPT_FLG_CH_VLD | LOPT_FLG_OPT_VLD | LOPT_FLG_STR_VLD, show_help);

    int ret = lopt_parse(argc, argv);
    if (ret != 0)
    {
        if (ret < 0)
        {
            printf("未知命令行参数, 不可辨识的参数：[%s].\n", argv[-ret]);
            exit(ret);
        }
        else
        {
            error(1900);
        }
    }

    if (input == NULL)
    {
        fatal(1101);
    }

    fseek(input, 0, SEEK_END);
    long long len = ftell(input);
    unsigned char *buf = (unsigned char *)malloc(len);
    rewind(input);
    fread(buf, len, 1, input);
    rewind(input);
    // if(do_purge && mode == MODE_SASM) // copy file to clear
    // {
    //     fwrite(buf, len, 1, output);
    //     rewind(output);
    // }

    if (mode == MODE_SDISASM)
    {
        if (input == NULL)
            fatal(2001);
        if (output == NULL)
            fatal(2002);
        static char strbuf[1024];
        dasm_end = dasm_end < (unsigned long long)len ? dasm_end : len;

        fprintf(output, ".loc %08llX\n", dasm_sta);
        for (unsigned long long i = dasm_sta; i < dasm_end; i += 4)
        {
            unsigned int cmd = *(unsigned int *)(buf + i);
            instr_t result = DecodeInstruction(cmd);
            if (result.opcode == -1)
            {
                if (do_invalid)
                    fputs("INVALID\n", output);
                else
                    switch (dmode)
                    {
                    case DM_DB:
                        fprintf(output, ".byte 0x%02X\n.byte 0x%02X\n.byte 0x%02X\n.byte 0x%02X\n",
                                (cmd & 0xff), (cmd & 0xff00) >> 8, (cmd & 0xff0000) >> 16, (cmd & 0xff000000) >> 24);
                        break;
                    case DM_DH:
                        fprintf(output, ".half 0x%04X\n.half 0x%04X\n",
                                cmd & 0xffff, (cmd & 0xffff0000) >> 16);
                        break;
                    case DM_DW:
                        fprintf(output, ".word 0x%08X\n", cmd);
                        break;
                    default:
                        error(2000);
                        break;
                    }
            }
            else
            {
                //printdis(strbuf, result);
                disas(result, i, strbuf);
                fprintf(output, "%s\n", strbuf);
            }
        }
    }
    if (mode == MODE_SASM)
    {
        if (input == NULL)
            fatal(2001);
        if (output == NULL)
            fatal(2002);
        if (script == NULL)
            fatal(2003);
        
        // 先将所有脚本行读入内存
        if (load_script_lines() != 0)
            fatal(2004);
        
        if (0 == genasm(buf, len, true))
            genasm(buf, len);

        if(!cerror)
            fwrite(buf, len, 1, output);
        else
        {
            fclose(output);
            output = NULL;
            remove(outputnm);
        }
        
        printf("汇编处理结束：%d个错误，%d个警告。\n", cerror, cwarn);
    }

    if (output)
        fclose(output);
    if (input)
        fclose(input);
    free(buf);
    // free(scriptnm);
    free(outputnm);

    return cerror;
}

// 获取并输出错误信息，并给出文件名、行号和行内容
void aerror(int linec, int code, const char *line)
{
    printf("(%s:%d) %s\n\t", scriptnm, linec, line);
    error(code);
}
#define ERROR(code) aerror(line, (code), linebuf)

// 获取并输出致命信息，并给出文件名、行号和行内容
// 同时中止程序
void afatal(int linec, int code, const char *line)
{
    printf("(%s:%d) %s\n\t", scriptnm, linec, line);
    fatal(code);
}
#define FATAL(code) afatal(line, (code), linebuf)

// 获取并输出警告信息，并给出文件名、行号和行内容
void awarn(int linec, int code, const char *line)
{
    printf("(%s:%d) %s\n\t", scriptnm, linec, line);
    warn(code);
}
#define WARN(code) awarn(line, (code), linebuf)

// 获取所给汇编指令文本对应指令大小
int get_asm_len(const char *src)
{
    char mnemonic[128];
    src = str_first_not(src, '\r');
    int eles = sscanf(src, "%s", mnemonic);
    strupr(mnemonic);

    if (eles == 0) return -1;

    for(int i = 0; i < OPTION_COUNT; ++i) {
        if (strcmp(instructions[i].name, mnemonic) == 0) return 4;
    }

    if(0 == strcmp(mnemonic, "LI"))
    {
        char *para = str_first_not(src + 2, '\r');
        instr_t tmp;
        if(parse_param(para, "$rt, #im", &tmp) < 0) return -1;
        if(tmp.imm > 0x7fff || (*(long long*)&tmp.imm) < -0x8000) return 8;
        return 4;
    }
    else if(0 == strcmp(mnemonic, "LA"))
    {
        return 8;
    }
    else if(0 == strcmp(mnemonic, "MOVE"))
    {
        return 4;
    }
    else if(0 == strcmp(mnemonic, "DMOVE"))
    {
        return 4;
    }
    else if(0 == strcmp(mnemonic, "B"))
    {
        return 4;
    }
    else if(0 == strcmp(mnemonic, "STRUCT"))
    {
        // struct StructName <...>
        // 需要找到结构体定义并计算大小
        char *p = str_first_not(src + 6, '\r');
        if (p == NULL) return -1;
        
        char struct_name[128];
        int name_len = 0;
        while (*p && !isspace(*p) && *p != '<' && name_len < 127)
        {
            struct_name[name_len++] = *p++;
        }
        struct_name[name_len] = '\0';
        
        struct_def_t *sdef = find_struct_def(struct_name);
        if (sdef == NULL) return -1;
        
        return sdef->total_size;
    }
    else return -1;
}

/******************************
 * 汇编标签（位置）信息存储用 *
 ******************************/
#ifndef LABEL_MAX_COUNT
#define LABEL_MAX_COUNT 16384
#endif
static unsigned long long tag_loc[LABEL_MAX_COUNT], tag_vma[LABEL_MAX_COUNT];
static char tag_nm[LABEL_MAX_COUNT][128];
static int tag_p = 0;

static int find_label_value(const char *name, unsigned long long *value)
{
    // 检查是否为结构体成员访问（StructName.fieldname）
    const char *dot = strchr(name, '.');
    if (dot != NULL)
    {
        char struct_name[128];
        size_t struct_name_len = dot - name;
        if (struct_name_len >= sizeof(struct_name))
            struct_name_len = sizeof(struct_name) - 1;
        strncpy(struct_name, name, struct_name_len);
        struct_name[struct_name_len] = '\0';
        
        const char *field_name = dot + 1;
        
        // 查找结构体定义
        struct_def_t *sdef = find_struct_def(struct_name);
        if (sdef != NULL)
        {
            // 查找字段
            for (size_t i = 0; i < sdef->fields.size(); ++i)
            {
                if (0 == strcmp(sdef->fields[i].name, field_name))
                {
                    *value = sdef->fields[i].offset;
                    return 1;
                }
            }
        }
    }
    
    // 普通标签查找
    for (int i = 0; i < tag_p; ++i)
    {
        if (0 == strcmp(tag_nm[i], name))
        {
            *value = tag_vma[i];
            return 1;
        }
    }
    return 0;
}

static int is_ident_start_char(char c)
{
    return isalpha((unsigned char)c) || c == '_' || c == '.' || c == '$' || c == '@';
}

static int is_ident_char(char c)
{
    return isalnum((unsigned char)c) || c == '_' || c == '.' || c == '$' || c == '@' || c == '?';
}

static int is_lex_boundary_char(char c)
{
    if (isspace((unsigned char)c))
        return 1;
    switch (c)
    {
    case '+':
    case '-':
    case '*':
    case '/':
    case '%':
    case '&':
    case '|':
    case '^':
    case '~':
    case '!':
    case '=':
    case '<':
    case '>':
    case '(':
    case ')':
    case '[':
    case ']':
    case '{':
    case '}':
    case ',':
        return 1;
    default:
        return 0;
    }
}

static int is_ident_token_start(const char *begin, const char *pos)
{
    if (pos <= begin)
        return 1;
    return is_lex_boundary_char(*(pos - 1));
}

static int eval_expr_with_labels(const char *expr, long long *out)
{
    std::string replaced;
    const char *p = expr;
    while (*p != '\0')
    {
        if (is_ident_start_char(*p) && is_ident_token_start(expr, p))
        {
            const char *start = p++;
            while (*p != '\0' && is_ident_char(*p))
                ++p;
            std::string name(start, p - start);
            unsigned long long val = 0;
            if (!find_label_value(name.c_str(), &val))
                return -2;
            char numbuf[64];
            snprintf(numbuf, sizeof(numbuf), "%llu", val);
            replaced += numbuf;
        }
        else
        {
            replaced.push_back(*p++);
        }
    }

    long long val = 0;
    int len = eval_int_expr(&val, replaced.c_str());
    if (len < 0)
        return -1;
    if (str_first_not(replaced.c_str() + len, '\r') != NULL)
        return -1;
    *out = val;
    return 0;
}

static void replace_labels_in_line(char *line)
{
    std::string src(line);
    size_t pos = 0;
    while (pos < src.size() && (src[pos] == ' ' || src[pos] == '\t'))
        ++pos;
    while (pos < src.size() && src[pos] != ' ' && src[pos] != '\t')
        ++pos;

    std::string out = src.substr(0, pos);
    for (size_t i = pos; i < src.size();)
    {
        char c = src[i];
        if (is_ident_start_char(c) && (i == 0 || is_lex_boundary_char(src[i - 1])))
        {
            size_t start = i++;
            while (i < src.size() && is_ident_char(src[i]))
                ++i;
            std::string name = src.substr(start, i - start);
            unsigned long long val = 0;
            if (find_label_value(name.c_str(), &val))
            {
                char numbuf[64];
                snprintf(numbuf, sizeof(numbuf), "%llu", val);
                out += numbuf;
            }
            else
            {
                out += name;
            }
        }
        else
        {
            out.push_back(c);
            ++i;
        }
    }

    if (out.size() < 4096)
        strcpy(line, out.c_str());
}

// 判断：是否为有分支延迟槽之指令（基于助记符）
// 0 - not
// 1 - j
// 2 - b
int is_lbinst(char *nm)
{
    strupr(nm);
    if(nm[0] == 'J' && nm[1] == 'A' && nm[3] == 'R') return 3; // jalr
    if(nm[0] == 'J' && nm[1] != 'R') return 1; // r for jr
    if(nm[0] == 'B' && nm[1] != 'R') return 2; // r for break
    if(nm[0] == 'J' && nm[1] == 'R') return 3; // jr - not relate to the lbl
    return 0;
}

static int is_delayslot = 0;

int mkasm(unsigned char *buf, char *asmb, unsigned long long now_vma)
{
    str_trim_end(asmb);
    char mnemonic[64];
    sscanf(asmb, "%s", mnemonic);
    int type = is_lbinst(mnemonic);
    if(type)
    {
        if(is_delayslot) return -5;
        if(type == 3)
        {
            is_delayslot = 2;
            goto mkasm_nparse;
        }
        char *lbl = str_last(asmb, ',');
        if(lbl == NULL) 
        {
            if((lbl = str_first(asmb, ' ')) == NULL
                && (lbl = str_first(asmb, '\t')) == NULL) return -3;
        }
        lbl = str_first_not(lbl + 1, '\r');
        if(lbl == NULL) return -3;
        str_trim_end(lbl);
        long long lbl_val = 0;
        int eval_ret = eval_expr_with_labels(lbl, &lbl_val);
        if (eval_ret == -2)
            return -4;
        if (eval_ret != 0)
            return -3;

        {
            char numbuf[64];
            snprintf(numbuf, sizeof(numbuf), "%lld", lbl_val);
            std::string prefix(asmb, (size_t)(lbl - asmb));
            std::string merged = prefix + numbuf;
            if (merged.size() >= 4096)
                return -3;
            strcpy(asmb, merged.c_str());
        }

        is_delayslot = 2; // 每次返回前减一，由于跳转位于延迟槽的异常判断位于之前，这里可以设置
        // 注解：即：本次返回减少1变为1，下一条位于延迟槽的指令方能正确判断。
        // 注解：下一条延迟槽指令返回时减少1变为0，即恢复正常状态。
        goto mkasm_nparse;
    }
    mkasm_nparse:

        replace_labels_in_line(asmb);
    instr_t ans;
    int ret = as(asmb, now_vma, &ans);
    // if(ret < -1) return ret;
    if(ret == -17) // 非真指令的处理（伪指令判别）
    {
        char mnemonic[128];
        asmb = str_first_not(asmb, '\r');
        int eles = sscanf(asmb, "%s", mnemonic);
        strupr(mnemonic);

        if(strcmp(mnemonic, "LI") == 0)
        {
            char *para = str_first_not(asmb + 2, '\r');
            instr_t tmp;
            if(parse_param(para, "$rt, #im", &tmp) < 0) return -1;
            if(tmp.imm > 0x7fff)
            {
                if (is_delayslot > 0)
                    return -6;

                int low = tmp.imm & 0xffff;
                tmp.opcode = LUI;
                tmp.imm >>= 16;
                if(low > 0x7fff) tmp.imm += 1;
                // 已禁用危险的“交换指令”逻辑：
                // if(is_delayslot)
                // {
                //     *((unsigned int *)buf) = *((unsigned int *)buf - 1);
                //     auto tins = DecodeInstruction(*((unsigned int *)buf - 1));
                //     if(is_lbinst((char*)instructions[tins.opcode].name) == 2)
                //         tins.imm -= 1, *((unsigned int *)buf) = EncodeInstruction(tins);
                //     *((unsigned int *)buf - 1) = EncodeInstruction(tmp);
                // }
                // else
                //     *((unsigned int *)buf) = EncodeInstruction(tmp);
                *((unsigned int *)buf) = EncodeInstruction(tmp);
                buf += 4;
                tmp.opcode = ADDIU;
                tmp.rs = tmp.rt;
                tmp.imm = low > 0x7fff ? -(0x10000 - low) : low;
                *((unsigned int *)buf) = EncodeInstruction(tmp);
                if(is_delayslot) --is_delayslot;
                return 8;
            }
            if((*(long long*)&tmp.imm) < -0x8000)
            {
                if (is_delayslot > 0)
                    return -6;

                int low = tmp.imm & 0xffff;
                int high = (tmp.imm >> 16) & 0xffff;
                tmp.opcode = LUI;
                tmp.imm = high;
                // 已禁用危险的“交换指令”逻辑：
                // if(is_delayslot)
                // {
                //     *((unsigned int *)buf) = *((unsigned int *)buf - 1);
                //     auto tins = DecodeInstruction(*((unsigned int *)buf - 1));
                //     if(is_lbinst((char*)instructions[tins.opcode].name) == 2)
                //         tins.imm -= 1, *((unsigned int *)buf) = EncodeInstruction(tins);
                //     *((unsigned int *)buf - 1) = EncodeInstruction(tmp);
                // }
                // else
                //     *((unsigned int *)buf) = EncodeInstruction(tmp);
                *((unsigned int *)buf) = EncodeInstruction(tmp);
                buf += 4;
                tmp.opcode = ORI;
                tmp.rs = tmp.rt;
                tmp.imm = low;
                *((unsigned int *)buf) = EncodeInstruction(tmp);
                if(is_delayslot) --is_delayslot;
                return 8;
            }
            tmp.opcode = ADDIU;
            tmp.rs = 0;
            *((unsigned int *)buf) = EncodeInstruction(tmp);
            if(is_delayslot) --is_delayslot;
            return 4;
        }
        else if(strcmp(mnemonic, "LA") == 0)
        {
            if (is_delayslot > 0)
                return -6;

            char *lbl = str_first(asmb + 2, ',');
            if(lbl == NULL) return -3;
            lbl = str_first_not(lbl + 1, '\r');
            if(lbl == NULL) return -3;
            for(int i = 0; i < tag_p; ++i)
            {
                if(0 == strcmp(tag_nm[i], lbl))
                {
                    sprintf(lbl, "%llu", tag_vma[i]);
                    parse_param(str_first_not(asmb + 2, '\r'), "$rt, #im", &ans);
                    int low = ans.imm & 0xffff;
                    ans.opcode = LUI;
                    ans.imm >>= 16;
                    if(low > 0x7fff) ans.imm += 1;
                    // 已禁用危险的“交换指令”逻辑：
                    // if(is_delayslot)
                    // {
                    //     *((unsigned int *)buf) = *((unsigned int *)buf - 1);
                    //     auto tins = DecodeInstruction(*((unsigned int *)buf - 1));
                    //     if(is_lbinst((char*)instructions[tins.opcode].name) == 2)
                    //         tins.imm -= 1, *((unsigned int *)buf) = EncodeInstruction(tins);
                    //     *((unsigned int *)buf - 1) = EncodeInstruction(ans);
                    // }
                    // else
                    //     *((unsigned int *)buf) = EncodeInstruction(ans);
                    *((unsigned int *)buf) = EncodeInstruction(ans);
                    buf += 4;
                    ans.opcode = ADDIU;
                    ans.rs = ans.rt;
                    ans.imm = low > 0x7fff ? -(0x10000 - low) : low;
                    *((unsigned int *)buf) = EncodeInstruction(ans);
                    if(is_delayslot) --is_delayslot;
                    return 8;
                }
            }
            return -4;
        }
        else if (strcmp(mnemonic, "MOVE") == 0)
        {
            char *para = str_first_not(asmb + 4, '\r');
            if (parse_param(para, "$rd, $rs", &ans) < 0) return -1;
            ans.opcode = move_only_word ? ADDU : DADDU;
            ans.rt = 0; // zero
            *((unsigned int *)buf) = EncodeInstruction(ans);
            if(is_delayslot) --is_delayslot;
            return 4;
        }
        else if (strcmp(mnemonic, "DMOVE") == 0)
        {
            char *para = str_first_not(asmb + 4, '\r');
            if (parse_param(para, "$rd, $rs", &ans) < 0) return -1;
            ans.opcode = DADDU;
            ans.rt = 0; // zero
            *((unsigned int *)buf) = EncodeInstruction(ans);
            if(is_delayslot) --is_delayslot;
            return 4;
        }
        else if (strcmp(mnemonic, "B") == 0)
        {
            if (parse_param(str_first_not(asmb + 1, '\r'), "&im", &ans) < 0) return -1;
            ans.opcode = BEQ;
            ans.rs = ans.rt = 0; // zero (beq zero, zero, &im)
            *((unsigned int *)buf) = EncodeInstruction(ans);
            if(is_delayslot) --is_delayslot;
            return 4;
        }
        else if (strcmp(mnemonic, "STRUCT") == 0)
        {
            // struct StructName <val1, val2, ...>
            const char *p = str_first_not(asmb + 6, '\r');
            if (p == NULL) return -3;
            
            // 提取结构体名称
            char struct_name[128];
            int name_len = 0;
            while (*p && !isspace(*p) && *p != '<' && name_len < 127)
            {
                struct_name[name_len++] = *p++;
            }
            struct_name[name_len] = '\0';
            
            // 查找结构体定义
            struct_def_t *sdef = find_struct_def(struct_name);
            if (sdef == NULL) return -4;  // 结构体未定义
            
            // 跳到 '<'
            p = str_first_not(p, '\r');
            if (p == NULL || *p != '<') return -3;
            p++;
            
            // 解析值列表
            unsigned char *write_ptr = buf;
            for (size_t field_idx = 0; field_idx < sdef->fields.size(); ++field_idx)
            {
                p = str_first_not(p, '\r');
                if (p == NULL) return -3;
                
                // 提取当前值表达式（直到逗号或 '>'）
                const char *delim = p;
                while (*delim && *delim != ',' && *delim != '>')
                    delim++;
                
                char expr_buf[256];
                size_t expr_len = delim - p;
                if (expr_len >= sizeof(expr_buf)) expr_len = sizeof(expr_buf) - 1;
                strncpy(expr_buf, p, expr_len);
                expr_buf[expr_len] = '\0';
                str_trim_end(expr_buf);
                
                // 求值
                long long value = 0;
                int eval_ret = eval_expr_with_labels(expr_buf, &value);
                if (eval_ret != 0) return -3;
                
                // 写入数据
                const struct_field_t &field = sdef->fields[field_idx];
                switch (field.size)
                {
                case 1:
                    *write_ptr = (unsigned char)value;
                    break;
                case 2:
                    *((unsigned short *)write_ptr) = (unsigned short)value;
                    break;
                case 4:
                    *((unsigned int *)write_ptr) = (unsigned int)value;
                    break;
                case 8:
                    *((unsigned long long *)write_ptr) = (unsigned long long)value;
                    break;
                }
                write_ptr += field.size;
                
                // 移动到下一个值
                p = delim;
                if (*p == ',')
                    p++;
            }
            
            // 应该以 '>' 结束
            p = str_first_not(p, '\r');
            if (p == NULL || *p != '>') return -3;
            
            if(is_delayslot) --is_delayslot;
            return sdef->total_size;
        }
    }
    else
    {
        // if (ret) return ret;
        if(ret == -19)
            return -1000;
        if (ret) return -3;

        *((unsigned int *)buf) = EncodeInstruction(ans);
        if(is_delayslot) --is_delayslot;
        return 4;
    }
    return -3;
}

/**
 * 处理字符串中的转义序列
 * @param src 指向 \ 的指针
 * @param dst 输出缓冲区（仅当 check_only=false 时使用）
 * @param check_only true 时只计算长度，不写入
 * @return 返回源字符串消耗的字符数（包括 \），-1 表示格式错误
 * 注：写入时 dst 应该是有效的内存位置
 */
int process_escape_char(const char *src, unsigned char *dst, bool check_only)
{
    if (*src != '\\') return 0;
    
    if (*++src == 'x') {
        char a = *++src, b = *++src;
        if(a >= 'a' && a <= 'f') a = a - 'a' + 'A';
        if(b >= 'a' && b <= 'f') b = b - 'a' + 'A';
        if(a >= 'A' && a <= 'F') a = a - 'A' + 10;
        else a -= '0';
        if(b >= 'A' && b <= 'F') b = b - 'A' + 10;
        else b -= '0';
        
        if (!check_only) {
            *dst = (a << 4) | b;
        }
        return 3;  // 返回 \xHH 的长度
    }
    else if (*src == 'n') {
        if (!check_only) *dst = '\n';
        return 2;  // 返回 \n 的长度
    }
    else if (*src == '\\') {
        if (!check_only) *dst = '\\';
        return 2;  // 返回 \\ 的长度
    }
    else if (*src == 'r') {
        if (!check_only) *dst = '\r';
        return 2;  // 返回 \r 的长度
    }
    else if (*src == '0') {
        if (!check_only) *dst = '\0';
        return 2;  // 返回 \0 的长度
    }
    
    return -1;  // 未知转义序列
}

// 将脚本所有行读入内存以支持多遍扫描和stdin
int load_script_lines()
{
    if (script == NULL)
        return -1;
    
    script_lines.clear();
    static char linebuf[4096];
    int fsret;
    
    while (EOF != (fsret = fscanf(script, "%[^\n]", linebuf)))
    {
        int t = fgetc(script);
        if('\n' != t && t != EOF) 
        {
            // 行太长或包含非标准行结尾，仍然添加到vector
        }
        
        if (fsret > 0)  // 有内容
        {
            script_lines.push_back(std::string(linebuf));
        }
        else  // 空行
        {
            script_lines.push_back(std::string(""));
        }
    }
    
    return 0;
}

/**
 * 解析并处理逗号分隔的数据定义值（.byte/.half/.word/.dword）
 * @param src 源字符串（从指令名之后开始）
 * @param buffer 写入缓冲区
 * @param now_loc 当前写入位置的指针（会被更新）
 * @param buffer_size 缓冲区大小
 * @param data_size 单个数据项的字节数（1/2/4/8）
 * @param check_only 是否仅检查模式
 * @param line 当前行号
 * @param linebuf 当前行内容（用于错误报告）
 * @return 处理的数据项数量
 */
static int parse_comma_separated_data(const char *src, unsigned char *buffer, 
                                       unsigned long long *now_loc, long long buffer_size,
                                       int data_size, bool check_only, 
                                       int line, const char *linebuf)
{
    const char *p = src;
    int count = 0;
    
    union {
        unsigned char u8;
        char i8;
        unsigned short u16;
        short i16;
        unsigned int u32;
        int i32;
        unsigned long long u64;
        long long i64;
    } tmp;
    
    // 解析逗号分隔的值
    while (p != NULL && *p != '\0')
    {
        p = str_first_not(p, '\r');
        if (p == NULL || *p == '\0') break;
        
        if (check_only)
        {
            count++;
            // 跳到下一个逗号或结束
            while (*p != '\0' && *p != ',')
                p++;
            if (*p == ',') p++;
        }
        else
        {
            long long expr_val = 0;
            const char *comma = strchr(p, ',');
            char expr_buf[256];
            
            // 提取当前表达式
            if (comma != NULL)
            {
                size_t len = comma - p;
                if (len >= sizeof(expr_buf)) len = sizeof(expr_buf) - 1;
                strncpy(expr_buf, p, len);
                expr_buf[len] = '\0';
                p = comma + 1;
            }
            else
            {
                strncpy(expr_buf, p, sizeof(expr_buf) - 1);
                expr_buf[sizeof(expr_buf) - 1] = '\0';
                p = NULL;
            }
            
            // 求值并范围检查
            str_trim_end(expr_buf);
            int eval_ret = eval_expr_with_labels(expr_buf, &expr_val);
            if (eval_ret == -2)
            {
                aerror(line, 4132, linebuf);
                return count;
            }
            else if (eval_ret != 0)
            {
                aerror(line, 4130, linebuf);
                return count;
            }
            
            tmp.i64 = expr_val;
            
            // 范围检查和写入
            if (*now_loc >= (unsigned long long)buffer_size)
            {
                afatal(line, 7000, linebuf);
            }
            
            switch (data_size)
            {
            case 1:
                if (tmp.i64 < -128 || tmp.i64 > 255) 
                    awarn(line, 4109, linebuf);
                *(buffer + *now_loc) = tmp.u8;
                *now_loc += 1;
                break;
            case 2:
                if (tmp.i64 < -32768 || tmp.i64 > 65535) 
                    awarn(line, 4109, linebuf);
                *((unsigned short *)(buffer + *now_loc)) = tmp.u16;
                *now_loc += 2;
                break;
            case 4:
                if (tmp.i64 < -2147483648LL || tmp.i64 > 4294967295LL) 
                    awarn(line, 4109, linebuf);
                *((unsigned int *)(buffer + *now_loc)) = tmp.u32;
                *now_loc += 4;
                break;
            case 8:
                *((unsigned long long *)(buffer + *now_loc)) = tmp.u64;
                *now_loc += 8;
                break;
            }
            
            count++;
        }
    }
    
    return count;
}

int genasm(unsigned char *buffer, long long buffer_size, bool check_only, int *line_idx)
{
    static char linebuf[4096];
    unsigned long long now_loc = 0, galign = 0, warn_loc = ~0x0;
    long long offset = 0;
    int line = 0;
    static char logic_filename[2048];
    int fsret = 0;
    std::stack<unsigned long long> loc_stack;
    
    // 从script_lines中读取行
    for (size_t script_line_idx = 0; script_line_idx < script_lines.size(); ++script_line_idx)
    {
        const std::string& script_line = script_lines[script_line_idx];
        strncpy(linebuf, script_line.c_str(), sizeof(linebuf) - 1);
        linebuf[sizeof(linebuf) - 1] = '\0';
        fsret = strlen(linebuf) > 0 ? 1 : 0;

        ++line;

        if (fsret == 0) continue; // 空行

        debug("%d, %llX : %s\n", line, now_loc, linebuf);

        int flg = 0;
        char *p = linebuf, *body = linebuf;

        if (now_loc >= warn_loc)
        {
            if (stop_if_overline)
                FATAL(4900);
            else
                WARN(4900);
            warn_loc = ~0x0; // it do not need to alarm ever
        }

        while (*p != '\0') // 处理注释及标签标记
        {
            if (flg)
            {
                if (*p == '\\')
                {
                    if (*++p == 'x')
                    {
                        p += 2;
                    }
                    ++p;
                    continue;
                }
                if (*p == '"')
                    flg = 0;
                ++p;
                continue;
            }
            if (*p == ':')
            {
                body = str_first_not(p + 1, ' '); // 如果紧随换行，body会是NULL
                if(body == NULL) body = p + 1; // 所以如果标签后紧随换行，用这行救一下
            }
            if (*p == '"')
            {
                flg = 1;
                ++p;
                continue;
            }
            if (*p == '#' || *p == ';')
            {
                *p = '\0';
                break;
            }
            ++p;
        }
        if (flg)
            aerror(line, 3004, linebuf);

        if (check_only && body != linebuf) // 有标签，要处理
        {
            char *str = str_first_not(linebuf, '\r');
            int count;
            if (tag_p >= LABEL_MAX_COUNT - 1)
                FATAL(3000);
            tag_loc[tag_p] = now_loc;
            tag_vma[tag_p] = now_loc + offset;
            if ((count = get_term(tag_nm[tag_p], str, ':')) >= 128)
                FATAL(3001);
            if (count != count_term(tag_nm[tag_p], ' ')) // 确保标签中没有空格
                ERROR(3002);
            if (count != count_term(tag_nm[tag_p++], '\t')) // 确保标签中没有制表符
                ERROR(3003);

            debug("%16s: %llX\n", tag_nm[tag_p - 1], tag_vma[tag_p - 1]);
        }

        // 常规指令字处理
        if ((body = str_first_not(body, '\r')) == NULL)//空行
        {
            continue;
        }

        else if (body[0] == '.') // 汇编器控制指令
        {
            // long long tmp;
            static char cmd[128];
            static union
            {
                unsigned char u8;
                char i8;
                unsigned short u16;
                short i16;
                unsigned int u32;
                int i32;
                unsigned long long u64;
                long long i64;
            } tmp;
            
            if(get_term2(cmd, body + 1) == 0)
            {
                WARN(4107);
            }
            else if (0 == strcmp(cmd, "byte"))
            {
                int count = parse_comma_separated_data(body + 5, buffer, &now_loc, 
                                                         buffer_size, 1, check_only, 
                                                         line, linebuf);
                if (check_only)
                    now_loc += count;
            }
            else if (0 == strcmp(cmd, "half"))
            {
                int count = parse_comma_separated_data(body + 5, buffer, &now_loc, 
                                                         buffer_size, 2, check_only, 
                                                         line, linebuf);
                if (check_only)
                    now_loc += count * 2;
            }
            else if (0 == strcmp(cmd, "word"))
            {
                int count = parse_comma_separated_data(body + 5, buffer, &now_loc, 
                                                         buffer_size, 4, check_only, 
                                                         line, linebuf);
                if (check_only)
                    now_loc += count * 4;
            }
            else if (0 == strcmp(cmd, "dword"))
            {
                int count = parse_comma_separated_data(body + 6, buffer, &now_loc, 
                                                         buffer_size, 8, check_only, 
                                                         line, linebuf);
                if (check_only)
                    now_loc += count * 8;
            }
            else if (0 == strcmp(cmd, "qword"))
            {
                fatal(9505); // FIXME: 定义128位数
            }
            else if (0 == strcmp(cmd, "align"))
            {
                if (is_delayslot)
                {
                    is_delayslot = 0;
                    awarn(line, 4120, linebuf);
                }
                unsigned long long align = 0;
                if (sscanf(body + 6, "%llu", &align) != 1)
                    aerror(line, 3100, linebuf);
                
                // align的值必须是2的幂次方
                if (align == 0 || (align & (align - 1)) != 0)
                    aerror(line, 9003, linebuf);

                align -= 1;
                now_loc = (now_loc + align) & ~align;
            }
            else if (0 == strcmp(cmd, "ascii"))
            {
                char *sta = str_first(linebuf, '"');
                if(sta == NULL)
                    aerror(line, 3109, linebuf);
                else
                    ++sta;
                while (*sta != '\0' && *sta != '"') // 处理字符串
                {
                    if (*sta == '\\')
                    {
                        int esc_len = process_escape_char(sta, 
                            (check_only ? NULL : (buffer + now_loc)), 
                            check_only);
                        if (esc_len < 0) {
                            aerror(line, 4101, linebuf);  // 未知转义序列
                            break;
                        }
                        if (!check_only && now_loc >= buffer_size) FATAL(7000);
                        now_loc++;
                        sta += esc_len;
                    }
                    else if (*sta == '"')
                    {
                        break;
                    }
                    else
                    {
                        if (!check_only) {
                            if (now_loc >= buffer_size) FATAL(7000);
                            *(buffer + now_loc) = *sta;
                        }
                        now_loc++;
                        sta++;
                    }
                }
                if (*sta != '"')
                    aerror(line, 4101, linebuf);
                
                // 应用全局对齐
                if (galign > 0) {
                    now_loc = (now_loc + galign) & ~galign;
                }
            }
            else if (0 == strcmp(cmd, "asciiz"))
            {
                char *sta = str_first(linebuf, '"');
                if(sta == NULL)
                    aerror(line, 3109, linebuf);
                else
                    ++sta;
                while (*sta != '\0' && *sta != '"') // 处理字符串
                {
                    if (*sta == '\\')
                    {
                        int esc_len = process_escape_char(sta, 
                            (check_only ? NULL : (buffer + now_loc)), 
                            check_only);
                        if (esc_len < 0) {
                            aerror(line, 4101, linebuf);  // 未知转义序列
                            break;
                        }
                        if (!check_only && now_loc >= buffer_size) FATAL(7000);
                        now_loc++;
                        sta += esc_len;
                    }
                    else if (*sta == '"')
                    {
                        break;
                    }
                    else
                    {
                        if (!check_only) {
                            if (now_loc >= buffer_size) FATAL(7000);
                            *(buffer + now_loc) = *sta;
                        }
                        now_loc++;
                        sta++;
                    }
                }
                if (*sta != '"')
                    aerror(line, 4101, linebuf);
                
                // 添加空终止符
                if (!check_only) {
                    if (now_loc >= buffer_size) FATAL(7000);
                    *(buffer + now_loc) = '\0';
                }
                now_loc++;
                
                // 应用全局对齐
                if (galign > 0) {
                    now_loc = (now_loc + galign) & ~galign;
                }
            }
            else if (0 == strcmp(cmd, "galign"))
            {
                WARN(9506);
                unsigned long long g = 0;
                if (sscanf(body + 7, "%llu", &g) != 1)
                    aerror(line, 4102, linebuf);
                else {
                    if (g == 0)
                        afatal(line, 4199, linebuf);
                    if (g % 2 != 0 && g != 1)
                        awarn(line, 4103, linebuf);

                    if (g & (g - 1)) // 不是2的幂次方
                        awarn(line, 9003, linebuf);

                    galign = g - 1;
                }
            }
            else if (0 == strcmp(cmd, "struct"))
            {
                // .struct StructName type1 field1, type2 field2, ...
                if (check_only)
                {
                    const char *p = body + 7;
                    p = str_first_not(p, '\r');
                    
                    // 读取结构体名称
                    char struct_name[128];
                    int name_len = 0;
                    while (p && *p && !isspace(*p) && name_len < 127)
                    {
                        struct_name[name_len++] = *p++;
                    }
                    struct_name[name_len] = '\0';
                    
                    if (name_len == 0)
                    {
                        aerror(line, 9010, linebuf);  // 结构体名称为空
                        continue;
                    }
                    
                    // 检查是否已定义
                    if (find_struct_def(struct_name) != NULL)
                    {
                        aerror(line, 9011, linebuf);  // 结构体重复定义
                        continue;
                    }
                    
                    struct_def_t new_struct;
                    strcpy(new_struct.name, struct_name);
                    new_struct.total_size = 0;
                    
                    // 解析字段定义：type1 field1, type2 field2, ...
                    p = str_first_not(p, '\r');
                    while (p && *p)
                    {
                        // 读取类型名
                        char typename_str[128];
                        int type_len = 0;
                        while (p && *p && !isspace(*p) && *p != ',' && type_len < 127)
                        {
                            typename_str[type_len++] = *p++;
                        }
                        typename_str[type_len] = '\0';
                        
                        if (type_len == 0)
                            break;
                        
                        int field_size = get_field_size_by_typename(typename_str);
                        if (field_size < 0)
                        {
                            aerror(line, 9012, linebuf);  // 未知字段类型
                            break;
                        }
                        
                        p = str_first_not(p, '\r');
                        
                        // 读取字段名（可选）
                        struct_field_t field;
                        field.size = field_size;
                        field.offset = new_struct.total_size;
                        field.name[0] = '\0';
                        
                        if (p && *p && *p != ',')
                        {
                            int field_name_len = 0;
                            while (p && *p && !isspace(*p) && *p != ',' && field_name_len < 127)
                            {
                                field.name[field_name_len++] = *p++;
                            }
                            field.name[field_name_len] = '\0';
                        }
                        
                        new_struct.fields.push_back(field);
                        new_struct.total_size += field_size;
                        
                        // 跳过逗号
                        p = str_first_not(p, '\r');
                        if (p && *p == ',')
                            p++;
                        p = str_first_not(p, '\r');
                    }
                    
                    if (new_struct.fields.size() == 0)
                    {
                        aerror(line, 9013, linebuf);  // 结构体没有字段
                    }
                    else
                    {
                        struct_defs.push_back(new_struct);
                        debug("Defined struct %s with %d fields, total size %d\\n", 
                              new_struct.name, (int)new_struct.fields.size(), new_struct.total_size);
                    }
                }
                // 在第二遍中不需要处理
            }
            else if (0 == strcmp(cmd, "offset"))
            {
                if (sscanf(body + 7, "%llX", &offset) != 1)
                    aerror(line, 4104, linebuf);
            }
            else if (0 == strcmp(cmd, "loc"))
            {
                if(is_delayslot)
                {
                    is_delayslot = 0;
                    awarn(line, 4120, linebuf);
                }
                unsigned long long tmp1, tmp2;
                int rst = sscanf(body + 4, "%llX,%llX", &tmp1, &tmp2);
                if (rst == 1)
                {
                    now_loc = tmp1;
                    warn_loc = ~0x0;
                }
                else if (rst == 2)
                {
                    now_loc = tmp1;
                    warn_loc = tmp2;
                }
                else
                {
                    aerror(line, 4105, linebuf);
                }
            }
            else if (0 == strcmp(cmd, "bloc"))
            {
                if(is_delayslot)
                {
                    is_delayslot = 0;
                    awarn(line, 4120, linebuf);
                }
                unsigned long long tmp1, tmp2;
                if (sscanf(body + 5, "%llX,%llX", &tmp1, &tmp2) != 2)
                {
                    aerror(line, 4106, linebuf);
                }
                else
                {
                    now_loc = tmp1;
                    warn_loc = tmp1 + tmp2;
                }
            }
            else if (0 == strcmp(cmd, "vma"))
            {
                if(is_delayslot)
                {
                    is_delayslot = 0;
                    awarn(line, 4120, linebuf);
                }
                unsigned long long tmp1, tmp2;
                int rst = sscanf(body + 4, "%llX,%llX", &tmp1, &tmp2);
                if (rst == 1)
                {
                    now_loc = tmp1 - offset;
                    warn_loc = ~0x0;
                }
                else if (rst == 2)
                {
                    now_loc = tmp1 - offset;
                    warn_loc = tmp2 - offset;
                }
                else
                {
                    aerror(line, 4108, linebuf);
                }
            }
            else if (0 == strcmp(cmd, "file"))
            {
                int rst = sscanf(body + 6, "%s %d", scriptnm, &line);
                if (rst == 1)
                {
                    line = 0;
                }
                else if (rst != 2)
                {
                    aerror(line, 4111, linebuf);
                }
                else line -= 1;
            }
            else if (0 == strcmp(cmd, "pushloc"))
            {
                loc_stack.push(now_loc);
            }
            else if (0 == strcmp(cmd, "poploc"))
            {
                if (is_delayslot)
                {
                    is_delayslot = 0;
                    awarn(line, 4120, linebuf);
                }
                if(loc_stack.empty())
                    aerror(line, 4112, linebuf);
                else
                {
                    now_loc = loc_stack.top();
                    loc_stack.pop();
                }
            }
            else
            {
                aerror(line, 4110, linebuf);
            }
        }

        else
        {
            if (check_only)
            {
                int len = get_asm_len(body);
                if(len == -1) aerror(line, 3200, linebuf);
                else now_loc += len;
            }

            else
            {
                if (now_loc >= buffer_size) FATAL(7000);
                int ret = mkasm(buffer + now_loc, body, now_loc + offset);
                if(ret == -1) aerror(line, 8000, linebuf);
                else if(ret == -2) aerror(line, 8001, linebuf);
                else if(ret == -3) aerror(line, 8002, linebuf);
                else if(ret == -4) aerror(line, 4201, linebuf);
                else if(ret == -5) aerror(line, 4203, linebuf);
                else if(ret == -6) aerror(line, 4204, linebuf);
                else if(ret == -1000) aerror(line, 4200, linebuf);
                else now_loc += ret;
            }
        }
    }
    if(is_delayslot)
        awarn(line, 4121, linebuf);

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
    outputnm = strdup(filename);
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

    script = fopen(filename, "r");
    // scriptnm = strdup(filename);
    strcpy(scriptnm, filename);
    if (script == NULL)
    {
        int err = errno;
        error(1902);
        error(1110);
        puts(strerror(err));
        return 2;
    }
    return 0;
}

int handle_W(const char *warnspec)
{
    if (warnspec == NULL)
        fatal(1901);

    const char *p = warnspec;
    while (*p != '\0')
    {
        while (*p == ' ' || *p == '\t' || *p == ',')
            ++p;
        if (*p == '\0')
            break;

        if (*p == 'W' || *p == 'w')
            ++p;

        int code = 0;
        int digits = 0;
        while (*p >= '0' && *p <= '9')
        {
            code = code * 10 + (*p - '0');
            ++p;
            ++digits;
        }

        if (digits == 0)
            return 1900;

        if (!is_warning_suppressed(code))
            suppressed_warn_codes.push_back(code);

        while (*p == ' ' || *p == '\t')
            ++p;
        if (*p == ',')
        {
            ++p;
            continue;
        }
        if (*p != '\0')
            return 1900;
    }

    return 0;
}

int handle_D(const char *hex)
{
    if (hex == NULL)
        fatal(1901);
    char buf[128];
    unsigned long data;
    sscanf(hex, "%lX", &data);
    instr_t inst = DecodeInstruction(data);
    disas(inst, 0, buf);
    puts(buf);
    return 0;
}

int handle_A(const char *asmb)
{
    if (asmb == NULL)
        fatal(1901);
    instr_t ans;
    int ret = as(asmb, 0, &ans);
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
    printf("%08X\n", EncodeInstruction(ans));
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
    {1200, "未知的输出偏好选项。输出未定义数时所选方案未知。检查 -d 选项值。"},
    {1500, "将忽略新指定的文件。"},
    {1902, "命令行参数解析时发生错误，请留意下方信息。"},
    {1901, "命令行参数解析时出错：需求的参数不完整。"},
    {1900, "命令行参数解析时发生错误，请留意上方信息。"},
    {2000, "反汇编失败。-d 参数错误。"},
    {2001, "输入文件未指定。"},
    {2002, "输出文件未指定。"},
    {2003, "脚本文件未指定。"},
    {2004, "脚本文件读取时发生意外错误。"},
    {3000, "指定了过多的标签（默认16384）。如需更大的空间，请重新编译，并指定更大的LABEL_MAX_COUNT。"},
    {3001, "标签长度超出许可范围（128）。"},
    {3002, "标签中有空格。"},
    {3003, "标签中有制表符。"},
    {3004, "字符串定义中有换行符"},
    {3100, "一次解析时发生异常。对齐指令参数数量不正确。"},
    {3101, "一次解析时发生异常。字符串定义中有换行符。"},
    {3102, "一次解析时发生异常。字符串自对齐指令参数不正确。期待一个10进制数。"},
    {3103, "一次解析时发现异常。字符串自对齐指令参数非2的倍数。"},
    {3104, "一次解析时发生异常。偏移定义指令参数不正确。期待一个16进制整数。"},
    {3105, "一次解析时发生异常。位置定义语句参数不正确。期待1～2个16进制整数。"},
    {3106, "一次解析时发生异常。容纳块定义语句参数不正确。期待2个16进制整数。"},
    {3107, "一次解析时发生异常。空汇编器指令。"},
    {3108, "一次解析时发生异常。虚拟地址定义语句参数不正确。期待1～2个16进制整数。"},
    {3109, "一次解析时发生异常。字符串定义中缺少起始或终止引号。"},
    {3110, "一次解析时发生异常。汇编器指令解析失败。不是有效的汇编器控制指令。"},
    {3199, "一次解析时遭遇致命错误。字符串自对齐指令参数被指定为0。"},
    {3200, "一次解析时发生异常。汇编指令解析失败。"},
    {3900, "一次解析时发生异常。容纳块溢出。"},
    {3910, "一次解析时发现异常。换行符未能取得CR。检查文件格式或操作系统。"},
    {4100, "二次解析时发生异常。对齐指令参数数量不正确。"},
    {4101, "二次解析时发生异常。字符串定义中有换行符。"},
    {4102, "二次解析时发生异常。字符串自对齐指令参数不正确。期待一个10进制数。"},
    {4103, "二次解析时发现异常。字符串自对齐指令参数非2的倍数。"},
    {4104, "二次解析时发生异常。偏移定义指令参数不正确。期待一个16进制整数。"},
    {4105, "二次解析时发生异常。位置定义语句参数不正确。期待1～2个16进制整数。"},
    {4106, "二次解析时发生异常。容纳块定义语句参数不正确。期待2个16进制整数。"},
    {4107, "二次解析时发生异常。空汇编器指令。"},
    {4108, "二次解析时发生异常。虚拟地址定义语句参数不正确。期待1～2个16进制整数。"},
    {4109, "二次解析时发现异常。定义的数据大小超出定义空间极限。"},
    {4110, "二次解析时发生异常。汇编器指令解析失败。不是有效的汇编器控制指令。"},
    {4111, "解析时失败。.file 指令的参数数目不符合预期。"},
    {4112, "解析时失败。.poploc 指令在空栈上操作。"},
    {4120, "二次解析时发现异常。改变处理位置时仍在等待解析延迟槽指令。检查最后的跳转指令延迟槽定义。"},
    {4121, "二次解析时发现异常。处理结束时仍在等待解析延迟槽指令。检查最后的跳转指令延迟槽定义。"},
    {4130, "二次解析时发现异常。数字定义格式不正确或没有定义。"},
    {4131, "二次解析时发现异常。尝试解析标签时未能取得标签起始。"},
    {4132, "二次解析时发现异常。尝试解析标签时发现标签不存在。"},
    {4199, "二次解析时遭遇致命错误。字符串自对齐指令参数被指定为0。"},
    {4200, "二次解析时发现异常。汇编指令解析完成时指令行未穷尽。"},
    {4201, "二次解析时发现异常。所指定的标签不存在。"},
    {4203, "二次解析时发现异常。分支延迟槽中的跳转指令。"},
    {4204, "二次解析时发现异常。延迟槽中禁止使用会展开为多条指令的伪指令。请手动展开为真实指令。"},
    {4900, "二次解析时发生异常。容纳块溢出。"},
    {4910, "二次解析时发现异常。换行符未能取得LF。检查文件格式或操作系统。"},
    {7000, "缓冲区错误。写错误：指定的地址超过了文件大小或所分配的缓冲区。"},
    {8000, "无效汇编指令。"},
    {8001, "未知寄存器名。"},
    {8002, "命令语法不正确。"},
    {9001, "处理LI指令时，所给数值超过可处理的极限。"},
    {9002, "处理对齐请求时，所给数值超过可处理的极限。"},
    {9003, "处理对齐请求时，所给数值不是2的幂。"},
    {9010, "结构体定义错误：结构体名称为空。"},
    {9011, "结构体定义错误：结构体名称已存在，不能重复定义。"},
    {9012, "结构体定义错误：未知的字段类型。有效类型：byte, half, word, dword。"},
    {9013, "结构体定义错误：结构体必须至少包含一个字段。"},
    {9501, "LI伪指令对负数情况未实现。"},
    {9505, ".qword（128位数定义）功能未实现。"},
    {9506, "galign 只对字符串定义有效。不建议使用。"},
    {9999, "索引未命中。"}};

void error(int code)
{
    ++cerror;
    for (int i = 0; i < (int)sizeof(errs)/(int)sizeof(err_t); ++i)
    {
        if (errs[i].code == code)
        {
            printf("E%04d %s\n", code, errs[i].msg);
            if (stop_firsterr)
                exit(code);
            return;
        }
    }
    fatal(9999);
}

void fatal(int code)
{
    for (int i = 0; i < (int)sizeof(errs)/(int)sizeof(err_t); ++i)
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
    if (is_warning_suppressed(code))
        return;

    ++cwarn;
    for (int i = 0; i < (int)sizeof(errs)/(int)sizeof(err_t); ++i)
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
    for (int i = 0; i < (int)sizeof(errs)/(int)sizeof(err_t); ++i)
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
    puts("Partially assembler for MIPS R5900.\n    Coded by Xiyan_shan");
    puts("部分汇编补丁应用器，RX79专版。\n    编写者：单希研");
    puts("Version 1.2.0");
    puts("用法 Usage: prtasm (-m [a/d]) -i [input.bin] -o [output.bin] (-s [script.txt] (Options))");
    puts("选项 Options: ");
    puts("-A [asm code]    [测试]立即转换汇编代码到十六进制数据。");
    puts("-D [hex code]    [测试]立即反汇编一个十六进制数。");
    puts("-i [input.bin]   指定输入文件。");
    puts("-o [output.bin]  指定输出文件。");
    puts("-s [script.txt]  指定脚本文件。");
    puts("-S               从标准输入读取脚本（仅当模式为汇编时可用）。");
    puts("-l [range]       指定反汇编限定地址(格式：xxx:xxx，16 进制)。（仅当模式为反汇编时可用）");
    puts("-m [a/d]         指定为汇编或反汇编模式。（反汇编时，选项 -s 被忽略。）");
    puts("-d [b/h/w]       将无法处理的指令值以.byte/.half/.word表示。（默认为.word）");
    //puts("-I               [未实现]指定为交互模式(忽略“-s”选项)。");
    puts("-n               将无法解析的指令值输出为INVALID而非d*。");
    puts("-W [code,...]    抑制指定警告码（可重复，支持逗号分隔，如 -W 4103,4121 或 -W W4900）。");
    puts("-r               当越界（如果有指定）时，停止汇编操作。");
    puts("-e               当第一个错误发生时，停止运行。");
    puts("--move-only-word move伪指令只移动低32位寄存器（生成ADDU而非DADDU）");
    // puts("--no-purge      指定不要复制输入文件到输出文件。");
    puts("例: \n\
    prtasm -i test.elf -o mod.elf -s mod.mips\n\
    prtasm -m d -i test.elf -o dis.mips -l 1050:135c --invalid-ops\n\
    prtasm -m a -i test.elf -o mod.elf -s mod.mips --no-purge");
    return 0;
}