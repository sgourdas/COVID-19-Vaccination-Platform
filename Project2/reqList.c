#include "functions.h"


reqList reqListCreate(char *date, char *virus, char *country, int accepted) {	// List create
	// Allocate space and assign values
	reqList l = (reqList) malloc(sizeof(reqListNode));
	l->date = (char *) malloc(strlen(date) + 1);
	strcpy(l->date, date);
	l->virus = (char *) malloc(strlen(virus) + 1);
	strcpy(l->virus, virus);
	l->country = (char *) malloc(strlen(country) + 1);
	strcpy(l->country, country);
	l->accepted = accepted;
	l->link = NULL;

	return l;

}

reqList reqListInsert(reqList list, char *date, char *virus, char *country, int accepted) {	// List insertion
	// Create new node
	reqList newNode = reqListCreate(date, virus, country, accepted);
	// Link
	newNode->link = list;

	return newNode;

}

void reqListSearch(reqList list, char *date1, char *date2, char *virus, char *country, int *accepted, int *rejected) {	// List search -- NULL if not exists, pointer if exists

	if(list == NULL)
		return;

	while(1) {
		// List date is bigger or equal to date1 and smaller or equal to date2
		if(!strcmp(virus, list->virus))
			if((compareDates(list->date, date1) >= 0) && (compareDates(list->date, date2) <= 0)) {

				if((country == NULL) || !strcmp(country, list->country)) {

					if(list->accepted == 1)
						(*accepted)++;
					else
						(*rejected)++;

				}

			}
	
		if(list->link != NULL)
			list = list->link;
		else
			break;

	}

}

void reqListPrint(reqList list) {	// List printing for debugging

	while(list != NULL) { // Parse all nodes

		printf("%s | %s | %s -> ", list->date, list->virus, list->country);

		list = list->link;
	
	}

	printf("\n");

}

void reqListDestroy(reqList list) {	// Free space of a List 
	
	if(list == NULL)
		return;
	// Recursively free extra nodes
	reqListDestroy(list->link);
	// Free current node
	free(list->date);
	free(list->virus);
	free(list->country);
	free(list);

}