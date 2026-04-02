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

#endif /* UTILS_H */