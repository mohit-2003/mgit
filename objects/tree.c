#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "../include/tree.h"
#include "../include/sha1.h"

#define INDEX_FILE ".mgit/index"

int create_tree(char hash_output[41])
{
    FILE *index = fopen(INDEX_FILE, "r");

    if (!index)
    {
        printf("Index not found\n");
        return 1;
    }

    char line[512];
    char buffer[8192] = "";
    int offset = 0;

    /* Read index and build tree content */
    while (fgets(line, sizeof(line), index))
    {
        char file[256];
        char hash[41];

        sscanf(line, "%s %s", file, hash);

        offset += sprintf(buffer + offset,
                          "100644 %s %s\n",
                          file, hash);
    }

    fclose(index);

    /* Create tree header */
    char header[64];
    int header_len = sprintf(header, "tree %d", offset) + 1;

    int total_size = header_len + offset;

    char *store = malloc(total_size);

    memcpy(store, header, header_len);
    memcpy(store + header_len, buffer, offset);

    /* Compute SHA1 */
    unsigned char hash_bin[20];
    sha1((unsigned char *)store, total_size, hash_bin);

    sha1_to_hex(hash_bin, hash_output);

    /* Object path */
    char dir[3];
    strncpy(dir, hash_output, 2);
    dir[2] = '\0';

    char path[512];

    sprintf(path, ".mgit/objects/%s", dir);
    mkdir(path, 0777);

    sprintf(path, ".mgit/objects/%s/%s", dir, hash_output + 2);

    FILE *obj = fopen(path, "wb");

    fwrite(store, 1, total_size, obj);

    fclose(obj);
    free(store);

    return 0;
}