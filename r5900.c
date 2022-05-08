#include "r5900.h"
#include "string.h"
#include "inscodec/codec.h"
#include "inscodec/disas.h"

#ifdef ANAL_DEBUG
#ifndef MK_STR
#define MK_STRR(x) #x
#define MK_STR(x) MK_STRR(x)
#endif
#define debug(...) printf("[r5900.c:" MK_STR(__LINE__) "] "__VA_ARGS__)
#else
#define debug(...)
#endif

int cfg_emu_anal_ignore_bmerr = 0;
unsigned long long cfg_emu_addr_max = 0x20000000;
unsigned long long cfg_exec_addr_max = 0x20000000;

struct emu_status *emu_init() {
    debug("emu_init()\n");
    struct emu_status *target = (struct emu_status *)malloc(sizeof(struct emu_status));
    memset(target, 0, sizeof(struct emu_status));
    target->func_assume_bookmark = -1;
    return target;
}

int is_ghi_conv(struct emu_status *emu_stat, int i) {
    if (i > 31)
        return 0;
    return emu_stat->gpr_hi_conv & (1 << i);
}

int is_glo_conv(struct emu_status *emu_stat, int i) {
    if (i > 31)
        return 0;
    return emu_stat->gpr_conv & (1 << i);
}

void clr_ghi_conv(struct emu_status *emu_stat, int i) {
    emu_stat->gpr_hi_conv &= ~(1 << i);
}

void clr_glo_conv(struct emu_status *emu_stat, int i) {
    emu_stat->gpr_conv &= ~(1 << i);
}

void flg_ghi_conv(struct emu_status *emu_stat, int i) {
    emu_stat->gpr_hi_conv |= (1 << i);
}

void flg_glo_conv(struct emu_status *emu_stat, int i) {
    emu_stat->gpr_conv |= (1 << i);
}

unsigned long long lbyte(unsigned char *memory_buffer, unsigned int address) {
    if (address >= cfg_emu_addr_max) return 0;
    unsigned char value = *(memory_buffer + address);
    return (unsigned long long)value;
}

unsigned long long lsbyte(unsigned char *memory_buffer, unsigned int address) {
    if (address >= cfg_emu_addr_max) return 0;
    char value = *(char *)(memory_buffer + address);
    return value & 0x80 ? 0xffffffffffffff00 : 0x0 | value; // sign_extend
}

unsigned long long lword(unsigned char *memory_buffer, unsigned int address) {
    if (address >= cfg_emu_addr_max) return 0;
    unsigned int value = *(unsigned int *)(memory_buffer + address);
    return (unsigned long long)value;
}

unsigned long long lsword(unsigned char *memory_buffer, unsigned int address) {
    if (address >= cfg_emu_addr_max) return 0;
    int value = *(int *)(memory_buffer + address);
    return value & 0x80000000 ? 0xffffffff00000000 : 0x0 | value; // sign_extend
}

unsigned long long ldword(unsigned char *memory_buffer, unsigned int address) {
    if (address >= cfg_emu_addr_max) return 0;
    unsigned long long value = *(unsigned long long *)(memory_buffer + address);
    return value;
}

