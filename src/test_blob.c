#include <stdio.h>
#include "../include/blob.h"

int main()
{
    char hash[41];

    create_blob("hello.txt", hash);

    printf("Blob hash: %s\n", hash);

    return 0;
}