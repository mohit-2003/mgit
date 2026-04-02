#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../include/index.h"
#include "../include/constants.h"

/**
 * @brief parses a single index or tree line into path and hash
 *
 * handles both plain index lines ("filepath hash") and tree object
 * lines ("100644 filepath hash"). path can have spaces in it, that's fine.
 *
 * @param line  the raw line to parse (gets modified — newline stripped)
 * @param path  output buffer for the file path
 * @param hash  output buffer for the 40-char hex hash
 * @return 1 on success, 0 if the line is too short or malformed
 */
static int extract_path_and_hash(char *line, char *path, char *hash)
{
    // Step 1: Remove the newline character at the end of the line
    int len = strlen(line);
    if (len > 0 && line[len - 1] == '\n')
    {
        line[len - 1] = '\0';
        len--;
    }

    // Step 2: Make sure the line is long enough.
    // It needs at least 1 char (path) + 1 char (space) + 40 chars (hash) = 42 chars
    if (len < 42)
        return 0;

    // Step 3: The hash is ALWAYS the last 40 characters. Copy it directly.
    strcpy(hash, &line[len - 40]);

    // Step 4: Figure out where the path starts.
    // If it's reading from a tree object, it has "100644 " at the start (7 chars).
    int start_pos = 0;
    if (strncmp(line, "100644 ", 7) == 0)
    {
        start_pos = 7;
    }

    // Step 5: Calculate the length of the file path.
    // Length = Total length minus 40 (for hash) minus 1 (for the space) minus the start position
    int path_len = len - 41 - start_pos;

    // Step 6: Copy exactly that many characters to get the path (spaces included!)
    strncpy(path, &line[start_pos], path_len);
    path[path_len] = '\0'; // Always add the null terminator in C!

    return 1; // Success
}

/**
 * @brief reads an index-style file into path and hash arrays
 *
 * generic helper used by both read_index() and read_last_commit().
 * stops at max entries or EOF, whichever comes first.
 *
 * @param filename  path to the file to read
 * @param paths     output array for file paths
 * @param hashes    output array for hashes
 * @param max       max number of entries to read
 * @return number of entries successfully read
 */
static int read_index_file(const char *filename, char paths[][PATH_BUF], char hashes[][HASH_SIZE], int max)
{
    FILE *f = fopen(filename, "r");
    if (!f)
        return 0;

    int count = 0;
    char line[PATH_BUF + HASH_SIZE];

    // Read line by line using fgets (which reads spaces perfectly)
    while (count < max && fgets(line, sizeof(line), f))
    {
        if (extract_path_and_hash(line, paths[count], hashes[count]))
        {
            count++;
        }
    }

    fclose(f);
    return count;
}

int add_index_entry(const char *filepath, const char hash[HASH_SIZE])
{
    FILE *src = fopen(INDEX_FILE, "r");
    FILE *tmp = fopen(".mgit/index_tmp", "w");
    if (!tmp)
    {
        if (src)
            fclose(src);
        return 1;
    }

    int found = 0;
    if (src)
    {
        char line[PATH_BUF + HASH_SIZE];
        char p[PATH_BUF], h[HASH_SIZE];

        while (fgets(line, sizeof(line), src))
        {
            if (extract_path_and_hash(line, p, h))
            {
                if (strcmp(p, filepath) == 0)
                {
                    fprintf(tmp, "%s %s\n", filepath, hash); // Update existing
                    found = 1;
                }
                else
                {
                    fprintf(tmp, "%s %s\n", p, h); // Keep old
                }
            }
        }
        fclose(src);
    }

    if (!found)
    {
        fprintf(tmp, "%s %s\n", filepath, hash); // Add as new entry
    }

    fclose(tmp);
    remove(INDEX_FILE);
    rename(".mgit/index_tmp", INDEX_FILE);
    return 0;
}

int read_index(char paths[][PATH_BUF], char hashes[][HASH_SIZE], int max)
{
    return read_index_file(INDEX_FILE, paths, hashes, max);
}

int read_last_commit(char paths[][PATH_BUF], char hashes[][HASH_SIZE], int max)
{
    return read_index_file(LAST_INDEX_FILE, paths, hashes, max);
}

