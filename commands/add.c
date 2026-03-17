#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "../include/add.h"
#include "../include/blob.h"
#include "../include/index.h"
#include "../include/ignore.h"
#include "../utils/dirent.h"

// Forward declaration
static void process_path(const char *path);

static void add_directory(const char *base_path)
{
    DIR *dir = opendir(base_path);
    if (!dir)
        return;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        // Skip "." and ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char path[1024];
        // Clean path construction to avoid "//"
        if (strcmp(base_path, ".") == 0)
        {
            snprintf(path, sizeof(path), "%s", entry->d_name);
        }
        else
        {
            snprintf(path, sizeof(path), "%s/%s", base_path, entry->d_name);
        }

        process_path(path);
    }
    closedir(dir);
}

static void process_path(const char *path)
{
    // 1. Check if it's ignored or the internal .mgit folder
    if (is_ignored(path) || strstr(path, ".mgit") != NULL)
    {
        return;
    }

    struct stat st;
    if (stat(path, &st) != 0)
        return;

    // 2. If it's a directory, recurse
    if (S_ISDIR(st.st_mode))
    {
        add_directory(path);
    }
    // 3. If it's a file, stage it
    else if (S_ISREG(st.st_mode))
    {
        char hash[41];
        if (create_blob(path, hash) != 0)
            return;

        char old_hash[41];
        // Only add to index if it's new or changed
        if (get_index_hash(path, old_hash) && strcmp(old_hash, hash) == 0)
        {
            return;
        }

        add_index_entry(path, hash);
        printf("added: %s\n", path);
    }
}

int cmd_add(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Usage: mgit add <file1> [dir/file2] ...\n");
        return 1;
    }

    for (int i = 2; i < argc; i++)
    {
        struct stat st;
        if (stat(argv[i], &st) != 0)
        {
            printf("fatal: pathspec '%s' did not match any files\n", argv[i]);
            continue;
        }
        process_path(argv[i]);
    }

    return 0;
}