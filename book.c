#include "book.h"

#ifdef BOOK_DEBUG
#ifndef MK_STR
#define MK_STRR(x) #x
#define MK_STR(x) MK_STRR(x)
#endif
#define debug(...) printf("[book.c:" MK_STR(__LINE__) "] "__VA_ARGS__)
#else
#define debug(...)
#endif

static char fmt_buf[1024];

struct book *book_init (const char *book_name) {
    struct book *book = (struct book*)malloc(sizeof(struct book));
    sprintf(fmt_buf, "%s.db", book_name);
    book->name = strdup(book_name);

    int ret = sqlite3_open(fmt_buf, &book->db);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "book_init: Can't open sqlite database: %s\n", sqlite3_errmsg(book->db));
        sqlite3_close(book->db);
        return NULL;
    }
    /* Database creating */
    sqlite3 *db = book->db;
    char *errmsg = NULL;
    ret = sqlite3_exec(db, "CREATE TABLE bookmarks(id integer primary key autoincrement, "
        "name text not null unique, loc int, ref_loc int,"
        "ref_oloc int, direction int, type int, array_length int);",
        NULL, NULL, &errmsg);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "book_init: cannot create table: bookmarks. (%s). Ignoring...\n", errmsg);
    }

    ret = sqlite3_exec(db, "CREATE TABLE comments(id integer primary key autoincrement, "
        "loc int not null, comment text not null);", NULL, NULL, &errmsg);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "book_init: cannot create table: comments. (%s). Ignoring...\n", errmsg);
    }

    ret = sqlite3_exec(db, "CREATE TABLE functions(loc int primary key,"
        " end_loc int not null, type int not null, bkmkidx int not null,"
        " foreign key(bkmkidx) references bookmarks(id));", NULL, NULL, &errmsg);
    if (ret) {
        fprintf(stderr, "book_init: cannot create table: functions. (%s). Ignoring...\n", errmsg);
    }

    ret = sqlite3_exec(db, "CREATE TABLE problems(id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "name TEXT NOT NULL, loc INT NOT NULL, reason TEXT NOT NULL);", NULL, NULL, &errmsg);
    if (ret) {
        fprintf(stderr, "book_init: cannot create table: problems. (%s). Ignoring...\n", errmsg);
    }
    /* End of Database creating */

    const int strdefc = 16;
    book->struct_count = 0;
    book->struct_max = strdefc;
    book->structs = (struct bin_struct*)malloc(strdefc * sizeof(struct bin_struct));

    memset(book->xtr_regid, 0, sizeof(book->xtr_data));
    return book;
}

int book_xtrinit(struct book *book) {
    for(int i = 0; i < 32; ++i) {
        if (book->xtr_regid[i]) {
            int ret = book->xtr_initializers[i](book->xtr_regid[i], "", book->xtr_data[i]);
            if (ret) return ret;
        }
    }
    return 0;
}

int book_close (struct book* book) {
    sqlite3_close (book->db);
    if (book->fbook) fragbook_destroy(book->fbook);
    free (book->name);
    free (book->structs);
    free (book);

    return 0;
}

