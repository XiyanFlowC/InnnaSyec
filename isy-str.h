#ifndef ISY__STR_H__
#define ISY__STR_H__

#pragma once

#include "book.h"
#include "bst.h"
#include "xyutils.h"
#include <string.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ISY_STR_ID (0x102F0000)
#define ISY_STR_NAME "str"

struct isy_str {
    int (*strenc)(const char *utf8_str, unsigned char *buffer, struct book* book);
    int (*strdec)(unsigned char *raw_str, char **utf8_str, struct book* book);
    struct bst_node *enc_tbl;
    struct bst_node *dec_tbl;
    sqlite3 *db;
};

/**
 * @brief Set a new string encoder for a book.
 * 
 * @param book The book.
 * @param callback a function pointer point to a function that recv 2 params. The utf8_str
 * is the string that need to be encoded, and the buffer is the target that need to be write, this
 * function should return a int which is the byte this function written to the buffer. -1 should
 * be returned if utf8_str is impossible to encode.
 */
void book_strenc_set(struct book* book,
    int (*callback)(const char *utf8_str, unsigned char *buffer, struct isy_str* data));

/**
 * @brief Set a new string decoder for a book.
 * 
 * @param book The book.
 * @param callback a function pointer point to a funciton that recv 2 params. The utf8_str
 * is a pointer to the char* which need to point to a NEW malloc() assigned memory which contains
 * the decoded utf8 string, the raw_str is the pointer point to the raw string. And, it should return
 * the length it handled to raw_str. -1 should be returned if the raw_str is unrecognizable.
 */
void book_strdec_set(struct book* book,
    int (*callback)(unsigned char *raw_str, char **utf8_str, struct isy_str* data));

/**
 * @brief Set default enc/dec routine to the book.
 * 
 * @param book 
 * @param dft_id 
 */
void book_strdft_set(struct book* book, int dft_id);

/**
 * @brief Initialize the str module to the specified book.
 * 
 * @param book 
 */
void book_strinit(struct book* book);

/**
 * @brief Search a string in the given memory.
 * 
 * @param str The string need to be search.
 * @param limit The boundary of the memory.
 * @param callback The callback will be called once the given str be found in the given memory.
 * @param info The info will be passed to the callback.
 * @return int Hit count
 */
int isy_strsearch(const char *str, int str_len, const char *memory, int limit, int (*callback)(int offset, void *info), void *info);

/**
 * @brief Guess the string may occur, structure definitions are ignored. Will be registered in book.
 * 
 * @param book The book will taken the string information.
 * @param strategy Strategy id.
 * @return int The guessed string's numbers.
 */
int isy_strguess(struct book *book, int strategy, const char *memory, int limit);

#ifdef __cplusplus
}
#endif
#endif /* ISY__STR_H__ */