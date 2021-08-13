#include "functions.h"

int maxSkipHeight = 22; // log(earthPopulation) -- log(7674000000)
/*
	Due to global variables in C being able to only be initialized with compile time
	constant expressions I have run (and assigned) log(7674000000) beforehand, 
							which equals to 22.
*/

citizenRecord dummyCitizen = NULL;

skipList skipListCreate(citizenRecord citizen, char *dateVaccinated) {

	if(dummyCitizen == NULL) {
		
		dummyCitizen = (citizenRecord) malloc(sizeof(citizenRc));

		dummyCitizen->citizenID = (char *) malloc(3);
		strcpy(dummyCitizen->citizenID, "-1");
		dummyCitizen->firstName = NULL;
		dummyCitizen->lastName = NULL;
		dummyCitizen->country = NULL;

	}
	// Create dummy nodes
	skipList initNode = (skipList) malloc(sizeof(skipListNode));	// Initial dummy node -- start of skip list
	skipList tmpDummy = initNode, dummyNode;

	initNode->citizen = dummyCitizen;
	initNode->dateVaccinated = NULL;
	initNode->bot = NULL;

	for(int i = 1 ; i < maxSkipHeight ; i++) {	// Making of the rest of the dummy nodes -- level1 - max height
		// Allocation and data member assignment
		dummyNode = (skipList) malloc(sizeof(skipListNode));

		dummyNode->citizen = dummyCitizen;
		dummyNode->dateVaccinated = NULL;
		dummyNode->bot = tmpDummy;
		dummyNode->link = NULL;

		tmpDummy->top = dummyNode;
		// Store new tmp node for next run
		tmpDummy = dummyNode;

	}

	tmpDummy->top = NULL;
	// Initial information / normal node creation
	skipList node = (skipList) malloc(sizeof(skipListNode));
	skipList tmp = node, topNode = NULL;

	node->citizen = citizen;
	node->link = NULL;
	node->bot = NULL;

	if(dateVaccinated != NULL) {

		node->dateVaccinated = (char *) malloc(strlen(dateVaccinated) + 1);
		// printf("%d!!!~\n", (int) strlen(dateVaccinated));
		strcpy(node->dateVaccinated, dateVaccinated);
	
	} else {
		
		node->dateVaccinated = NULL;
	
	}
	// Build new node height
	for(int i = 1 ; i < maxSkipHeight ; i++) {

		if(rand() % 2) {
			// Allocation and data member assignment
			topNode = (skipList) malloc(sizeof(skipListNode));

			topNode->citizen = citizen;
			topNode->dateVaccinated = node->dateVaccinated;
			topNode->link = NULL;
			topNode->bot = tmp;

			tmp->top = topNode;
			// Store new tmp node for next run
			tmp = topNode;

		} else {

			break;

		}

	}
	
	tmp->top = NULL;
	// Horizontal node linking
	tmpDummy = initNode;
	tmp = node;

	for(int j = 0 ; j < maxSkipHeight ; j++) {

		if(tmp == NULL)	// Normal node height end -- stop linking
			break;

		tmpDummy->link = tmp;

		tmpDummy = tmpDummy->top;
		tmp = tmp->top;

	}

	return initNode;

}

