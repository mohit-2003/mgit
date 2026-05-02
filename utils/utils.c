#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h> /* _mkdir */
#endif

#include "../include/utils.h"
#include "../include/constants.h"
#include "../include/dirent.h"

void strip_newline(char *s)
{
    s[strcspn(s, "\r\n")] = '\0';
}

int file_exists(const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f)
        return 0;
    fclose(f);
    return 1;
}

int is_regular_file(const char *path)
{
    struct stat st;
    if (stat(path, &st) != 0)
        return 0;
    return S_ISREG(st.st_mode);
}

int is_directory(const char *path)
{
    struct stat st;
    if (stat(path, &st) != 0)
        return 0;
    return S_ISDIR(st.st_mode);
}

void mgit_mkdir(const char *path)
{
#ifdef _WIN32
    _mkdir(path); /* Windows/MinGW: _mkdir, no permission arg */
#else
    mkdir(path, 0777); /* Linux/Mac: mkdir with permissions */
#endif
}

void object_path(const char *hash, char *out)
{
    snprintf(out, PATH_BUF, "%s/%.2s/%s", OBJECTS_DIR, hash, hash + 2);
}

void ensure_object_dir(const char *hash)
{
    char dir[PATH_BUF];
    snprintf(dir, sizeof(dir), "%s/%.2s", OBJECTS_DIR, hash);
    mgit_mkdir(dir);
}

int is_mgit_repository()
{
    DIR *dir = opendir(MGIT_DIR);
    if (dir)
    {
        closedir(dir);
        return 1;
    }
    return 0;
}

void ensure_parent_dirs(const char *filepath)
{
    char temp[PATH_BUF];
    strncpy(temp, filepath, sizeof(temp) - 1);
    temp[sizeof(temp) - 1] = '\0';

    char *p = temp;
    while (*p)
    {
        if (*p == '/' || *p == '\\')
        {
            *p = '\0';
            mgit_mkdir(temp);
            *p = '/';
        }
        p++;
    }
}

int checkout_file(const char *filepath, const char *blob_hash)
{
    char obj_path[PATH_BUF];
    object_path(blob_hash, obj_path);
    FILE *in = fopen(obj_path, "rb");
    if (!in)
        return 0;

    // skip past the binary header to get to the raw file content
    int c;
    while ((c = fgetc(in)) != EOF && c != '\0')
        ;

    ensure_parent_dirs(filepath);

    FILE *out = fopen(filepath, "wb");
    if (!out)
    {
        fclose(in);
        return 0;
    }

    char buffer[CONTENT_BUF];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), in)) > 0)
    {
        fwrite(buffer, 1, bytes, out);
    }

    fclose(in);
    fclose(out);
    return 1;
}

char *getHomeDirectory()
{
    char *home = getenv("HOME");
    if (!home)
        home = getenv("USERPROFILE");

    return home;
}