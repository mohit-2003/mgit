#include <stdio.h>
#include <string.h>
#include "../include/ignore.h"

#define IGNORE_FILE ".mgitignore"

int is_ignored(const char *filepath)
{
    FILE *file = fopen(IGNORE_FILE, "r");
    if (!file)
        return 0;

    char pattern[256];
    int ignored = 0;

    while (fgets(pattern, sizeof(pattern), file))
    {
        // Remove newline
        pattern[strcspn(pattern, "\r\n")] = 0;

        if (strlen(pattern) == 0 || pattern[0] == '#')
            continue;

        // Simple match: if the path starts with the pattern
        // or contains it as a folder segment (e.g., "node_modules/")
        if (strstr(filepath, pattern) != NULL)
        {
            ignored = 1;
            break;
        }
    }

    fclose(file);
    return ignored;
}