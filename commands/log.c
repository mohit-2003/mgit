#include <stdio.h>
#include <string.h>

#include "../include/log.h"

#define HEAD_FILE ".mgit/HEAD"

/* Read current HEAD commit hash */
static int get_head_commit(char hash[41])
{
    FILE *head = fopen(HEAD_FILE, "r");

    if (!head)
        return 0;

    char ref[256];
    fgets(ref, sizeof(ref), head);
    fclose(head);

    ref[strcspn(ref, "\n")] = 0;

    /* 🔥 CASE 1: HEAD contains direct hash */
    if (strlen(ref) == 40)
    {
        strcpy(hash, ref);
        return 1;
    }

    /* 🔥 CASE 2: HEAD contains ref path */
    if (strncmp(ref, "ref:", 4) == 0)
    {
        memmove(ref, ref + 5, strlen(ref));
    }

    FILE *ref_file = fopen(ref, "r");

    if (!ref_file)
        return 0;

    fgets(hash, 41, ref_file);
    fclose(ref_file);

    hash[strcspn(hash, "\n")] = 0;

    return 1;
}

static void print_commit(const char *hash)
{
    char path[256];
    sprintf(path, ".mgit/objects/%c%c/%s", hash[0], hash[1], hash + 2);

    FILE *file = fopen(path, "r");

    if (!file)
        return;

    printf("commit %s\n", hash);

    char line[512];
    char parent[41] = "";

    while (fgets(line, sizeof(line), file))
    {
        if (strncmp(line, "parent ", 7) == 0)
        {
            sscanf(line, "parent %s", parent);
        }
        else if (strncmp(line, "author ", 7) == 0)
        {
            printf("%s", line);
        }
        else if (strncmp(line, "date ", 5) == 0)
        {
            printf("%s", line);
        }
        else if (line[0] == '\n')
        {
            /* message starts */
            printf("\n");

            while (fgets(line, sizeof(line), file))
                printf("    %s", line);

            break;
        }
    }

    printf("\n");

    fclose(file);

    /* recursive call */
    if (strlen(parent) > 0)
    {
        print_commit(parent);
    }
}

int cmd_log()
{
    char hash[41];

    if (!get_head_commit(hash))
    {
        printf("No commits yet\n");
        return 1;
    }

    print_commit(hash);

    return 0;
}