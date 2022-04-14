#include "r5900.h"
#include "string.h"
#include "inscodec/codec.h"

struct emu_status *emu_init() {
    struct emu_status *target = (struct emu_status*)malloc(sizeof(struct emu_status));
    memset(target, 0, sizeof(struct emu_status));
    target->func_assume_bookmark = -1;
    return target;
}

int is_ghi_conv (struct emu_status *emu_stat, int i) {
    if (i > 31) return 0;
    return emu_stat->gpr_hi_conv & (1 << i);
}

int is_glo_conv (struct emu_status *emu_stat, int i) {
    if (i > 31) return 0;
    return emu_stat->gpr_conv & (1 << i);
}

void clr_ghi_conv (struct emu_status *emu_stat, int i) {
    emu_stat->gpr_hi_conv &= ~(1 << i);
}

void clr_glo_conv (struct emu_status *emu_stat, int i) {
    emu_stat->gpr_conv &= ~(1 << i);
}

void flg_ghi_conv (struct emu_status *emu_stat, int i) {
    emu_stat->gpr_hi_conv |= (1 << i);
}

void flg_glo_conv (struct emu_status *emu_stat, int i) {
    emu_stat->gpr_conv |= (1 << i);
}

unsigned long long lbyte(unsigned char *memory_buffer, unsigned int address) {
    unsigned char value = *(memory_buffer + address);
    return (unsigned long long)value;
}

unsigned long long lsbyte(unsigned char *memory_buffer, unsigned int address) {
    char value = *(char *)(memory_buffer + address);
    return value & 0x80 ? 0xffffffffffffff00 : 0x0 | value; // sign_extend
}

int emu_exec(struct emu_status *emu_stat, unsigned char *memory_buffer) {
    unsigned int pc_cache = emu_stat->pc;
    unsigned int code = *((unsigned int*)(memory_buffer + emu_stat->pc));
    emu_stat->pc += 4;
    struct instr_t instr = DecodeInstruction(code);

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
        case ADDI:
        case ADDIU:
        emu_stat->gpr[instr.rd].r32.lo += *(long long*)&(instr.imm);
        break;
        case LB:
        emu_stat->gpr[instr.rt].data = lsbyte(memory_buffer, addr);
        break;
        case LBU:
        emu_stat->gpr[instr.rt].data = lbyte(memory_buffer, addr);
        break;
    }

    return 0;
}

int emu_anal_exec(struct emu_status *emu_stat, struct book *book, unsigned char *memory_buffer) {
    unsigned int pc_cache = emu_stat->pc;
    unsigned int code = *((unsigned int*)(memory_buffer + emu_stat->pc));
    emu_stat->pc += 4;
    struct instr_t instr = DecodeInstruction(code);

    emu_exec(emu_stat, memory_buffer);

    int idx;
    int dtype = -1;
    switch (instr.opcode) {
        case PADDUW:
        if (is_ghi_conv(emu_stat, instr.rs) && is_glo_conv(emu_stat, instr.rs) &&
            is_ghi_conv(emu_stat, instr.rt) && is_glo_conv(emu_stat, instr.rt)) {
            flg_ghi_conv(emu_stat, instr.rd);
            flg_glo_conv(emu_stat, instr.rd);
        } else {
            clr_ghi_conv(emu_stat, instr.rd);
            clr_glo_conv(emu_stat, instr.rd);
        }
        break;
        case LUI:
        flg_glo_conv(emu_stat, instr.rt);
        break;
        case ADD:
        case ADDU:
        if (is_glo_conv (emu_stat, instr.rs) && is_glo_conv (emu_stat, instr.rt)) {
            flg_glo_conv (emu_stat, instr.rd);
        }
        else clr_glo_conv (emu_stat, instr.rd);
        case ADDI:
        case ADDIU: // can be part II of LI
        if (is_glo_conv(emu_stat, instr.rd)) {
            idx = bookmark_mkf (book, "li_%08X_%08X", pc_cache, emu_stat->gpr[instr.rd].r32.lo);
            if (idx != -1) {
                struct bookmark* bm = bookmark_get(book, idx);
                bm->loc = pc_cache;
                bm->ref_loc = emu_stat->gpr[instr.rd].r32.lo;
            }
        }
        break;
        case LBU:
        dtype = 0;
        case LB:
        if (dtype == -1) dtype = 1;
        case LHU:
        if (dtype == -1) dtype = 2;
        case LH:
        if (dtype == -1) dtype = 3;
        case LWU:
        if (dtype == -1) dtype = 4;
        case LW:
        if (dtype == -1) dtype = 5;
        case LD:
        if (dtype == -1) dtype = 6;
        case LQ:
        if (dtype == -1) dtype = 7;

        if (is_glo_conv(emu_stat, instr.rs)) {
            flg_glo_conv(emu_stat, instr.rt);

            int idx = bookmark_mkf (book, "ref_%08X", pc_cache);

            struct bookmark* bm = bookmark_get(book, idx);
            bm->loc = pc_cache;
            bm->ref_loc = (emu_stat->gpr[instr.rs].data & 0xffffffff) + instr.imm;
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

            if (0) {} // TODO: Handle if ref_loc > memsize

            int ref_loc = (unsigned int)(bm->ref_loc);
            const char* fmts[] = {
                "byte_%08X",
                "sbyte_%08X",
                "half_%08X",
                "short_%08X",
                "word_%08X",
                "long_%08X",
                "double_%08X",
                "quad_%08X",
            };
            idx = bookmark_mkf (book, fmts[dtype], ref_loc);

            bm = bookmark_get(book, idx);
            bm->loc = ref_loc;
            bm->type = MARK_TYPE_BYTE;
        }
        else clr_glo_conv(emu_stat, instr.rt);
        break;
    }
    return 0;
}

