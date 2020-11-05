#include "operations.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define TRUE 1
#define FALSE 0

/* Given a path, fills pointers with strings for the parent path and child
 * file name
 * Input:
 *  - path: the path to split. ATENTION: the function may alter this parameter
 *  - parent: reference to a char*, to store parent path
 *  - child: reference to a char*, to store child file name
 */
void split_parent_child_from_path(char * path, char ** parent, char ** child) {

	int n_slashes = 0, last_slash_location = 0;
	int len = strlen(path);

	// deal with trailing slash ( a/x vs a/x/ )
	if (path[len-1] == '/') {
		path[len-1] = '\0';
	}

	for (int i=0; i < len; ++i) {
		if (path[i] == '/' && path[i+1] != '\0') {
			last_slash_location = i;
			n_slashes++;
		}
	}

	if (n_slashes == 0) { // root directory
		*parent = "";
		*child = path;
		return;
	}

	path[last_slash_location] = '\0';
	*parent = path;
	*child = path + last_slash_location + 1;

}


/*
 * Initializes tecnicofs and creates root node.
 */
void init_fs() {
	inode_table_init();

	/* create root inode */
	int root = inode_create(T_DIRECTORY);

	if (root != FS_ROOT) {
		fprintf(stderr, "failed to create node for tecnicofs root\n");
		exit(EXIT_FAILURE);
	}
}


/*
 * Destroy tecnicofs and inode table.
 */
void destroy_fs() {
	inode_table_destroy();
}


/*
 * Checks if content of directory is not empty.
 * Input:
 *  - entries: entries of directory
 * Returns: SUCCESS or FAIL
 */

int is_dir_empty(DirEntry *dirEntries) {
	if (dirEntries == NULL) {
		return FAIL;
	}
	for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
		if (dirEntries[i].inumber != FREE_INODE) {
			return FAIL;
		}
	}
	return SUCCESS;
}


/*
 * Looks for node in directory entry from name.
 * Input:
 *  - name: path of node
 *  - entries: entries of directory
 * Returns:
 *  - inumber: found node's inumber
 *  - FAIL: if not found
 */
int lookup_sub_node(char *name, DirEntry *entries) {
	if (entries == NULL) {
		return FAIL;
	}
	for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
        if (entries[i].inumber != FREE_INODE && strcmp(entries[i].name, name) == 0) {
            return entries[i].inumber;
        }
    }
	return FAIL;
}


/*
 * Creates a new node given a path.
 * Input:
 *  - name: path of node
 *  - nodeType: type of node
 * Returns: SUCCESS or FAIL
 */
int create(char *name, type nodeType, lockArray *threadLocks){

	int parent_inumber, child_inumber;
	char *parent_name, *child_name, *buf, *parent_copy, *buf2, name_copy[MAX_FILE_NAME];
	/* use for copy */
	type pType;
	union Data pdata;

	strcpy(name_copy, name);
	split_parent_child_from_path(name_copy, &parent_name, &child_name);
	/* use for copy */
	parent_copy = malloc(sizeof(char) * strlen(parent_name));

	if(*parent_name == FS_ROOT){ // if parent directory corresponds to the root
		parent_inumber = FS_ROOT;
	} else {
		strcpy(parent_copy, parent_name); // copies parent path
		split_parent_child_from_path(parent_copy, &buf, &buf2); // gets parent dir name (buf2)
		parent_inumber = lock_path(parent_name, buf2, threadLocks); // locks the path up to the parent dir, while returning its parent inumber
	}
	free(parent_copy);
	
	if (parent_inumber == FAIL) {
		printf("failed to create %s, invalid parent dir %s\n",
		        name, parent_name);
		unlock(threadLocks);
		return FAIL;
	}
	
	lock(parent_inumber, threadLocks, WR); //locks with wr
	inode_get(parent_inumber, &pType, &pdata);
	
	if(pType != T_DIRECTORY) {
		printf("failed to create %s, parent %s is not a dir\n",
		        name, parent_name);
		unlock(threadLocks);
		return FAIL;
	}
	
	if (lookup_sub_node(child_name, pdata.dirEntries) != FAIL) {
		printf("failed to create %s, already exists in dir %s\n",
		       child_name, parent_name);
		unlock(threadLocks);
		return FAIL;
	}
	
	/* create node and add entry to folder that contains new node */
	child_inumber = inode_create(nodeType);
	lock(child_inumber, threadLocks, WR); //locks with wr

	if (child_inumber == FAIL) {
		printf("failed to create %s in  %s, couldn't allocate inode\n",
		        child_name, parent_name);
		unlock(threadLocks);
		return FAIL;
	}

	if (dir_add_entry(parent_inumber, child_inumber, child_name) == FAIL) {
		printf("could not add entry %s in dir %s\n",
		       child_name, parent_name);
		unlock(threadLocks);
		return FAIL;
	}

	unlock(threadLocks);

	return SUCCESS;
}


