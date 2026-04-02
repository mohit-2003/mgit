#include <stdio.h>
#include <string.h>

#include "../include/ignore.h"
#include "../include/constants.h"
#include "../include/utils.h"

// TODO: implement proper glob pattern matching instead of simple substring match
int is_ignored(const char *filepath)
{
    FILE *f = fopen(IGNORE_FILE, "r");
    if (!f)
        return 0;

    char pattern[PATH_BUF];
    int ignored = 0;

    while (fgets(pattern, sizeof(pattern), f))
    {
        strip_newline(pattern);

        /* Skip empty lines and comments */
        if (pattern[0] == '\0' || pattern[0] == '#')
            continue;

        /* Simple substring match: if the path contains the pattern */
        if (strstr(filepath, pattern) != NULL)
        {
            ignored = 1;
            break;
        }
    }

    fclose(f);
    return ignored;
}