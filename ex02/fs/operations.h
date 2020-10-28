#ifndef FS_H
#define FS_H
#include "state.h"

typedef struct lockArray {
	int lock[INODE_TABLE_SIZE];
    int lockCount;
} lockArray;

void init_fs();
void destroy_fs();
int is_dir_empty(DirEntry *dirEntries);
int create(char *name, type nodeType, lockArray *threadLocks);
int delete(char *name, lockArray *threadLocks);
int lookup(char *name, lockArray *threadLocks);
int move(char *startDir, char *endDir, lockArray *threadLocks);
void print_tecnicofs_tree(char *fileName);
void lock(int inumber, lockArray *threadLocks, int mode);
void unlock(lockArray *threadLocks);
int wrlock_path(char *name, char *parent, lockArray *threadLocks);

#endif /* FS_H */
