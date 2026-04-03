# mgit

**mgit** is a lightweight, custom version control system written in C. It replicates the core functionality of standard Git, allowing users to initialize repositories, stage changes, create commits, manage branches, and view repository history.

### Supported Commands

- **`mgit init`**
  Initialises a new empty mgit repository in the current directory. Creates the `.mgit` folder structure and writes the initial HEAD file.

- **`mgit config <name|email> <value>`**
  Sets or lists the global user name and email configuration.

- **`mgit add <path>...`**
  Stages the specified files. Use `.` to stage all changes in the current directory.

- **`mgit unstage <file>`**
  Removes a single file from the index. Use `.` to unstage everything.

- **`mgit commit "<message>"`**
  Records changes to the repository. Builds a tree from the current index and creates a new commit object.

- **`mgit log`**
  Resolves HEAD and walks the commit chain, printing the commit history.

- **`mgit status`**
  Displays three sections: staged changes, unstaged changes, and untracked files.

- **`mgit checkout [-b] <branch_or_commit>`**
  Switches branches, checks out detached HEADs, or creates new branches using the `-b` flag.

- **`mgit branch [-d|-m] <branch_name>`**
  Lists, creates, deletes (`-d`), or renames (`-m`) branches.

- **`mgit help <command>`**
  Displays detailed help information for a specific mgit command.

## Project Structure

The source code is organized into modular components to handle different aspects of the version control system:

- **`src/`**: Contains the main entry point (`main.c`) and test files for the various components.
- **`commands/`**: Houses the individual implementation files for each CLI command (e.g., `add.c`, `commit_cmd.c`, `checkout.c`, `help.c`).
- **`core/`**: Contains the core logic for the staging area (`index.c`) and ignore rules (`ignore.c`).
- **`objects/`**: Implements the core Git object types (`blob.c`, `tree.c`, `commit.c`).
- **`utils/`**: Utility functions, including SHA-1 hashing implementations (`sha1.c`, `utils.c`).
- **`include/`**: C header files corresponding to the modules above (`mgit.h`, `constants.h`, etc.).

## Getting Started

### Prerequisites

- A standard C compiler (e.g., GCC or Clang).

### Building `mgit`

To compile the project, compile all the `.c` files in the source directories and link them into an executable. For example, using GCC:

```bash
gcc src/main.c commands/*.c core/*.c objects/*.c utils/*.c -Iinclude -o mgit
```