skipList skipListInsert(skipList list, citizenRecord citizen, char *dateVaccinated) {	// skip list insertion
	// Allocation and assignment of data members
	skipList newNode = (skipList) malloc(sizeof(skipListNode));
	skipList tmp = newNode, topNode = NULL;

	newNode->citizen = citizen;
	newNode->link = NULL;
	newNode->bot = NULL;

	if(dateVaccinated != NULL) {

		newNode->dateVaccinated = (char *) malloc(strlen(dateVaccinated) + 1);
		strcpy(newNode->dateVaccinated, dateVaccinated);
	
	} else {
		
		newNode->dateVaccinated = NULL;
	
	}
	// Build new node height
	int i;
	for(i = 1 ; i < maxSkipHeight ; i++) {

		if(rand() % 2) {
			// Allocation and data member assignment 
			topNode = (skipList) malloc(sizeof(skipListNode));

			topNode->citizen = citizen;
			topNode->dateVaccinated = newNode->dateVaccinated;
			topNode->link = NULL;
			topNode->bot = tmp;

			tmp->top = topNode;
			// Store new tmp node for next run
			tmp = topNode;

		} else {

			break;

		}

	}

	tmp->top = NULL;
	// Horizontal linking
	skipList *prevList = (skipList *) malloc(maxSkipHeight * sizeof(skipList)); // Store left nodes relative to the new node -- layer0 -> prevList[maxheight - 1]
	skipList tmpDummy = list;
	// Use dummy nodes for init of prevlist
	for(int j = 0 ; j < maxSkipHeight ; j++) {

		prevList[j] = tmpDummy;

		tmpDummy = tmpDummy->top;

	}
	//
	tmpDummy = list;
	i = 0;

	while((tmpDummy->top != NULL) && (tmpDummy->top->link != NULL))	{	// Get tmpDummy to the highest populated dummy node (populated: link isnt null)
	
		tmpDummy = tmpDummy->top;
		i++;	// Count populated layers

	}

	skipList currNode = tmpDummy->link;	// currNode starts from the first normal node, of the heighest populated layer

	/* 						(currNode == NULL): means node will be inserted at the end
		(currNode->citizen->citizenID > citizen->citizenID): means the node will be inserted here
							(else): node will be inserted somewhere in the middle */

	for(int j = i ; j >= 0 ; j--) { // Record path, starting from top left normal node -- i is the number of layers we have to go down (number of populated layers)
		// Horizontal placement -- find the node that will be on the left of the new node for this layer
		while((currNode != NULL) && (atoi(currNode->citizen->citizenID) < atoi(citizen->citizenID))) { // Until current node is >= citizenID, or we have reached the end

			prevList[j] = currNode;
			currNode = currNode->link;

		}	// prevList[j] now has the left node, for this insertion, on this level

		if((currNode != NULL) && (atoi(currNode->citizen->citizenID) == atoi(citizen->citizenID))) { // Entry exists

			free(prevList);

			return NULL;

		}

		currNode = prevList[j]->bot;	// Go down a layer

		// if(currNode == NULL)	// This is essentially what is being done, but since we have a correct implementation we can keep this commented
		// 	break;

	} 	// Until we parse all populated layers -- We have the left node for every layer

	for(int layer = 0 ; ; layer++) {	// Link from bot to top until we link for every height of the new node

		newNode->link = prevList[layer]->link;
		prevList[layer]->link = newNode;

		newNode = newNode->top;

		if(newNode == NULL)	// Link until you fill all new node levels
			break;

	}

	free(prevList);

	return newNode;

}

skipList skipListGet(skipList list, citizenRecord citizen) {

	if(list == NULL)
		return NULL;

	while((list->top != NULL) && (list->top->link != NULL)) // Get to the top populated dummy node
		list = list->top;

	skipList currNode = list->link, prevNode = list;	// Start from the first top left normal node

	while(currNode != NULL) { // Parse all layers until you get out of bounds
		// Horizontal placement -- Find the node that should be on the left of this node, for this layer
		while((currNode != NULL) && (atoi(currNode->citizen->citizenID) < atoi(citizen->citizenID))) { // Until current node is >= citizenID, or we have reached the end
		
			prevNode = currNode;
			currNode = currNode->link;

		}

		if((currNode != NULL) && (atoi(currNode->citizen->citizenID) == atoi(citizen->citizenID)))
			return currNode;

		currNode = prevNode->bot;	// Go down a layer

	}

	return NULL;

}

void skipListPrint(skipList list) {		// Print the entire skip list

	int i = 0;

	while((list != NULL)) {
	
		printf("L%d: ", i);

		skipListPrintLayer(list->link);

		list = list->top;
		i++;

	}

}

void skipListPrintLayer(skipList list) {

	while(list != NULL) {

		printf("%s", list->citizen->citizenID);

		list = list->link;

		if(list != NULL)
			printf(" -> ");

	}

	printf("\n");

}

void skipListDestroy(skipList list) {
	
	if(list == NULL)
		return;
	// Free the dummy citizen once
	if(dummyCitizen != NULL) {

		free(dummyCitizen->citizenID);
		free(dummyCitizen);
		dummyCitizen = NULL;

	}
	// Call only from bottom nodes
	if((list->link != NULL) && (list->bot == NULL))
		skipListDestroy(list->link);
	// Call upwards
	if(list->top != NULL)
		skipListDestroy(list->top);
	// If this is a bottom node free the date it holds
	if(list->bot == NULL)
		free(list->dateVaccinated);
	// Free curr node
	free(list);

}