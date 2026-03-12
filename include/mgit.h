#ifndef MGIT_H
#define MGIT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* command functions */

int cmd_init();
int cmd_config(int argc, char *argv[]);
int cmd_add(int argc, char *argv[]);
int cmd_commit(int argc, char *argv[]);
int cmd_log();
int cmd_status();
int cmd_checkout(int argc, char *argv[]);

#endif