#ifndef ADD_H
#define ADD_H

/**
 * @brief Stage a file into the mgit index.
 *
 * Converts the file into a blob object and records its hash
 * in the staging area.
 *
 * @param argc CLI argument count
 * @param argv CLI argument list
 *
 * @return 0 on success
 */
int cmd_add(int argc, char *argv[]);

#endif