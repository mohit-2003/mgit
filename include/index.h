#ifndef INDEX_H
#define INDEX_H

#include "constants.h"

/**
 * @brief adds or updates a file entry in the index
 *
 * rewrites the index through a temp file. if the filepath already exists,
 * its hash gets updated. if it's new, it gets appended at the end.
 *
 * @param filepath  path of the file to stage
 * @param hash      blob hash for this file
 * @return 0 on success, 1 if the temp file couldn't be opened
 *
 * @note uses a tmp file + rename to avoid corrupting the index on failure
 */
int add_index_entry(const char *filepath, const char hash[HASH_SIZE]);

/**
 * @brief reads all staged files from the index into arrays
 *
 * @param paths   output array for file paths
 * @param hashes  output array for hashes
 * @param max     max entries to read
 * @return number of entries read
 */
int read_index(char paths[][PATH_BUF], char hashes[][HASH_SIZE], int max);

/**
 * @brief reads all files from the last commit snapshot into arrays
 *
 * @param paths   output array for file paths
 * @param hashes  output array for hashes
 * @param max     max entries to read
 * @return number of entries read
 */
int read_last_commit(char paths[][PATH_BUF], char hashes[][HASH_SIZE], int max);

/**
 * @brief looks up a file's hash in the current index
 *
 * @param filepath  file to look up
 * @param hash      output buffer for the hash if found
 * @return 1 if found, 0 if not in the index
 */
int get_index_hash(const char *filepath, char hash[HASH_SIZE]);

/**
 * @brief looks up a file's hash from the last commit snapshot
 *
 * @param filepath  file to look up
 * @param hash      output buffer for the hash if found
 * @return 1 if found, 0 if not in the last commit
 */
int get_last_commit_hash(const char *filepath, char hash[HASH_SIZE]);

/**
 * @brief dumps the current index to stdout
 *
 * prints nothing useful if the index is empty, just says so.
 */
void print_index(void);

/**
 * @brief rebuilds the last commit snapshot from the current HEAD
 *
 * follows HEAD -> commit object -> tree object, then writes all
 * tree entries into LAST_INDEX_FILE so status and add can diff against it.
 *
 * @note skips the binary "tree <size>\0" header before reading tree entries
 * @note if HEAD has no commits yet, this just returns early and does nothing
 */
void rebuild_last_commit_index(void);

/**
 * @brief checks if the index file is empty (nothing staged)
 *
 * @return 1 if empty or file doesn't exist, 0 if there's something staged
 */
int index_is_empty(void);

/**
 * @brief wipes the index file after a successful commit
 *
 * just opens it in "w" mode so it gets truncated to zero. next add starts fresh.
 */
void clear_index(void);

#endif