void book_anal_func (struct book *book, int func_bgn, int bookmark_id) {
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(book->db, "INSERT INTO functions VALUES (?, -1, 0, ?);", -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, func_bgn);
    sqlite3_bind_int(stmt, 2, bookmark_id);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

int book_anal_ctrlf_sus (struct book *book, int flow_bgn) {
    if (book->fbook == NULL) book->fbook = fragbook_init(128);
    frag_reg(book->fbook, 0, flow_bgn, 0);

    return 0;
}

int book_anal_ctrlf_qry (struct book *book, int loc) {
    int ans = frag_query(book->fbook, loc, loc);
    return ans == -1 ? 0 : 1;
}

void book_anal_ctrlf_cnf (struct book *book) { // TODO: if the control flow jump over, this can be wrong.
    fragbook_defrag(book->fbook);
    struct frag_t* f = frag_get(book->fbook, 0);
    book_anal_func_cnf(book, f->begin, f->end);

    fragbook_destroy(book->fbook);
    book->fbook = NULL;
}

void book_anal_ctrlf_upd (struct book *book, int flow_bgn, int new_loc) {
    frag_lget(book->fbook, flow_bgn)->end = new_loc;
}

int book_anal_func_cnf (struct book *book, int func_bgn, int func_end) {
    debug("book_anal_func_cnf: %08X:%08X\n", func_bgn, func_end);
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(book->db, "UPDATE functions SET end_loc = ?, type = 1 WHERE loc = ?;", -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, func_end);
    sqlite3_bind_int(stmt, 2, func_bgn);
    int ret = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return ret == SQLITE_DONE ? 0 : ret;
}

int book_anal_func_get (struct book *book) {
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(book->db, "SELECT * FROM functions WHERE type = 0;", -1, &stmt, NULL);
    int ret = sqlite3_step(stmt);
    if (SQLITE_ROW == ret) {
        ret = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        return ret;
    }
    if (SQLITE_ERROR == ret) {
        fprintf(stderr, "Query Error: %s\n", sqlite3_errmsg(book->db));
        return -1;
    }

    sqlite3_finalize(stmt);
    return -1;
}

int book_anal_func_qry (struct book *book, int loc) {
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(book->db, "SELECT * FROM functions WHERE loc <= ?1 AND end_loc >= ?1;", -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, loc);
    int ret;
    if(SQLITE_ROW == (ret = sqlite3_step(stmt))) {
        int ret = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        return ret;
    }
    else if (ret != SQLITE_DONE) {
        fprintf(stderr, "book_anal_func_qry: Query failed, SQLite Error: %s. Aborted.\n", sqlite3_errmsg(book->db));
    }
    sqlite3_finalize(stmt);
    return 0; // not found
}

int book_func_end (struct book *book, int loc) {
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(book->db, "SELECT * FROM functions WHERE loc = ?;", -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, loc);
    int ret;
    if(SQLITE_ROW == (ret = sqlite3_step(stmt))) {
        int ret = sqlite3_column_int(stmt, 1);
        sqlite3_finalize(stmt);
        return ret;
    }
    else if (ret != SQLITE_DONE) {
        fprintf(stderr, "book_anal_func_qry: Query failed, SQLite Error: %s. Aborted.\n", sqlite3_errmsg(book->db));
    }
    sqlite3_finalize(stmt);
    return 0; // not found
}

int bookmark_mk (struct book *book, const char *mark_name) {
    sqlite3_stmt *stmt;
    int ret = sqlite3_prepare_v2(book->db, "INSERT INTO bookmarks VALUES (NULL, ?, NULL, NULL, NULL, NULL, NULL, NULL);", -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, mark_name, strlen(mark_name), SQLITE_STATIC);
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_DONE) {
        if (sqlite3_errcode(book->db) == 0x13) return -2;

        fprintf(stderr, "bookmark_mk: cannot make %s to bookmarks. 0x%X(%s). Abort\n", mark_name, sqlite3_errcode(book->db), sqlite3_errmsg(book->db));
        return -1;
    }
    sqlite3_finalize(stmt);

    ret = sqlite3_prepare_v2(book->db, "SELECT * FROM bookmarks WHERE name = ?;", -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, mark_name, strlen(mark_name), SQLITE_STATIC);
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        fprintf(stderr, "bookmark_mk: cannot select record from bookmarks. (%s). Abort.\n", sqlite3_errmsg(book->db));
        return -1;
    }
    ret = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);

    return ret;
}

int bookmark_rm (struct book *book, int mark_index) {
    sqlite3_stmt * stmt;
    sqlite3_prepare_v2(book->db, "DELETE FROM bookmarks WHERE id = ?;", -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, mark_index);
    if (sqlite3_step(stmt)) {
        sqlite3_finalize(stmt);
        fprintf(stderr, "bookmark_rm: cannot delete record from bookmarks. (%s). Abort.\n", sqlite3_errmsg(book->db));
        return -1;
    }
    sqlite3_finalize(stmt);

    return 0;
}

int bookmark_mkf (struct book *book, const char *mark_name_fmt, ...) {
    va_list list;
    va_start(list, mark_name_fmt);

    vsprintf(fmt_buf, mark_name_fmt, list);

    return bookmark_mk(book, fmt_buf);
}

int confp_mkf (struct book *book, unsigned long long loc, const char *reason, const char *name_fmt, ...) {
    va_list list;
    va_start(list, name_fmt);

    vsprintf(fmt_buf, name_fmt, list);

    return bookmark_mk(book, fmt_buf);
}

int bookmark_cmt (struct book *book, struct bookmark *bookmark) {
    int ret = bookmark_upd (book, bookmark);
    if (ret) {
        return ret;
    }
    bookmark_free(bookmark);
    return 0;
}

int bookmark_free (struct bookmark *bm) {
    free(bm->name);
    free(bm);
    return 0;
}

int bookmark_upd (struct book *book, struct bookmark *bookmark) {
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(book->db, "UPDATE bookmarks "
        "SET name=?, loc=?, ref_loc=?, ref_oloc=?, direction=?, type=?, array_length=? "
        "WHERE id = ?;", -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, bookmark->name, -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 2, bookmark->loc);
    sqlite3_bind_int64(stmt, 3, bookmark->ref_loc);
    sqlite3_bind_int64(stmt, 4, bookmark->ref_oloc);
    sqlite3_bind_int(stmt, 5, bookmark->direction);
    sqlite3_bind_int(stmt, 6, bookmark->type);
    sqlite3_bind_int(stmt, 7, bookmark->array_length);
    sqlite3_bind_int(stmt, 8, bookmark->id);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        fprintf(stderr, "bookmark_upd: cannot update bookmark id=%d. (%s). Abort.\n", bookmark->id, sqlite3_errmsg(book->db));
        return -1;
    }
    sqlite3_finalize(stmt);

    return 0;
}

