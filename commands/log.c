#include <stdio.h>
#include <string.h>
#include <time.h>

#include "../include/mgit.h"
#include "../include/utils.h"
#include "../include/constants.h"

/**
 * @brief resolves HEAD to a commit hash
 *
 * handles both symbolic refs (normal branch) and bare hashes (detached HEAD).
 *
 * @param hash  output buffer for the 40-char commit hash (must be HASH_SIZE)
 * @return 1 if a commit hash was found, 0 if HEAD doesn't exist or has no commits yet
 */
static int get_head_commit(char hash[HASH_SIZE])
{
    hash[0] = '\0';

    FILE *f = fopen(HEAD_FILE, "r");
    if (!f)
        return 0;

    char line[LINE_BUF];
    if (!fgets(line, sizeof(line), f))
    {
        fclose(f);
        return 0;
    }
    fclose(f);
    strip_newline(line);

    if (strncmp(line, "ref:", 4) == 0)
    {
        char ref_path[PATH_BUF];
        sscanf(line, "ref: %1023s", ref_path);

        FILE *ref = fopen(ref_path, "r");
        if (!ref)
            return 0;

        char buf[HASH_SIZE + 4];
        if (!fgets(buf, sizeof(buf), ref))
        {
            fclose(ref);
            return 0;
        }
        fclose(ref);

        strip_newline(buf);
        if (strlen(buf) == 40)
        {
            strncpy(hash, buf, HASH_SIZE);
            return 1;
        }
        return 0;
    }

    if (strlen(line) == 40)
    {
        strncpy(hash, line, HASH_SIZE);
        hash[HASH_SIZE - 1] = '\0';
        return 1;
    }
    return 0;
}

/**
 * @brief prints a single commit and then recursively walks to its parent
 *
 * reads the commit object file, prints hash, author, date, and message,
 * then calls itself on the parent hash if one exists.
 *
 * @param hash  40-char hex hash of the commit to print
 *
 * @note recursion depth equals the number of commits — could stack overflow
 *       on a very long history, but fine for typical use
 */
static void print_commit(const char *hash)
{
    char path[PATH_BUF];
    object_path(hash, path);

    FILE *f = fopen(path, "r");
    if (!f)
    {
        printf("Error: object %s not found.\n", hash);
        return;
    }

    printf("commit %s\n", hash);

    char line[LINE_BUF];
    char parent[HASH_SIZE] = "";
    int in_message = 0;

    while (fgets(line, sizeof(line), f))
    {
        if (in_message)
        {
            printf("    %s", line);
            continue;
        }

        if (strncmp(line, "parent ", 7) == 0)
        {
            sscanf(line, "parent %40s", parent);
        }
        else if (strncmp(line, "author ", 7) == 0)
        {
            printf("%s", line);
        }
        else if (strncmp(line, "date ", 5) == 0)
        {
            long ts = 0;
            sscanf(line, "date %ld", &ts);
            time_t t = (time_t)ts;
            char buf[64];
            // format - e.g. "Wed Mar 15 14:30:00 2023"
            strftime(buf, sizeof(buf), "%a %b %d %H:%M:%S %Y", localtime(&t));
            printf("date   %s\n", buf);
        }
        else if (line[0] == '\n')
        {
            printf("\n");
            in_message = 1;
        }
    }
    fclose(f);
    printf("\n");

    if (parent[0] != '\0')
        print_commit(parent);
}

int cmd_log()
{
    char hash[HASH_SIZE];

    if (!get_head_commit(hash))
    {
        /*
         * No commits yet — informational, not an error.
         * Return 0 so the terminal prompt stays green.
         */
        printf("No commits yet.\n");
        return 0;
    }

    print_commit(hash);
    return 0;
}