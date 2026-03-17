#ifndef COMMIT_H
#define COMMIT_H

/**
 * @brief Create a commit object.
 *
 * Builds the commit structure, hashes it, and stores it in the
 * object database.
 *
 * @param tree_hash SHA1 hash of the root tree
 * @param message commit message
 * @param commit_hash output buffer for resulting commit hash
 *
 * @return 0 on success
 */
int create_commit(const char *tree_hash,
                  const char *message,
                  char commit_hash[41]);

#endif