int bookmark_ren (struct bookmark *bookmark, const char *new_name) {
    if (bookmark->name == NULL) return -1;

    free(bookmark->name);
    bookmark->name = strdup(new_name);
    return 0;
}

int bookmark_renf (struct bookmark *bookmark, const char *new_name_fmt, ...) {
    va_list args;
    va_start(args, new_name_fmt);

    vsprintf(fmt_buf, new_name_fmt, args);

    return bookmark_ren(bookmark, fmt_buf);
}

struct bookmark *_bm_first_where (struct book *book, sqlite3_stmt *stmt) {
    while (sqlite3_step(stmt) != SQLITE_DONE) {
        struct bookmark *ans = (struct bookmark*)malloc(sizeof(struct bookmark));
        ans->id = sqlite3_column_int(stmt, 0);
        ans->name = strdup((const char *)sqlite3_column_text(stmt, 1));
        ans->loc = sqlite3_column_int64(stmt, 2);
        ans->ref_loc = sqlite3_column_int64(stmt, 3);
        ans->ref_oloc = sqlite3_column_int64(stmt, 4);
        ans->direction = sqlite3_column_int(stmt, 5);
        ans->type = sqlite3_column_int(stmt, 6);
        ans->array_length = sqlite3_column_int(stmt, 7);

        sqlite3_finalize(stmt);
        return ans;
    }
    sqlite3_finalize(stmt);

    return NULL;
}

void _bm_each_where (struct book *book, sqlite3_stmt *stmt, int (*callback)(struct bookmark*)) {
    while (sqlite3_step(stmt) != SQLITE_DONE) {
        struct bookmark *ans = (struct bookmark*)malloc(sizeof(struct bookmark));
        ans->id = sqlite3_column_int(stmt, 0);
        ans->name = strdup((const char *)sqlite3_column_text(stmt, 1));
        ans->loc = sqlite3_column_int64(stmt, 2);
        ans->ref_loc = sqlite3_column_int64(stmt, 3);
        ans->ref_oloc = sqlite3_column_int64(stmt, 4);
        ans->direction = sqlite3_column_int(stmt, 5);
        ans->type = sqlite3_column_int(stmt, 6);
        ans->array_length = sqlite3_column_int(stmt, 7);

        if(callback(ans)) {
            bookmark_free(ans);
            sqlite3_finalize(stmt);
            return;
        }
        bookmark_cmt(book, ans);
    }
    sqlite3_finalize(stmt);
}

struct bookmark *bookmark_lget (struct book *book, unsigned long long location) {
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(book->db, "SELECT * FROM bookmarks WHERE loc = ?;", -1, &stmt, NULL);
    sqlite3_bind_int64(stmt, 1, location);
    
    return _bm_first_where (book, stmt);
}

struct bookmark *bookmark_get (struct book *book, int index) {
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(book->db, "SELECT * FROM bookmarks WHERE id = ?;", -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, index);

    return _bm_first_where (book, stmt);
}

int confp_mk (struct book *book, const char *name, unsigned long long loc, const char *reason) {
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(book->db, "INSERT INTO problems VALUES (NULL, ?, ?, ?);", -1, &stmt, NULL);
    sqlite3_bind_int64(stmt, 2, loc);
    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, reason, -1, SQLITE_STATIC);

    int ret = sqlite3_step(stmt);
    if (ret != SQLITE_DONE) return -1;

    sqlite3_finalize(stmt);
    return 0;
}


int book_regxtr(struct book *book, int id, xtr_callback finalizer, xtr_callback executer, xtr_callback initializer, void *data_ptr, const char *name) {
    for (int i = 0; i < 32; ++i) {
        if (book->xtr_regid[i] == 0) {
            book->xtr_regid[i] = id;
            book->xtr_data[i] = data_ptr;
            book->xtr_initializers[i] = initializer;
            book->xtr_executers[i] = executer;
            book->xtr_finializers[i] = finalizer;

            return 0;
        }
    }
    return -1;
}

void *book_getxtrdata(struct book *book, int reg_num) {
    int id = book_get_xtridx_byid(book, reg_num);
    return id != -1 ? book->xtr_data[id] : NULL;
}

int book_xtrinvoke(struct book *book, int reg_num, const char *parameter) {
    int id = book_get_xtridx_byid(book, reg_num);
    if (id != -1) {
        return book->xtr_executers[id](reg_num, parameter, book->xtr_data[id]);
    }
    return -1;
}

int book_get_xtridx_bynm(struct book* book, const char *name) {
    for (int i = 0; i < 32; ++i) {
        if (!strcmp(book->xtr_nm[i], name)) {
            return i;
        }
    }
    return -1;
}

int book_get_xtridx_byid(struct book* book, int id) {
    for (int i = 0; i < 32; ++i) {
        if (book->xtr_regid[i] == id) {
            return i;
        }
    }
    return -1;
}

#undef debug
