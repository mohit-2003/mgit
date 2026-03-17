#include <stdio.h>
#include <string.h>

#include "../include/unstage.h"

#define INDEX_FILE ".mgit/index"

/* Clear entire staging area */
static int unstage_all()
{
    FILE *file = fopen(INDEX_FILE, "w");

    if (!file)
    {
        printf("Error clearing index\n");
        return 1;
    }

    fclose(file);

    printf("Cleared staging area\n");
    return 0;
}

int cmd_unstage(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Usage: mgit unstage <file | .>\n");
        return 1;
    }

    /* Handle: mgit unstage . */
    if (strcmp(argv[2], ".") == 0)
    {
        return unstage_all();
    }

    const char *target = argv[2];

    FILE *file = fopen(INDEX_FILE, "r");

    if (!file)
    {
        printf("Index is empty\n");
        return 1;
    }

    FILE *temp = fopen(".mgit/index_tmp", "w");

    if (!temp)
    {
        printf("Error creating temp index\n");
        fclose(file);
        return 1;
    }

    char path[256];
    char hash[41];

    int found = 0;

    while (fscanf(file, "%s %s", path, hash) == 2)
    {
        if (strcmp(path, target) != 0)
        {
            fprintf(temp, "%s %s\n", path, hash);
        }
        else
        {
            found = 1;
        }
    }

    fclose(file);
    fclose(temp);

    remove(INDEX_FILE);
    rename(".mgit/index_tmp", INDEX_FILE);

    if (found)
        printf("Removed %s from staging\n", target);
    else
        printf("%s not found in staging\n", target);

    return 0;
}