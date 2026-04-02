#include "../include/mgit.h"
#include "../include/utils.h"

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("mgit: a command is required.\n");
        printf("commands: init, config, add, unstage, commit, log, status, checkout\n");
        return 0;
    }

    const char *cmd = argv[1];

    if (strcmp(cmd, "init") == 0)
        return cmd_init();

    // All other commands require a .mgit repository to be present.
    if (!is_mgit_repository())
    {
        printf("fatal: not a mgit repository: .mgit");
        return 1;
    }

    if (strcmp(cmd, "config") == 0)
        return cmd_config(argc, argv);
    else if (strcmp(cmd, "add") == 0)
        return cmd_add(argc, argv);
    else if (strcmp(cmd, "unstage") == 0)
        return cmd_unstage(argc, argv);
    else if (strcmp(cmd, "commit") == 0)
        return cmd_commit(argc, argv);
    else if (strcmp(cmd, "log") == 0)
        return cmd_log();
    else if (strcmp(cmd, "status") == 0)
        return cmd_status();
    else if (strcmp(cmd, "checkout") == 0)
        return cmd_checkout(argc, argv);
    else if (strcmp(cmd, "branch") == 0)
        return cmd_branch(argc, argv);
    else
    {
        printf("mgit: unknown command '%s'\n", cmd);
        return 0;
    }
}