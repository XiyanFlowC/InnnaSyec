#include "fragcov.h"

int is_frag_overlap (struct frag_t *a, struct frag_t *b) {
    return (a->begin >= b->begin && a->begin <= b->end) ||
        (a->end >= b->begin && a->end <= b->end);
}

int is_frag_include (struct frag_t *sub, struct frag_t *obj) {
    return obj->begin >= sub->begin && obj->end <= sub->end;
}

int is_frag_ident (struct frag_t *a, struct frag_t *b) {
    return a->begin == b->begin && a->end == b->end;
}

int frag_merge (struct frag_t *dst, struct frag_t *tar) {
    if (!is_frag_overlap(dst, tar)) return -1; // no overlap, can't merge

    dst->begin = dst->begin < tar->begin ? dst->begin : tar->begin;
    dst->end = dst->end > tar->end ? dst->end : tar->end;
    tar->_sta = -1;

    return 0;
}

struct fragbook *fragbook_init (int frag_max) {
    struct fragbook *ans = (struct fragbook *)malloc(sizeof(struct fragbook));
    if (ans == NULL) return NULL;

    ans->frag_max = frag_max;
    ans->frags = (struct frag_t *)malloc(sizeof(struct frag_t) * frag_max);
    if (ans->frags == NULL) {
        free(ans);
        return NULL;
    }

    ans->frag_count = 0;

    return ans;
}

void fragbook_destroy (struct fragbook *fbook) {
    free(fbook->frags);
    free(fbook);
}

int fragbook_defrag (struct fragbook *fbook) {
    for (int i = 0; i < fbook->frag_count; ++i) {
        if (!fbook->frags[i]._sta)
            for (int j = i + 1; j < fbook->frag_count; ++j) {
                if (!fbook->frags[j]._sta) {
                    frag_merge (fbook->frags + i, fbook->frags + j);
                }
            }
    }

    return 0;
}

int frag_arran (struct fragbook *fbook) {
    int dst, src;

    for (src = 0; src < fbook->frag_count; ++ src) { // set two pointers to the begin of gap
        if (fbook->frags[src]._sta) {
            dst = src;
            break;
        }
    }

    while (src < fbook->frag_count) {
        while (src < fbook->frag_count && fbook->frags[src]._sta) ++src;
        while (dst < fbook->frag_count && !fbook->frags[dst]._sta) ++dst;
        if (src >= fbook->frag_count) break;
        assert(dst < src);

        memcpy(fbook->frags + dst++, fbook->frags + src, sizeof(struct frag_t));
        fbook->frags[src++]._sta = -1;
    }
    return 0;
}

int frag_reg (struct fragbook *fbook, int frag_type, unsigned int begin, unsigned int end) {
    if (fbook->frag_count == fbook->frag_max) {
        frag_arran(fbook);
        if (fbook->frag_count == fbook->frag_max) return -1; // fbook is full
    }

    fbook->frags[fbook->frag_count].begin = begin;
    fbook->frags[fbook->frag_count].end = end;
    fbook->frags[fbook->frag_count]._sta = 0;
    
    fbook->frags[fbook->frag_count++].type = frag_type;

    return 0;
}


int frag_rm (struct fragbook *fbook, int frag_idx) {
    if (fbook->frag_count <= frag_idx) return -1;

    fbook->frags[frag_idx]._sta = -1;
    return 0;
}

struct frag_t *frag_get (struct fragbook *fbook, int frag_idx) {
    return fbook->frags + frag_idx;
}

struct frag_t *frag_lget (struct fragbook *fbook, int bgn_loc) {
    for (int i = 0; i < fbook->frag_count; ++i) {
        if (fbook->frags[i]._sta) continue;

        if (fbook->frags[i].begin == bgn_loc) return fbook->frags + i;
    }

    return NULL;
}

int frag_query (struct fragbook *fbook, unsigned int begin, unsigned int end) {
    struct frag_t frg = {
        .begin = begin,
        ._sta = 0,
        .end = end,
        .type = 0
    };
    for (int i = 0; i < fbook->frag_count; ++i) {
        if (fbook->frags[i]._sta) continue;

        if (is_frag_include(fbook->frags + i, &frg)) return i;
    }
    return -1;
}
