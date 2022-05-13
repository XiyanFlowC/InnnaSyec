#ifndef BST_H__
#define BST_H__

#pragma once

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

union bst_store {
    unsigned long long u64;
    long long i64;
    void *data;
};

struct bst_node {
    long long key;
    union bst_store value;
    int _dst;
    struct bst_node *left, *right;
};

/**
 * @brief Create a new node to given BST.
 * 
 * @param node The root of the BST.
 * @param key The key of the node.
 * @param value The value of the node.
 * @return struct bst_node * New root, NULL if any error occured.
 */
struct bst_node *bst_insert (struct bst_node *node, long long key, union bst_store value);

/**
 * @brief Query a data.
 * 
 * @param key Key.
 * @return void* Data, NULL if not found.
 */
union bst_store bst_query (struct bst_node *node, long long key);

/**
 * @brief Free entire BST (by given root or node as root).
 * 
 * @param node The root to free.
 * @return int 0 if success.
 */
int bst_free (struct bst_node *node);

#ifdef __cplusplus
}
#endif

#endif /* BST_H__ */