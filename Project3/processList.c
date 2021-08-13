#include "functions.h"


processList processListCreate(char *country, int processID) {	// List create
	// Allocate space and assign values
	processList l = (processList) malloc(sizeof(processListNode));
	l->country = (char *) malloc(strlen(country) + 1);
	strcpy(l->country, country);
	l->processID = processID;
	l->link = NULL;

	return l;

}

processList processListInsert(processList list, char *country, int processID) {	// List insertion
	// Create new node
	processList newNode = processListCreate(country, processID);
	// Link
	newNode->link = list;

	return newNode;

}

int processListSearch(processList list, char *country) {	// List search -- -1 if not exists, process id if exists

	while(1) {

		if(!strcmp(list->country, country))
			return list->processID;
	
		if(list->link != NULL)
			list = list->link;
		else
			break;

	}

	return -1;

}

char **processListGet(processList list, int processID, int *arraySize) {	// List get -- returns an array with the country paths of the process with given id / NULL if none
																			// Only extra allocation that is made is for array indexes
	*arraySize = 0;
	// Find how many paths does this process have stored
	for(processList tmplist = list ; tmplist != NULL ; tmplist = tmplist->link)
		if(tmplist->processID == processID)
			(*arraySize)++;

	if(*arraySize == 0)
		return NULL;

	char **array = (char **) malloc((*arraySize) * sizeof(char *));

	int i = 0;
	for(processList tmplist = list ; i < (*arraySize) ; tmplist = tmplist->link)
		if(tmplist->processID == processID)
			array[i++] = tmplist->country;

	return array;

}

void processListPrint(processList list) {	// List printing for debugging

	while(list != NULL) { // Parse all nodes

		printf("%s[%d] -> ", list->country, list->processID);

		list = list->link;
	
	}

	printf("\n");

}

void processListDestroy(processList list) {	// Free space of a List 
	// Recursively free extra nodes
	if(list->link != NULL)
		processListDestroy(list->link);
	// Free current node
	free(list->country);
	free(list);

}