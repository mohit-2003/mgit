#include "../include/mgit.h"

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("mgit: command required\n");
        return 1;
    }

    // mgit init command
    if (strcmp(argv[1], "init") == 0)
    {
        return cmd_init();
    }

    // mgit config command
    else if (strcmp(argv[1], "config") == 0)
    {
        return cmd_config(argc, argv);
    }

    // mgit add command
    else if (strcmp(argv[1], "add") == 0)
    {
        return cmd_add(argc, argv);
    }

    // mgit unstage command
    else if (strcmp(argv[1], "unstage") == 0)
    {
        return cmd_unstage(argc, argv);
    }

    // mgit commit command
    else if (strcmp(argv[1], "commit") == 0)
    {
        return cmd_commit(argc, argv);
    }

    // mgit log command
    else if (strcmp(argv[1], "log") == 0)
    {
        return cmd_log();
    }

    // mgit status command
    else if (strcmp(argv[1], "status") == 0)
    {
        return cmd_status();
    }

    // mgit checkout command
    else if (strcmp(argv[1], "checkout") == 0)
    {
        return cmd_checkout(argc, argv);
    }

    // Handle unknown command
    else
    {
        printf("mgit: unknown command '%s'\n", argv[1]);
        return 1;
    }
}