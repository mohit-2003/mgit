#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/dirent.h"
#include "../include/mgit.h"
#include "../include/utils.h"
#include "../include/constants.h"

/**
 * @brief resolves HEAD to a commit hash and current branch name
 *
 * handles both symbolic refs (attached HEAD) and bare hashes (detached HEAD).
 *
 * @param commit_hash     output buffer for the 40-char commit hash (must be HASH_SIZE)
 * @param current_branch  output buffer for the branch name, or "HEAD (detached)" if detached
 * @return 1 if a commit hash was found, 0 if HEAD doesn't exist or branch has no commits yet
 */
static int get_current_head_info(char *commit_hash, char *current_branch)
{
    commit_hash[0] = '\0';
    current_branch[0] = '\0';

    FILE *head = fopen(HEAD_FILE, "r");
    if (!head)
        return 0;

    char line[LINE_BUF];
    if (!fgets(line, sizeof(line), head))
    {
        fclose(head);
        return 0;
    }
    fclose(head);

    line[strcspn(line, "\r\n")] = '\0';

    if (strncmp(line, "ref:", 4) == 0)
    {
        char ref_path[PATH_BUF];
        sscanf(line, "ref: %1023s", ref_path);

        const char *branch_name = strrchr(ref_path, '/');
        if (branch_name)
        {
            strcpy(current_branch, branch_name + 1);
        }
        else
        {
            strcpy(current_branch, ref_path);
        }

        FILE *ref = fopen(ref_path, "r");
        if (ref)
        {
            char temp_line[LINE_BUF];
            if (fgets(temp_line, sizeof(temp_line), ref))
            {
                temp_line[strcspn(temp_line, "\r\n")] = '\0';
                strncpy(commit_hash, temp_line, HASH_SIZE - 1);
                commit_hash[HASH_SIZE - 1] = '\0';
            }
            fclose(ref);
        }
    }
    else if (strlen(line) == 40)
    {
        strncpy(commit_hash, line, HASH_SIZE);
        commit_hash[HASH_SIZE - 1] = '\0';
        strcpy(current_branch, "HEAD (detached)");
    }

    return (commit_hash[0] != '\0');
}

/**
 * @brief lists all branches, highlighting the current one in green
 *
 * prints a "*" next to the active branch. if HEAD is detached,
 * prints a red detached HEAD notice at the bottom.
 *
 * @return 0 on success, 1 if the branch directory couldn't be opened
 */
static int list_branches()
{
    char commit_hash[HASH_SIZE];
    char current_branch[PATH_BUF];
    get_current_head_info(commit_hash, current_branch);

    DIR *dir = opendir(HEADS_DIR);
    if (!dir)
    {
        printf("fatal: could not open branch directory.\n");
        return 1;
    }

    struct dirent *entry;
    int has_branches = 0;

    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_name[0] == '.')
            continue;

        has_branches = 1;
        if (strcmp(entry->d_name, current_branch) == 0)
        {
            printf("* \033[32m%s\033[0m\n", entry->d_name);
        }
        else
        {
            printf("  %s\n", entry->d_name);
        }
    }
    closedir(dir);

    if (!has_branches)
    {
        printf("No branches found.\n");
    }

    if (strcmp(current_branch, "HEAD (detached)") == 0)
    {
        printf("* \033[31m(HEAD detached at %s)\033[0m\n", commit_hash);
    }

    return 0;
}

/**
 * @brief creates a new branch pointing to the current HEAD commit
 *
 * @param branch_name  name for the new branch
 * @return 0 on success, 1 if the branch already exists, HEAD has no commits, or file write fails
 */
static int create_branch(const char *branch_name)
{
    char branch_path[PATH_BUF];
    snprintf(branch_path, sizeof(branch_path), "%s/%s", HEADS_DIR, branch_name);

    if (file_exists(branch_path))
    {
        printf("fatal: A branch named '%s' already exists.\n", branch_name);
        return 1;
    }

    char current_hash[HASH_SIZE];
    char current_branch[PATH_BUF];

    if (!get_current_head_info(current_hash, current_branch))
    {
        printf("fatal: Not a valid object name: 'HEAD'. (Do you have any commits yet?)\n");
        return 1;
    }

    mgit_mkdir(REFS_DIR);
    mgit_mkdir(HEADS_DIR);

    FILE *f = fopen(branch_path, "w");
    if (!f)
    {
        printf("fatal: could not create branch '%s'\n", branch_name);
        return 1;
    }

    fprintf(f, "%s\n", current_hash);
    fclose(f);

    return 0;
}

