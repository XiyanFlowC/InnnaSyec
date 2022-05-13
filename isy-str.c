#include "isy-str.h"

int isystrfinalizer(int reg_num, const char *param, void *data) {
    return 0;
}

long long conv_utf8_code(const char *str) {
    int byte_count = 0;
    char first_byte = str[0];
    while (first_byte & 0x80) {
        byte_count++;
        first_byte <<= 1;
    }

    long long ans = str[0] & (0xFF >> byte_count);

    for (int i = 1; i < byte_count; ++i) {
        ans <<= 6;
        ans |= str[i] & 0x3F;
    }

    return ans;
}

int utf8_dbcs_length(const char *str) {
    int length = 0;
    while (*str) {
        if (*str & 80) {
            int byte_count = 0;
            char first_byte = str[0];
            while (first_byte & 0x80) {
                byte_count++;
                first_byte <<= 1;
            }
            length += 2;
            str += byte_count;
        } else ++length, ++str;
    }
    return length;
}

int isyldtblfunc(FILE *file, struct isy_str *data) {
    char chbuf[16];
    unsigned int key;
    
    while(EOF != fscanf(file, "%x=%16s", &key, chbuf)) {
        union bst_store v = {.i64 = conv_utf8_code(chbuf)}, revv = {.i64 = key};
        data->dec_tbl = bst_insert(data->dec_tbl, key, v);
        data->enc_tbl = bst_insert(data->enc_tbl, v.i64, revv);
    }

    return 0;
}

int isystrexecuter(int reg_num, const char *param, void *data) {
    char buffer[1024];
    int leng = get_term2(buffer, param);
    if (strcmp(buffer, "table") == 0) {
        FILE *tbl = fopen(param + leng, "r");
        isyldtblfunc(tbl, (struct isy_str *)data);
        fclose(tbl);
    }
    return 0;
}

int isystrinitializer(int reg_num, const char *param, void *data) {
    struct isy_str *p = (struct isy_str *)data;
    char *errmsg;
    if (SQLITE_OK != sqlite3_exec(p->db, "CREATE TABLE stringdb (id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "str STRING, loc INT NOT NULL, ref INT);", NULL, NULL, &errmsg)) {
            
        fprintf(stderr, "isy-str: Cannot create table stringdb, %s. Aborted.\n", errmsg);
    }

    return 0;
}

int noenc(const char *str, unsigned char *buffer, struct isy_str* data) {
    strcpy((char *)buffer, str);
    return strlen(str);
}

int nodec(unsigned char *raw, char **result, struct isy_str* data) {
    *result = strdup((char *)raw);
    return strlen((char *)raw);
}

int tblenc(const char *str, unsigned char *buffer, struct isy_str* data) {
    int length = utf8_dbcs_length(str);

    while (*str) {
        long long code = conv_utf8_code(str++);

        if (code <= 0x7f) {
            *buffer++ = code & 0xff;
            length += 1;
            continue;
        }
        
        if (code > 0x7f) {
            ++str;
        }
        if (code > 0x7ff) {
            ++str;
        }
        if (code > 0xffff) {
            ++str;
        }
        if (code > 0x1fffff) {
            ++str;
        }

        code = bst_query(data->enc_tbl, code).i64;

        *buffer++ = (code & 0xff00) >> 8;
        *buffer++ = code & 0xff;

        length += 2;
    }
    return length;
}

int tbldec(unsigned char *raw, char **result, struct isy_str* data) {
    int length;
    char *str = (char *)raw;
    // predict buffer size:
    while(*str) {
        if (*str & 0x80) {
            length += 3;
            str += 2;
        }
        else length++, str++;
    }

    char *buffer = (char *)malloc(length);
    char *p = buffer;
    while(*str) {
        long long code;
        if (*str & 0x80) {
            code = *str++ << 8;
            code |= *str;
        } else code = *str;

        code = bst_query(data->dec_tbl, code).i64;

        if (code > 0x1fffff) {
            *p++ = 0xf0 | ((code >> 18) & 0x7);
            *p++ = 0x80 | ((code >> 12) & 0x3f);
            *p++ = 0x80 | ((code >> 6) & 0x3f);
            *p++ = 0x80 | (code & 0x3f);
        }
        else if (code > 0xffff) {
            *p++ = 0xe0 | ((code >> 12) & 0xf);
            *p++ = 0x80 | ((code >> 6) & 0x3f);
            *p++ = 0x80 | (code & 0x3f);
        }
        else if (code > 0x7ff) {
            *p++ = 0xc0 | ((code >> 6) & 0x1f);
            *p++ = 0x80 | (code & 0x3f);
        }
        else {
            *p++ = code & 0x7f;
        }
    }
    *p = 0;
    p = strdup(buffer);
    free(buffer);
    return p;
}

