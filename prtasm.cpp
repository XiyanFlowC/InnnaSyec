#include "r5900.hpp"
#include "liteopt.h"
#include <errno.h>
#include <string.h>
// #include <string>
// #include <stack>
// #include <vector>
// #include <map>
#include <stdio.h>

int get_term2(char *dst, const char *src)
{
    int i = 0;
    while(*src != '\0' && *src != '\t' && *src != ' ')
    {
        *dst++ = *src++;
        ++i;
    }
    *dst = '\0';
    return i;
}

// 同时过滤空白字符（' ', '\\n', '\\t'）
char *str_first_not(const char *src, const char ch)
{
    while (*src != '\0' && (*src == ' ' || *src == '\n' || *src == '\t' || *src == ch))
        ++src;
    if (*src == '\0')
        return NULL;
    return (char *)src;
}

char *str_first(const char *src, const char ch)
{
    while (*src != '\0' && *src != ch)
        ++src;
    if (*src == '\0')
        return NULL;
    return (char *)src;
}

char *str_last(const char *src, const char ch)
{
    const char *end = src;
    src += strlen(src);
    while (src != end && *src != ch)
        --src;
    if (*src != ch)
        return NULL;
    return (char *)src;
}

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
static FILE *input, *output, *script;
static char *scriptnm, *outputnm;
static bool do_invalid = false /*, do_purge = true*/, stop_if_overline = false, stop_firsterr = false;
int cerror = 0, cwarn = 0;

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
int check();
int genasm(unsigned char *buffer);