/**
 * @brief deletes a branch by removing its ref file
 *
 * @param branch_name  name of the branch to delete
 * @return 0 on success, 1 if branch doesn't exist, is currently checked out, or remove fails
 *
 * @note refuses to delete the currently checked out branch
 */
static int delete_branch(const char *branch_name)
{
    char branch_path[PATH_BUF];
    snprintf(branch_path, sizeof(branch_path), "%s/%s", HEADS_DIR, branch_name);

    if (!file_exists(branch_path))
    {
        printf("error: branch '%s' not found.\n", branch_name);
        return 1;
    }

    char current_hash[HASH_SIZE];
    char current_branch[PATH_BUF];
    get_current_head_info(current_hash, current_branch);

    if (strcmp(current_branch, branch_name) == 0)
    {
        printf("error: Cannot delete branch '%s'\n", branch_name);
        return 1;
    }

    if (remove(branch_path) == 0)
    {
        printf("Deleted branch %s.\n", branch_name);
        return 0;
    }
    else
    {
        printf("error: failed to delete branch '%s'.\n", branch_name);
        return 1;
    }
}

/**
 * @brief renames a branch by renaming its ref file
 *
 * if the renamed branch is the currently checked out one,
 * HEAD gets updated to point to the new name.
 *
 * @param old_name  current branch name
 * @param new_name  new branch name
 * @return 0 on success, 1 if old branch doesn't exist, new name already taken, or rename fails
 *
 * @note HEAD is rewritten if we're renaming the active branch, otherwise it'd point to a dead ref
 */
static int rename_branch(const char *old_name, const char *new_name)
{
    char old_path[PATH_BUF];
    char new_path[PATH_BUF];
    snprintf(old_path, sizeof(old_path), "%s/%s", HEADS_DIR, old_name);
    snprintf(new_path, sizeof(new_path), "%s/%s", HEADS_DIR, new_name);

    if (!file_exists(old_path))
    {
        printf("error: refname refs/heads/%s not found\n", old_name);
        return 1;
    }

    if (file_exists(new_path))
    {
        printf("fatal: A branch named '%s' already exists.\n", new_name);
        return 1;
    }

    if (rename(old_path, new_path) != 0)
    {
        printf("error: failed to rename branch.\n");
        return 1;
    }

    // Safety: If we just renamed the branch we are currently on, we MUST update HEAD!
    char current_hash[HASH_SIZE];
    char current_branch[PATH_BUF];
    get_current_head_info(current_hash, current_branch);

    if (strcmp(current_branch, old_name) == 0)
    {
        FILE *head_file = fopen(HEAD_FILE, "w");
        if (head_file)
        {
            fprintf(head_file, "ref: %s/%s\n", HEADS_DIR, new_name);
            fclose(head_file);
        }
    }

    return 0;
}

int cmd_branch(int argc, char *argv[])
{
    if (argc == 2)
    {
        return list_branches();
    }
    else if (argc == 3)
    {
        return create_branch(argv[2]);
    }
    else if (argc == 4)
    {
        if (strcmp(argv[2], "-d") == 0 || strcmp(argv[2], "-D") == 0)
        {
            return delete_branch(argv[3]);
        }
        else if (strcmp(argv[2], "-m") == 0 || strcmp(argv[2], "-M") == 0)
        {
            // mgit branch -m <new_name> -> Rename CURRENT branch
            char current_hash[HASH_SIZE];
            char current_branch[PATH_BUF];
            get_current_head_info(current_hash, current_branch);

            if (strcmp(current_branch, "HEAD (detached)") == 0)
            {
                printf("fatal: it does not make sense to create 'HEAD' manually.\n");
                return 1;
            }

            return rename_branch(current_branch, argv[3]);
        }
    }
    else if (argc == 5)
    {
        if (strcmp(argv[2], "-m") == 0 || strcmp(argv[2], "-M") == 0)
        {
            // mgit branch -m <old_name> <new_name>
            return rename_branch(argv[3], argv[4]);
        }
    }

    printf("Usage: mgit branch [branch_name]\n");
    printf("       mgit branch -d <branch_name>\n");
    printf("       mgit branch -m [<old_name>] <new_name>\n");
    return 1;
}