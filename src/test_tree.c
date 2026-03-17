#include <stdio.h>
#include "../include/tree.h"

int main()
{
    char hash[41];

    create_tree(hash);

    printf("Tree hash: %s\n", hash);

    return 0;
}