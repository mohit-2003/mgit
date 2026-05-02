#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/blob.h"
#include "../include/sha1.h"
#include "../include/utils.h"
#include "../include/constants.h"

int create_blob(const char *filepath, char hash_output[HASH_SIZE])
{
    FILE *f = fopen(filepath, "rb");
    if (!f)
    {
        printf("Error: cannot open '%s'\n", filepath);
        return 1;
    }

    /* Read entire file into memory */
    fseek(f, 0, SEEK_END); // moves the file pointer to the end of the file
    long size = ftell(f);  // returns the current file pointer position, which is the size of the file in bytes
    rewind(f);             // resets the file pointer back to the beginning of the file for subsequent reading

    unsigned char *data = malloc(size);
    if (!data)
    {
        fclose(f);
        return 1;
    }
    fread(data, 1, size, f);
    fclose(f);

    /*
     * Git blob format:  "blob <size>\0<raw file bytes>"
     * The null byte is the separator between header and content.
     */
    char header[64];
    int hlen = sprintf(header, "blob %ld", size) + 1; // +1 for '\0'

    size_t total = (size_t)hlen + (size_t)size;
    unsigned char *store = malloc(total);
    if (!store)
    {
        free(data);
        return 1;
    }

    // copy header first, then file bytes right after it
    memcpy(store, header, hlen);
    memcpy(store + hlen, data, size);
    free(data);

    /* Compute SHA-1 over the full blob (header + content) */
    unsigned char hash_bin[20];
    sha1(store, total, hash_bin);
    sha1_to_hex(hash_bin, hash_output);

    /* Write to object store if not already there */
    char obj_path[PATH_BUF];
    object_path(hash_output, obj_path);
    ensure_object_dir(hash_output); // creates the two-char subdirectory if missing

    FILE *obj = fopen(obj_path, "wb");
    if (!obj)
    {
        printf("Error: cannot write object for '%s'\n", filepath);
        free(store);
        return 1;
    }
    fwrite(store, 1, total, obj);
    fclose(obj);

    free(store);
    return 0;
}