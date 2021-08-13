#include "functions.h"


bloomList bloomListCreate(char *virus, char *bloom) {	// List create
	// Allocate space and assign values
	bloomList l = (bloomList) malloc(sizeof(bloomListNode));

	l->virus = (char *) malloc(strlen(virus) + 1);
	strcpy(l->virus, virus);
	l->bloom = bloom;
	l->link = NULL;

	return l;

}

bloomList bloomListInsert(bloomList list, char *virus, char *bloom) {	// List insertion

	bloomList tmp = list;

	while(tmp != NULL) {

		if(!strcmp(tmp->virus, virus)) {

			free(tmp->bloom);

			tmp->bloom = bloom;
			// for(int bloomIndex = 0 ; bloomIndex < bloomSize ; bloomIndex++)
			// 	tmp->bloom[bloomIndex] |= bloom[bloomIndex];

			return tmp;

		}

		tmp = tmp->link;

	}
	// Create new node
	bloomList newNode = bloomListCreate(virus, bloom);
	// Link
	newNode->link = list;

	return newNode;

}

int bloomListSearch(bloomList list, char *virus, char *id) {	// Returns 0 if the id seems to exist in the filter, else if not

	while(1) {

		if(!strcmp(list->virus, virus))
			return bloomExists(list->bloom, id);
	
		if(list->link != NULL)
			list = list->link;
		else
			break;

	}

	return -1;

}

void bloomListPrint(bloomList list) {	// List printing for debugging

	while(list != NULL) { // Parse all nodes

		printf("%s -> ", list->virus);

		list = list->link;
	
	}

	printf("\n");

}

void bloomListDestroy(bloomList list) {	// Free space of a List 
	// Recursively free extra nodes
	if(list->link != NULL)
		bloomListDestroy(list->link);
	// Free current node
	free(list->virus);
	free(list->bloom);
	free(list);

}