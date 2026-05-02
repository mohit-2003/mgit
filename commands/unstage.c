#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../include/mgit.h"
#include "../include/constants.h"
#include "../include/index.h"

/**
 * @brief clears the entire staging area
 *
 * just truncates the index file to zero. all staged files are dropped.
 *
 * @return 0 on success, 1 if the index file couldn't be opened
 */
static int unstage_all(void)
{
    /* Read all files from last commit */
    char (*commit_paths)[PATH_BUF] = malloc(MAX_FILES * PATH_BUF);
    char (*commit_hashes)[HASH_SIZE] = malloc(MAX_FILES * HASH_SIZE);
    // Get the commit count
    int commit_count = read_last_commit(commit_paths, commit_hashes, MAX_FILES);

    /* Wipe the index */
    FILE *f = fopen(INDEX_FILE, "w");
    if (!f)
    {
        printf("Error: cannot clear index.\n");
        free(commit_paths);
        free(commit_hashes);
        return 1;
    }
    /* Restore all committed files back into the index */
    for (int i = 0; i < commit_count; i++)
        fprintf(f, "%s %s\n", commit_paths[i], commit_hashes[i]);
    fclose(f);

    free(commit_paths);
    free(commit_hashes);

    printf("Staging area reset.\n");
    return 0;
}

int cmd_unstage(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Usage: mgit unstage <file | .>\n");
        return 1;
    }

    if (strcmp(argv[2], ".") == 0)
        return unstage_all();

    const char *target = argv[2];

    /* First try to remove from index (unstage new file or modification) */
    FILE *src = fopen(INDEX_FILE, "r");
    FILE *tmp = fopen(TMP_INDEX, "w");
    if (!tmp)
    {
        if (src)
            fclose(src);
        printf("Error: cannot create temp file.\n");
        return 1;
    }

    char path[PATH_BUF], hash[HASH_SIZE];
    int found_in_index = 0;

    if (src)
    {
        char line[PATH_BUF + HASH_SIZE];
        while (fgets(line, sizeof(line), src))
        {
            int len = strlen(line);
            if (len > 0 && line[len - 1] == '\n')
                line[--len] = '\0';
            if (len < 42)
                continue;
            strcpy(hash, &line[len - 40]);
            int start = (strncmp(line, "100644 ", 7) == 0) ? 7 : 0;
            int plen = len - 41 - start;
            strncpy(path, &line[start], plen);
            path[plen] = '\0';

            if (strcmp(path, target) == 0)
                found_in_index = 1;
            else
                fprintf(tmp, "%s %s\n", path, hash);
        }
        fclose(src);
    }
    fclose(tmp);

    remove(INDEX_FILE);
    rename(TMP_INDEX, INDEX_FILE);

    if (found_in_index)
    {
        printf("Unstaged: %s\n", target);
        return 0;
    }

    /* Not found in index — check if it was a staged deletion (missing from index
       but present in last commit). Restore it to the index. */
    char commit_hash[HASH_SIZE];
    if (get_last_commit_hash(target, commit_hash))
    {
        add_index_entry(target, commit_hash);
        printf("Unstaged deletion: %s\n", target);
        return 0;
    }

    printf("'%s' was not in the staging area.\n", target);
    return 0;
}