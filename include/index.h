#ifndef INDEX_H
#define INDEX_H

/**
 * @brief Add or update a file entry in the index.
 *
 * Stores the mapping between a file path and the SHA1 blob hash
 * representing its content.
 *
 * @param filepath Path to file being staged.
 * @param hash SHA1 hash of the blob object.
 *
 * @return 0 on success, non-zero on failure.
 */
int add_index_entry(const char *filepath, const char hash[41]);

/**
 * @brief Print index contents.
 *
 * Used mainly for debugging or testing.
 */
void print_index();

#endif