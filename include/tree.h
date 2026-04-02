#ifndef TREE_H
#define TREE_H

#include "constants.h"

/**
 * @brief builds a tree object from the index and last commit, writes it to object store
 *
 * merges the last commit's file list with whatever is currently staged —
 * staged files overrid commit files if they overlap. the result is hashed
 * and saved under .mgit/objects/.
 *
 * @param hash_output output buffer for the tree's SHA-1 hex hash (must be HASH_SIZE)
 * @return 0 on success, 1 on failure (nothing to commit, malloc fail, write error)
 *
 * @note merge strategy: start with last commit files, then overlay staged files on top.
 *       if a file exists in both, the staged version wins (hash gets updated in place).
 *       brand-new staged files just get appended to the list.
 */
int create_tree(char hash_output[HASH_SIZE]);

#endif /* TREE_H */
