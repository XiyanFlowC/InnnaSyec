#ifndef R5900_H
#define R5900_H
#pragma once

#include "book.h"
#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

// keep them as 8, 16, 32, and 64 bits integers
typedef unsigned char byte_t;
typedef unsigned short half_t;
typedef unsigned int word_t;
typedef unsigned long long dword_t;

/*********************************************************
 * ELF Definitions, For competitable with Windows
 *********************************************************/
struct elf_header
{
    byte_t ident[16]; /*we don't care info in it*/
    half_t type;
    half_t machine;
    word_t version;
    word_t entry;
    word_t phoff;
    word_t shoff;
    word_t flags;
    half_t ehsize;
    half_t phentsize;
    half_t phnum;
    half_t shentsize;
    half_t shnum;
    half_t shstrndx;
};

struct section_header
{
    word_t name;
    word_t type;
    word_t flags;
    word_t addr;
    word_t offset;
    word_t size;
    word_t link;
    word_t info;
    word_t align;
    word_t entsize;
};

struct program_header
{
    word_t type;
    word_t offset;
    word_t vaddr;
    word_t paddr;
    word_t filesz;
    word_t memsz;
    word_t flags;
    word_t align;
};

union gpr_layout {
    unsigned long long data;
    struct {
        word_t lo;
        word_t hi;
    } r32;
    struct {
        half_t llo;
        half_t lhi;
        half_t hlo;
        half_t hhi;
    } r16;
};

union fpr_layout {
    dword_t u64;
    word_t u32;
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

    word_t pc;

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

/**
 * @brief Analyze a function from emu_stat->pc.
 * 
 * @param emu_stat 
 * @param book 
 * @param memory_buffer 
 * @return int 
 */
int emu_anal_func(struct emu_status *emu_stat, struct book *book, unsigned char *memory_buffer);

#ifdef __cplusplus
}
#endif

#endif