#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <string>
#include <map>

static char linebuf[4096];
static char tokenbuf[1024];
FILE *out;
FILE *incs[128];
char filenm[128][1024];
int incp = 0;
int linec[128] = {1};

#define ST_NORMAL 0
#define ST_BLOCK 1
#define ST_PBLOCK 2
int stage[128] = {ST_NORMAL};
int stagep = 0;

std::map<std::string, std::string> dict;

int next_word(char *word, const char *buf)
{
    int p = 0, f = 0, q = 0;
    while (buf[p])
    {
        if (!isalpha(buf[p]))
        {
            if (f)
            {
                break;
            }
            else
            {
                ++p;
                continue;
            }
        }
        word[q++] = buf[p++];
    }
    return p;
}

void procline()
{
    std::string x(linebuf);
    for (auto i = dict.begin(); i != dict.end(); ++i)
    {
        size_t loc;
        while (std::string::npos != (loc = x.find(i->first)))
        {
            if (i->second == std::string("[DEFINED]"))
                printf("%s:%d - 符号[%s]仅定义，但没有值。结果可能不正确。\n",
                       filenm[incp], linec[incp], i->first.c_str());
            x.replace(loc, i->first.length(), i->second);
        }
    }
    strcpy(linebuf, x.c_str());
}

void procdef(const char *cmd)
{
    if (stage[stagep] == ST_BLOCK || stage[stagep] == ST_PBLOCK)
        return;
    static char nml[1024];
    int i = sscanf(cmd, "%s %s", tokenbuf, nml);
    if (i == 1)
    {
        dict[std::string(tokenbuf)] = std::string("[DEFINED]");
    }
    else if (i == 2)
    {
        dict[std::string(tokenbuf)] = std::string(nml);
    }
    else
    {
        printf("%s:%d - 符号定义命令数量不正确。\n", filenm[incp], linec[incp]);
    }
}

void procudef(const char *cmd)
{
    if (stage[stagep] == ST_BLOCK || stage[stagep] == ST_PBLOCK)
        return;
    int i = sscanf(cmd, "%s", tokenbuf);
    if (i == 1)
    {
        std::string x(tokenbuf);
        auto loc = dict.find(x);
        if (loc == dict.end())
        {
            printf("%s:%d - 符号消定义时找不到指定的符号[%s]。\n", filenm[incp], linec[incp], tokenbuf);
        }
        dict.erase(loc);
    }
    else
    {
        printf("%s:%d - 符号定义命令数量不正确。\n", filenm[incp], linec[incp]);
    }
}

bool eval(const char *expr)
{
    return !strcmp(expr, "[DEFINED]");
}

void procif(const char *cmd)
{
    if (stage[stagep] == ST_BLOCK || stage[stagep] == ST_PBLOCK)
    {
        stage[++stagep] = ST_PBLOCK;
        return;
    }
    if (stage[stagep] == ST_NORMAL)
    {
        stage[++stagep] = eval(cmd) ? ST_NORMAL : ST_BLOCK;
    }
}

void procifdef(const char *cmd)
{
    if (stage[stagep] == ST_BLOCK || stage[stagep] == ST_PBLOCK)
    {
        stage[++stagep] = ST_PBLOCK;
        return;
    }
    if (stage[stagep] == ST_NORMAL)
    {
        sscanf(cmd, "%s", tokenbuf);
        stage[++stagep] = dict.find(std::string(tokenbuf)) != dict.end() ? ST_NORMAL : ST_BLOCK;
    }
}

void procelifd(const char *cmd)
{
    if (stagep == 0)
    {
        printf("%s:%d - 此处不能有条件分歧否则如果定义语句。\n", filenm[incp], linec[incp]);
    }
    if (stage[stagep] == ST_PBLOCK)
    {
        return;
    }
    if (stage[stagep] == ST_NORMAL)
    {
        stage[stagep] = ST_PBLOCK;
    }
    if (stage[stagep] == ST_BLOCK)
    {
        sscanf(cmd, "%s", tokenbuf);
        stage[stagep] = dict.find(std::string(tokenbuf)) != dict.end() ? ST_NORMAL : ST_BLOCK;
    }
}