int emu_exec(struct emu_status *emu_stat, unsigned char *memory_buffer) {
    unsigned int pc_cache = emu_stat->pc;
    unsigned int code = *((unsigned int *)(memory_buffer + emu_stat->pc));
    emu_stat->pc += 4;
    struct instr_t instr = DecodeInstruction(code);
    // debug("emu_exec: Op-%d is submitted to emu.\n", instr.opcode);

    int idx;
    unsigned int addr = (instr.imm + emu_stat->gpr[instr.rs].r32.lo) & 0xffffffff;
    switch (instr.opcode) {
    case PADDUW:
        emu_stat->gpr[instr.rd].r32.lo = emu_stat->gpr[instr.rt].r32.lo + emu_stat->gpr[instr.rs].r32.lo;
        emu_stat->gpr[instr.rd].r32.hi = emu_stat->gpr[instr.rt].r32.hi + emu_stat->gpr[instr.rs].r32.hi;
        emu_stat->gpr_hi[instr.rd].r32.lo = emu_stat->gpr_hi[instr.rt].r32.lo + emu_stat->gpr_hi[instr.rs].r32.lo;
        emu_stat->gpr_hi[instr.rd].r32.hi = emu_stat->gpr_hi[instr.rt].r32.hi + emu_stat->gpr_hi[instr.rs].r32.hi;
        break;
    case LUI:
        emu_stat->gpr[instr.rt].r16.llo = 0;
        emu_stat->gpr[instr.rt].r16.lhi = instr.imm & 0xffff;
        break;
    case ADD:
    case ADDU:
        emu_stat->gpr[instr.rd].r32.lo = emu_stat->gpr[instr.rs].r32.lo + emu_stat->gpr[instr.rt].r32.lo;
        break;
    case SUB:
    case SUBU:
        emu_stat->gpr[instr.rd].r32.lo = emu_stat->gpr[instr.rs].r32.lo - emu_stat->gpr[instr.rt].r32.lo;
        break;
    case ADDI:
    case ADDIU:
        emu_stat->gpr[instr.rd].r32.lo += *(long long *)&(instr.imm);
        break;
    case ORI:
        emu_stat->gpr[instr.rd].r32.lo |= *(long long *)&(instr.imm);
        break;
    case ANDI:
        emu_stat->gpr[instr.rd].r32.lo &= *(long long *)&(instr.imm);
        break;
    case XORI:
        emu_stat->gpr[instr.rd].r32.lo ^= *(long long *)&(instr.imm);
        break;
    case LB:
        emu_stat->gpr[instr.rt].data = lsbyte(memory_buffer, addr);
        break;
    case LBU:
        emu_stat->gpr[instr.rt].data = lbyte(memory_buffer, addr);
        break;
    case LW:
        emu_stat->gpr[instr.rt].data = lsword(memory_buffer, addr);
        break;
    case LWU:
        emu_stat->gpr[instr.rt].data = lword(memory_buffer, addr);
        break;
    case LD:
        emu_stat->gpr[instr.rt].data = ldword(memory_buffer, addr);
    }

    return 0;
}

int emu_anal_exec(struct emu_status *emu_stat, struct book *book, unsigned char *memory_buffer) {
    unsigned int pc_cache = emu_stat->pc;
    unsigned int code = *((unsigned int *)(memory_buffer + emu_stat->pc));
    // emu_stat->pc += 4;
    struct instr_t instr = DecodeInstruction(code);

    int idx;
    int dtype = -1;
    switch (instr.opcode) {
    case PADDUW:
        if (is_ghi_conv(emu_stat, instr.rs) && is_glo_conv(emu_stat, instr.rs) &&
            is_ghi_conv(emu_stat, instr.rt) && is_glo_conv(emu_stat, instr.rt)) {
            flg_ghi_conv(emu_stat, instr.rd);
            flg_glo_conv(emu_stat, instr.rd);
        }
        else {
            clr_ghi_conv(emu_stat, instr.rd);
            clr_glo_conv(emu_stat, instr.rd);
        }
        break;
    case LUI:
        flg_glo_conv(emu_stat, instr.rt);
        break;
    case ADD:
    case ADDU:
        if (is_glo_conv(emu_stat, instr.rs) && is_glo_conv(emu_stat, instr.rt)) {
            flg_glo_conv(emu_stat, instr.rd);
        }
        else
            clr_glo_conv(emu_stat, instr.rd);
    case ADDI:
    case ADDIU: // can be part II of LI
    case ORI:
        if (is_glo_conv(emu_stat, instr.rd)) {
            idx = bookmark_mkf(book, "li_%08X_%08X", pc_cache, emu_stat->gpr[instr.rd].r32.lo);
            if (idx == -2) break;
            if (idx != -1) {
                struct bookmark *bm = bookmark_get(book, idx);
                bm->loc = pc_cache;
                bm->ref_loc = emu_stat->gpr[instr.rd].r32.lo;
                bookmark_cmt(book, bm);
            }
        }
        break;
    case LBU:
        dtype = 0;
    case LB:
        if (dtype == -1)
            dtype = 1;
    case LHU:
        if (dtype == -1)
            dtype = 2;
    case LH:
        if (dtype == -1)
            dtype = 3;
    case LWU:
        if (dtype == -1)
            dtype = 4;
    case LW:
        if (dtype == -1)
            dtype = 5;
    case LD:
        if (dtype == -1)
            dtype = 6;
    case LQ:
        if (dtype == -1)
            dtype = 7;

        if (is_glo_conv(emu_stat, instr.rs)) {
            flg_glo_conv(emu_stat, instr.rt);

            int idx = bookmark_mkf(book, "ref_%08X_%08X", pc_cache, (instr.imm + emu_stat->gpr[instr.rs].r32.lo) & 0xffffffff);

            if (idx == -1) return -100;
            if (idx == -2) break;

            struct bookmark *bm = bookmark_get(book, idx);
            bm->loc = pc_cache;
            bm->ref_loc = (instr.imm + emu_stat->gpr[instr.rs].r32.lo) & 0xffffffff;
            enum mark_type_enum ref_type[] = {
                MARK_TYPE_REF_BYTE,
                MARK_TYPE_REF_SBYTE,
                MARK_TYPE_REF_HALF,
                MARK_TYPE_REF_SHORT,
                MARK_TYPE_REF_WORD,
                MARK_TYPE_REF_LONG,
                MARK_TYPE_REF_DOUBLE,
                MARK_TYPE_REF_QUAD,
            };
            enum mark_type_enum data_type[] = {
                MARK_TYPE_BYTE,
                MARK_TYPE_SBYTE,
                MARK_TYPE_HALF,
                MARK_TYPE_SHORT,
                MARK_TYPE_WORD,
                MARK_TYPE_LONG,
                MARK_TYPE_DOUBLE,
                MARK_TYPE_QUAD,
            };
            bm->type = ref_type[dtype];

            if (0) {
            } // TODO: Handle if ref_loc > memsize
            // switch(dtype) {
            //     case 0:
            //     case 1:
            //     case 2:
            //     case 3:
            //     case 4:
            //     case 5:
            //     case 6:
            //     case 7:
            //     break;
            //     default:
            //     return -101;
            // }

            int ref_loc = (unsigned int)(bm->ref_loc);
            const char *fmts[] = {
                "byte_%08X",
                "sbyte_%08X",
                "half_%08X",
                "short_%08X",
                "word_%08X",
                "long_%08X",
                "double_%08X",
                "quad_%08X",
            };
            bookmark_cmt(book, bm);

            idx = bookmark_mkf(book, fmts[dtype], ref_loc);

            if (idx == -1) return -100;
            if (idx == -2) break;

            bm = bookmark_get(book, idx);
            bm->loc = ref_loc;
            bm->type = dtype[data_type];
            bookmark_cmt(book, bm);
        }
        else
            clr_glo_conv(emu_stat, instr.rt);
        break;
    }

    emu_exec(emu_stat, memory_buffer);

    return 0;
}

