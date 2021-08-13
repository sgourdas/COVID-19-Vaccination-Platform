#include "functions.h"


/*
	Y is for vaccinated in date range
	N is for not vaccinated
	V is for vaccinated
*/

popList popListCreate(char *country, char vaccinated) {	// List create
	// Allocate space and assign values
	popList l = (popList) malloc(sizeof(popListNode));
	l->country = country;
	// Assign correct values based on parameters
	if(vaccinated == 'Y') {
		
		l->vaccinated = 0;
		l->notVaccinated = 0;
		l->vaccinatedRange = 1;
	
	} else if(vaccinated == 'N'){

		l->vaccinated = 0;
		l->notVaccinated = 1;
		l->vaccinatedRange = 0;

	} else {

		l->vaccinated = 1;
		l->notVaccinated = 0;
		l->vaccinatedRange = 0;

	}

	l->link = NULL;

	return l;

}

popList popListInsert(popList list, char *country, char vaccinated) {	// List insertion

	while(1) {	// Parse all nodes

		if(!strcmp(list->country, country)) {	// On correct country
			// Increment the correct counter and return
			if(vaccinated == 'Y')
				list->vaccinatedRange++;
			else if(vaccinated == 'N')
				list->notVaccinated++;
			else
				list->vaccinated++;

			return list;

		}
	
		if(list->link != NULL)
			list = list->link;
		else
			break;

	}
	// Create new node and link
	list->link = popListCreate(country, vaccinated);

	return list->link;

}

// popList popListSearch(popList list, char *pop) {	// List search -- 0 if exists, 1 if pop not exists, < 0 if id doesnt exist

// 	while(1) {

// 		if(!strcmp(list->pop, pop))
// 			return list;
	
// 		if(list->link != NULL)
// 			list = list->link;
// 		else
// 			break;

// 	}

// 	return NULL;

// }

void popListPrint(popList list) {	// List printing for debugging

	while(list != NULL) { // Parse all nodes

		printf("%s %d %.2f%%\n", list->country, list->vaccinated, (double) list->vaccinatedRange / (list->vaccinated + list->notVaccinated) * 100);

		list = list->link;
	
	}

	printf("\n");

}

void popListDestroy(popList list) {	// Free space of a List 
	
	if(list == NULL)
		return;
	// Recursively free extra nodes
	popListDestroy(list->link);
	// Free current node
	free(list);
	// Do not free pop

}