void procndef(const char *cmd)
{
    if (stage[stagep] == ST_BLOCK || stage[stagep] == ST_PBLOCK)
        return;
    if (stage[stagep] == ST_NORMAL)
    {
        sscanf(cmd, "%s", tokenbuf);
        stage[++stagep] = dict.find(std::string(tokenbuf)) != dict.end() ? ST_BLOCK : ST_NORMAL;
    }
}

void procelse()
{
    if (stagep == 0)
    {
        printf("%s:%d - 此处不能有条件分歧否则语句。\n", filenm[incp], linec[incp]);
    }
    if (stage[stagep] == ST_BLOCK)
        stage[stagep] = ST_NORMAL;
    else if (stage[stagep] == ST_NORMAL)
        stage[stagep] = ST_BLOCK;
}

void procendif()
{
    if (stagep == 0)
    {
        printf("%s:%d - 此处不能有条件分歧终止语句。\n", filenm[incp], linec[incp]);
        return;
    }
    stagep--;
}

void procerror(const char *msg)
{
    printf("%s:%d - 错误：%s\n", filenm[incp], linec[incp], msg);
    exit(-1);
}

void procwarn(const char *msg)
{
    printf("%s:%d - %s\n", filenm[incp], linec[incp], msg);
}

int main(int argc, char **argv)
{
    strcpy(filenm[0], argv[1]);
    incs[0] = fopen(argv[1], "r");
    out = fopen(argv[2], "w");

    while (incp >= 0)
    {
        while (EOF != fscanf(incs[incp], "%[^\n]", linebuf))
        {
            fgetc(incs[incp]);
            int offset = 0;
            offset = sscanf(linebuf, "%s", tokenbuf);
            if (offset == 0 || offset == -1)
                goto lineend;
            offset = strlen(tokenbuf) + 1;

            if (0 == strcmp(tokenbuf, "#define"))
            {
                procdef(linebuf + offset);
                goto lineend;
            }
            else if (0 == strcmp(tokenbuf, "#undef"))
            {
                procudef(linebuf + offset);
                goto lineend;
            }
            else if (0 == strcmp(tokenbuf, "#ifdef"))
            {
                procifdef(linebuf + offset);
                goto lineend;
            }
            else if (0 == strcmp(tokenbuf, "#elifd"))
            {
                procelifd(linebuf + offset);
                goto lineend;
            }
            else if (0 == strcmp(tokenbuf, "#ifndef"))
            {
                procndef(linebuf + offset);
                goto lineend;
            }
            procline();
            if (tokenbuf[0] != '#' && stage[stagep] == ST_NORMAL)
            {
                fprintf(out, "%s\n", linebuf);
                goto lineend;
            }

            if (0 == strcmp(tokenbuf, "#else"))
                procelse();
            else if (0 == strcmp(tokenbuf, "#endif"))
                procendif();
            else if (0 == strcmp(tokenbuf, "#error"))
                procerror(linebuf + offset);
            else if (0 == strcmp(tokenbuf, "#warn"))
                procwarn(linebuf + offset);
            else if (0 == strcmp(tokenbuf, "#include"))
            {
                if(stage[stagep] != ST_NORMAL) goto lineend;
                if (incp == 127)
                {
                    fprintf(stderr, "%s:%d - 包含文件层次过深。包含将不执行。\n", filenm[incp], linec[incp]);
                    goto lineend;
                }
                linec[incp]++;
                incs[++incp] = fopen(linebuf + offset, "r");
                strcpy(filenm[incp], linebuf + offset);
                if (incs[incp] == NULL)
                {
                    --incp;
                    int err = errno;
                    fprintf(stderr, "%s:%d - 无法打开包含文件：[%s]，错误原因：%s。\n",
                            filenm[incp], linec[incp], linebuf + offset, strerror(errno));
                }
                continue;
            }
        lineend:
            linebuf[0] = '\0';
            linec[incp]++;
        }
        fclose(incs[incp]);
        --incp;
    }

    return 0;
}