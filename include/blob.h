#ifndef BLOB_H
#define BLOB_H

#include "constants.h"

/**
 * @brief hashes a file and writes it to the object store as a blob
 *
 * reads the file, builds a git-style blob header, computes SHA-1 over
 * the whole thing, then saves it under .mgit/objects/ if it's not there already.
 *
 * @param filepath path to the file to turn into a blob
 * @param hash_output output buffer that gets the hex SHA-1 hash (must be HASH_SIZE)
 * @return 0 on success, 1 if something went wrong (file not found, malloc fail, etc.)
 *
 * @note blob format is "blob <size>\0<raw bytes>" — same as git does it
 */
int create_blob(const char *filepath, char hash_output[HASH_SIZE]);

#endif /* BLOB_H */
