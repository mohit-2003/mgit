#include <stdio.h>
#include <string.h>

#include "../include/add.h"
#include "../include/blob.h"
#include "../include/index.h"

int cmd_add(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Usage: mgit add <file>\n");
        return 1;
    }

    const char *filepath = argv[2];

    char hash[41];

    /* Create blob object */
    if (create_blob(filepath, hash) != 0)
    {
        printf("Failed to create blob\n");
        return 1;
    }

    /* Add to index */
    if (add_index_entry(filepath, hash) != 0)
    {
        printf("Failed to update index\n");
        return 1;
    }

    printf("Added %s\n", filepath);

    return 0;
}
