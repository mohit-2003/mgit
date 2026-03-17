#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/index.h"

#define INDEX_FILE ".mgit/index"

int add_index_entry(const char *filepath, const char hash[41])
{
    FILE *file = fopen(".mgit/index", "r");
    FILE *temp = fopen(".mgit/index_tmp", "w");

    int found = 0;

    if (file)
    {
        char path[256], h[41];

        while (fscanf(file, "%s %s", path, h) == 2)
        {
            if (strcmp(path, filepath) == 0)
            {
                fprintf(temp, "%s %s\n", filepath, hash);
                found = 1;
            }
            else
            {
                fprintf(temp, "%s %s\n", path, h);
            }
        }

        fclose(file);
    }

    if (!found)
    {
        fprintf(temp, "%s %s\n", filepath, hash);
    }

    fclose(temp);

    remove(".mgit/index");
    rename(".mgit/index_tmp", ".mgit/index");

    return 0;
}

void print_index()
{
    FILE *file = fopen(INDEX_FILE, "r");

    if (!file)
    {
        printf("Index is empty\n");
        return;
    }

    char line[512];

    while (fgets(line, sizeof(line), file))
        printf("%s", line);

    fclose(file);
}

int get_index_hash(const char *filepath, char hash[41])
{
    FILE *file = fopen(".mgit/index", "r");

    if (!file)
        return 0;

    char path[256], h[41];

    while (fscanf(file, "%s %s", path, h) == 2)
    {
        if (strcmp(path, filepath) == 0)
        {
            strcpy(hash, h);
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return 0;
}

int get_last_commit_hash(const char *filepath, char hash[41])
{
    FILE *file = fopen(".mgit/last_commit_index", "r");

    if (!file)
        return 0;

    char path[256], h[41];

    while (fscanf(file, "%s %s", path, h) == 2)
    {
        if (strcmp(path, filepath) == 0)
        {
            strcpy(hash, h);
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return 0;
}