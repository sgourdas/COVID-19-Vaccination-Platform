#include "functions.h"


void bloomInsert(char *bloomFilter, char *id) {

	int bloomBitsSize = bloomSize << 3; // sizeof(bloomFilter) * 8
	int hashKey;

	for(int i = 0 ; i < 16 ; i++) {	// K = 16
			
		hashKey = hash_i((unsigned char *) id, i) % bloomBitsSize;

		setBit(bloomFilter, hashKey, TRUE);

	}

}

int bloomExists(char *bloomFilter, char *id) {	// Returns 0 if the id seems to exist in the filter, else if not

	int bloomBitsSize = bloomSize << 3; // sizeof(bloomFilter) * 8
	int exists = 0, hashKey;

	for(int i = 0 ; i < 16 ; i++) {	// K = 16
		
		hashKey = hash_i((unsigned char *) id, i) % bloomBitsSize;

		exists += checkBit(bloomFilter, hashKey);

	}

	return exists;

}

void setBit(char *bloomFilter, int pos, int value) {

	int i = pos >> 3;	// Equal to (pos / 8), but faster -- Finds the index
	int offset = pos % 8;	// Finds the bit offset in that index

	unsigned char mask = 1;	// internal representation: 00000001

	mask = mask << offset;	// shift the bit based on offset

	if(value == TRUE)
		bloomFilter[i] |= mask;	// OR with the mask bits
	else
		bloomFilter[i] &= ~mask; // AND with the inverted mask bits

}

int checkBit(char *bloomFilter, int pos) {

	int i = pos >> 3;	// Equal to (pos / 8), but faster -- Finds the index
	int offset = pos % 8;	// Finds the bit offset in that index

	unsigned char mask = 1;	// internal representation: 00000001

	mask = mask << offset;	// shift the bit based on offset

	if(bloomFilter[i] & mask)
		return 0; // exists
	else
		return -1; // not exists

}