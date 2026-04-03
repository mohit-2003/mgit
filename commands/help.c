#include <stdio.h>
#include <string.h>

#include "../include/mgit.h"

int cmd_help(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("usage: mgit help <command>\n");
        printf("commands: init, config, add, unstage, commit, log, status, checkout, branch, help\n");
        return 1;
    }

    const char *target = argv[2];

    /* Match the requested command and print its details */
    if (strcmp(target, "init") == 0)
    {
        printf("mgit init\n");
        printf("  Initialises a new empty mgit repository in the current directory.\n");
        printf("  Creates the .mgit folder structure and writes the initial HEAD file.\n");
    }
    else if (strcmp(target, "config") == 0)
    {
        printf("mgit config <name|email> <value>\n");
        printf("  Sets or lists the global user name and email.\n");
    }
    else if (strcmp(target, "add") == 0)
    {
        printf("mgit add <path>...\n");
        printf("  Stages the specified files. Use '.' to stage all changes in the directory.\n");
    }
    else if (strcmp(target, "unstage") == 0)
    {
        printf("mgit unstage <file>\n");
        printf("  Removes a single file from the index. Use '.' to unstage everything.\n");
    }
    else if (strcmp(target, "commit") == 0)
    {
        printf("mgit commit \"<message>\"\n");
        printf("  Records changes to the repository. Builds a tree from the current index\n");
        printf("  and creates a new commit object.\n");
    }
    else if (strcmp(target, "log") == 0)
    {
        printf("mgit log\n");
        printf("  Resolves HEAD and walks the commit chain, printing the commit history.\n");
    }
    else if (strcmp(target, "status") == 0)
    {
        printf("mgit status\n");
        printf("  Displays three sections: staged changes, unstaged changes, and untracked files.\n");
    }
    else if (strcmp(target, "checkout") == 0)
    {
        printf("mgit checkout [-b] <branch_or_commit>\n");
        printf("  Switches branches, checks out detached HEADs, or creates new branches with -b.\n");
    }
    else if (strcmp(target, "branch") == 0)
    {
        printf("mgit branch [-d|-m] <branch_name>\n");
        printf("  Lists, creates, deletes, or renames branches.\n");
    }
    else if (strcmp(target, "help") == 0)
    {
        printf("mgit help <command>\n");
        printf("  Displays detailed help information for a specific mgit command.\n");
    }
    else
    {
        printf("mgit help: unknown command '%s'\n", target);
        return 1;
    }

    return 0;
}