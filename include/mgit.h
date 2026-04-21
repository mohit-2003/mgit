#ifndef MGIT_H
#define MGIT_H

/**
 * @brief initialises a new empty mgit repository in the current directory
 *
 * creates the .mgit folder structure and writes the initial HEAD file
 * pointing to the main branch.
 *
 * @return 0 on success, 1 if already initialised or HEAD file creation failed
 *
 * @note mgit_mkdir abstracts over Windows (_mkdir) and Linux (mkdir)
 */
int cmd_init(void);

/**
 * @brief entry point for the "mgit config" command
 *
 * supports setting and listing the global user name and email.
 * reads the existing config first so it doesn't wipe the other field
 * when only one is being updated.
 *
 * @param argc argument count from the command dispatcher
 * @return 0 on success, 1 on bad usage or file errors
 *
 * @note only --global scope is supported, no local/system config
 * @note config file lives at $HOME/CONFIG_FILE (USERPROFILE on windows)
 */
int cmd_config(int argc, char *argv[]);

/**
 * @brief entry point for the "mgit add" command
 *
 * loops over all paths passed on the command line and stages them.
 * "." is a special case that adds everything in the working directory.
 *
 * @param argc argument count from the command dispatcher
 * @param argv argument vector, argv[2] onwards are the paths to add
 * @return 0 on success, 1 if no paths were given
 */
int cmd_add(int argc, char *argv[]);

/**
 * @brief entry point for the "mgit unstage" command
 *
 * removes a single file from the index, or clears everything if "." is passed.
 * rewrites the index through a temp file, skipping the target entry.
 *
 * @param argc argument count from the command dispatcher
 * @param argv argument vector — argv[2] is the file to unstage or "."
 * @return 0 on success, 1 on bad usage or file errors
 */
int cmd_unstage(int argc, char *argv[]);

/**
 * @brief entry point for the "mgit commit" command
 *
 * parses the commit message, builds a tree from the current index,
 * creates the commit object, then clears the index and rebuilds the
 * last commit snapshot.
 *
 * @param argc argument count from the command dispatcher
 * @param argv argument vector — expects either "-m <msg>" or just "<msg>" at argv[2]
 * @return 0 on success or wrong usage, 1 on internal error (tree/commit creation failed)
 *
 * @note returns 0 (not 1) on wrong usage — bad args aren't treated as a fatal error
 */
int cmd_commit(int argc, char *argv[]);

/**
 * @brief entry point for the "mgit log" command
 *
 * resolves HEAD to a commit hash and walks the chain printing each one.
 *
 * @return 0 always — no commits yet is informational, not an error
 */
int cmd_log(void);

/**
 * @brief entry point for the "mgit status" command
 *
 * prints three sections: staged changes, unstaged, and untracked files.
 * compares index vs last commit for staged, disk vs last commit for unstaged,
 * and working tree vs both for untracked..
 *
 * @return 0 on success, 1 on memory allocation failure
 */
int cmd_status(void);

/**
 * @brief entry point for the "mgit checkout" command
 *
 * handles branch switching, detached HEAD checkout, and new branch creation
 * with -b. aborts if there are uncommitted local changes that would be overwritten.
 *
 * @param argc argument count from the command dispatcher
 * @param argv argument vector — argv[2] is [-b] and argv[3] is the branch/commit target
 * @return 0 on success, 1 on bad usage, conflicts, or missing refs
 *
 * @note aborts if any staged file differs on disk — matches git's "would be overwritten" check
 * @note after switching, LAST_INDEX_FILE is synced from the new index so status stays correct
 */
int cmd_checkout(int argc, char *argv[]);

/**
 * @brief entry point for the "mgit branch" command
 *
 * dispatches to list, create, delete, or rename based on argc and flags.
 *
 * @param argc argument count from the command dispatcher
 * @param argv argument vector — argv[2] onward are flags and branch names
 * @return 0 on success, 1 on bad usage or operation failure
 */
int cmd_branch(int argc, char *argv[]);

/**
 * @brief entry point for the "mgit help" command
 *
 * displays detailed help information and usage instructions
 * for a specific mgit command.
 *
 * @param argc argument count from the command dispatcher
 * @param argv argument vector — argv[2] is the command name to explain
 * @return 0 on success, 1 on missing or unknown command
 */
int cmd_help(int argc, char *argv[]);

/**
 * @brief entry point for the "mgit restore" command
 *
 * recovers a deleted or modified file from the last commit back to disk.
 * also restores the file in the index so status shows clean.
 *
 * @param argc argument count
 * @param argv argv[2] is the file to restore
 * @return 0 on success, 1 on failure
 */
int cmd_restore(int argc, char *argv[]);

#endif