/*
 * Deletes a node given a path.
 * Input:
 *  - name: path of node
 * Returns: SUCCESS or FAIL
 */
int delete(char *name, lockArray *threadLocks){

	int parent_inumber, child_inumber;
	char *parent_name, *child_name, *buf, *parent_copy, *buf2, name_copy[MAX_FILE_NAME];
	/* use for copy */
	type pType, cType;
	union Data pdata, cdata;

	strcpy(name_copy, name);
	split_parent_child_from_path(name_copy, &parent_name, &child_name);
	/* use for copy */
	parent_copy = malloc(sizeof(char) * strlen(parent_name));

	if(*parent_name == FS_ROOT){ // if parent directory corresponds to the root
		parent_inumber = FS_ROOT;
	} else {
		strcpy(parent_copy, parent_name); // copies parent path
		split_parent_child_from_path(parent_copy, &buf, &buf2); // gets parent dir name (buf2)
		parent_inumber = lock_path(parent_name, buf2, threadLocks);  // locks the path up to the parent dir, while returning its parent inumber
	}
	free(parent_copy);

	if (parent_inumber == FAIL) {
		printf("failed to delete %s, invalid parent dir %s\n",
		        child_name, parent_name);
		unlock(threadLocks);
		return FAIL;
	}

	lock(parent_inumber, threadLocks, WR);  //locks with wr
	inode_get(parent_inumber, &pType, &pdata);

	if(pType != T_DIRECTORY) {
		printf("failed to delete %s, parent %s is not a dir\n",
		        child_name, parent_name);
		unlock(threadLocks);
		return FAIL;
	}

	child_inumber = lookup_sub_node(child_name, pdata.dirEntries);

	if (child_inumber == FAIL) {
		printf("could not delete %s, does not exist in dir %s\n",
		       name, parent_name);
		unlock(threadLocks);
		return FAIL;
	}

	lock(child_inumber, threadLocks, WR); //locks with wr
	inode_get(child_inumber, &cType, &cdata);

	if (cType == T_DIRECTORY && is_dir_empty(cdata.dirEntries) == FAIL) {
		printf("could not delete %s: is a directory and not empty\n",
		       name);
		unlock(threadLocks);
		return FAIL;
	}

	/* remove entry from folder that contained deleted node */
	if (dir_reset_entry(parent_inumber, child_inumber) == FAIL) {
		printf("failed to delete %s from dir %s\n",
		       child_name, parent_name);
		unlock(threadLocks);
		return FAIL;
	}

	if (inode_delete(child_inumber) == FAIL) {
		printf("could not delete inode number %d from dir %s\n",
		       child_inumber, parent_name);
		unlock(threadLocks);
		return FAIL;
	}

	unlock(threadLocks);
	return SUCCESS;
}


/*
 * Lookup for a given path.
 * Input:
 *  - name: path of node
 * Returns:
 *  inumber: identifier of the i-node, if found
 *     FAIL: otherwise
 */
int lookup(char *name, lockArray *threadLocks) {
	char full_path[MAX_FILE_NAME];
	char delim[] = "/";

	strcpy(full_path, name);

	/* start at root node */
	int current_inumber = FS_ROOT;

	/* use for copy */
	type nType;
	union Data data;

	/* get root inode data */
	lock(current_inumber, threadLocks, RD);
	inode_get(current_inumber, &nType, &data);

	char *saveptr;
	char *path = strtok_r(full_path, delim, &saveptr);

	/* search for all sub nodes */
	while (path != NULL && (current_inumber = lookup_sub_node(path, data.dirEntries)) != FAIL) {
		lock(current_inumber, threadLocks, RD);
		inode_get(current_inumber, &nType, &data);
		path = strtok_r(NULL, delim, &saveptr);
	}
	unlock(threadLocks);
	return current_inumber;
}

