#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/mgit.h"
#include "../include/constants.h"

int cmd_config(int argc, char *argv[])
{
    if (argc < 4)
    {
        printf("Usage:\n");
        printf("  mgit config --global --name  \"Your Name\"\n");
        printf("  mgit config --global --email \"you@example.com\"\n");
        printf("  mgit config --global --list\n");
        return 1;
    }

    if (strcmp(argv[2], "--global") != 0)
    {
        printf("Only --global config is supported.\n");
        return 1;
    }

    /* Locate home directory (works on Windows and Linux) */
    char *home = getenv("HOME");
    if (!home)
        home = getenv("USERPROFILE");
    if (!home)
    {
        printf("Error: cannot determine home directory.\n");
        return 1;
    }

    char path[PATH_BUF];
    snprintf(path, sizeof(path), "%s/%s", home, CONFIG_FILE);

    /* Read existing name/email if the file already exists */
    char name[256] = "";
    char email[256] = "";

    FILE *f = fopen(path, "r");
    if (f)
    {
        char line[LINE_BUF];
        while (fgets(line, sizeof(line), f))
        {
            if (strncmp(line, "name =", 6) == 0)
                sscanf(line, "name = %[^\n]", name);
            if (strncmp(line, "email =", 7) == 0)
                sscanf(line, "email = %[^\n]", email);
        }
        fclose(f);
    }

    /* --list: just print current values */
    if (strcmp(argv[3], "--list") == 0)
    {
        if (name[0] == '\0' && email[0] == '\0')
            printf("No config set.\n");
        else
        {
            printf("name  = %s\n", name);
            printf("email = %s\n", email);
        }
        return 0;
    }

    /* --name / --email: require a value argument */
    if (argc < 5)
    {
        printf("Error: missing value for config option.\n");
        return 1;
    }

    if (strcmp(argv[3], "--name") == 0)
        strncpy(name, argv[4], sizeof(name) - 1);
    else if (strcmp(argv[3], "--email") == 0)
        strncpy(email, argv[4], sizeof(email) - 1);
    else
    {
        printf("Unknown config option: %s\n", argv[3]);
        return 1;
    }

    /* Write updated config back */
    f = fopen(path, "w");
    if (!f)
    {
        printf("Error: cannot write config file.\n");
        return 1;
    }
    fprintf(f, "[user]\n");
    fprintf(f, "name = %s\n", name);
    fprintf(f, "email = %s\n", email);
    fclose(f);

    printf("Config updated.\n");
    return 0;
}