int isy_strsearch(const char *str, int str_len, const char *memory, int limit, 
    int (*callback)(int offset, void *info), void *info) { // KMP

    int leng = str_len; // strlen(str);
    short *pmt = (short *)malloc(sizeof(short) * leng);

    pmt[0] = 0;
    int j = 0;
    // pattern preproc
    for (int i = 0; i < leng; ++i) {
        if (str[i] == str[j]) {
            pmt[i] = ++j;
        } else {
            if (j == 0) {
                pmt[i] = 0;
            } else {
                j = pmt[j - 1];
                --i;
            }
        }
    }

    j = 0;
    int ret = 0;
    // search
    for (int i = 0; i < limit; ++i) {
        if (str[i] == str[j]) {
            if (++j >= leng) {
                ++ret;
                if (ret = callback(i - leng + 1, info)) return ret;
            }
        } else {
            j = pmt[j];
            --i;
        }
    }
    free(pmt);

    return ret;
}

void book_strdft_set(struct book *book, int plan) {
    if (plan == 0) {
        book_strenc_set(book, noenc);
        book_strdec_set(book, nodec);
    } else if (plan == 1) {
        book_strenc_set(book, tblenc);
        book_strdec_set(book, tbldec);
    }
}

void book_strenc_set(struct book* book, int (*callback)(const char *utf8_str, unsigned char *buffer, struct isy_str* data)) {
    struct isy_str *data = (struct isy_str*)book_getxtrdata(book, ISY_STR_ID);
    data->strenc = callback;
}

void book_strdec_set(struct book* book, int (*callback)(unsigned char *raw_str, char **utf8_str, struct isy_str* data)) {
    struct isy_str *data = (struct isy_str*)book_getxtrdata(book, ISY_STR_ID);
    data->strdec = callback;
}

void book_strinit(struct book* book) {
    struct isy_str *data = (struct isy_str *)malloc(sizeof(struct isy_str));
    memset(data, 0, sizeof(struct isy_str));
    data->db = book->db;
    book_regxtr(book, ISY_STR_ID, isystrfinalizer, isystrexecuter, isystrinitializer, data, ISY_STR_NAME);
    book_strdft_set(book, 0);
}

int is_valid_second(char ch) {
    return (ch >= 0x40 && ch <= 0x7E) || (ch >= 80 && ch <= 0xFC);
}

int is_valid_first(char ch) {
    if ((ch >= 0x20 && ch <= 0x7e) || (ch == '\n' || ch == '\r' || ch == '\0')) return 1;
    if ((ch >= 0x81 && ch <= 0x9f) || (ch >= 0xE0 && ch <= 0xEF)) return 2;
    return 0;
}

int is_valid_ch(const char *str) {
    if (is_valid_first(*str) == 1) return 1;
    if (is_valid_first(*str) == 2) return is_valid_second(*++str);
    return 0;
}

int is_valid_str(const char *str) {
    for (int i = 0; i < 4; ++i) {
        int ret = is_valid_ch(str);
        if (!ret) return 0;
        str += ret;
    }
    return 1;
}

void book_str_anal(struct book* book, unsigned char *memory, int begin, int end) {
    for (int i = begin; i < end; i += 4) {
        if (is_valid_str(memory + i)) {
            int idx = bookmark_mkf(book, "str_%X", i);
            if (idx == -1) {
                fprintf(stderr, "failed to create bookmark.\n");
                return;
            }
            struct bookmark *bm = bookmark_get(book, idx);
            bm->loc = i;
            bm->type = MARK_TYPE_STRING;
            bookmark_cmt(book, bm);
        }
    }
}
