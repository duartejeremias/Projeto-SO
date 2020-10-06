#ifndef FS_H
#define FS_H
#include "state.h"

void init_fs();
void destroy_fs();
int is_dir_empty(DirEntry *dirEntries);
int create(char *name, type nodeType, FILE *outputFile);
int delete(char *name, FILE *outputFile);
int lookup(char *name);
void print_tecnicofs_tree(FILE *fp);

#endif /* FS_H */