int emu_anal_instr(struct emu_status *emu_stat, struct book *book, unsigned char *memory_buffer) {
    if (emu_stat->pc > cfg_exec_addr_max) return -3;

    unsigned int pc_cache = emu_stat->pc;
    unsigned int code = *((unsigned int *)(memory_buffer + emu_stat->pc));
    struct instr_t instr = DecodeInstruction(code);

    #ifdef ANAL_DEBUG
    char buffer[128];
    disas(instr, emu_stat->pc, buffer);
    debug("anal: %08X:\t%32s\n", emu_stat->pc, buffer);
    #endif

    int idx;
    switch (instr.opcode) {
    case J: // can be a call like return func(); or a goto;
        emu_stat->pc += 4;
        emu_anal_instr(emu_stat, book, memory_buffer);
        idx = bookmark_mkf(book, "jmpukn_%08X_%08X", pc_cache, instr.imm);
        if (idx == -1) {
            if (!cfg_emu_anal_ignore_bmerr) return -100;
            break;
        }
        if (idx == -2) {
            break;
        }
        else {
            struct bookmark *bm = bookmark_get(book, idx);
            bm->loc = pc_cache;
            bm->ref_loc = instr.imm;
            bm->type = MARK_TYPE_JUMP | MARK_TYPE_UNK; // it can be call (if it is the end)
            if (instr.imm == pc_cache) { // it stuck the execution
                bm->direction = REF_DIR_SELF;
                bookmark_renf(bm, "stuck_%08X", instr.imm);
                bm->type = MARK_TYPE_JUMP;
            }
            else if (instr.imm > emu_stat->func_begin && instr.imm < pc_cache) { // it is loop
                bm->direction = REF_DIR_UP;
                bm->type = MARK_TYPE_JUMP;
                bookmark_renf(bm, "rpt_%08X", pc_cache);
            }
            else {
                // can be return, so assume it is:
                emu_stat->func_assume_bookmark = idx;
                emu_stat->func_assume_end = pc_cache + 4; // the delay slot
            }
            bookmark_cmt(book, bm);
        }
        return -2; // ctrl flow hit the assuming ending
        break;
    case BEQ:
    case BGEZ:
    case BLEZ:
    case BNE:
        emu_stat->pc += 4;
        emu_anal_instr(emu_stat, book, memory_buffer);
        // emu_stat->pc = pc_cache;
        idx = bookmark_mkf(book, "br_%08X_%08X", pc_cache, instr.imm);
        if (idx == -1) {
            if (!cfg_emu_anal_ignore_bmerr) return -100;
            break;
        }
        if (idx == -2) {
            break;
        }
        else {
            struct bookmark *bm = bookmark_get(book, idx);
            bm->type = MARK_TYPE_BRANCH;
            bm->loc = pc_cache;
            bm->ref_loc = *(long long *)(&instr.imm) + pc_cache + 4;
            bm->ref_oloc = pc_cache + 8;
            if (*(long long *)(&instr.imm) == -4) {
                bm->direction = REF_DIR_SELF;
                bookmark_renf(bm, "stuck_%08X", pc_cache);
            }
            else
                bm->direction = (*(long long *)&instr.imm) >= 0 ? REF_DIR_DOWN : REF_DIR_UP;

            bookmark_cmt(book, bm);
        }
        return *(long long *)(&instr.imm) + pc_cache + 4; // require new ctrl flow anal at ref_loc
        break;
    case BGEZAL:
    case BLTZAL:
        emu_stat->pc += 4;
        emu_anal_instr(emu_stat, book, memory_buffer);
        idx = bookmark_mkf(book, "ccal_%08X_%08X", pc_cache, instr.imm);
        if (idx == -1) {
            if (!cfg_emu_anal_ignore_bmerr) return -100;
            break;
        }
        if (idx == -2) {
            break;
        }
        else {
            struct bookmark *bm = bookmark_get(book, idx);
            bm->type = MARK_TYPE_BCALL;
            bm->loc = pc_cache;
            bm->ref_loc = (*(long long *)&instr.imm) + 4 + pc_cache;
            bm->ref_oloc = pc_cache + 8;
            bookmark_cmt(book, bm);
        }
        debug("func reg\n");
        book_anal_func(book, *(long long *)(&instr.imm) + pc_cache + 4, idx);
        // return *(long long*)(&instr.imm) + pc_cache + 4; // req new ctrl flow anal
        break;
    case JAL:
    //case JALR:
        emu_stat->pc += 4;
        emu_anal_instr(emu_stat, book, memory_buffer);

        idx = bookmark_mkf(book, "cal_%08X_%08X", pc_cache, instr.imm);
        if (idx == -1) {
            if (!cfg_emu_anal_ignore_bmerr) return -100;
            break;
        }
        if (idx == -2) {
            break;
        }
        else {
            struct bookmark *bm = bookmark_get(book, idx);
            bm->type = MARK_TYPE_CALL;
            bm->loc = pc_cache;
            bm->ref_loc = instr.imm;
            bookmark_cmt(book, bm);
        }
        
        if (instr.imm > cfg_exec_addr_max) {
            confp_mkf(book, "This call jump to an area that un-executable.", pc_cache, "out_jump_%08X", pc_cache);
        } else {
            debug("func reg %08X\n", instr.imm);
            book_anal_func(book, instr.imm, idx);
        }
        break;
    case JR: // switch case or return
        emu_stat->pc += 4;
        emu_anal_instr(emu_stat, book, memory_buffer);
        if (instr.rs == 31) { // jr ra - return
            if (emu_stat->func_end < pc_cache + 4) { // update subroutine edge
                emu_stat->func_end = pc_cache + 4;

                if (emu_stat->func_assume_bookmark != -1 && emu_stat->func_assume_end < emu_stat->func_end) {
                    struct bookmark *bm = bookmark_get(book, emu_stat->func_assume_bookmark);
                    bm->type = MARK_TYPE_JUMP;
                    bookmark_renf(bm, "goto_%08X_%08X", bm->loc, bm->ref_loc);
                    // what if more than one bookmarks is unknown, need improvement!

                    emu_stat->func_assume_end = 0;
                    emu_stat->func_assume_bookmark = -1;
                    bookmark_cmt(book, bm);
                }
            }
            return -1; // ctrl flow hit the certain return
        }
        break;
    case SYSCALL:
        emu_stat->pc += 4;
        idx = bookmark_mkf(book, "syscall_%08X_%llu", pc_cache, instr.imm);
        if (idx == -1) {
            if (!cfg_emu_anal_ignore_bmerr) return -100;
            break;
        }
        if (idx == -2) {
            break;
        }
        else {
            struct bookmark *bm = bookmark_get(book, idx);
            bm->loc = pc_cache;
            bm->type = MARK_TYPE_SYSCALL;
            bookmark_cmt(book, bm);
        }
        break;
    default:
        emu_anal_exec(emu_stat, book, memory_buffer);
    }
    return 0;
}

