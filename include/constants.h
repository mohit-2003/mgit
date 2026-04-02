#ifndef CONSTANTS_H
#define CONSTANTS_H

/* Repository paths  */
#define MGIT_DIR ".mgit"
#define OBJECTS_DIR ".mgit/objects"
#define REFS_DIR ".mgit/refs"
#define HEADS_DIR ".mgit/refs/heads"
#define HEAD_FILE ".mgit/HEAD"
#define INDEX_FILE ".mgit/index"
#define TMP_INDEX ".mgit/index_tmp"
#define LAST_INDEX_FILE ".mgit/last_commit_index"
#define IGNORE_FILE ".mgitignore"
#define CONFIG_FILE ".mgitconfig"

/* Buffer sizes  */
#define HASH_SIZE 41     /* 40 hex chars + null terminator   */
#define PATH_BUF 1024    /* general file path buffer          */
#define LINE_BUF 512     /* one line from a file              */
#define CONTENT_BUF 4096 /* commit/tree content               */
#define MAX_FILES 1000   /* max entries in index              */

#endif
