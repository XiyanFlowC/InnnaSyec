#ifndef BOOK_H
#define BOOK_H

#pragma once

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <sqlite3.h>
#include "fragcov.h"

enum ref_dir_enum {
    REF_DIR_UP,
    REF_DIR_DOWN,
    REF_DIR_SELF,
};

enum mark_type_enum {
    MARK_TYPE_JUMP, // jump to somewhere
    MARK_TYPE_BRANCH,
    MARK_TYPE_CALL, // function call
    MARK_TYPE_BCALL, // branch call
    MARK_TYPE_REF_STRUCT, // ref to a struct
    MARK_TYPE_REF_STRING, // ref to a string
    MARK_TYPE_REF_HALF, // ref to a half value
    MARK_TYPE_REF_WORD, // ref to a word value
    MARK_TYPE_REF_DOUBLE, // ref to a double word value
    MARK_TYPE_REF_QUAD, // ref to a quad word value
    MARK_TYPE_REF_BYTE, // ref to a byte value
    MARK_TYPE_REF_DATA, // ref to an unknown value
    MARK_TYPE_REF_SBYTE, // ref to a signed byte
    MARK_TYPE_REF_SHORT, // ref to signed half
    MARK_TYPE_REF_LONG, // ref to a signed word
    MARK_TYPE_STRUCT,
    MARK_TYPE_STRING,
    MARK_TYPE_HALF,
    MARK_TYPE_WORD,
    MARK_TYPE_LONG,
    MARK_TYPE_DOUBLE,
    MARK_TYPE_QUAD,
    MARK_TYPE_BYTE,
    MARK_TYPE_DATA,
    MARK_TYPE_SBYTE,
    MARK_TYPE_SHORT,
    MARK_TYPE_INT,
    MARK_TYPE_SYSCALL, // a syscall
    MARK_TYPE_SUBROUTINE, // a subroutine's start, ref_loc is its end
    MARK_TYPE_LI_MARK, // consider ref_loc as an imm value
    MARK_TYPE_UNK = 0x10000,
    MARK_TYPE_ERR = 0x20000,
};

struct bookmark {
    int id;
    char *name;
    unsigned long long loc; // location of the bookmark
    // unsigned char *loc_ptr;
    unsigned long long ref_loc; // ref location, jump to, data loc, etc.
    // void *ref_loc_ptr;
    unsigned long long ref_oloc; // ref to location, failed jump to
    // void *ref_oloc_ptr;
    enum ref_dir_enum direction;
    enum mark_type_enum type;
    int array_length; // if it is an array, mark the length
};

enum eval_seq_op {
    EVAL_VAL_VALUE,
    EVAL_OP_PUSH,
    EVAL_OP_POP,
    EVAL_OP_DUP,
    EVAL_COND_EQ,
    EVAL_COND_LT,
    EVAL_COND_GT,
    EVAL_COND_LE,
    EVAL_COND_GE,
    EVAL_FLAG_FLIP,
    EVAL_FLAG_RESET,
};

struct eval_seq {
    int seq_length;
    enum eval_seq_op *seq;
};

enum bin_type {
    BIN_TYPE_BYTE,
    BIN_TYPE_HALF,
    BIN_TYPE_WORD,
    BIN_TYPE_DOUBLE,
    BIN_TYPE_QUAD,
    BIN_TYPE_SINGLE_FLOAT,
    BIN_TYPE_DOUBLE_FLOAT,
    BIN_TYPE_CHAR,
    BIN_TYPE_REF,
    BIN_TYPE_STRING,
};

struct bin_field {
    char *name;
    enum bin_type type;
    int array_length;
};

struct bin_struct {
    char *name;
    int field_count;
    struct bin_field *fields;
};

struct patch
{
    unsigned long long loc;
    unsigned long long limit;
    char *assembly;
};

struct comment
{
    unsigned long long loc;
    char *comment;
};

struct function {
    unsigned long long loc;
    unsigned long long end_loc;
    int type;
    int bkmkidx; // correspoding bookmarks' index
};

