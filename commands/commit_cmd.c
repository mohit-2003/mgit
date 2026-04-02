#include <stdio.h>
#include <string.h>

#include "../include/commit.h"
#include "../include/tree.h"
#include "../include/utils.h"
#include "../include/constants.h"
#include "../include/index.h"
#include "../include/mgit.h"

int cmd_commit(int argc, char *argv[])
{
    const char *message = NULL;

    if (argc >= 4 && strcmp(argv[2], "-m") == 0)
        message = argv[3];
    else if (argc >= 3 && argv[2][0] != '-')
        message = argv[2];
    else
    {
        printf("usage: mgit commit \"message\"\n");
        return 0; /* wrong usage — not a failure, don't set error exit */
    }

    if (index_is_empty())
    {
        printf("no changes added to commit (use \"git add\" and/or \"git commit\")\n");
        return 0;
    }

    char tree_hash[HASH_SIZE];
    char commit_hash[HASH_SIZE];

    if (create_tree(tree_hash) != 0)
    {
        printf("error: failed to create tree.\n");
        return 1; /* genuine internal error */
    }

    if (create_commit(tree_hash, message, commit_hash) != 0)
    {
        printf("error: commit failed.\n");
        return 1; /* genuine internal error */
    }

    clear_index();
    rebuild_last_commit_index();

    printf("committed: %s\n", commit_hash);
    return 0;
}