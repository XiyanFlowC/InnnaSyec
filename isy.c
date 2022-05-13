#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "inscodec/codec.h"
#include "inscodec/disas.h"
#include "r5900.h"
#include "xyutils.h"
#include "liteopt.h"
#include "book.h"
#include "fragcov.h"
#include "isy-str.h"

unsigned char *flat_memory;

int query_echo(void *para, int col, char **col_val, char **col_name)
{
    printf("++++Record++++\n");
    for (int i = 0; i < col; ++i)
    {
        printf("%s: %s\n", col_name[i], col_val[i]);
    }
    puts("----Record----");
    return 0;
}

int main(int argc, char **argv)
{
    if (argc == 1) // Interactive mode
    {
        puts("Not implemented yet.");
        exit(-1);
    }
    // cfg_emu_anal_ignore_bmerr = 1;

    flat_memory = (unsigned char *)calloc(0x20000000, sizeof(unsigned char));

    struct book *lib = book_init(argv[1]);
    book_strinit(lib);
    book_xtrinit(lib);
    // lib->entry_point = loadelf(flat_memory, argv[1]);
    struct emu_status *sta = emu_init();
    emu_ldfile(sta, lib, flat_memory, argv[1]);

    printf("File %s loaded. PC set to %08X\n", argv[1], sta->pc);
    // emu_anal(sta, lib, flat_memory);
    // int emu_anal_exec(struct emu_status *emu_stat, struct book *book, unsigned char *memory_buffer);
    // sta->pc = 0x108E14;
    // emu_anal_exec(sta, lib, flat_memory);
    // emu_anal_exec(sta, lib, flat_memory);

    char line[512], cmd[512];
    printf("%08X> ", sta->pc);
    while (EOF != scanf("%[^\n]", line))
    {
        getchar();
        int cnt = get_term2(cmd, line);
        if (0 == strcmp(cmd, "analyze"))
        {
            if (1 != sscanf(line + cnt, "%s", cmd))
            {
                fprintf(stderr, "param required.");
                exit(-1);
            }

            if (0 == strcmp(cmd, "all"))
            {
                emu_anal(sta, lib, flat_memory);
            }
            else if (0 == strcmp(cmd, "func"))
            {
                int idx = bookmark_mkf(lib, "func_%08X", sta->pc);
                if (idx == -1)
                    return -1;
                if (idx != -2) {
                    struct bookmark *bm = bookmark_get(lib, idx);
                    bm->loc = sta->pc;
                    bm->type = MARK_TYPE_SUBROUTINE;
                    bookmark_cmt(lib, bm);
                    book_anal_func(lib, sta->pc, idx);
                }
                emu_anal_func(sta, lib, flat_memory);
            }
        }
        else if (0 == strcmp(cmd, "jump"))
        {
            unsigned int loc;
            sscanf(line + cnt, "%08X", &loc);
            sta->pc = loc;
        }
        else if (0 == strcmp(cmd, "quit"))
        {
            break;
        }
        else if (0 == strcmp(cmd, "query"))
        {
            // scanf("%[^\n]\n", cmd);
            char *err;
            if (sqlite3_exec(lib->db, line + cnt, query_echo, NULL, &err) != SQLITE_OK)
            {
                fprintf(stderr, "query error: %s\n", err);
            }
        }
        else if (0 == strcmp(cmd, "disas"))
        {
            int bgn = book_anal_func_qry(lib, sta->pc);
            if (bgn)
            {
                int end = book_func_end(lib, bgn);
                for (int i = bgn; i <= end; i += 4)
                {
                    unsigned int code = *((unsigned int *)(flat_memory + i));
                    struct instr_t instr = DecodeInstruction(code);
                    disas(instr, i, cmd);
                    printf("%08X:\t%-16s\t", i, cmd);
                    sqlite3_stmt *stmt;
                    sqlite3_prepare_v2(lib->db, "SELECT * FROM bookmarks WHERE loc = ?;", -1, &stmt, NULL);
                    sqlite3_bind_int(stmt, 1, i);
                    while (sqlite3_step(stmt) == SQLITE_ROW)
                    {
                        if (MARK_TYPE_LI_MARK == sqlite3_column_int(stmt, 6)) {
                            printf("; li 0x%X", sqlite3_column_int(stmt, 2)); // ref_loc
                        } else
                        printf("; %s", sqlite3_column_text(stmt, 1));
                    }
                    putchar('\n');
                }
            }
        }
        else
        {
            fputs("Unknown command.\n", stderr);
        }
        printf("%08X> ", sta->pc);
        line[0] = '\0';
    }

    book_close(lib);
}
