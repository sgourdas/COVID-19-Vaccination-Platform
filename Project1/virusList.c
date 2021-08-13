#include "functions.h"


virusList virusListCreate(char *virus, citizenRecord citizen, char *dateVaccinated) {	// List create
	// Allocate space and assign values
	virusList l = (virusList) malloc(sizeof(virusListNode));

	if(dateVaccinated != NULL) {	// If there is a date -- citizen has been vaccinated

		l->vaccinated = skipListCreate(citizen, dateVaccinated);
		l->nonVaccinated = NULL;

	} else {	// Date is NULL -- citizen has not been vaccinated

		l->nonVaccinated = skipListCreate(citizen, NULL);
		l->vaccinated = NULL;

	}

	l->bloom = (char *) malloc(bloomSize);;
	// for(int i = 0 ; i < bloomSize ; i++)
	// 	l->bloom[i] = 0;
	bloomInsert(l->bloom, citizen->citizenID);
	l->virus = (char *) malloc(strlen(virus) + 1);
	strcpy(l->virus, virus);
	l->link = NULL;

	return l;

}

int virusListInsert(virusList list, char *virus, citizenRecord citizen, char *dateVaccinated) {	// List insertion

	while(1) {

		if(!strcmp(list->virus, virus)) {

			bloomInsert(list->bloom, citizen->citizenID);

			if(dateVaccinated != NULL) {	// If there is a date -- citizen has been vaccinated

				if(list->vaccinated != NULL)
					skipListInsert(list->vaccinated, citizen, dateVaccinated);
				else
					list->vaccinated = skipListCreate(citizen, dateVaccinated);

			} else {	// Date is NULL -- citizen has not been vaccinated

				if(list->nonVaccinated != NULL)
					skipListInsert(list->nonVaccinated, citizen, dateVaccinated);
				else
					list->nonVaccinated = skipListCreate(citizen, dateVaccinated);

			}

			return 0;

		}
	
		if(list->link != NULL)
			list = list->link;
		else
			break;

	}
	// Create new node and link
	list->link = virusListCreate(virus, citizen, dateVaccinated);

	return 1;

}

int virusListBloomSearch(virusList list, char *virus, citizenRecord citizen) {	// List search -- 0 if exists, 1 if virus not exists, < 0 if id doesnt exist

	while(1) {

		if(!strcmp(list->virus, virus))
			return bloomExists(list->bloom, citizen->citizenID);
	
		if(list->link != NULL)
			list = list->link;
		else
			break;

	}

	return 1;

}

skipList virusListSkipSearch(virusList list, char *virus, citizenRecord citizen) {	// List search -- 0 if exists, 1 if virus not exists, < 0 if id doesnt exist

	if(citizen == NULL)
		return NULL;

	if(virus == NULL) {	// If we are searching without a virus argument

		while(1) {	// Go through every virus

			skipList res = skipListGet(list->vaccinated, citizen);	// Get result

			if(res)
				printf("%s YES %s\n", list->virus, res->dateVaccinated);
			else
				printf("%s NO\n", list->virus);
		
			if(list->link != NULL)
				list = list->link; // Go next
			else
				return res;

		}

	}

	while(1) {	// Loop until

		if(!strcmp(list->virus, virus))	// You find the virus in search for and return the result
			return skipListGet(list->vaccinated, citizen);
	
		if(list->link != NULL)
			list = list->link;	// Next virus
		else
			break;

	}

	return NULL;

}

popList virusListPopSearch(virusList list, char *country, char *virus, char *date1, char *date2) {

	while(list != NULL) {	// Parse list

		if(!strcmp(list->virus, virus))	// On virus match, return
			return skipListPopStats(list->vaccinated, list->nonVaccinated, country, virus, date1, date2);

		list = list->link;

	}

	return NULL;

}

ageList virusListAgeSearch(virusList list, char *country, char *virus, char *date1, char *date2) {

	while(list != NULL) { // Parse list

		if(!strcmp(list->virus, virus)) // On virus match, return
			return skipListAgeStats(list->vaccinated, list->nonVaccinated, country, virus, date1, date2);

		list = list->link;

	}

	return NULL;

}

skipList virusListVac(virusList list, char *virus, citizenRecord citizen, char *date) {

	while(list != NULL) {	// Parse

		if(!strcmp(list->virus, virus)) {	// On virus match

			bloomInsert(list->bloom, citizen->citizenID); // Insert in bloom filter as vaccinated

			return skipListVac(list->vaccinated, list->nonVaccinated, virus, citizen, date);	// Transfer from unvac list to vac list
			
		}

		list = list->link;

	}

	return NULL;

}

void virusListUnvac(virusList list, char *virus) {

	while(list != NULL) {	// Search for correct virus

		if(!strcmp(list->virus, virus))
			return skipListUnvac(list->nonVaccinated); // Call skiplist to do the job

		list = list->link;

	}

}

void virusListPrint(virusList list) {	// List printing for debugging

	while(list != NULL) { // Parse all nodes

		printf("%s -> ", list->virus);

		list = list->link;
	
	}

	printf("\n");

}

skipList virusListSkipExists(virusList list, char *virus, citizenRecord citizen) {

	skipList temp = NULL;

	while(1) {	// Loop until

		if(!strcmp(list->virus, virus))	{// You find the virus in search for and return the result
			
			temp = skipListGet(list->vaccinated, citizen);

			if(temp != NULL)
				return temp;
			else
				break;
		
		}
	
		if(list->link != NULL)
			list = list->link;	// Next virus
		else
			break;

	}

	while(1) {	// Loop until

		if(!strcmp(list->virus, virus))	// You find the virus in search for and return the result
			return skipListGet(list->nonVaccinated, citizen);
	
		if(list->link != NULL)
			list = list->link;	// Next virus
		else
			break;

	}

	return NULL;

}

void virusListDestroy(virusList list) {	// Free space of a List 
	// Recursively free extra nodes
	if(list->link != NULL)
		virusListDestroy(list->link);

	if(list->vaccinated != NULL)
		skipListDestroy(list->vaccinated);
	if(list->nonVaccinated != NULL)
		skipListDestroy(list->nonVaccinated);
	// Free current node
	free(list->bloom);
	free(list->virus);
	free(list);

}