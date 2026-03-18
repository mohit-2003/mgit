#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "../include/status.h"
#include "../include/blob.h"
#include "../include/index.h"
#include "../include/ignore.h"

#define MAX_FILES 1000

static int is_regular_file(const char *path)
{
    struct stat st;

    if (stat(path, &st) != 0)
        return 0;

    return S_ISREG(st.st_mode);
}

/* Check if file exists in list */
static int find_file(char paths[][256], int count, const char *target)
{
    for (int i = 0; i < count; i++)
    {
        if (strcmp(paths[i], target) == 0)
            return i;
    }
    return -1;
}

int cmd_status()
{
    char index_paths[MAX_FILES][256];
    char index_hashes[MAX_FILES][41];

    char commit_paths[MAX_FILES][256];
    char commit_hashes[MAX_FILES][41];

    int index_count = read_index(index_paths, index_hashes, MAX_FILES);
    int commit_count = read_last_commit(commit_paths, commit_hashes, MAX_FILES);

    printf("=== Staged Files ===\n");

    /* STAGED FILES */
    for (int i = 0; i < index_count; i++)
    {
        int pos = find_file(commit_paths, commit_count, index_paths[i]);

        if (pos == -1)
        {
            printf("new file: %s\n", index_paths[i]);
        }
        else if (strcmp(index_hashes[i], commit_hashes[pos]) != 0)
        {
            printf("modified: %s\n", index_paths[i]);
        }
    }

    printf("\n=== Modified Files ===\n");

    /* MODIFIED FILES (working vs index) */
    for (int i = 0; i < index_count; i++)
    {
        char hash[41];

        if (create_blob(index_paths[i], hash) != 0)
            continue;

        if (strcmp(hash, index_hashes[i]) != 0)
        {
            printf("modified: %s\n", index_paths[i]);
        }
    }

    printf("\n=== Untracked Files ===\n");

    /* UNTRACKED FILES */
    FILE *tmp;

#ifdef _WIN32
    system("dir /b > .mgit_status_tmp");
#else
    system("ls > .mgit_status_tmp");
#endif

    tmp = fopen(".mgit_status_tmp", "r");

    char file[256];

    while (fscanf(tmp, "%s", file) == 1)
    {
        /* Skip internal */
        if (strcmp(file, ".mgit") == 0 ||
            strcmp(file, ".mgitignore") == 0 ||
            strcmp(file, ".mgit_status_tmp") == 0)
            continue;

        /* 🔥 Skip directories */
        if (!is_regular_file(file))
            continue;

        if (is_ignored(file))
            continue;

        if (find_file(index_paths, index_count, file) == -1 &&
            find_file(commit_paths, commit_count, file) == -1)
        {
            printf("untracked: %s\n", file);
        }
    }

    fclose(tmp);
    remove(".mgit_status_tmp");

    return 0;
}