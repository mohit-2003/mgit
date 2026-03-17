#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

#include "../include/commit.h"
#include "../include/sha1.h"

#define HEAD_FILE ".mgit/HEAD"
#define CONFIG_FILE ".mgitconfig"

static void read_config(char *name, char *email)
{
    /* Read user name and email from .mgitconfig */
    // Get home directory -> C:/Users/Username
    char *home = getenv("HOME");
    if (home == NULL)
        home = getenv("USERPROFILE");

    if (home == NULL)
    {
        printf("Cannot determine home directory\n");
        return 1;
    }
    char config_path[512];
    sprintf(config_path, "%s/%s", home, CONFIG_FILE);
    FILE *file = fopen(config_path, "r");

    if (!file)
        return;

    char line[256];

    while (fgets(line, sizeof(line), file))
    {
        if (strncmp(line, "name =", 6) == 0)
            sscanf(line, "name = %[^\n]", name);

        if (strncmp(line, "email =", 7) == 0)
            sscanf(line, "email = %[^\n]", email);
    }

    fclose(file);
}

static void read_head(char parent[41])
{
    FILE *file = fopen(HEAD_FILE, "r");

    if (!file)
        return;

    fgets(parent, 41, file);

    fclose(file);
}

static void write_head(const char *hash)
{
    FILE *file = fopen(HEAD_FILE, "w");

    if (!file)
        return;

    fprintf(file, "%s", hash);

    fclose(file);
}

int create_commit(const char *tree_hash,
                  const char *message,
                  char commit_hash[41])
{
    char name[256] = "";
    char email[256] = "";
    char parent[41] = "";

    read_config(name, email);
    read_head(parent);

    time_t now = time(NULL);

    char content[4096];

    int offset = 0;

    offset += sprintf(content + offset, "tree %s\n", tree_hash);

    if (strlen(parent) > 0)
        offset += sprintf(content + offset, "parent %s\n", parent);

    offset += sprintf(content + offset,
                      "author %s <%s>\n",
                      name,
                      email);

    offset += sprintf(content + offset,
                      "date %ld\n\n",
                      now);

    offset += sprintf(content + offset,
                      "%s\n",
                      message);

    char header[64];

    int header_len = sprintf(header, "commit %d", offset) + 1;

    int total = header_len + offset;

    char *store = malloc(total);

    memcpy(store, header, header_len);
    memcpy(store + header_len, content, offset);

    unsigned char hash_bin[20];

    sha1((unsigned char *)store, total, hash_bin);

    sha1_to_hex(hash_bin, commit_hash);

    char dir[3];
    strncpy(dir, commit_hash, 2);
    dir[2] = '\0';

    char path[512];

    sprintf(path, ".mgit/objects/%s", dir);
    mkdir(path, 0777);

    sprintf(path,
            ".mgit/objects/%s/%s",
            dir,
            commit_hash + 2);

    FILE *obj = fopen(path, "wb");

    fwrite(store, 1, total, obj);

    fclose(obj);

    write_head(commit_hash);

    free(store);

    return 0;
}