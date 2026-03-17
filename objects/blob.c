#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "../include/blob.h"
#include "../include/sha1.h"

int create_blob(const char *filepath, char hash_output[41])
{
    FILE *file = fopen(filepath, "rb");

    if (!file)
    {
        printf("Error opening file: %s\n", filepath);
        return 1;
    }

    /* Determine file size */
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    /* Read file data */
    char *data = malloc(size);
    fread(data, 1, size, file);
    fclose(file);

    /* Create blob header */
    char header[64];
    int header_len = sprintf(header, "blob %ld", size) + 1;

    int total_size = header_len + size;

    char *store = malloc(total_size);

    memcpy(store, header, header_len);
    memcpy(store + header_len, data, size);

    free(data);

    /* Compute SHA1 */
    unsigned char hash_bin[20];
    sha1((unsigned char *)store, total_size, hash_bin);

    sha1_to_hex(hash_bin, hash_output);

    /* Determine object path */
    char dir[3];
    strncpy(dir, hash_output, 2);
    dir[2] = '\0';

    char path[512];
    sprintf(path, ".mgit/objects/%s", dir);

    mkdir(path, 0777);

    sprintf(path, ".mgit/objects/%s/%s", dir, hash_output + 2);

    /* Write object if not already present */
    FILE *obj = fopen(path, "wb");

    if (!obj)
    {
        printf("Error writing object\n");
        free(store);
        return 1;
    }

    fwrite(store, 1, total_size, obj);

    fclose(obj);
    free(store);

    return 0;
}