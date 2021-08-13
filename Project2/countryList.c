#include "functions.h"


countryList countryListCreate(char *country) {	// List create
	// Allocate space and assign values
	countryList l = (countryList) malloc(sizeof(countryListNode));
	l->population = 1;
	l->country = (char *) malloc(strlen(country) + 1);
	strcpy(l->country, country);
	l->link = NULL;

	return l;

}

countryList countryListInsert(countryList list, char *country) {	// List insertion

	while(1) {

		if(!strcmp(list->country, country)) {

			list->population++;

			return list;

		}
	
		if(list->link != NULL)
			list = list->link;
		else
			break;

	}
	// Create new node and link
	list->link = countryListCreate(country);

	return list->link;

}

countryList countryListSearch(countryList list, char *country) {	// List search -- 0 if exists, 1 if country not exists, < 0 if id doesnt exist

	while(1) {

		if(!strcmp(list->country, country))
			return list;
	
		if(list->link != NULL)
			list = list->link;
		else
			break;

	}

	return NULL;

}

void countryListPrint(countryList list) {	// List printing for debugging

	while(list != NULL) { // Parse all nodes

		printf("%s[%d] -> ", list->country, list->population);

		list = list->link;
	
	}

	printf("\n");

}

void countryListDestroy(countryList list) {	// Free space of a List 
	// Recursively free extra nodes
	if(list->link != NULL)
		countryListDestroy(list->link);
	// Free current node
	free(list->country);
	free(list);

}