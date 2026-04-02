#ifndef COMMIT_H
#define COMMIT_H

#include "constants.h"

/**
 * @brief builds a commit object from a tree hash and message, writes it, and updates HEAD
 *
 * @param tree_hash    SHA-1 hash of the tree object for this commit
 * @param message      commit message string
 * @param commit_hash  output buffer for the resulting 40-char hex hash (must be HASH_SIZE)
 * @return 0 on success, 1 on failure
 */
int create_commit(const char *tree_hash,
                  const char *message,
                  char commit_hash[HASH_SIZE]);

#endif