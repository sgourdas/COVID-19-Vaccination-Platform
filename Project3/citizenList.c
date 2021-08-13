#include "functions.h"


citizenList citizenListCreate(citizenRecord citizen) {	// List create
	// Allocate space and assign values
	citizenList l = (citizenList) malloc(sizeof(citizenListNode));

	l->citizen = citizen;
	l->link = NULL;

	return l;

}

citizenList citizenListInsert(citizenList list, citizenRecord citizen) {	// List insertion
	// Create new node
	citizenList newNode = citizenListCreate(citizen);
	// Link
	newNode->link = list;

	return newNode;

}

void citizenListDestroy(citizenList list) {	// Free space of a List 
	// Recursively free extra nodes
	if(list->link != NULL)
		citizenListDestroy(list->link);
	// Free current node
	citizenDestroy(list->citizen);
	free(list);

}