int emu_anal_instr(struct emu_status *emu_stat, struct book *book, unsigned char *memory_buffer) {
    unsigned int pc_cache = emu_stat->pc;
    unsigned int code = *((unsigned int*)(memory_buffer + emu_stat->pc));
    struct instr_t instr = DecodeInstruction(code);

    int idx;
    switch (instr.opcode) {
        case J: // can be a call like return func(); or a goto;
        idx = bookmark_mkf (book, "jmpukn_%08X_%08X", emu_stat->pc, instr.imm);
        if (idx == -1) return -1;
        else {
            struct bookmark *bm = bookmark_get(book, idx);
            bm->loc = emu_stat->pc;
            bm->ref_loc = instr.imm;
            bm->type = MARK_TYPE_JUMP | MARK_TYPE_UNK; // it can be call (if it is the end)
            if (instr.imm == emu_stat->pc) { // it stuck the execution
                bm->direction = REF_DIR_SELF;
                bookmark_renf (bm, "stuck_%08X", instr.imm);
                bm->type = MARK_TYPE_JUMP;
            } else if (instr.imm > emu_stat->func_begin && instr.imm < emu_stat->pc) { // it is loop
                bm->direction = REF_DIR_UP;
                bm->type = MARK_TYPE_JUMP;
                bookmark_renf ("rpt_%08X", emu_stat->pc);
            } else {
                // can be return, so assume it is:
                emu_stat->func_assume_bookmark = idx;
                emu_stat->func_assume_end = pc_cache + 4; // the delay slot
            }
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
        idx = bookmark_mkf (book, "br_%08X_%08X", pc_cache, instr.imm);
        if (idx == -1) return -1;
        else {
            struct bookmark *bm = bookmark_get(book, idx);
            bm->type = MARK_TYPE_BRANCH;
            bm->loc = pc_cache;
            bm->ref_loc = *(long long*)(&instr.imm) + pc_cache + 4;
            bm->ref_oloc = pc_cache + 8;
            if (*(long long*)(&instr.imm) == -4) {
                bm->direction = REF_DIR_SELF;
                bookmark_renf (bm, "stuck_%08X", pc_cache);
            } else bm->direction = (*(long long*)&instr.imm) >= 0 ? REF_DIR_DOWN : REF_DIR_UP;
        }
        return *(long long*)(&instr.imm) + pc_cache + 4; // require new ctrl flow anal at ref_loc
        break;
        case BGEZAL:
        case BLTZAL:
        emu_stat->pc += 4;
        emu_anal_instr(emu_stat, book, memory_buffer);
        idx = bookmark_mkf (book, "ccal_%08X_%08X", pc_cache, instr.imm);
        if (idx == -1) return -1;
        else {
            struct bookmark *bm = bookmark_get(book, idx);
            bm->type = MARK_TYPE_BCALL;
            bm->loc = pc_cache;
            bm->ref_loc = (*(long long*)&instr.imm) + 4 + pc_cache;
            bm->ref_oloc = pc_cache + 8;
        }
        // return *(long long*)(&instr.imm) + pc_cache + 4; // req new ctrl flow anal
        break;
        case JAL:
        case JALR:
        emu_stat->pc += 4;
        emu_anal_instr(emu_stat, book, memory_buffer);
        idx = bookmark_mkf (book, "cal_%08X_%08X", pc_cache, instr.imm);
        if (idx == -1) return -1;
        else {
            struct bookmark *bm = bookmark_get(book, idx);
            bm->type = MARK_TYPE_CALL;
            bm->loc = pc_cache;
            bm->ref_loc = instr.imm;
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
                    bookmark_renf (bm, "goto_%08X_%08X", bm->loc, bm->ref_loc);
                    // what if more than one bookmarks is unknown, need improvement!

                    emu_stat->func_assume_end = 0;
                    emu_stat->func_assume_bookmark = -1;
                }
            }
            return -1; // ctrl flow hit the certain return
        }
        break;
        default:
        emu_anal_exec(emu_stat, book, memory_buffer);
    }
    return 0;
}

int emu_anal_ctrlflow(struct emu_status *emu_stat, struct book *book, unsigned char *memory_buffer) {

}

int emu_anal_func(struct emu_status *emu_stat, struct book *book, unsigned char *memory_buffer) {
    return 0;
}

int emu_anal(struct emu_status *emu_stat, struct book* book, unsigned char *memory_buffer) {
    unsigned long long pc = emu_stat->pc = book->entry_point;
    

    return 0;
}
