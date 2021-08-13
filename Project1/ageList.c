#include "functions.h"

/*
	Y is for vaccinated in date range
	N is for not vaccinated
	V is for vaccinated
*/

ageList ageListCreate(char *country, int age, char vaccinated) {	// List create
	// Allocate space and assign values
	ageList l = (ageList) malloc(sizeof(ageListNode));
	// Init all to 0
	l->country = country;
	l->vaccinatedA = 0;
	l->vaccinatedB = 0;
	l->vaccinatedC = 0;
	l->vaccinatedD = 0;
	l->nonVaccinatedA = 0;
	l->nonVaccinatedB = 0;
	l->nonVaccinatedC = 0;
	l->nonVaccinatedD = 0;
	l->vaccinatedRangeA = 0;
	l->vaccinatedRangeB = 0;
	l->vaccinatedRangeC = 0;
	l->vaccinatedRangeD = 0;
	l->link = NULL;
	// Identify input and increment accordingly
	if(vaccinated == 'Y') {

		if(age < 20)
			l->vaccinatedRangeA = 1;
		else if(age < 40)
			l->vaccinatedRangeB = 1;
		else if(age < 60)
			l->vaccinatedRangeC = 1;
		else
			l->vaccinatedRangeD = 1;

	} else if(vaccinated == 'N')  {

		if(age < 20)
			l->nonVaccinatedA = 1;
		else if(age < 40)
			l->nonVaccinatedB = 1;
		else if(age < 60)
			l->nonVaccinatedC = 1;
		else
			l->nonVaccinatedD = 1;

	} else {

		if(age < 20)
			l->nonVaccinatedA = 1;
		else if(age < 40)
			l->nonVaccinatedB = 1;
		else if(age < 60)
			l->nonVaccinatedC = 1;
		else
			l->nonVaccinatedD = 1;

	}

	return l;

}

ageList ageListInsert(ageList list, char *country, int age, char vaccinated) {	// List insertion

	while(1) {

		if(!strcmp(list->country, country)) {	// On correct country
			// Change values based on aprameters
			if(vaccinated == 'Y') {

				if(age < 20)
					list->vaccinatedRangeA++;
				else if(age < 40)
					list->vaccinatedRangeB++;
				else if(age < 60)
					list->vaccinatedRangeC++;
				else
					list->vaccinatedRangeD++;

			} else if(vaccinated == 'N') {

				if(age < 20)
					list->nonVaccinatedA++;
				else if(age < 40)
					list->nonVaccinatedB++;
				else if(age < 60)
					list->nonVaccinatedC++;
				else
					list->nonVaccinatedD++;

			} else {

				if(age < 20)
					list->vaccinatedA++;
				else if(age < 40)
					list->vaccinatedB++;
				else if(age < 60)
					list->vaccinatedC++;
				else
					list->vaccinatedD++;
		
			}

			return list;

		}
	
		if(list->link != NULL)
			list = list->link;
		else
			break;

	}
	// Create new node and link
	list->link = ageListCreate(country, age, vaccinated);

	return list->link;

}

// ageList ageListSearch(ageList list, char *country) {

// 	while(1) {

// 		if(!strcmp(list->country, country))
// 			return list;
	
// 		if(list->link != NULL)
// 			list = list->link;
// 		else
// 			break;

// 	}

// 	return NULL;

// }

void ageListPrint(ageList list) {	// List printing for debugging

	while(list != NULL) { // Parse all nodes

		printf("%s\n", list->country);
		if(list->vaccinatedA + list->nonVaccinatedA != 0 )
			printf("0-20 %d %.2f%%\n", list->vaccinatedA, (double) list->vaccinatedA / (list->vaccinatedA + list->nonVaccinatedA) * 100);
		else
			printf("0-20 %d 0.00%%\n", list->vaccinatedA);
		if(list->vaccinatedB + list->nonVaccinatedB != 0 )
			printf("20-40 %d %.2f%%\n", list->vaccinatedB, (double) list->vaccinatedB / (list->vaccinatedB + list->nonVaccinatedB) * 100);
		else
			printf("20-40 %d 0.00%%\n", list->vaccinatedB);
		if(list->vaccinatedC + list->nonVaccinatedC != 0 )
			printf("40-60 %d %.2f%%\n", list->vaccinatedC, (double) list->vaccinatedC / (list->vaccinatedC + list->nonVaccinatedC) * 100);
		else
			printf("40-60 %d 0.00%%\n", list->vaccinatedC);
		if(list->vaccinatedD + list->nonVaccinatedD != 0 )
			printf("60+ %d %.2f%%\n", list->vaccinatedD, (double) list->vaccinatedD / (list->vaccinatedD + list->nonVaccinatedD) * 100);
		else
			printf("60+ %d 0.00%%\n", list->vaccinatedD);

		list = list->link;

		printf("\n");
	
	}

	printf("\n");

}

void ageListDestroy(ageList list) {	// Free space of a List 

	if(list == NULL)
		return;
	// Recursively free extra nodes
	if(list->link != NULL)
		ageListDestroy(list->link);
	// Free current node
	free(list);
	// Do not free country because it is from citizen struct
}