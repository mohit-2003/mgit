#include <stdio.h>
#include <sys/stat.h>

#include "../include/mgit.h"
#include "../include/utils.h"
#include "../include/constants.h"

int cmd_init(void)
{
    struct stat st = {0};

    if (stat(MGIT_DIR, &st) == 0)
    {
        printf("Repository already initialised.\n");
        return 1;
    }

    /* mgit_mkdir handles Windows (_mkdir) and Linux (mkdir) automatically */
    mgit_mkdir(MGIT_DIR);
    mgit_mkdir(OBJECTS_DIR);
    mgit_mkdir(REFS_DIR);
    mgit_mkdir(HEADS_DIR);

    FILE *head = fopen(HEAD_FILE, "w");
    if (!head)
    {
        printf("Error: could not create HEAD file.\n");
        return 1;
    }
    fprintf(head, "ref: .mgit/refs/heads/main\n");
    fclose(head);

    printf("Initialised empty mgit repository in .mgit/\n");
    return 0;
}