/*
 * Move directory or file
 * Input:
 *  - startDir: original path to the directory or file;
 * 	- endDir: new path of the directory or file
 * Returns:
 *  - SUCCESS or FAIL
 */
int move(char *startDir, char *endDir, lockArray *threadLocks){
	int start_inumber, end_inumber;
	type nType;
	union Data data;
	char *startParentName, *startChildName;
	char *endParentName, *endChildName;

	// checking if file exists
	if((start_inumber = lookup(startDir, threadLocks)) < 0){
		fprintf(stderr, "Error: File does not exist.\n");
		return FAIL;
	}

	split_parent_child_from_path(endDir, &endParentName, &endChildName);
	// checking if end directory exists
	if((end_inumber = lookup(endParentName, threadLocks)) < 0){
		fprintf(stderr, "Error: End path does not exist.\n");
		return FAIL;
	}

	inode_get(end_inumber, &nType, &data); // getting the endPath directory contents
	split_parent_child_from_path(startDir, &startParentName, &startChildName);

	// if file to be moved already exists in the end path contents
	if(lookup_sub_node(endChildName, data.dirEntries) >= 0){
		fprintf(stderr, "Error: File already exists in end path.\n");
		return FAIL;
	}

	// remove file from original path
	int parent_inumber = lookup(startParentName, threadLocks);
	if(dir_reset_entry(parent_inumber, start_inumber) == FAIL){
		fprintf(stderr, "Error: Could not remove inode from orinal path.\n");
		return FAIL;
	}

	// add directory/file to new path
	int end_parent_inumber = lookup(endParentName, threadLocks);
	if(dir_add_entry(end_parent_inumber, start_inumber, endChildName) == FAIL){
		fprintf(stderr, "Error: Could not insert inode in new path.\n");
		return FAIL;
	}

	return SUCCESS;
}

/*
 * Prints tecnicofs tree.
 * Input:
 *  - fp: pointer to output file
 */
void print_tecnicofs_tree(char *fileName){
	FILE *outputFile = fopen(fileName, "w");
	inode_print_tree(outputFile, FS_ROOT, "");
	fclose(outputFile);
}

/*
 * Locks the inode corresponding to inumber
 * Input:
 *  - inumber: integer corresponding to the inumber
 *  - threadLocks: the array of locks of the thread
 *  - mode: arbitray integer corresponding to lock mode (RD or WR)  
 */
void lock(int inumber, lockArray *threadLocks, int mode){
	inode_lock(inumber, mode);
	threadLocks->lock[threadLocks->lockCount] = inumber;
	threadLocks->lockCount++;
}

/*
 * Unlocks an array of locks
 * Input:
 *  - threadLocks: the array of locks of the thread
 */
void unlock(lockArray *threadLocks) {
	for(int i = 0; i < threadLocks->lockCount; i++){
		inode_unlock(threadLocks->lock[i]);
	}
	threadLocks->lockCount = 0;
}

/*
 * Iterates through a given path while locking the nodes
 * Input:
 *  - name: original path
 *  - threadLocks: the array of locks of the thread
 *  - parent: name of the folder where memory is going to be written
 */
int lock_path(char *name, char *parent, lockArray *threadLocks){
	char full_path[MAX_FILE_NAME];
	char delim[] = "/";

	strcpy(full_path, name);

	/* start at root node */
	int current_inumber = FS_ROOT;

	/* use for copy */
	type nType;
	union Data data;
	
	/* get root inode data */
	lock(current_inumber, threadLocks, RD); // locks
	inode_get(current_inumber, &nType, &data);

	char *saveptr;
	char *path = strtok_r(full_path, delim, &saveptr);
	/* search for all sub nodes */
	while (path != NULL && (current_inumber = lookup_sub_node(path, data.dirEntries)) != FAIL) {
		// if path has reached parent directory 
		if(strcmp(path, parent) == 0){
			return current_inumber; //returns its inumber
		}
		lock(current_inumber, threadLocks, RD);
		inode_get(current_inumber, &nType, &data);
		path = strtok_r(NULL, delim, &saveptr);
	}
	return FAIL;
}