struct emu_status *emu_stat_dup(struct emu_status *ori) {
    struct emu_status *ans = (struct emu_status *)malloc(sizeof(struct emu_status));
    memcpy(ans, ori, sizeof(struct emu_status));
    return ans;
}

void emu_stat_free (struct emu_status *emu_stat) {
    free (emu_stat);
}

int emu_anal_ctrlflow(struct emu_status *emu_stat, struct book *book, unsigned char *memory_buffer) {
    debug("++++++++new ctrlflow %08X\n", emu_stat->pc);
    int counter = 100000;
    while (1) {
        if (counter <= 0) {
            fprintf(stderr, "Dead loop may occured near %08X(Over 100000 instructions executed), kill branch by default.\n");
            return -2;
        }

        int ret = emu_anal_instr(emu_stat, book, memory_buffer);
        // emu_stat->pc += 4;
        book_anal_ctrlf_upd(book, emu_stat->func_begin, emu_stat->func_end = emu_stat->pc);
        if (ret) {
            if (ret == -1 || ret == -2) {
                debug("---------ctrlflow terminate %08X <- %08X (type=%d)\n", emu_stat->pc, emu_stat->func_begin, ret);
                return 0;
            }
            if (ret == -3){
                debug("---------ctrlflow terminate force by out of range [%08X <- %08X (type=%d)]\n", emu_stat->pc, emu_stat->func_begin, ret);
                return 0;
            }
            if (ret == -100)
                return -1;

            // control cover check
            if (book_anal_func_qry(book, ret)) continue; // branched to anather function.
            if (book_anal_ctrlf_qry(book, ret)) continue; // the ctrl flow is covered.

            // otherwise, the ctrl flow is new, need to be checked.
            book_anal_ctrlf_sus(book, ret);
            
            struct emu_status *dup = emu_stat_dup(emu_stat);
            dup->func_assume_bookmark = -1;
            dup->func_begin = ret;
            dup->pc = ret;
            ret = emu_anal_ctrlflow(dup, book, memory_buffer);
            emu_stat->func_end = dup->func_end > emu_stat->func_end ? dup->func_end : emu_stat->func_end;
            free(dup);

            if (ret) return ret;
        }
        --counter;
    }
}

