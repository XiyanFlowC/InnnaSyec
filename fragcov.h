#ifndef FRAGCOV_H
#define FRAGCOV_H

#ifdef __cplusplus
extern "C" {
#endif

struct frag_t {
    int type;
    int _sta;
    unsigned int begin;
    unsigned int end;
};

struct fragbook {
    int frag_max;
    int frag_count;
    struct frag_t *frags;
};

/**
 * @brief Initialize the fragment note book with a given max count limit.
 * 
 * @param frag_max The amount of the fragments this note book can record.
 * @return struct fragbook* The initialized notebook, NULL if failed.
 */
struct fragbook *fragbook_init (int frag_max);

/**
 * @brief Merge repeated fragments. Before this subroutine is excuted,
 * the result of the frag query can be incorrect.
 * 
 * @param fbook 
 * @return int 0 if success.
 */
int fragbook_defrag (struct fragbook *fbook);

/**
 * @brief Dispose a fragment notebook.
 * 
 * @param fbook The fragment notebook need to be destroyed.
 */
void fragbook_destroy (struct fragbook *fbook);

/**
 * @brief Register a fragment to notebook.
 * 
 * @param fbook The fragment notebook
 * @param frag_type fragment type (unused, but saved)
 * @param begin the begining location of the fragment
 * @param end the ending location of the fragment
 * @return int The index of the registered fragment, -1 if failed.
 */
int frag_reg (struct fragbook *fbook, int frag_type, unsigned int begin, unsigned int end);

/**
 * @brief Remove a fragment from the notebook by its index
 * 
 * @param fbook 
 * @param frag_idx 
 * @return int 
 */
int frag_rm (struct fragbook *fbook, int frag_idx);

/**
 * @brief Get a fragment by it's index.
 * 
 * @param fbook Fragment notebook.
 * @param frag_idx The index of the fragment.
 * @return struct frag_t* The fragment.
 */
struct frag_t *frag_get (struct fragbook *fbook, int frag_idx);

/**
 * @brief Qeury if the given fragment is registered in some fragment.
 * 
 * @param fbook Fragment note book.
 * @param begin The begining of the fragment.
 * @param end The ending of the fragment.
 * @return int -1 if not found, otherwise the index of the fragment.
 */
int frag_query (struct fragbook *fbook, unsigned int begin, unsigned int end);

#ifdef __cplusplus
}
#endif

#endif