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

static void process_path(const char *fpath);

/**
 * @brief walks a directory and stages all files inside it
 *
 * opens the dir at base_path and calls process_path() on every entry.
 * skips "." and ".." so we don't loop forever.
 *
 * @param base_path path to the directory we want to add
 */
static void add_directory(const char *base_path)
{
    DIR *dir = opendir(base_path);
    if (!dir)
        return;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        // skip current and parent dir entries
        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0)
            continue;

        char childpath[PATH_BUF];
        // avoid "./filename when base is "."
        if (strcmp(base_path, ".") == 0)
            // childpath is just the filename, no leading "./"
            snprintf(childpath, sizeof(childpath), "%s", entry->d_name);
        else
            // childpath is base_path + "/" + filename
            snprintf(childpath, sizeof(childpath), "%s/%s", base_path, entry->d_name);

        process_path(childpath);
    }
    closedir(dir);
}

/**
 * @brief decides what to do with a path - recurse if dir, stage if file
 *
 * skips anything inside .mgit/ or matched by .mgitignore.
 * for regular files, hashes the content and adds to index only if
 * something actually changed (checks both index and last commit).
 *
 * @param fpath path to the file or directory to process
 */
static void process_path(const char *fpath)
{
    // Skip .mgit folder and ignored files
    if (strncmp(fpath, ".mgit/", 6) == 0)
        return;
    if (is_ignored(fpath))
        return;

    if (is_directory(fpath))
    {
        add_directory(fpath);
        return;
    }

    if (!is_regular_file(fpath))
        return;

    /** create_blob() writes the object to disk and gives the hash */
    char hash[HASH_SIZE];
    if (create_blob(fpath, hash) != 0)
        return;

    char old_hash[HASH_SIZE];

    // skip if already staged with the same content
    if (get_index_hash(fpath, old_hash) && strcmp(old_hash, hash) == 0)
        return;

    // also skip if it matches what's already in the last commit
    if (get_last_commit_hash(fpath, old_hash) && strcmp(old_hash, hash) == 0)
        return;

    add_index_entry(fpath, hash);
    printf("added: %s\n", fpath);
}

int cmd_add(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Usage: mgit add <path> [path2 ...]\n");
        return 1;
    }

    for (int i = 2; i < argc; i++)
    {
        if (strcmp(argv[i], ".") == 0)
        {
            add_directory(".");

            // Also stage deletions: anything in last commit or index
            // that no longer exists on disk
            char (*commit_paths)[PATH_BUF] = malloc(MAX_FILES * PATH_BUF);
            char (*commit_hashes)[HASH_SIZE] = malloc(MAX_FILES * HASH_SIZE);
            int commit_count = read_last_commit(commit_paths, commit_hashes, MAX_FILES);

            for (int j = 0; j < commit_count; j++)
            {
                if (!is_regular_file(commit_paths[j]))
                {
                    remove_from_index(commit_paths[j]);
                    printf("deleted: %s\n", commit_paths[j]);
                }
            }
            free(commit_paths);
            free(commit_hashes);
            continue;
        }

        struct stat st;
        if (stat(argv[i], &st) != 0)
        {
            char old_hash[HASH_SIZE];
            if (get_index_hash(argv[i], old_hash) || get_last_commit_hash(argv[i], old_hash))
            {
                remove_from_index(argv[i]);
                printf("deleted: %s\n", argv[i]);
            }
            else
            {
                printf("fatal: '%s' did not match any files\n", argv[i]);
            }
            continue;
        }
        process_path(argv[i]);
    }

    return 0;
}