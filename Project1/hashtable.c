#include "functions.h"

int TABLESIZE = 1000;

hashTable HTCreate(void) {	// Create a hash table structure
	// Allocate space needed
	hashTable new_table = (hashTable) malloc(TABLESIZE * sizeof(hashNode));
	// Init data member values
	for(int i = 0 ; i < TABLESIZE ; i++) {

		new_table[i] = (hashNode) malloc(sizeof(hashNodeType));
		new_table[i]->citizen = NULL;
		new_table[i]->link = NULL;

	}

	return new_table;

}

citizenRecord HTInsert(hashTable table, char *id, char *fn, char *ln, char *country, int age) {	// Insert data to the hash table
	// Create citizenRecord struct
	citizenRecord stud = citizenCreate(id, fn, ln, country, age);
	// Go to the correct hash table index 
	hashNode index = table[hash_i((unsigned char *) stud->citizenID, 0) % TABLESIZE];

	if(index->citizen == NULL) {	//If there is nothing in this index of the hash add the new one
			
		index->citizen = stud;

	} else {

		hashNode tmp = index;	// Used for linking

		while((index = index->link) != NULL)	// Reach the end of the list
			tmp = index;	// For going back if we reach the end
		
		if(index == NULL) {		// Add the node to the end of the index list

			hashNode new_node = (hashNode) malloc(sizeof(hashNodeType));
			new_node->citizen = stud;
			new_node->link = NULL;
			tmp->link = new_node;

		}

	}

	return stud;	

}

citizenRecord HTGet(hashTable table, char *id) {		// Used to lookup a citizen based on id

	hashNode index = table[hash_i((unsigned char *) id, 0) % TABLESIZE];

	if(index->citizen == NULL)	// Return NULL if there is no entry
		return NULL;

	while(strcmp(index->citizen->citizenID, id) && (index != NULL))	{ // Parse the system until we find a match
		
		index = index->link;

		if(!index)	// If index has reached the end (NULL)
			return NULL;

	}

	return index->citizen;

}

int HTRemove(hashTable table, char *id) {	// Remove citizen with 'id' id from the hash table

	hashNode index = table[hash_i((unsigned char *) id, 0) % TABLESIZE], backup = index;

	while(strcmp(index->citizen->citizenID, id)) {	// Parse the list untill we find a match

		backup = index;
		index = index->link;

		if(!index)	// If index has reached the end (NULL)
			return -1;
	
	}

	backup->link = index->link;		// Fill the gap created
	citizenDestroy(index->citizen);	// Free stud space
	free(index);

	return 0;

}

void HTDestroy(hashTable table) {	// Frees space of a hash table
	// Call hashNodeDestroy for every table index
	for(int i = 0 ; i < TABLESIZE ; i++)
		NodeDestroy(table[i]);
	// Free table space
	if(table != NULL)
		free(table);

}

void NodeDestroy(hashNode node) {	// A function that destroyes nodes
	// Destroy the stud portion
	if(node->citizen != NULL)
		citizenDestroy(node->citizen);
	// Destroy chain recursively
	if(node->link != NULL)
		NodeDestroy(node->link);
	// Free the space of current node
	free(node);

}

citizenRecord citizenCreate(char *id, char *fn, char *ln, char *country, int age) {	// Creates citizenRecord struct
	// Allocate space and copy values
	citizenRecord stud = (citizenRecord) malloc(sizeof(citizenRc));
	stud->citizenID = malloc(strlen(id) + 1);
	strcpy(stud->citizenID, id);
	stud->firstName = malloc(strlen(fn) + 1);
	strcpy(stud->firstName, fn);
	stud->lastName = malloc(strlen(ln) + 1);
	strcpy(stud->lastName, ln);
	stud->country = country;
	stud->age = age;

	return stud;

}

void citizenDestroy(citizenRecord stud) {
	// Free all space aquired from heap
	free(stud->citizenID);
	free(stud->firstName);
	free(stud->lastName);
	// free(stud->country); // Do not free -- member of countryList
	free(stud);

}