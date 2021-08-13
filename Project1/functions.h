#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

#include "structs.h"

enum {FALSE, TRUE};

extern int bloomSize;

// bloom.c
void bloomInsert(char *, char *);
int bloomExists(char *, char *);
void setBit(char *, int, int);
int checkBit(char *, int);
// hash.c
unsigned long hash_i(unsigned char *, unsigned int);
// countryList.c
countryList countryListCreate(char *country);
countryList countryListInsert(countryList list, char *country);
countryList countryListSearch(countryList list, char *country);
void countryListPrint(countryList list);
void countryListDestroy(countryList list);
// virusList.c
virusList virusListCreate(char *, citizenRecord, char *);
int virusListInsert(virusList, char *, citizenRecord, char *);
int virusListBloomSearch(virusList, char *, citizenRecord);
skipList virusListSkipSearch(virusList, char *, citizenRecord);
popList virusListPopSearch(virusList, char *, char *, char *, char *);
ageList virusListAgeSearch(virusList , char *, char *, char *, char *);
skipList virusListVac(virusList, char *, citizenRecord, char *);
void virusListUnvac(virusList, char *);
void virusListPrint(virusList);
skipList virusListSkipExists(virusList, char *, citizenRecord);
void virusListDestroy(virusList);
// util.c
int configure(int, char **, char **);
char *subStr(char *, int, char);
int partCount(char *);
int compareDates(char *, char *);
char *getDate(void);
int compareCitizens(citizenRecord, char *, char *, char *, char*, int);
// hashtable.c
hashTable HTCreate(void);
citizenRecord HTInsert(hashTable, char *, char *, char *, char *, int);
citizenRecord HTGet(hashTable, char *);
int HTRemove(hashTable, char *);
void HTDestroy(hashTable);
void NodeDestroy(hashNode);
citizenRecord citizenCreate(char *, char *, char *, char *, int);
void citizenDestroy(citizenRecord );
// skipList.c
skipList skipListCreate(citizenRecord, char *);
skipList skipListInsert(skipList, citizenRecord, char *);
skipList skipListGet(skipList, citizenRecord);
popList skipListPopStats(skipList, skipList, char *, char *, char *, char *);
ageList skipListAgeStats(skipList, skipList, char *, char *, char *, char *);
skipList skipListVac(skipList, skipList, char *, citizenRecord, char *);
void skipListUnvac(skipList);
void skipListPrint(skipList);
void skipListPrintLayer(skipList);
void skipListDestroy(skipList);
// ageList.c
ageList ageListCreate(char *, int, char);
ageList ageListInsert(ageList, char *, int, char);
// ageList ageListSearch(ageList, char *);
void ageListPrint(ageList);
void ageListDestroy(ageList);
//
popList popListCreate(char *, char);
popList popListInsert(popList, char *, char);
void popListPrint(popList);
void popListDestroy(popList);