int main(int argc, const char **argv)
{
    lopt_regopt("stop-if-out-of-range", 'r', LOPT_FLG_CH_VLD | LOPT_FLG_OPT_VLD | LOPT_FLG_STR_VLD,
                [](const char *ans) -> int
                {
                    stop_if_overline = true;
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
            instr_t result = disasm(cmd);
            if (result.opcode == INVALID)
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
                printdis(strbuf, result);
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
        if (0 == check())
            genasm(buf);

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
    free(scriptnm);
    free(outputnm);

    return 0;
}

void aerror(int linec, int code, const char *line)
{
    printf("(%s:%d) %s\n\t", scriptnm, linec, line);
    error(code);
}

void afatal(int linec, int code, const char *line)
{
    printf("(%s:%d) %s\n\t", scriptnm, linec, line);
    fatal(code);
}

void awarn(int linec, int code, const char *line)
{
    printf("(%s:%d) %s\n\t", scriptnm, linec, line);
    warn(code);
}

int get_asm_len(const char *src)
{
    char mnemonic[128];
    src = str_first_not(src, '\r');
    int eles = sscanf(src, "%s", mnemonic);
    if(inst_id_bynm(mnemonic) != -1) return 4;
    if(0 == strcmp(mnemonic, "LI"))
    {
        char *para = str_first_not(src + 2, '\r');
        instr_t tmp;
        if(parse_param(para, "$t, #", &tmp) < 0) return -1;
        if(tmp.imm > 0x7fff || tmp.imm < -0x8000) return 8;
        return 4;
    }
    else if(0 == strcmp(mnemonic, "LA"))
    {
        return 8;
    }
    else return -1;
}

#ifndef LABEL_MAX_COUNT
#define LABEL_MAX_COUNT 512
#endif
unsigned long long tag_loc[LABEL_MAX_COUNT], tag_vma[LABEL_MAX_COUNT];
char tag_nm[LABEL_MAX_COUNT][128];
int tag_p = 0;

int check()
{
    static char linebuf[4096];
    unsigned long long now_loc = 0, galign = 0, warn_loc = ~0x0;
    long long offset = 0;
    int line = 1;
    while (EOF != fscanf(script, "%[^\n]", linebuf))
    {
        int t = fgetc(script);
        if('\n' != t && t != EOF) awarn(line, 3910, linebuf);

        int flg = 0;
        char *p = linebuf, *body = linebuf;

        if (now_loc >= warn_loc)
        {
            if (stop_if_overline)
                afatal(line, 3900, linebuf);
            else
                awarn(line, 3900, linebuf);
            warn_loc = ~0x0;
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

        if (body != linebuf) // 有标签，要处理
        {
            char *str = str_first_not(linebuf, '\r');
            int count;
            if (tag_p >= LABEL_MAX_COUNT - 1)
                afatal(line, 3000, linebuf);
            tag_loc[tag_p] = now_loc;
            tag_vma[tag_p] = now_loc + offset;
            if ((count = get_term(tag_nm[tag_p], str, ':')) >= 128)
                afatal(line, 3001, linebuf);
            if (count != count_term(tag_nm[tag_p], ' '))
                aerror(line, 3002, linebuf);
            if (count != count_term(tag_nm[tag_p++], '\t'))
                aerror(line, 3003, linebuf);
        }

        // 常规指令字处理
        if ((body = str_first_not(body, '\r')) == NULL)//空行
        {
            ++line;
            continue;
        }

        else if (body[0] == '.') // 汇编器控制指令
        {
            char cmd[128];
            if(get_term2(cmd, body + 1) == 0)
            {
                awarn(line, 3107, linebuf);
            }
            else if (0 == strcmp(cmd, "byte"))
                now_loc += 1;
            else if (0 == strcmp(cmd, "half"))
                now_loc += 2;
            else if (0 == strcmp(cmd, "word"))
                now_loc += 4;
            else if (0 == strcmp(cmd, "dword"))
                now_loc += 8;
            else if (0 == strcmp(cmd, "qword"))
                now_loc += 16;
            else if (0 == strcmp(cmd, "align"))
            {
                unsigned long long align = 0;
                if (sscanf(body + 6, "%llu", &align) != 1)
                    aerror(line, 3100, linebuf);
                align -= 1;
                now_loc = (now_loc + align) & ~align;
            }
            else if (0 == strcmp(cmd, "ascii"))
            {
                char *sta = str_first(linebuf, '"');
                if(sta == NULL)
                    aerror(line, 3108, linebuf);
                else
                    ++sta;
                int leng = 0;
                while (*sta != '\0') // 处理字符串
                {
                    if (*sta == '\\')
                    {
                        if (*++sta == 'x')
                        {
                            sta += 2;
                        }
                        ++sta;
                        ++leng;
                        continue;
                    }
                    if (*sta == '"')
                        break;
                    ++sta, ++leng;
                    continue;
                }
                if (*sta == '\0')
                    aerror(line, 3101, linebuf);
                now_loc += leng;
                now_loc = (now_loc + galign) & ~galign;
            }
            else if (0 == strcmp(cmd, "asciiz"))
            {
                char *sta = str_first(linebuf, '"');
                if(sta == NULL)
                    aerror(line, 3108, linebuf);
                else
                    ++sta;
                int leng = 0;
                while (*sta != '\0') // 处理字符串
                {
                    if (*sta == '\\')
                    {
                        if (*++sta == 'x')
                        {
                            sta += 2;
                        }
                        ++sta;
                        ++leng;
                        continue;
                    }
                    if (*sta == '"')
                        break;
                    ++sta, ++leng;
                    continue;
                }
                if (*sta == '\0')
                    aerror(line, 3101, linebuf);
                now_loc += leng + 1; // 末尾的填充零
                now_loc = (now_loc + galign) & ~galign;
            }
            else if (0 == strcmp(cmd, "galign"))
            {
                if (sscanf(body + 7, "%lld", &galign) != 1)
                    aerror(line, 3102, linebuf);
                if (galign % 2 != 0 && galign != 1)
                    awarn(line, 3103, linebuf);
                if (galign == 0)
                    afatal(line, 3199, linebuf);
                galign -= 1;
            }
            else if (0 == strcmp(cmd, "offset"))
            {
                if (sscanf(body + 7, "%llX", &offset) != 1)
                    aerror(line, 3104, linebuf);
            }
            else if (0 == strcmp(cmd, "loc"))
            {
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
                    aerror(line, 3105, linebuf);
                }
            }
            else if (0 == strcmp(cmd, "bloc"))
            {
                unsigned long long tmp1, tmp2;
                if (sscanf(body + 5, "%llX,%llX", &tmp1, &tmp2) != 2)
                {
                    aerror(line, 3106, linebuf);
                }
                else
                {
                    now_loc = tmp1;
                    warn_loc = tmp1 + tmp2;
                }
            }
            else if (0 == strcmp(cmd, "vma"))
            {
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
                    aerror(line, 3108, linebuf);
                }
            }
            else
            {
                aerror(line, 3110, linebuf);
            }
        }

        else
        {
            int len = get_asm_len(body);
            if(len == -1) aerror(line, 3200, linebuf);
            else now_loc += len;
        }

        linebuf[0] = '\0';
        ++line;
    }

    rewind(script);
    return 0;
}

int is_lbinst(char *nm)
{
    strupr(nm);
    if(nm[0] == 'J') return 1;
    if(nm[0] == 'B' && nm[1] != 'R') return 1; // r for break
    return 0;
}

int mkasm(unsigned char *buf, char *asmb)
{
    char mnemonic[64];
    sscanf(asmb, "%s", mnemonic);
    if(is_lbinst(mnemonic))
    {
        char *lbl = str_last(asmb, ',');
        if(lbl == NULL) 
        {
            lbl = str_first(asmb, ' ');
            if(lbl == NULL) return -3;
        }
        lbl = str_first_not(lbl + 1, '\r');
        if(lbl == NULL) return -3;
        for(int i = 0; i < tag_p; ++i)
        {
            if(0 == strcmp(tag_nm[i], lbl))
            {
                sprintf(lbl, "%llu", tag_vma[i]);
                goto mkasm_nparse;
            }
        }
        return -4;
    }
    mkasm_nparse:

    instr_t ans;
    int ret = parse_asm(asmb, &ans);
    if(ret < -1) return ret;
    if(ret == -1) // 非真指令的处理（伪指令判别）
    {
        char mnemonic[128];
        asmb = str_first_not(asmb, '\r');
        int eles = sscanf(asmb, "%s", mnemonic);
        strupr(mnemonic);

        if(strcmp(mnemonic, "LI") == 0)
        {
            char *para = str_first_not(asmb + 2, '\r');
            instr_t tmp;
            if(parse_param(para, "$t, #", &tmp) < 0) return -1;
            if(tmp.imm > 0x7fff)
            {
                int low = tmp.imm & 0xffff;
                tmp.opcode = LUI;
                tmp.imm >>= 16;
                if(low > 0x7fff) tmp.imm += 1;
                *((unsigned int *)buf) = asmble(tmp);
                buf += 4;
                tmp.opcode = ADDIU;
                tmp.rs = zero;
                tmp.imm = low > 0x7fff ? 0x10000 - low : low;
                *((unsigned int *)buf) = asmble(tmp);
                return 8;
            }
            if(tmp.imm < -0x8000)
            {
                error(9501);
                int low = tmp.imm & 0xffff;
                return 8;
            }
            tmp.opcode = ADDIU;
            tmp.rs = zero;
            *((unsigned int *)buf) = asmble(ans);
            return 4;
        }
        else if(strcmp(mnemonic, "LA") == 0)
        {
            char *lbl = str_first(asmb + 2, ',');
            if(lbl == NULL) return -3;
            lbl = str_first_not(lbl + 1, '\r');
            if(lbl == NULL) return -3;
            for(int i = 0; i < tag_p; ++i)
            {
                if(0 == strcmp(tag_nm[i], lbl))
                {
                    sprintf(lbl, "%llu", tag_vma[i]);
                    parse_param(str_first_not(asmb + 2, '\r'), "$t, #", &ans);
                    int low = ans.imm & 0xffff;
                    ans.opcode = LUI;
                    ans.imm >>= 16;
                    if(low > 0x7fff) ans.imm += 1;
                    *((unsigned int *)buf) = asmble(ans);
                    buf += 4;
                    ans.opcode = ADDIU;
                    ans.rs = zero;
                    ans.imm = low > 0x7fff ? 0x10000 - low : low;
                    *((unsigned int *)buf) = asmble(ans);
                    return 8;
                }
            }
            return -4;
        }
    }
    else
    {
        if(ret != strlen(asmb)) return -1000;
        *((unsigned int *)buf) = asmble(ans);
        return 4;
    }
    return -3;
}

int genasm(unsigned char *buffer)
{
    static char linebuf[4096];
    unsigned long long now_loc = 0, galign = 0, warn_loc = ~0x0;
    long long offset = 0;
    int line = 1;
    while (EOF != fscanf(script, "%[^\n]", linebuf))
    {
        int t = fgetc(script);
        if('\n' != t && t != EOF) awarn(line, 4910, linebuf);

        int flg = 0;
        char *p = linebuf, *body = linebuf;

        if (now_loc >= warn_loc)
        {
            if (stop_if_overline)
                afatal(line, 4900, linebuf);
            else
                awarn(line, 4900, linebuf);
            warn_loc = ~0x0;
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

        // // 标签处理。: 因为目前所有数字都是位于末尾且只有一个，姑且先这么瞎搞一下：
        // for(int i = 0; i < tag_p; ++i)
        // {
        //     char *pos;
        //     pos = strstr(body, tag_nm[i]);
        //     if(pos != NULL)
        //     {
        //         sprintf(pos, "0x%llX", tag_vma[i]);
        //     }
        // }

        // 常规指令字处理
        if ((body = str_first_not(body, '\r')) == NULL)//空行
        {
            ++line;
            continue;
        }

        else if (body[0] == '.') // 汇编器控制指令
        {
            static char cmd[128];
            static union
            {
                unsigned char u8;
                unsigned short u16;
                unsigned int u32;
                unsigned long long u64;
            } tmp;
            
            if(get_term2(cmd, body + 1) == 0)
            {
                awarn(line, 4107, linebuf);
            }
            else if (0 == strcmp(cmd, "byte"))
            {
                sscanf(body + 5, "%hhu", &tmp.u8);
                *(buffer + now_loc) = tmp.u8;
                now_loc += 1;
            }
            else if (0 == strcmp(cmd, "half"))
            {
                sscanf(body + 5, "%hu", &tmp.u16);
                *((unsigned short *)(buffer + now_loc)) = tmp.u16;
                now_loc += 2;
            }
            else if (0 == strcmp(cmd, "word"))
            {
                sscanf(body + 5, "%u", &tmp.u32);
                *((unsigned int *)(buffer + now_loc)) = tmp.u32;
                now_loc += 4;
            }
            else if (0 == strcmp(cmd, "dword"))
            {
                sscanf(body + 5, "%llu", &tmp.u64);
                *((unsigned long long *)(buffer + now_loc)) = tmp.u64;
                now_loc += 8;
            }
            else if (0 == strcmp(cmd, "qword"))
            {
                fatal(9505); // FIXME: 定义128位数
            }
            else if (0 == strcmp(cmd, "align"))
            {
                unsigned long long align = 0;
                if (sscanf(body + 6, "%llu", &align) != 1)
                    aerror(line, 3100, linebuf);
                align -= 1;
                now_loc = (now_loc + align) & ~align;
            }
            else if (0 == strcmp(cmd, "ascii"))
            {
                char *sta = str_first(linebuf, '"');
                if(sta == NULL)
                    aerror(line, 3108, linebuf);
                else
                    ++sta;
                while (*sta != '\0') // 处理字符串
                {
                    if (*sta == '\\')
                    {
                        if (*++sta == 'x')
                        {
                            char a = *++sta;
                            char b = *++sta;
                            if(a >= 'a' && a <= 'f') a = a - 'a' + 'A';
                            if(b >= 'a' && b <= 'f') b = b - 'a' + 'A';
                            *(buffer + now_loc++) = (a<<4) | b;
                        }
                        if(*sta == 'n')
                        {
                            *(buffer + now_loc++) = '\n';
                        }
                        if(*sta == '\\')
                        {
                            *(buffer + now_loc++) = '\\';
                        }
                        if(*sta == 'r')
                        {
                            *(buffer + now_loc++) = '\r';
                        }
                        if(*sta == '0')
                        {
                            *(buffer + now_loc++) = '\0';
                        }
                        ++sta;
                        continue;
                    }
                    if (*sta == '"')
                        break;
                    *(buffer + now_loc++) = *sta++;
                    continue;
                }
                if (*sta == '\0')
                    aerror(line, 4101, linebuf);
                now_loc = (now_loc + galign) & ~galign;
            }
            else if (0 == strcmp(cmd, "asciiz"))
            {
                char *sta = str_first(linebuf, '"');
                if(sta == NULL)
                    aerror(line, 3108, linebuf);
                else
                    ++sta;
                while (*sta != '\0') // 处理字符串
                {
                    if (*sta == '\\')
                    {
                        if (*++sta == 'x')
                        {
                            char a = *++sta;
                            char b = *++sta;
                            if(a >= 'a' && a <= 'f') a = a - 'a' + 'A';
                            if(b >= 'a' && b <= 'f') b = b - 'a' + 'A';
                            *(buffer + now_loc++) = (a<<4) | b;
                        }
                        if(*sta == 'n')
                        {
                            *(buffer + now_loc++) = '\n';
                        }
                        if(*sta == '\\')
                        {
                            *(buffer + now_loc++) = '\\';
                        }
                        if(*sta == 'r')
                        {
                            *(buffer + now_loc++) = '\r';
                        }
                        if(*sta == '0')
                        {
                            *(buffer + now_loc++) = '\0';
                        }
                        ++sta;
                        continue;
                    }
                    if (*sta == '"')
                        break;
                    *(buffer + now_loc++) = *sta++;
                    continue;
                }
                if (*sta == '\0')
                    aerror(line, 4101, linebuf);
                *(buffer + now_loc++) = '\0';
                now_loc = (now_loc + galign) & ~galign;
            }
            else if (0 == strcmp(cmd, "galign"))
            {
                if (sscanf(body + 7, "%lld", &galign) != 1)
                    aerror(line, 4102, linebuf);
                if (galign % 2 != 0 && galign != 1)
                    awarn(line, 4103, linebuf);
                if (galign == 0)
                    afatal(line, 4199, linebuf);
                galign -= 1;
            }
            else if (0 == strcmp(cmd, "offset"))
            {
                if (sscanf(body + 7, "%llX", &offset) != 1)
                    aerror(line, 4104, linebuf);
            }
            else if (0 == strcmp(cmd, "loc"))
            {
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
            else
            {
                aerror(line, 4110, linebuf);
            }
        }

        else
        {
            int ret = mkasm(buffer + now_loc, body);
            if(ret == -1) aerror(line, 8000, linebuf);
            else if(ret == -2) aerror(line, 8001, linebuf);
            else if(ret == -3) aerror(line, 8002, linebuf);
            else if(ret == -4) aerror(line, 4201, linebuf);
            else if(ret == -1000) aerror(line, 4200, linebuf);
            else now_loc += ret;
        }

        linebuf[0] = '\0';
        ++line;
    }

    rewind(script);
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
    scriptnm = strdup(filename);
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
    {1200, "未知的输出偏好选项。输出未定义数时所选方案未知。检查 -d 选项值。"},
    {1500, "将忽略新指定的文件。"},
    {1902, "命令行参数解析时发生错误，请留意下方信息。"},
    {1901, "命令行参数解析时出错：需求的参数不完整。"},
    {1900, "命令行参数解析时发生错误，请留意上方信息。"},
    {2000, "反汇编失败。-d 参数错误。"},
    {2001, "输入文件未指定。"},
    {2002, "输出文件未指定。"},
    {2003, "脚本文件未指定。"},
    {3000, "指定了过多的标签（默认512）。如需更大的空间，请重新编译，并指定更大的LABEL_MAX_COUNT。"},
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
    {4110, "二次解析时发生异常。汇编器指令解析失败。不是有效的汇编器控制指令。"},
    {4199, "二次解析时遭遇致命错误。字符串自对齐指令参数被指定为0。"},
    {4200, "二次解析时发现异常。汇编指令解析完成时指令行未穷尽。"},
    {4201, "二次解析时发现异常。所指定的标签不存在。"},
    {4900, "二次解析时发生异常。容纳块溢出。"},
    {4910, "二次解析时发现异常。换行符未能取得CR。检查文件格式或操作系统。"},
    {8000, "无效汇编指令。"},
    {8001, "未知寄存器名。"},
    {8002, "命令语法不正确。"},
    {9001, "处理LI指令时，所给数值超过阈限。"},
    {9501, "LI伪指令对负数情况未实现。"},
    {9505, ".qword（128位数定义）功能未实现。"},
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
    puts("Partically assembler for MIPS R5900.\n    Coded by Xiyan_shan");
    puts("部分汇编补丁应用器，RX79专版。\n    编写者：单希研");
    puts("用法 Usage: prtasm (-m [a/d]) -i [input.bin] -o [output.bin] (-s [script.txt] (Options))");
    puts("选项 Options: ");
    puts("-A [asm code]   [测试]立即转换汇编代码到十六进制数据。");
    puts("-D [hex code]   [测试]立即反汇编一个十六进制数。");
    puts("-i [input.bin]  指定输入文件。");
    puts("-o [output.bin] 指定输出文件。");
    puts("-s [script.txt] 指定脚本文件。");
    puts("-l [range]      指定反汇编限定地址(格式：xxx:xxx，16 进制)。（仅当模式为反汇编时可用）");
    puts("-m [a/d]        指定为汇编或反汇编模式。（反汇编时，选项 -s 被忽略。）");
    puts("-d [b/h/w]      将无法处理的指令值以.byte/.half/.word表示。（默认为.word）");
    puts("-I              [未实现]指定为交互模式(忽略“-s”选项)。");
    puts("-n              将无法解析的指令值输出为INVALID而非d*。");
    puts("-r              当越界（如果有指定）时，停止汇编操作。");
    puts("-e              当第一个错误发生时，停止运行。");
    // puts("--no-purge      指定不要复制输入文件到输出文件。");
    puts("例: \n\
    prtasm -i test.elf -o mod.elf -s mod.mips\n\
    prtasm -m d -i test.elf -o dis.mips -l 1050:135c --invalid-ops\n\
    prtasm -m a -i test.elf -o mod.elf -s mod.mips --no-purge");
    return 0;
}