int emu_anal_func(struct emu_status *emu_stat, struct book *book, unsigned char *memory_buffer) {
    debug("++++++++func anal %08X\n", emu_stat->pc);
    emu_stat->func_begin = emu_stat->pc;
    book_anal_ctrlf_sus (book, emu_stat->pc);
    emu_anal_ctrlflow (emu_stat, book, memory_buffer);
    book_anal_ctrlf_cnf (book);
    debug("--------func anal %08X <- %08X\n", emu_stat->pc, emu_stat->func_begin);
    return 0;
}

int emu_anal(struct emu_status *emu_stat, struct book *book, unsigned char *memory_buffer) {
    unsigned long long pc = emu_stat->pc = book->entry_point;
    debug("PC set to entry point (%08X)\n", emu_stat->pc);

    emu_stat->gpr_conv = 0;
    emu_stat->fpr_conv = 0;
    emu_anal_func(emu_stat, book, memory_buffer);

    int func;
    while (-1 != (func = book_anal_func_get(book))) {
        printf("func anal on going... [%08X]\n", func);

        pc = emu_stat->pc = func;
        debug("PC set to func begin of %08X\n", func);

        emu_stat->gpr_conv = 0;
        emu_stat->fpr_conv = 0;
        emu_anal_func(emu_stat, book, memory_buffer);
    }

    return 0;
}
#undef debug
