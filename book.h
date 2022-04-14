#ifndef BOOK_H
#define BOOK_H

#pragma once

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <sqlite3.h>

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
 * @brief Add a function's head location to the book for futher check.
 * 
 * @param book 
 * @param func_bgn 
 */
void book_anal_func (struct book *book, int func_bgn);

/**
 * @brief Create a new bookmark to the book. Success return the index, or -1.
 * 
 * @param book The operating book.
 * @param mark_name The bookmark's name
 * @return int Index of the bookmark if success, -1 if failed.
 */
int bookmark_mk (struct book *book, const char *mark_name);

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
 * @brief Get book mark by location.
 * 
 * @param book 
 * @param location 
 * @return struct bookmark * The first match if success, NULL if not found.
 */
struct bookmark *bookmark_lget (struct book *book, unsigned int location);

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

int sturct_mk (struct book *book, const char *name);

int struct_rm (struct book *book, int index);

int struct_search (struct book *book, const char *name);

struct bin_struct *struct_get (struct book *book, int index);

#endif