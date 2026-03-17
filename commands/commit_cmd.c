#include <stdio.h>
#include <string.h>

#include "../include/commit_cmd.h"
#include "../include/tree.h"
#include "../include/commit.h"

#define INDEX_FILE ".mgit/index"

static void save_last_commit_snapshot()
{
    FILE *src = fopen(".mgit/index", "r");
    FILE *dst = fopen(".mgit/last_commit_index", "w");

    if (!src || !dst)
        return;

    char line[512];

    while (fgets(line, sizeof(line), src))
        fputs(line, dst);

    fclose(src);
    fclose(dst);
}

static int index_empty()
{
    FILE *file = fopen(INDEX_FILE, "r");

    if (!file)
        return 1;

    int c = fgetc(file);

    fclose(file);

    return (c == EOF);
}

static void clear_index()
{
    FILE *file = fopen(INDEX_FILE, "w");

    if (file)
        fclose(file);
}

int cmd_commit(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Usage: mgit commit \"message\"\n");
        return 1;
    }

    if (index_empty())
    {
        printf("Nothing to commit\n");
        return 1;
    }

    const char *message = argv[2];

    char tree_hash[41];
    char commit_hash[41];

    /* Create tree from index */
    if (create_tree(tree_hash) != 0)
    {
        printf("Failed to create tree\n");
        return 1;
    }

    /* Create commit object */
    if (create_commit(tree_hash, message, commit_hash) != 0)
    {
        printf("Commit failed\n");
        return 1;
    }

    /* Clear staging area */
    save_last_commit_snapshot();
    clear_index();

    printf("Committed: %s\n", commit_hash);

    return 0;
}