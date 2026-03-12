#include <stdio.h>
#include <string.h>
#include "../include/sha1.h"

int main()
{
    unsigned char hash[20];
    char hex[41];

    char *text = "hello";

    sha1((unsigned char *)text, strlen(text), hash);
    sha1_to_hex(hash, hex);

    printf("SHA1: %s\n", hex);

    return 0;
}