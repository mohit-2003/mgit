#ifndef TREE_H
#define TREE_H

/**
 * @brief Create a tree object from the index.
 *
 * Reads the staging area (.mgit/index), builds a tree object,
 * computes its SHA1 hash, and stores it in the object database.
 *
 * @param hash_output Buffer to store resulting tree hash.
 *
 * @return 0 on success
 */
int create_tree(char hash_output[41]);

#endif