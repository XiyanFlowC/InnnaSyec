#include "bst.h"

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define ABS(x) ((x) >= 0 ? (x) : -(x))

struct bst_node *bst_new_node(long long key, union bst_store value) {
    struct bst_node *ret = (struct bst_node *)malloc(sizeof(struct bst_node));
    if (ret == NULL) return ret;

    ret->_dst = -1;
    ret->left = ret->right = NULL;
    ret->key = key;
    ret->value = value;
    return ret;
}

int bst_dst_calc(struct bst_node *node) {
    if (node == NULL) return 0;
    if (node->_dst != -1) return node->_dst;
}

struct bst_node *bst_left_rotate(struct bst_node *node) {
    struct bst_node *rchild = node->right;
    node->right = rchild->left;
    rchild->left = node;
    node->_dst = -1;

    rchild->_dst = MAX(bst_dst_calc(rchild->right), bst_dst_calc(rchild->left));
    return rchild;
}

struct bst_node *bst_right_rotate(struct bst_node *node) {
    struct bst_node *lchild = node->left;
    node->left = lchild->right;
    lchild->right = node;
    node->_dst = -1;

    lchild->_dst = MAX(bst_dst_calc(lchild->right), bst_dst_calc(lchild->left));
    return lchild;
}

struct bst_node *bst_balance (struct bst_node *node) {
    if (bst_dst_calc(node->left) - bst_dst_calc(node->right) > 1) {
        if (bst_dst_calc(node->left->left) - bst_dst_calc(node->left->right) < 0)
            node->left = bst_left_rotate(node->left);
        return bst_right_rotate(node);
    } else if (bst_dst_calc(node->right) - bst_dst_calc(node->left) > 1) {
        if (bst_dst_calc(node->right->right) - bst_dst_calc(node->right->left) < 0)
            node->right = bst_right_rotate(node->right);
        return bst_left_rotate(node);
    }
    return node;
}

struct bst_node *bst_insert (struct bst_node *node, long long key, union bst_store value) { // AVL
    if (node == NULL) return bst_new_node(key, value);

    if (key > node->key) {
        if (node->right) {
            node->right = bst_insert(node->right, key, value);
        }
        else {
            node->right = bst_new_node(key, value);
        }
    } else {
        if (node->left) {
            node->left = bst_insert(node->left, key, value);
        } else {
            node->left = bst_new_node(key, value);
        }
    }

    node->_dst = MAX(bst_dst_calc(node->left), bst_dst_calc(node->right));

    return bst_balance(node);
}

union bst_store bst_query (struct bst_node *node, long long key) {
    union bst_store zero = {0ull};
    if (node == NULL) return zero;

    if (key == node->key) {
        return node->value;
    } else if (key > node->key) {
        return bst_query(node->right, key);
    } else if (key < node->key) {
        return bst_query(node->left, key);
    }
}

int bst_free(struct bst_node *node) {
    if (node == NULL) return 0;

    bst_free(node->left);
    bst_free(node->right);

    free(node);

    return 0;
}
