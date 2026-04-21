#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../include/mgit.h"
#include "../include/index.h"
#include "../include/constants.h"
#include "../include/utils.h"
#include "../include/blob.h"

int cmd_restore(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Usage: mgit restore <file | .>\n");
        return 1;
    }

    const char *target = argv[2];

    /* mgit restore . — restore all tracked files */
    if (strcmp(target, ".") == 0)
    {
        char (*commit_paths)[PATH_BUF] = malloc(MAX_FILES * PATH_BUF);
        char (*commit_hashes)[HASH_SIZE] = malloc(MAX_FILES * HASH_SIZE);
        if (!commit_paths || !commit_hashes)
        {
            printf("Error: memory allocation failed.\n");
            free(commit_paths);
            free(commit_hashes);
            return 1;
        }

        int commit_count = read_last_commit(commit_paths, commit_hashes, MAX_FILES);
        if (commit_count == 0)
        {
            printf("No committed files to restore.\n");
            free(commit_paths);
            free(commit_hashes);
            return 0;
        }

        int restored = 0;
        for (int i = 0; i < commit_count; i++)
        {
            /* Skip files that exist on disk and are unchanged */
            if (is_regular_file(commit_paths[i]))
            {
                char disk_hash[HASH_SIZE];
                if (create_blob(commit_paths[i], disk_hash) == 0 &&
                    strcmp(disk_hash, commit_hashes[i]) == 0)
                {
                    /* File is clean, just make sure index is in sync */
                    add_index_entry(commit_paths[i], commit_hashes[i]);
                    continue;
                }
            }

            /* File is deleted or modified — restore it */
            if (checkout_file(commit_paths[i], commit_hashes[i]))
            {
                add_index_entry(commit_paths[i], commit_hashes[i]);
                printf("Restored: %s\n", commit_paths[i]);
                restored++;
            }
            else
            {
                printf("error: could not restore '%s'\n", commit_paths[i]);
            }
        }

        if (restored == 0)
            printf("Nothing to restore.\n");

        free(commit_paths);
        free(commit_hashes);
        return 0;
    }

    /* mgit restore <file> — restore single file */
    char blob_hash[HASH_SIZE];
    if (!get_index_hash(target, blob_hash) &&
        !get_last_commit_hash(target, blob_hash))
    {
        printf("error: '%s' is not tracked by mgit\n", target);
        return 1;
    }

    if (!checkout_file(target, blob_hash))
    {
        printf("error: could not restore '%s'\n", target);
        return 1;
    }

    add_index_entry(target, blob_hash);
    printf("Restored: %s\n", target);
    return 0;
}