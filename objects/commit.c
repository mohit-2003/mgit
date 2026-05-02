#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../include/commit.h"
#include "../include/sha1.h"
#include "../include/utils.h"
#include "../include/constants.h"

/**
 * @brief reads user name and email from the config file
 * @param name   output buffer for the user's name
 * @param email  output buffer for the user's email
 */
static void read_config(char *name, char *email)
{
    char *home = getHomeDirectory();
    if (!home)
        return;

    char path[PATH_BUF];
    snprintf(path, sizeof(path), "%s/%s", home, CONFIG_FILE);

    FILE *f = fopen(path, "r");
    if (!f)
        return;

    char line[LINE_BUF];
    while (fgets(line, sizeof(line), f))
    {
        if (strncmp(line, "name =", 6) == 0)
            sscanf(line, "name = %[^\n]", name);
        if (strncmp(line, "email =", 7) == 0)
            sscanf(line, "email = %[^\n]", email);
    }
    fclose(f);
}

/**
 * @brief gets the current HEAD commit hash
 *
 * handles both the symbolic ref case (normal branch) and a bare hash
 * (detached HEAD). if there are no commits yet, parent is left as an empty string.
 *
 * @param parent output buffer for the parent commit hash (HASH_SIZE)
 *
 * @note parent[0] == '\0' means there's no parent (first commit on the branch)
 */
static void read_head(char parent[HASH_SIZE])
{
    parent[0] = '\0';

    FILE *f = fopen(HEAD_FILE, "r");
    if (!f)
        return;

    char line[LINE_BUF];
    if (!fgets(line, sizeof(line), f))
    {
        fclose(f);
        return;
    }
    fclose(f);

    strip_newline(line);

    if (strncmp(line, "ref:", 4) == 0)
    {
        /* HEAD is a symbolic ref: "ref: .mgit/refs/heads/main" */
        char ref_path[PATH_BUF];
        sscanf(line, "ref: %1023s", ref_path);

        FILE *ref = fopen(ref_path, "r");
        if (!ref)
            return; /* branch exists but has no commits yet */

        if (fgets(line, sizeof(line), ref))
        {
            strip_newline(line);
            if (strlen(line) == 40)
                strncpy(parent, line, HASH_SIZE);
        }
        fclose(ref);
    }
    else if (strlen(line) == 40)
    {
        /* HEAD contains a bare hash (detached HEAD) */
        strncpy(parent, line, HASH_SIZE);
    }
}

/**
 * @brief updates HEAD to point to the new commit hash
 *
 * if HEAD is a symbolic ref, writes the hash into the branch file.
 * if it's a detached HEAD, overwrites the hash directly in HEAD.
 *
 * @param hash the new commit hash to write
 */
static void write_head(const char *hash)
{
    FILE *f = fopen(HEAD_FILE, "r");
    if (!f)
        return;

    char line[LINE_BUF];
    if (!fgets(line, sizeof(line), f))
    {
        fclose(f);
        return;
    }
    fclose(f);

    strip_newline(line);

    if (strncmp(line, "ref:", 4) == 0)
    {
        /* Write hash into the branch file */
        char ref_path[PATH_BUF];
        sscanf(line, "ref: %1023s", ref_path);

        FILE *ref = fopen(ref_path, "w");
        if (!ref)
            return;
        fprintf(ref, "%s\n", hash);
        fclose(ref);
    }
    else
    {
        /* Detached HEAD: overwrite the hash directly */
        FILE *head = fopen(HEAD_FILE, "w");
        if (!head)
            return;
        fprintf(head, "%s\n", hash);
        fclose(head);
    }
}

/**
 * @brief builds a commit object and writes it to the object store
 *
 * assembles the commit content (tree, parent, author, date, message),
 * hashes it, saves it under .mgit/objects/, then updates HEAD.
 *
 * @param tree_hash    SHA-1 hash of the tree object for this commit
 * @param message      commit message string
 * @param commit_hash  output buffer that gets the final commit hash (HASH_SIZE)
 * @return 0 on success, 1 on failure (buffer overflow, file write error, etc.)
 *
 * @note skips the "parent" line entirely if this is the very first commit
 * @note SAFE_APPEND macro is defined and cleaned up locally to guard against buffer overflows
 */
int create_commit(const char *tree_hash,
                  const char *message,
                  char commit_hash[HASH_SIZE])
{
    char name[256] = "Unknown";
    char email[256] = "unknown@example.com";
    char parent[HASH_SIZE] = "";

    read_config(name, email);
    read_head(parent);

    time_t now = time(NULL);

    char content[CONTENT_BUF];
    int offset = 0;
    int written = 0;

/* BUG FIX: Safe Append Macro to prevent Buffer Overflow crashes */
#define SAFE_APPEND(...)                                                                       \
    do                                                                                         \
    {                                                                                          \
        written = snprintf(content + offset, sizeof(content) - offset, __VA_ARGS__);           \
        if (written < 0 || written >= (int)(sizeof(content) - offset))                         \
        {                                                                                      \
            printf("fatal: commit content exceeds maximum buffer size (message too long).\n"); \
            return 1;                                                                          \
        }                                                                                      \
        offset += written;                                                                     \
    } while (0)

    /* Safely append all data */
    SAFE_APPEND("tree %s\n", tree_hash);

    if (parent[0] != '\0')
        SAFE_APPEND("parent %s\n", parent);

    SAFE_APPEND("author %s <%s>\n", name, email);
    SAFE_APPEND("date %ld\n\n", (long)now);
    SAFE_APPEND("%s\n", message);

#undef SAFE_APPEND /* Clean up macro */

    /* Hash the plain-text content */
    unsigned char hash_bin[20];
    sha1((unsigned char *)content, (size_t)offset, hash_bin);
    sha1_to_hex(hash_bin, commit_hash);

    /* Write to object store */
    char obj_path[PATH_BUF];
    object_path(commit_hash, obj_path);
    ensure_object_dir(commit_hash);

    FILE *obj = fopen(obj_path, "w");
    if (!obj)
        return 1;
    fwrite(content, 1, offset, obj);
    fclose(obj);

    write_head(commit_hash);
    return 0;
}