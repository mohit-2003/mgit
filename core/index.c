#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/index.h"

#define INDEX_FILE ".mgit/index"

int add_index_entry(const char *filepath, const char hash[41])
{
    FILE *file = fopen(INDEX_FILE, "a");

    if (!file)
    {
        printf("Error opening index file\n");
        return 1;
    }

    fprintf(file, "%s %s\n", filepath, hash);

    fclose(file);

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