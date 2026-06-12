#include <stdio.h>
#include <string.h>

#include "../include/commit.h"
#include "../include/tree.h"
#include "../include/utils.h"
#include "../include/constants.h"
#include "../include/index.h"
#include "../include/mgit.h"

// Helper function to check if staged index is different from last commit
int has_staged_changes()
{
    char (*index_paths)[PATH_BUF] = malloc(MAX_FILES * PATH_BUF);
    char (*index_hashes)[HASH_SIZE] = malloc(MAX_FILES * HASH_SIZE);
    int index_count = read_index(index_paths, index_hashes, MAX_FILES);

    char (*commit_paths)[PATH_BUF] = malloc(MAX_FILES * PATH_BUF);
    char (*commit_hashes)[HASH_SIZE] = malloc(MAX_FILES * HASH_SIZE);
    int commit_count = read_last_commit(commit_paths, commit_hashes, MAX_FILES);

    int changed = 0;

    if (index_count != commit_count)
    {
        changed = 1; // File count is different (added/deleted)
    }
    else
    {
        // Compare paths and hashes
        for (int i = 0; i < index_count; i++)
        {
            int found = 0;
            for (int j = 0; j < commit_count; j++)
            {
                if (strcmp(index_paths[i], commit_paths[j]) == 0)
                {
                    found = 1;
                    if (strcmp(index_hashes[i], commit_hashes[j]) != 0)
                    {
                        changed = 1; // File modified
                    }
                    break;
                }
            }
            if (!found)
                changed = 1; // New file added
            if (changed)
                break;
        }
    }

    free(index_paths);
    free(index_hashes);
    free(commit_paths);
    free(commit_hashes);

    return changed;
}

int cmd_commit(int argc, char *argv[])
{
    const char *message = NULL;

    if (argc >= 4 && strcmp(argv[2], "-m") == 0)
        message = argv[3];
    else if (argc >= 3 && argv[2][0] != '-')
        message = argv[2];
    else
    {
        printf("usage: mgit commit \"message\"\n");
        return 0;
    }

    if (index_is_empty())
    {
        printf("no changes added to commit (use \"mgit add\" and/or \"mgit commit\")\n");
        return 0;
    }

    // Check if staged changes differ from last commit to avoid creating empty commits.
    if (!has_staged_changes())
    {
        printf("nothing to commit, working tree clean\n");
        return 0;
    }

    char tree_hash[HASH_SIZE];
    char commit_hash[HASH_SIZE];

    if (create_tree(tree_hash) != 0)
    {
        printf("error: failed to create tree.\n");
        return 1;
    }

    if (create_commit(tree_hash, message, commit_hash) != 0)
    {
        printf("error: commit failed.\n");
        return 1;
    }

    rebuild_last_commit_index();

    /* Repopulate index with committed files so unstaged changes are detectable */
    FILE *src = fopen(LAST_INDEX_FILE, "r");
    FILE *dst = fopen(INDEX_FILE, "w");
    if (src && dst)
    {
        char buf[PATH_BUF + HASH_SIZE];
        while (fgets(buf, sizeof(buf), src))
            fputs(buf, dst);
    }
    if (src)
        fclose(src);
    if (dst)
        fclose(dst);

    printf("committed: %s\n", commit_hash);
    return 0;
}