struct book {
    sqlite3 *db;
    char *name;
    int struct_count;
    int struct_max;
    struct bin_struct *structs;
    unsigned int entry_point;

    /*temporary data, introduced by mistake, remove them in the future!*/
    struct fragbook* fbook;
    /*do not save these data!*/
};

/**
 * @brief Initialize a new book.
 * 
 * @param book_name The name of the book.
 * @param bookmark_num 
 * @param patch_count 
 * @param comment_count 
 * @return struct book* 
 */
struct book *book_init (const char *book_name);

/**
 * @brief Open a book.
 * 
 * @param book_name The name of the book.
 * @return struct book* The open book. NULL if failed.
 */
struct book *book_open (const char *book_name);

/**
 * @brief Save all changes to the book and close the book.
 * 
 * @param book The book need to close.
 * @return int 0 if success.
 */
int book_close (struct book* book);

/**
 * @brief Load a struct descripter file for a book.
 * 
 * @param book The recv book.
 * @param filename The struct descripter file.
 * @return int 0 if success.
 */
int book_ldstruct(struct book* book, const char *filename);

/**
 * @brief Export structs by loaded descripter. Create/Refill data tables in the meantime.
 * 
 * @param book The mod book.
 * @param buffer The loaded elf buffer.
 * @return int 0 if success.
 */
int book_exstruct(struct book* book, unsigned char *buffer);

/**
 * @brief Import structs by loaded descripter and data tables.
 * 
 * @param book The guide book.
 * @param buffer The loaded elf buffer.
 * @return int 0 if success.
 */
int book_imstruct(struct book* book, unsigned char *buffer);

/**
 * @brief Execute script file on book.
 * 
 * @param filename The script filename.
 * @return int 0 if success.
 */
int book_exec(const char *filename);

/**
 * @brief Set a new string encoder for a book.
 * 
 * @param book The book.
 * @param callback a function pointer point to a function that recv 2 params. The utf8_str
 * is the string that need to be encoded, and the buffer is the target that need to be write, this
 * function should return a int which is the byte this function written to the buffer. -1 should
 * be returned if utf8_str is impossible to encode.
 */
void book_strenc_set(struct book* book, int (*callback)(const char *utf8_str, unsigned char *buffer));

/**
 * @brief Set a new string decoder for a book.
 * 
 * @param book The book.
 * @param callback a function pointer point to a funciton that recv 2 params. The utf8_str
 * is a pointer to the char* which need to point to a NEW malloc() assigned memory which contains
 * the decoded utf8 string, the raw_str is the pointer point to the raw string. And, it should return
 * the length it handled to raw_str. -1 should be returned if the raw_str is unrecognizable.
 */
void book_strdec_set(struct book* book, int *(*callback)(unsigned char *raw_str, char **utf8_str));

/**
 * @brief Add a function's head location to the book for futher check.
 * 
 * @param book 
 * @param func_bgn 
 */
void book_anal_func (struct book *book, int func_bgn, int bookmark_id);

/**
 * @brief assume an start point of ctrl flow.
 * 
 * @param book 
 * @param flow_bgn 
 * @return int 
 */
int book_anal_ctrlf_sus (struct book *book, int flow_bgn);

/**
 * @brief Confirm assumed ctrlflow and register them as an function.
 * 
 * @param book 
 */
void book_anal_ctrlf_cnf (struct book *book);

/**
 * @brief Update ctrl flow information.
 * 
 * @param book 
 * @param flow_bgn 
 * @param new_loc 
 */
void book_anal_ctrlf_upd (struct book *book, int flow_bgn, int new_loc);

/**
 * @brief Confirm a function's information.
 * 
 * @param book The book
 * @param func_bgn The function's begining.
 * @param func_end The ending of the function should be set.
 * @return int 
 */
int book_anal_func_cnf (struct book *book, int func_bgn, int func_end);

int book_anal_ctrlf_qry (struct book *book, int loc);

//void book_anal_func_upd (struct book *book, int func_bgn, int func_end);

