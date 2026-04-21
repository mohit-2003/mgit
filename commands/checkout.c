#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/mgit.h"
#include "../include/utils.h"
#include "../include/constants.h"
#include "../include/index.h"
#include "../include/sha1.h"

/**
 * @brief computes the SHA-1 hash of a file on disk in mgit blob format
 *
 * builds the same "blob <size>\0<content>" structure that create_blob() would,
 * then hashes it — used to compare disk state against what's stored.
 *
 * @param filepath  path to the file to hash
 * @param hash_out  output buffer for the 40-char hex hash (must be HASH_SIZE)
 * @return 1 on success, 0 if the file can't be opened or malloc fails
 */
static int compute_file_hash(const char *filepath, char hash_out[HASH_SIZE])
{
    FILE *f = fopen(filepath, "rb");
    if (!f)
        return 0;

    // Get file size
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    // Prepare the "blob <size>\0" header
    char header[64];
    int hlen = sprintf(header, "blob %ld", size) + 1; // +1 to include '\0'

    // Allocate memory for header + content
    size_t total_size = hlen + size;
    char *buffer = malloc(total_size);
    if (!buffer)
    {
        fclose(f);
        return 0;
    }

    // Read the file into the buffer right after the header
    memcpy(buffer, header, hlen);
    fread(buffer + hlen, 1, size, f);
    fclose(f);

    // Compute SHA-1
    unsigned char hash_bin[20];
    sha1((unsigned char *)buffer, total_size, hash_bin);
    sha1_to_hex(hash_bin, hash_out);

    free(buffer);
    return 1;
}

/**
 * @brief resolves a branch name or raw hash to a commit hash
 *
 * if target is 40 chars it's treated as a bare hash, otherwise we look
 * for a matching branch file under HEADS_DIR.
 *
 * @param target       branch name or 40-char commit hash
 * @param commit_hash  output buffer for the resolved commit hash (must be HASH_SIZE)
 * @param is_branch    set to 1 if target resolved via a branch file, 0 if bare hash
 * @return 1 if resolved successfully, 0 if nothing matched
 */
static int resolve_target(const char *target, char *commit_hash, int *is_branch)
{
    *is_branch = 0;
    if (strlen(target) == 40)
    {
        strncpy(commit_hash, target, HASH_SIZE);
        commit_hash[HASH_SIZE - 1] = '\0';
        return 1;
    }

    char ref_path[PATH_BUF];
    snprintf(ref_path, sizeof(ref_path), "%s/%s", HEADS_DIR, target);
    FILE *f = fopen(ref_path, "r");
    if (f)
    {
        char temp_line[LINE_BUF];
        if (fgets(temp_line, sizeof(temp_line), f))
        {
            temp_line[strcspn(temp_line, "\r\n")] = '\0';
            if (strlen(temp_line) == 40)
            {
                strncpy(commit_hash, temp_line, HASH_SIZE - 1);
                commit_hash[HASH_SIZE - 1] = '\0';
                *is_branch = 1;
                fclose(f);
                return 1;
            }
        }
        fclose(f);
    }
    return 0;
}

/**
 * @brief reads the tree hash out of a commit object
 *
 * @param commit_hash  hash of the commit to read
 * @param tree_hash    output buffer for the tree hash (must be HASH_SIZE)
 * @return 1 if found, 0 if the commit object couldn't be opened or has no tree line
 */
