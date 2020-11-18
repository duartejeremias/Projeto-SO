int move(char *startDir, char *endDir, lockArray *threadLocks){
	int start_parent_inumber, start_child_inumber, end_parent_inumber;
	type nType;
	union Data data;
	char copy[MAX_FILE_NAME];
	char *startParentName, *startChildName;
	char *endParentName, *endChildName;
	char *buf, *buf2;

	// checking if file exists
	split_parent_child_from_path(startDir, &startParentName, &startChildName);
	if(*startParentName == FS_ROOT) {
		start_parent_inumber = FS_ROOT;
	} else {
		strcpy(copy, startParentName);
		split_parent_child_from_path(copy, &buf, &buf2);
		start_parent_inumber = lock_path(startParentName, buf2, threadLocks);
	}
	
	if(start_parent_inumber == FAIL) {
		fprintf(stderr, "Error: starting directory does not exist.\n");
		unlock(threadLocks);
		return FAIL;
	}
	
	lock(start_parent_inumber, threadLocks, WR);

	inode_get(start_parent_inumber, &nType, &data);
	start_child_inumber = lookup_sub_node(startChildName, data.dirEntries);

	if(start_child_inumber == FAIL){
		fprintf(stderr, "Error: File doesnt exist in start path.\n");
		unlock(threadLocks);
		return FAIL;
	}
	unlock(threadLocks);
	lock(start_child_inumber, threadLocks, WR);

	if(dir_reset_entry(start_parent_inumber, start_child_inumber) == FAIL){
		fprintf(stderr, "Error: Could not remove inode from orinal path.\n");
		unlock(threadLocks);
		return FAIL;
	}

	// checking if end directory exists
	split_parent_child_from_path(endDir, &endParentName, &endChildName);
	if(*endParentName == FS_ROOT) {
		end_parent_inumber = FS_ROOT;
	} else {
		strcpy(copy, endParentName);
		split_parent_child_from_path(copy, &buf, &buf2);
		end_parent_inumber = lock_path(endParentName, buf2, threadLocks);
	}
	// checking if end directory exists
	if(end_parent_inumber == FAIL){
		fprintf(stderr, "Error: End path does not exist.\n");
		if(dir_add_entry(start_parent_inumber, start_child_inumber, startChildName) == FAIL){
			fprintf(stderr, "Error: Could not insert inode in new path.\n");
			unlock(threadLocks);
			return FAIL;
		}
		unlock(threadLocks);
		return FAIL;
	}

	lock(end_parent_inumber, threadLocks, WR);

	inode_get(end_parent_inumber, &nType, &data); // getting the endPath directory contents

	// if file to be moved already exists in the end path contents
	if(lookup_sub_node(endChildName, data.dirEntries) >= 0){
		fprintf(stderr, "Error: File already exists in end path.\n");
		if(dir_add_entry(start_parent_inumber, start_child_inumber, startChildName) == FAIL){
			fprintf(stderr, "Error: Could not insert inode in new path.\n");
			unlock(threadLocks);
			return FAIL;
		}
		unlock(threadLocks);
		return FAIL;
	}

	// add directory/file to new path
	if(dir_add_entry(end_parent_inumber, start_child_inumber, endChildName) == FAIL){
		fprintf(stderr, "Error: Could not insert inode in new path.\n");
		unlock(threadLocks);
		return FAIL;
	}

	unlock(threadLocks);

	return SUCCESS;
}