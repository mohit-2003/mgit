#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/tree.h"
#include "../include/index.h"
#include "../include/sha1.h"
#include "../include/utils.h"
#include "../include/constants.h"

int create_tree(char hash_output[HASH_SIZE])
{
    /* Load currently staged files */
    char (*index_paths)[PATH_BUF] = malloc(MAX_FILES * PATH_BUF);
    char (*index_hashes)[HASH_SIZE] = malloc(MAX_FILES * HASH_SIZE);
    int index_count = read_index(index_paths, index_hashes, MAX_FILES);

    char (*commit_paths)[PATH_BUF] = malloc(MAX_FILES * PATH_BUF);
    char (*commit_hashes)[HASH_SIZE] = malloc(MAX_FILES * HASH_SIZE);
    int commit_count = read_last_commit(commit_paths, commit_hashes, MAX_FILES);

    /* Build the merged file list */
    char (*merged_paths)[PATH_BUF] = malloc(MAX_FILES * PATH_BUF);
    char (*merged_hashes)[HASH_SIZE] = malloc(MAX_FILES * HASH_SIZE);
    int merged_count = 0;

    /* Step 1: copy all files from the last commit */
    for (int i = 0; i < commit_count && merged_count < MAX_FILES; i++)
    {
        strncpy(merged_paths[merged_count], commit_paths[i], PATH_BUF - 1);
        strncpy(merged_hashes[merged_count], commit_hashes[i], HASH_SIZE - 1);
        merged_paths[merged_count][PATH_BUF - 1] = '\0';
        merged_hashes[merged_count][HASH_SIZE - 1] = '\0';
        merged_count++;
    }

    /* Step 2: overlay staged files — update in place if already present */
    for (int i = 0; i < index_count; i++)
    {
        int found = 0;
        for (int j = 0; j < merged_count; j++)
        {
            if (strcmp(merged_paths[j], index_paths[i]) == 0)
            {
                /* file was committed before — update its hash */
                strncpy(merged_hashes[j], index_hashes[i], HASH_SIZE - 1);
                found = 1;
                break;
            }
        }
        if (!found && merged_count < MAX_FILES)
        {
            /* new file — append it */
            strncpy(merged_paths[merged_count], index_paths[i], PATH_BUF - 1);
            strncpy(merged_hashes[merged_count], index_hashes[i], HASH_SIZE - 1);
            merged_paths[merged_count][PATH_BUF - 1] = '\0';
            merged_hashes[merged_count][HASH_SIZE - 1] = '\0';
            merged_count++;
        }
    }

    if (merged_count == 0)
    {
        printf("Error: nothing to commit.\n");
        return 1;
    }

    /* Build the tree content string: "100644 filepath hash\n" per entry */
    char buf[CONTENT_BUF] = "";
    int offset = 0;
    for (int i = 0; i < merged_count; i++)
    {
        offset += snprintf(buf + offset, sizeof(buf) - offset,
                           "100644 %s %s\n",
                           merged_paths[i], merged_hashes[i]);
    }

    /* Prepend binary header: "tree <size>\0" */
    char header[64];
    int hlen = sprintf(header, "tree %d", offset) + 1; /* +1 for '\0' */

    size_t total = (size_t)hlen + (size_t)offset;
    char *store = malloc(total);
    if (!store)
        return 1;

    memcpy(store, header, hlen);
    memcpy(store + hlen, buf, offset);

    /* Hash and write to object store */
    unsigned char hash_bin[20];
    sha1((unsigned char *)store, total, hash_bin);
    sha1_to_hex(hash_bin, hash_output);

    char obj_path[PATH_BUF];
    object_path(hash_output, obj_path);
    ensure_object_dir(hash_output);

    FILE *obj = fopen(obj_path, "wb");
    if (!obj)
    {
        free(store);
        return 1;
    }
    fwrite(store, 1, total, obj);
    fclose(obj);

    free(store);

    // free memory
    free(index_paths);
    free(index_hashes);
    free(commit_paths);
    free(commit_hashes);
    free(merged_paths);
    free(merged_hashes);

    return 0;
}