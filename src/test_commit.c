#include <stdio.h>
#include "../include/tree.h"
#include "../include/commit.h"

int main()
{
    char tree_hash[41];
    char commit_hash[41];

    create_tree(tree_hash);

    create_commit(tree_hash,
                  "first commit",
                  commit_hash);

    printf("Commit: %s\n", commit_hash);

    return 0;
}