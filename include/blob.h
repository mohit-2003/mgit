#ifndef BLOB_H
#define BLOB_H

#include <stddef.h>

/**
 * @brief Create a blob object from a file.
 *
 * Reads the contents of a file, formats it as a Git-style blob
 * object ("blob <size>\0<data>"), computes its SHA-1 hash, and
 * stores it in the object database.
 *
 * @param filepath Path to file to convert into a blob.
 * @param hash_output Buffer to store resulting SHA-1 hex string.
 *
 * @return 0 on success, non-zero on failure.
 */
int create_blob(const char *filepath, char hash_output[41]);

#endif