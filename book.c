#include "book.h"

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
    /* Database seeding */
    sqlite3 *db = book->db;
    char *errmsg = NULL;
    ret = sqlite3_exec(db, "CREATE TABLE bookmarks(id int primary key autoincrement, "
        "name text not null, loc int, ref_loc int,"
        "ref_oloc int, direction int, type int, array_length int);",
        NULL, NULL, errmsg);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "book_init: cannot create table: bookmarks. (%s). Ignoring...\n", errmsg);
    }

    ret = sqlite3_exec(db, "CREATE TABLE comments(id int primary key autoincrement, "
        "loc int not null, comment text not null);", NULL, NULL, errmsg);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "book_init: cannot create table: comments. (%s). Ignoring...\n", errmsg);
    }

    ret = sqlite3_exec(db, "CREATE TABLE functions(loc int primary key,"
        " end_loc int not null, type int not null, bkmkidx int not null,"
        " foreign key(bkmkidx) references bookmarks(id));", NULL, NULL, errmsg);
    if (ret) {
        fprintf(stderr, "book_init: cannot create table: functions. (%s). Ignoring...\n", errmsg);
    }
    /* End of Database seeding */

    const int strdefc = 16;
    book->struct_count = 0;
    book->struct_max = strdefc;
    book->structs = (struct bin_struct*)malloc(strdefc * sizeof(struct bin_struct));
    return book;
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
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(book->db, "UPDATE functions SET end_loc = ? WHERE loc = ?;", -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, func_end);
    sqlite3_bind_int(stmt, 2, func_bgn);
    int ret = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return ret;
}

int book_anal_func_qry (struct book *book, int loc) {
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(book->db, "SELECT * FROM functions WHERE loc >= ?1 AND end_loc <= ?1;", &stmt, NULL);
    sqlite3_bind_int(stmt, 1, loc);
    if(SQLITE_OK == sqlite3_step(stmt)) {
        return sqlite3_column_int(stmt, 1);
    }
    else {
        fprintf(stderr, "book_anal_ctrlf_qry: Query failed, SQLite Error: %d. Aborted.\n", sqlite3_errcode());
    }
    sqlite3_finalize(stmt);
    return 0; // not found
}

int bookmark_mk (struct book *book, const char *mark_name) {
    char *errmsg;
    sqlite3_stmt *stmt;
    int ret = sqlite3_prepare_v2(book->db, "INSERT INTO bookmarks VALUES (NULL, ?);", -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, mark_name, strlen(mark_name), SQLITE_STATIC);
    ret = sqlite3_step(stmt);
    if (ret) {
        sqlite3_finalize(stmt);
        fprintf(stderr, "bookmark_mk: cannot insert record to bookmarks. (%s). Abort.\n", sqlite3_errmsg(book->db));
        return -1;
    }
    sqlite3_finalize(stmt);

    return 0;
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
    va_start(mark_name_fmt, list);

    vsprintf(fmt_buf, mark_name_fmt, list);

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

    if (sqlite3_step(stmt)) {
        sqlite3_finalize(stmt);
        fprintf(stderr, "bookmark_upd: cannot update bookmarks. (%s). Abort.\n", sqlite3_errmsg(book->db));
        return -1;
    }
    sqlite3_finalize(stmt);

    return 0;
}

int bookmark_ren (struct bookmark *bookmark, const char *new_name) {
    if (bookmark->name == NULL) return -1;

    free(bookmark->name);
    bookmark->name = strdup(new_name);
}

int bookmark_renf (struct bookmark *bookmark, const char *new_name_fmt, ...) {
    va_list args;
    va_start(new_name_fmt, args);

    vsprintf(fmt_buf, new_name_fmt, args);

    return bookmark_ren(bookmark, fmt_buf);
}

struct bookmark *_bm_first_where (struct book *book, sqlite3_stmt *stmt) {
    while (sqlite3_step(stmt) == SQLITE_OK) {
        struct bookmark *ans = (struct bookmark*)malloc(sizeof(struct bookmark));
        ans->id = sqlite3_column_int(stmt, 1);
        ans->name = strdup(sqlite3_column_text(stmt, 2));
        ans->loc = sqlite3_column_int64(stmt, 3);
        ans->ref_loc = sqlite3_column_int64(stmt, 4);
        ans->ref_oloc = sqlite3_column_int64(stmt, 5);
        ans->direction = sqlite3_column_int(stmt, 6);
        ans->type = sqlite3_column_int(stmt, 7);
        ans->array_length = sqlite3_column_int(stmt, 8);

        sqlite3_finalize(stmt);
        return ans;
    }
    sqlite3_finalize(stmt);

    return NULL;
}

void _bm_each_where (struct book *book, sqlite3_stmt *stmt, int (*callback)(struct bookmark*)) {
    while (sqlite3_step(stmt) == SQLITE_OK) {
        struct bookmark *ans = (struct bookmark*)malloc(sizeof(struct bookmark));
        ans->id = sqlite3_column_int(stmt, 1);
        ans->name = strdup(sqlite3_column_text(stmt, 2));
        ans->loc = sqlite3_column_int64(stmt, 3);
        ans->ref_loc = sqlite3_column_int64(stmt, 4);
        ans->ref_oloc = sqlite3_column_int64(stmt, 5);
        ans->direction = sqlite3_column_int(stmt, 6);
        ans->type = sqlite3_column_int(stmt, 7);
        ans->array_length = sqlite3_column_int(stmt, 8);

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

struct bookmark *bookmark_get (struct book *book, unsigned int index) {
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(book->db, "SELECT * FROM bookmarks WHERE id = ?;", -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, index);

    return _bm_first_where (book, stmt);
}
