#include <stdio.h>
#include <string.h>

#include "../include/mgit.h"
#include "../include/constants.h"

/**
 * @brief clears the entire staging area
 *
 * just truncates the index file to zero. all staged files are dropped.
 *
 * @return 0 on success, 1 if the index file couldn't be opened
 */
static int unstage_all(void)
{
    FILE *f = fopen(INDEX_FILE, "w");
    if (!f)
    {
        printf("Error: cannot clear index.\n");
        return 1;
    }
    fclose(f);
    printf("Staging area cleared.\n");
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

    FILE *src = fopen(INDEX_FILE, "r");
    if (!src)
    {
        printf("Staging area is already empty.\n");
        return 1;
    }

    FILE *tmp = fopen(TMP_INDEX, "w");
    if (!tmp)
    {
        fclose(src);
        printf("Error: cannot create temp file.\n");
        return 1;
    }

    char path[PATH_BUF], hash[HASH_SIZE];
    int found = 0;

    while (fscanf(src, "%1023s %40s", path, hash) == 2)
    {
        if (strcmp(path, target) == 0)
            found = 1;
        else
            fprintf(tmp, "%s %s\n", path, hash);
    }

    fclose(src);
    fclose(tmp);

    remove(INDEX_FILE);
    rename(TMP_INDEX, INDEX_FILE);

    if (found)
        printf("Unstaged: %s\n", target);
    else
        printf("'%s' was not in the staging area.\n", target);

    return 0;
}