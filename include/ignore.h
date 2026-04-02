#ifndef IGNORE_H
#define IGNORE_H

/**
 * @brief checks if a file path matches any pattern in the ignore file
 *
 * reads IGNORE_FILE line by line and does a simple substring match
 * against the given path. skips empty lines and comments.
 *
 * @param filepath  path to check against the ignore rules
 * @return 1 if the path should be ignored, 0 if not (or if ignore file doesn't exist)
 */
int is_ignored(const char *filepath);

#endif
