#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>

/**
 * @brief strips trailing newline or carriage return from a string in place
 *
 * @param s string to modify
 */
void strip_newline(char *s);

/**
 * @brief checks if a file exists by trying to open it
 *
 * @param path file path to check
 * @return 1 if the file exists and is readable, 0 otherwise
 */
int file_exists(const char *path);

/**
 * @brief checks if a path points to a regular file
 *
 * @param path path to check
 * @return 1 if it's a regular file, 0 otherwise (dir, missing, etc.)
 */
int is_regular_file(const char *path);

/**
 * @brief checks if a path points to a directory
 *
 * @param path path to check
 * @return 1 if it's a directory, 0 otherwise
 */
int is_directory(const char *path);

/**
 * @brief creates a directory, works on both Windows and Linux
 *
 * @param path directory path to create
 *
 * @note on Windows uses _mkdir, on Linux/Mac uses mkdir with 0777 permissions
 */
void mgit_mkdir(const char *path);

/**
 * @brief builds the full object store path for a given hash
 *
 * splits the hash into a two-char directory and the rest as filename,
 * same layout as git (e.g. objects/ab/cdef1234...).
 *
 * @param hash  40-char hex hash of the object
 * @param out   output buffer for the path (must be PATH_BUF)
 */
void object_path(const char *hash, char *out);

/**
 * @brief creates the two-char subdirectory under objects/ for a hash if it doesn't exist
 *
 * @param hash hash whose first two chars determine the subdirectory name
 */
void ensure_object_dir(const char *hash);

/**
 * @brief checks if the current directory is an mgit repository
 *
 * @return 1 if .mgit/ exists and is openable, 0 otherwise
 */
int is_mgit_repository();

/**
 * @brief creates any missing parent directories for a file path
 *
 * walks the path string and calls mgit_mkdir() on each directory component.
 * needed before writing a checked-out file that lives in a subdirectory.
 *
 * @param filepath  file path whose parent directories should exist
 */
void ensure_parent_dirs(const char *filepath);

/**
 * @brief restores a single file from the object store to disk
 *
 * opens the blob object, skips the "blob <size>\0" header, then writes
 * the raw content to the destination path.
 *
 * @param filepath   destination path to write the file to
 * @param blob_hash  hash of the blob object to restore
 * @return 1 on success, 0 if the object or output file couldn't be opened
 */
int checkout_file(const char *filepath, const char *blob_hash);

#endif /* UTILS_H */