int get_index_hash(const char *filepath, char hash[HASH_SIZE])
{
    char p[PATH_BUF], h[HASH_SIZE];
    FILE *f = fopen(INDEX_FILE, "r");
    if (!f)
        return 0;

    char line[PATH_BUF + HASH_SIZE];
    while (fgets(line, sizeof(line), f))
    {
        if (extract_path_and_hash(line, p, h) && strcmp(p, filepath) == 0)
        {
            strncpy(hash, h, HASH_SIZE);
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}

int get_last_commit_hash(const char *filepath, char hash[HASH_SIZE])
{
    char p[PATH_BUF], h[HASH_SIZE];
    FILE *f = fopen(LAST_INDEX_FILE, "r");
    if (!f)
        return 0;

    char line[PATH_BUF + HASH_SIZE];
    while (fgets(line, sizeof(line), f))
    {
        if (extract_path_and_hash(line, p, h) && strcmp(p, filepath) == 0)
        {
            strncpy(hash, h, HASH_SIZE);
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}

void print_index(void)
{
    FILE *f = fopen(INDEX_FILE, "r");
    if (!f)
    {
        printf("Index is empty.\n");
        return;
    }
    char line[LINE_BUF];
    while (fgets(line, sizeof(line), f))
        printf("%s", line);
    fclose(f);
}

/**
 * @brief opens a git object file by its hash for binary reading
 *
 * builds the path as objects/xx/xxxxxx... where xx is the first two chars of the hash.
 *
 * @param hash  the 40-char hex hash of the object
 * @return FILE* on success, NULL if the object doesn't exist
 */
static FILE *open_object_bin(const char *hash)
{
    char path[PATH_BUF];
    snprintf(path, sizeof(path), "%s/%.2s/%s", OBJECTS_DIR, hash, hash + 2);
    return fopen(path, "rb");
}

void rebuild_last_commit_index(void)
{
    FILE *head = fopen(HEAD_FILE, "r");
    if (!head)
        return;

    char line[PATH_BUF + HASH_SIZE];
    if (!fgets(line, sizeof(line), head))
    {
        fclose(head);
        return;
    }
    fclose(head);

    // Clean newline
    line[strcspn(line, "\r\n")] = '\0';

    char commit_hash[HASH_SIZE] = "";

    // Check if HEAD points to a branch (ref:) or is a direct hash
    if (strncmp(line, "ref:", 4) == 0)
    {
        char ref_path[PATH_BUF];
        sscanf(line, "ref: %1023s", ref_path);

        FILE *ref = fopen(ref_path, "r");
        if (!ref)
            return;
        if (fgets(line, sizeof(line), ref))
        {
            line[strcspn(line, "\r\n")] = '\0';
            if (strlen(line) == 40)
                strncpy(commit_hash, line, HASH_SIZE);
        }
        fclose(ref);
    }
    else if (strlen(line) == 40)
    {
        strncpy(commit_hash, line, HASH_SIZE);
    }

    if (commit_hash[0] == '\0')
        return;

    // Get the tree hash from the commit object
    char obj_path[PATH_BUF];
    snprintf(obj_path, sizeof(obj_path), "%s/%.2s/%s", OBJECTS_DIR, commit_hash, commit_hash + 2);
    FILE *commit_obj = fopen(obj_path, "r");
    if (!commit_obj)
        return;

    char tree_hash[HASH_SIZE] = "";
    while (fgets(line, sizeof(line), commit_obj))
    {
        if (strncmp(line, "tree ", 5) == 0)
        {
            sscanf(line, "tree %40s", tree_hash);
            break;
        }
    }
    fclose(commit_obj);
    if (tree_hash[0] == '\0')
        return;

    // Read the tree object
    FILE *tree_obj = open_object_bin(tree_hash);
    if (!tree_obj)
        return;

    // Skip the binary header "tree <size>\0"
    int c;
    while ((c = fgetc(tree_obj)) != EOF && c != '\0')
        ;

    FILE *out = fopen(LAST_INDEX_FILE, "w");
    if (!out)
    {
        fclose(tree_obj);
        return;
    }

    // Read the tree entries line by line using our simple parser
    char p[PATH_BUF], h[HASH_SIZE];
    while (fgets(line, sizeof(line), tree_obj))
    {
        if (extract_path_and_hash(line, p, h))
        {
            fprintf(out, "%s %s\n", p, h);
        }
    }

    fclose(tree_obj);
    fclose(out);
}

int index_is_empty(void)
{
    FILE *f = fopen(INDEX_FILE, "r");
    if (!f)
        return 1;
    int c = fgetc(f);
    fclose(f);
    return (c == EOF);
}

void clear_index(void)
{
    FILE *f = fopen(INDEX_FILE, "w");
    if (f)
        fclose(f);
}