#include "functions.h"


dirList dirListCreate(char *dir) {	// List create
	// Allocate space and assign values
	dirList l = (dirList) malloc(sizeof(dirListNode));
	l->dir = (char *) malloc(strlen(dir) + 1);
	strcpy(l->dir, dir);
	l->link = NULL;

	return l;

}

dirList dirListInsert(dirList list, char *dir) {	// List insertion
	// Create new node
	dirList newNode = dirListCreate(dir);
	// Link
	newNode->link = list;

	return newNode;

}

dirList dirListSearch(dirList list, char *dir) {	// List search -- NULL if not exists, pointer if exists

	while(1) {

		if(!strcmp(list->dir, dir))
			return list;
	
		if(list->link != NULL)
			list = list->link;
		else
			break;

	}

	return NULL;

}

void dirListPrint(dirList list) {	// List printing for debugging

	while(list != NULL) { // Parse all nodes

		printf("%s -> ", list->dir);

		list = list->link;
	
	}

	printf("\n");

}

void dirListDestroy(dirList list) {	// Free space of a List 
	// Recursively free extra nodes
	if(list->link != NULL)
		dirListDestroy(list->link);
	// Free current node
	free(list->dir);
	free(list);

}