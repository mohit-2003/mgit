#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "../include/mgit.h"
#include "../include/blob.h"
#include "../include/index.h"
#include "../include/ignore.h"
#include "../include/utils.h"
#include "../include/constants.h"
#include "../include/dirent.h"

// global working tree file list
static char wt_paths[MAX_FILES][PATH_BUF];
static int wt_count = 0;

/**
 * @brief returns 1 if the path is an mgit internal file that should be hidden from status
 *
 * @param path file path to check
 * @return 1 if it should be skipped, 0 otherwise
 */
static int is_mgit_internal(const char *path)
{
    if (strcmp(path, ".mgitconfig") == 0)
        return 1;
    return 0;
}

/**
 * @brief recursively collects all trackable files from a directory into wt_paths
 *
 * skips .mgit/, ignored files, and mgit internal files.
 * results go into the global wt_paths array, wt_count tracks how many we found.
 *
 * @param base directory to start from (use "." for the working root)
 *
 * @note exits the process if we hit MAX_FILES — better than silently dropping files
 */
static void collect_files(const char *base)
{
    DIR *dir = opendir(base);
    if (!dir)
        return;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        /* Skip current and parent directories */
        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0)
            continue;

        /* BUG FIX: EXACT MATCH: Skip the .mgit folder entirely */
        if (strcmp(entry->d_name, ".mgit") == 0)
            continue;

        char path[PATH_BUF];
        if (strcmp(base, ".") == 0)
            snprintf(path, sizeof(path), "%s", entry->d_name);
        else
            snprintf(path, sizeof(path), "%s/%s", base, entry->d_name);

        /* Skip .mgit internals */
        if (is_mgit_internal(path))
            continue;

        /* Skip ignored files */
        if (is_ignored(path))
            continue;

        if (is_directory(path))
        {
            collect_files(path);
        }
        else if (is_regular_file(path))
        {
            /* BUG FIX: Stop silently ignoring files if we hit the limit! */
            if (wt_count >= MAX_FILES)
            {
                printf("fatal: repository exceeds maximum file limit of %d.\n", MAX_FILES);
                closedir(dir);
                exit(1);
            }

            strncpy(wt_paths[wt_count], path, PATH_BUF - 1);
            wt_paths[wt_count][PATH_BUF - 1] = '\0';
            wt_count++;
        }
    }
    closedir(dir);
}

/**
 * @brief linear search for a path in a paths array
 *
 * @param paths  array of path strings to search
 * @param count  number of entries in the array
 * @param target path to look for
 * @return index of the match, or -1 if not found
 */
static int find_in(char paths[][PATH_BUF], int count, const char *target)
{
    for (int i = 0; i < count; i++)
        if (strcmp(paths[i], target) == 0)
            return i;
    return -1;
}

int cmd_status()
{
    /* Allocating on the heap to prevent stack overflow */
    char (*index_paths)[PATH_BUF] = malloc(MAX_FILES * PATH_BUF);
    char (*index_hashes)[HASH_SIZE] = malloc(MAX_FILES * HASH_SIZE);
    char (*commit_paths)[PATH_BUF] = malloc(MAX_FILES * PATH_BUF);
    char (*commit_hashes)[HASH_SIZE] = malloc(MAX_FILES * HASH_SIZE);

    if (!index_paths || !index_hashes || !commit_paths || !commit_hashes)
    {
        printf("Error: memory allocation failed.\n");
        return 1;
    }

    int index_count = read_index(index_paths, index_hashes, MAX_FILES);
    int commit_count = read_last_commit(commit_paths, commit_hashes, MAX_FILES);

    /* 1. Staged changes */
    printf("Changes to be committed:\n");
    printf("  (use \"git unstage <file>...\" to unstage)\n");
    int any_staged = 0;
    for (int i = 0; i < index_count; i++)
    {
        int pos = find_in(commit_paths, commit_count, index_paths[i]);
        if (pos == -1)
        {
            printf("    new file:  %s\n", index_paths[i]);
            any_staged = 1;
        }
        else if (strcmp(index_hashes[i], commit_hashes[pos]) != 0)
        {
            printf("    modified:  %s\n", index_paths[i]);
            any_staged = 1;
        }
    }
    if (!any_staged)
        printf("    (nothing staged)\n");

    /* 2. Not staged (disk vs last commit) */
    printf("\nChanges not staged for commit:\n");
    printf("  (use \"git add <file>...\" to update what will be committed)\n");
    int any_modified = 0;
    for (int i = 0; i < commit_count; i++)
    {
        const char *path = commit_paths[i];

        /* Already staged with a new hash — don't double-report */
        int staged_idx = find_in(index_paths, index_count, path);
        if (staged_idx != -1 &&
            strcmp(index_hashes[staged_idx], commit_hashes[i]) != 0)
            continue;

        if (!is_regular_file(path))
        {
            printf("    deleted:   %s\n", path);
            any_modified = 1;
            continue;
        }

        char disk_hash[HASH_SIZE];
        if (create_blob(path, disk_hash) != 0)
            continue;

        if (strcmp(disk_hash, commit_hashes[i]) != 0)
        {
            printf("    modified:  %s\n", path);
            any_modified = 1;
        }
    }
    if (!any_modified)
        printf("    (nothing modified)\n");

    /* 3. Untracked files  */
    printf("\nUntracked files:\n");
    printf("  (use \"git add <file>...\" to include in what will be committed)\n");
    wt_count = 0;
    collect_files(".");

    int any_untracked = 0;
    for (int i = 0; i < wt_count; i++)
    {
        if (find_in(index_paths, index_count, wt_paths[i]) != -1)
            continue;
        if (find_in(commit_paths, commit_count, wt_paths[i]) != -1)
            continue;
        printf("    %s\n", wt_paths[i]);
        any_untracked = 1;
    }
    if (!any_untracked)
        printf("    (nothing untracked)\n");

    free(index_paths);
    free(index_hashes);
    free(commit_paths);
    free(commit_hashes);

    return 0;
}