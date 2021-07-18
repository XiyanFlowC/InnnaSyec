#ifndef XY_LITEOPT_H__
#define XY_LITEOPT_H__
#pragma once
#include <stdlib.h>
#include <string.h>

#ifndef LITEOPT_TBLSIZ
#define LITEOPT_TBLSIZ 32
#endif

struct opt_reg
{
    char ch_opt;
    char long_opt[26];
    unsigned char flg;
    int (*callback)(const char *);
};

#define LOPT_FLG_OPT_VLD 0x8  /* This opt is valid */
#define LOPT_FLG_VAL_NEED 0x4 /* This opt needs a value */
#define LOPT_FLG_CH_VLD 0x2   /* This opt has a ch prs */
#define LOPT_FLG_STR_VLD 0x1  /* This opt has a str prs */
#define LOPT_FLG_CHK(val, flg) ((val) & (flg))

static struct opt_reg *__reged_opt = NULL;

static void __lopt_mktbl()
{
    __reged_opt = (struct opt_reg *)calloc(LITEOPT_TBLSIZ, sizeof(struct opt_reg));
}

static int __lopt_callbych(char opt, const char *_str)
{
    int i;
    for (i = 0; i < LITEOPT_TBLSIZ; ++i)
    {
        if (__reged_opt[i].ch_opt == opt)
        {
            if (!LOPT_FLG_CHK(__reged_opt[i].flg, LOPT_FLG_CH_VLD))
                return -i; /* NOT REGED OPT */
            return __reged_opt[i].callback(_str);
        }
    }
    return -i; /* NOT REGED OPT */
}

static int __lopt_callbystr(const char *opt, const char *_str)
{
    int i;
    for (i = 0; i < LITEOPT_TBLSIZ; ++i)
    {
        if (strcmp(__reged_opt[i].long_opt, opt) == 0)
        {
            if (!LOPT_FLG_CHK(__reged_opt[i].flg, LOPT_FLG_STR_VLD))
                return -i; /* NOT REGED OPT */
            return __reged_opt[i].callback(_str);
        }
    }
    return -i;
}

static void lopt_regopt(const char *name, char chname, unsigned char flg, int (*callback)(const char *))
{
    static int opt_idx = 0;
    if (__reged_opt == NULL)
        __lopt_mktbl();
    __reged_opt[opt_idx].callback = callback;
    __reged_opt[opt_idx].ch_opt = chname;
    __reged_opt[opt_idx].flg = flg;
    __reged_opt[opt_idx].flg |= LOPT_FLG_OPT_VLD;
    strcpy(__reged_opt[opt_idx++].long_opt, name);
}

static int lopt_parse(int argc, const char **argv)
{
    int i, rst;
    for (i = 1; i < argc; ++i)
    {
        if (argv[i][0] != '-')
            continue; /* INVALID OPT FOUND */
        if (argv[i][1] == '-')
        {
            if (argc > i + 1)
                rst = __lopt_callbystr(argv[i] + 2, argv[i + 1]);
            else
                rst = __lopt_callbystr(argv[i] + 2, NULL);

            if (rst < 0)
                return -i;
            if (rst > 0)
                return rst;
        }
        else
        {
            int j = 1;
            while (argv[i][j] != '\0')
            {
                if (argc > i + 1)
                    rst = __lopt_callbych(argv[i][j], argv[i + 1]);
                else
                    rst = __lopt_callbych(argv[i][j], NULL);

                if (rst < 0)
                    return -i;
                if (rst > 0)
                    return rst;
                ++j;
            }
        }
    }
    return 0;
}

#undef LITEOPT_TBLSIZ

#endif
