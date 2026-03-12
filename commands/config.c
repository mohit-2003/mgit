#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/mgit.h"

#define CONFIG_FILE ".mgitconfig"

int cmd_config(int argc, char *argv[])
{
    // Show error if no subcommand is provided
    if (argc < 4)
    {
        printf("Usage:\n");
        printf("mgit config --global --name \"Your Name\"\n");
        printf("mgit config --global --email \"your@email.com\"\n");
        printf("mgit config --global --list\n");
        return 1;
    }

    if (strcmp(argv[2], "--global") != 0)
    {
        printf("Only --global config supported\n");
        return 1;
    }

    // Get home directory -> C:/Users/Username
    char *home = getenv("HOME");
    if (home == NULL)
        home = getenv("USERPROFILE");

    if (home == NULL)
    {
        printf("Cannot determine home directory\n");
        return 1;
    }

    // Construct path to config file -> C:/Users/Username/.mgitconfig
    char path[512];
    sprintf(path, "%s/%s", home, CONFIG_FILE);

    char name[256] = "";
    char email[256] = "";

    // Read existing config if it exists
    FILE *file = fopen(path, "r");

    if (file != NULL)
    {
        char line[256];
        // Parse config file for existing name and email
        while (fgets(line, sizeof(line), file))
        {
            if (strncmp(line, "name =", 6) == 0)
                sscanf(line, "name = %[^\n]", name);

            if (strncmp(line, "email =", 7) == 0)
                sscanf(line, "email = %[^\n]", email);
        }

        fclose(file);
    }

    // Handle --list option to display current config
    if (strcmp(argv[3], "--list") == 0)
    {
        if (strlen(name) == 0 && strlen(email) == 0)
        {
            printf("No config set.\n");
            return 0;
        }

        printf("name = %s\n", name);
        printf("email = %s\n", email);
        return 0;
    }

    // For --name and --email options, ensure value is provided
    if (argc < 5)
    {
        printf("Missing value for config option\n");
        return 1;
    }

    // Update config based on command line arguments
    if (strcmp(argv[3], "--name") == 0)
        strcpy(name, argv[4]);
    else if (strcmp(argv[3], "--email") == 0)
        strcpy(email, argv[4]);
    else
    {
        printf("Unknown config option\n");
        return 1;
    }

    // Write updated config back to file
    file = fopen(path, "w");

    if (file == NULL)
    {
        printf("Error writing config file\n");
        return 1;
    }

    fprintf(file, "[user]\n");
    fprintf(file, "name = %s\n", name);
    fprintf(file, "email = %s\n", email);

    fclose(file);

    printf("Config updated successfully\n");

    return 0;
}