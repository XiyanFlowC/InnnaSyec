#ifndef R5900_H
#define R5900_H
#pragma once

#include "book.h"

#ifdef __cplusplus
extern "C" {
#endif

union gpr_layout {
    unsigned long long data;
    struct {
        unsigned int lo;
        unsigned int hi;
    } r32;
    struct {
        unsigned short llo;
        unsigned short lhi;
        unsigned short hlo;
        unsigned short hhi;
    } r16;
};

union fpr_layout {
    unsigned long long u64;
    unsigned int u32;
    double f64;
    float f32;
};

// struct r5900_gpr_t {
//     union gpr_layout lo;
//     union gpr_layout hi;
// };

struct emu_status {
    union gpr_layout gpr[32];
    union gpr_layout gpr_hi[32];

    union fpr_layout fpr[32];

    unsigned int pc;

    unsigned int gpr_conv;
    unsigned int gpr_hi_conv;
    unsigned int fpr_conv;

    unsigned int gpr_mod;
    unsigned int gpr_hi_mod;
    unsigned int fpr_mod;

    unsigned int func_begin; // current code chunck's begining
    unsigned int func_end; // subrountine's ending
    unsigned int func_assume_end; // subroutine's assumeing ending (need check)
    int func_assume_bookmark; // The book mark giving the assuming ending
    // unsigned int exec_flow[64]; // for anal, ctrl flow trace
};

/**
 * @brief Initialize a emulator's status.
 * 
 * @return struct emu_status* Initialized, new emulator's status. (need to be free)
 */
struct emu_status *emu_init();

/**
 * @brief Execute the emulator in given status.
 * 
 * @param emu_stat 
 * @param memory_buffer 
 * @return int 0 if success, others if failed.
 */
int emu_exec(struct emu_status *emu_stat, unsigned char *memory_buffer);

/**
 * @brief Execute the emulator for analyzing. (full automatic, call once)
 * 
 * @param emu_stat 
 * @param memory_buffer 
 * @return int 0 if success, -1 if error ocurred.
 */
int emu_anal(struct emu_status *emu_stat, struct book* book, unsigned char *memory_buffer);

/**
 * @brief Load elf to specified memory buffer (it should be large enough!)
 * 
 * @param emu_stat The pc will be set after load.
 * @param book The entry_point will be set after load.
 * @param memory_buffer The memory will be filled after load.
 * @param file_name The elf file position.
 * @return int 0 if success, -1 if failed.
 */
int emu_ldfile(struct emu_status *emu_stat, struct book* book, unsigned char *memory_buffer, const char *file_name);

#ifdef __cplusplus
}
#endif

#endif