static int get_tree_from_commit(const char *commit_hash, char *tree_hash)
{
    char path[PATH_BUF];
    object_path(commit_hash, path);
    FILE *f = fopen(path, "r");
    if (!f)
        return 0;

    char line[LINE_BUF];
    while (fgets(line, sizeof(line), f))
    {
        if (strncmp(line, "tree ", 5) == 0)
        {
            sscanf(line, "tree %40s", tree_hash);
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}

/**
 * @brief reads a tree object's entries into path and hash arrays
 *
 * skips the binary header then parses "100644 <path> <hash>" lines.
 *
 * @param tree_hash  hash of the tree object to read
 * @param paths      output array for file paths
 * @param hashes     output array for blob hashes
 * @param count      output — number of entries read
 * @return 1 on success, 0 if the tree object couldn't be opened
 */
static int read_tree_into_arrays(const char *tree_hash, char paths[][PATH_BUF], char hashes[][HASH_SIZE], int *count)
{
    *count = 0;
    char tree_path[PATH_BUF];
    object_path(tree_hash, tree_path);

    FILE *f = fopen(tree_path, "rb");
    if (!f)
        return 0;

    // skip the binary header
    int c;
    while ((c = fgetc(f)) != EOF && c != '\0')
        ;

    char line[LINE_BUF];
    while (fgets(line, sizeof(line), f) && *count < MAX_FILES)
    {
        int len = strlen(line);
        if (len > 0 && line[len - 1] == '\n')
        {
            line[len - 1] = '\0';
            len--;
        }

        if (len < 42 || strncmp(line, "100644 ", 7) != 0)
            continue;

        strcpy(hashes[*count], &line[len - 40]);
        int path_len = len - 41 - 7;
        strncpy(paths[*count], &line[7], path_len);
        paths[*count][path_len] = '\0';

        (*count)++;
    }
    fclose(f);
    return 1;
}

int cmd_checkout(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Usage: mgit checkout [-b] <branch_or_commit>\n");
        return 1;
    }

    const char *target = argv[2];
    int create_new_branch = 0;

    /* 1. Handle -b flag: mgit checkout -b <new_branch> */
    if (strcmp(argv[2], "-b") == 0)
    {
        if (argc < 4)
        {
            printf("fatal: branch name required\n");
            return 1;
        }
        target = argv[3];
        create_new_branch = 1;

        char branch_path[PATH_BUF];
        snprintf(branch_path, sizeof(branch_path), "%s/%s", HEADS_DIR, target);

        if (file_exists(branch_path))
        {
            printf("fatal: A branch named '%s' already exists.\n", target);
            return 1;
        }

        /* Get the current commit hash from HEAD to point the new branch to it */
        char current_hash[HASH_SIZE] = "";
        FILE *head_read = fopen(HEAD_FILE, "r");
        if (head_read)
        {
            char head_line[LINE_BUF];
            if (fgets(head_line, sizeof(head_line), head_read))
            {
                head_line[strcspn(head_line, "\r\n")] = '\0';
                if (strncmp(head_line, "ref: ", 5) == 0)
                {
                    FILE *ref_f = fopen(head_line + 5, "r");
                    if (ref_f)
                    {
                        fgets(current_hash, HASH_SIZE, ref_f);
                        current_hash[strcspn(current_hash, "\r\n")] = '\0';
                        fclose(ref_f);
                    }
                }
                else
                {
                    strncpy(current_hash, head_line, HASH_SIZE - 1);
                }
            }
            fclose(head_read);
        }

        if (strlen(current_hash) != 40)
        {
            printf("fatal: You don't have any commits yet to create a branch from.\n");
            return 1;
        }

        /* Create the branch file */
        mgit_mkdir(REFS_DIR);
        mgit_mkdir(HEADS_DIR);
        FILE *b_file = fopen(branch_path, "w");
        if (!b_file)
        {
            printf("fatal: could not create branch file\n");
            return 1;
        }
        fprintf(b_file, "%s\n", current_hash);
        fclose(b_file);
    }

    /* 2. Standard "Already on branch" check */
    char current_head_line[PATH_BUF] = "";
    FILE *hf = fopen(HEAD_FILE, "r");
    if (hf)
    {
        if (fgets(current_head_line, sizeof(current_head_line), hf))
            current_head_line[strcspn(current_head_line, "\r\n")] = '\0';
        fclose(hf);
    }

    char expected_ref[PATH_BUF];
    snprintf(expected_ref, sizeof(expected_ref), "ref: %s/%s", HEADS_DIR, target);

    if (!create_new_branch && (strcmp(current_head_line, expected_ref) == 0 || strcmp(current_head_line, target) == 0))
    {
        printf("Already on '%s'\n", target);
        return 0;
    }

    char commit_hash[HASH_SIZE];
    int is_branch = 0;

    if (!resolve_target(target, commit_hash, &is_branch))
    {
        printf("fatal: pathspec or reference '%s' did not match any file(s) known to mgit\n", target);
        return 1;
    }

    /* 3. Safety Abort: Check for uncommitted changes */
    char (*current_paths)[PATH_BUF] = malloc(MAX_FILES * PATH_BUF);
    char (*current_hashes)[HASH_SIZE] = malloc(MAX_FILES * HASH_SIZE);
    int current_count = read_index(current_paths, current_hashes, MAX_FILES);

    for (int i = 0; i < current_count; i++)
    {
        char disk_hash[HASH_SIZE];
        if (!compute_file_hash(current_paths[i], disk_hash) || strcmp(disk_hash, current_hashes[i]) != 0)
        {
            printf("error: Your local changes to the following files would be overwritten by checkout:\n\t%s\n", current_paths[i]);
            printf("Please commit your changes before you switch branches.\nAborting\n");
            free(current_paths);
            free(current_hashes);
            return 1;
        }
    }

    /* 4. Proceed with File Updates */
    char tree_hash[HASH_SIZE] = "";
    get_tree_from_commit(commit_hash, tree_hash);

    char (*target_paths)[PATH_BUF] = malloc(MAX_FILES * PATH_BUF);
    char (*target_hashes)[HASH_SIZE] = malloc(MAX_FILES * HASH_SIZE);
    int target_count = 0;
    read_tree_into_arrays(tree_hash, target_paths, target_hashes, &target_count);

    for (int i = 0; i < current_count; i++)
    {
        int found = 0;
        for (int j = 0; j < target_count; j++)
            if (strcmp(current_paths[i], target_paths[j]) == 0)
            {
                found = 1;
                break;
            }
        if (!found)
            remove(current_paths[i]);
    }

    FILE *new_index = fopen(INDEX_FILE, "w");
    printf("Updating files...\n");
    for (int i = 0; i < target_count; i++)
    {
        if (checkout_file(target_paths[i], target_hashes[i]))
            fprintf(new_index, "%s %s\n", target_paths[i], target_hashes[i]);
    }
    fclose(new_index);

    free(current_paths);
    free(current_hashes);
    free(target_paths);
    free(target_hashes);

    /* 5. Update HEAD and Sync Commit Index */
    FILE *head_file = fopen(HEAD_FILE, "w");
    if (head_file)
    {
        if (is_branch)
        {
            fprintf(head_file, "ref: %s/%s\n", HEADS_DIR, target);
            printf("Switched to %s branch '%s'\n", create_new_branch ? "a new" : "", target);
        }
        else
        {
            fprintf(head_file, "%s\n", commit_hash);
            printf("Note: checking out '%s'.\nYou are in 'detached HEAD' state.\n", commit_hash);
        }
        fclose(head_file);
    }

    // sync the new index into last commit index so status is correct after switch
    FILE *src = fopen(INDEX_FILE, "r");
    FILE *dst = fopen(LAST_INDEX_FILE, "w");
    if (src && dst)
    {
        char buf[LINE_BUF];
        while (fgets(buf, sizeof(buf), src))
            fputs(buf, dst);
    }
    if (src)
        fclose(src);
    if (dst)
        fclose(dst);

    return 0;
}