/**
 * @brief Query if a location is included in a confirmed function.
 * 
 * @param book The book.
 * @param loc The location need to be checked.
 * @return int 0 if not, or the func_bgn.
 */
int book_anal_func_qry (struct book *book, int loc);

/**
 * @brief Try get a location of a function that have not been analyzed yet.
 * 
 * @param book The book.
 * @return int -1 if no such function, or the location.
 */
int book_anal_func_get (struct book *book);

/**
 * @brief Create a new bookmark to the book. Success return the index, or -1.
 * 
 * @param book The operating book.
 * @param mark_name The bookmark's name
 * @return int Index of the bookmark if success, -1 if failed.
 */
int bookmark_mk (struct book *book, const char *mark_name);

/**
 * @brief Destroy a bookmark object without save.
 * 
 * @param bm The object need to be destroyed.
 * @return int 0 if success.
 */
int bookmark_free (struct bookmark *bm);

/**
 * @brief Create a new bookmark with given format and argument (by sprintf).
 * 
 * @param book The operating book.
 * @param mark_name_fmt The bookmark's name
 * @param ... format arguments
 * @return int Index of the bookmark if success, -1 if failed.
 */
int bookmark_mkf (struct book *book, const char *mark_name_fmt, ...);

/**
 * @brief Commit the changes done to the bookmark and destroy the object.
 * 
 * @param book The book note.
 * @param bookmark The bookmark need to be update.
 * @return int 0 if success, otherwise failed.
 */
int bookmark_cmt (struct book *book, struct bookmark *bookmark);

/**
 * @brief Update the changes done to the bookmark to the book.
 * 
 * @param book The book.
 * @param bookmark The bookmark nedd to be update.
 * @return int 0 if success, otherwise failed.
 */
int bookmark_upd (struct book *book, struct bookmark *bookmark);

/**
 * @brief Remove the given bookmark from the book.
 * 
 * @param book The operating book.
 * @param mark_index The bookmark's index.
 * @return int 0 if success.
 */
int bookmark_rm (struct book *book, int mark_index);

/**
 * @brief Get the first matching book mark by location.
 * 
 * @param book 
 * @param location 
 * @return struct bookmark * The first match if success, NULL if not found.
 */
struct bookmark *bookmark_lget (struct book *book, unsigned long long location);

/**
 * @brief Get bookmark's index by it's name.
 * 
 * @param book The operating book.
 * @param name The bookmark's name.
 * @return int The first matched bookmark's index if success, -1 if not found.
 */
int bookmark_search (struct book *book, const char *name);

/**
 * @brief Rename the specified bookmark.
 * 
 * @param bookmark The book mark need to be rename.
 * @param new_name The new name.
 * @return int 0 if success.
 */
int bookmark_ren (struct bookmark *bookmark, const char *new_name);

/**
 * @brief Rename the bookmark in format. (sprintf)
 * 
 * @param bookmark The bookmark need to be renamed.
 * @param new_name_fmt The new name's format.
 * @param ... The arguments.
 * @return int 0 if success.
 */
int bookmark_renf (struct bookmark *bookmark, const char *new_name_fmt, ...);

/**
 * @brief Geth bookmark by it's index.
 * 
 * @param book The operating book.
 * @param index The bookmark's index.
 * @return struct bookmark* The specified bookmark if success, NULL if not found.
 */
struct bookmark *bookmark_get (struct book *book, int index);

/**
 * @brief Make a confussing point that hint a un-expected problem find here.
 * 
 * @param book The operating book.
 * @param name The name of the CP.
 * @param loc The location of the CP.
 * @param reason The reason why this CP is thrown.
 * @return int 
 */
int confp_mk (struct book *book, const char *name, unsigned long long loc, const char *reason);

int confp_mkf (struct book *book, unsigned long long loc, const char *reason, const char *name_fmt, ...);

int sturct_mk (struct book *book, const char *name);

int struct_rm (struct book *book, int index);

int struct_search (struct book *book, const char *name);

struct bin_struct *struct_get (struct book *book, int index);

#endif