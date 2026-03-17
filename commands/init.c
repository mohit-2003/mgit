#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "../include/mgit.h"

int cmd_init()
{

    struct stat st = {0};

    /* Check if repository already exists */
    if (stat(".mgit", &st) == 0)
    {
        printf("Repository already initialized.\n");
        return 1;
    }

    /* Create main repository folder */
    mkdir(".mgit");

    /* Create objects directory */
    mkdir(".mgit/objects");

    /* Create refs directory */
    mkdir(".mgit/refs");

    /* Create heads directory */
    mkdir(".mgit/refs/heads");

    /* Create HEAD file */
    FILE *head = fopen(".mgit/HEAD", "w");

    if (head == NULL)
    {
        printf("Error creating HEAD file\n");
        return 1;
    }

    fprintf(head, "");

    fclose(head);

    printf("Initialized empty mgit repository\n");

    return 0;
}