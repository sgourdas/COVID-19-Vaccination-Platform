#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <poll.h>

#include "structs.h"

#define FIRST_MSG 1024

enum {FALSE, TRUE};

extern int bloomSize;
extern int bufferSize;

// bloom.c
void bloomInsert(char *, char *);
int bloomExists(char *, char *);
void setBit(char *, int, int);
int checkBit(char *, int);
// hash.c
unsigned long hash_i(unsigned char *, unsigned int);
// util.c
int configure(int, char **, int *, char **);
char *subStr(char *, int, char);
int partCount(char *);
int compareDates(char *, char *);
int checkDates(char *, char *);
int compareCitizens(citizenRecord, char *, char *, char *, char*, int);
char *itoa2(int);
void itoa3(int, char *);
void writeString(char *, int, int **, int, char **, int, int);
void writeString2(char *, char *, int, int, int);
char *readString(char, int, char *, int *, int, int);
citizenRecord citizenCreate(char *, char *, char *, char *, int);
void citizenDestroy(citizenRecord);
// skipList.c
skipList skipListCreate(citizenRecord, char *);
skipList skipListInsert(skipList, citizenRecord, char *);
skipList skipListGet(skipList, citizenRecord);
skipList skipListVac(skipList, skipList, char *, citizenRecord, char *);
void skipListUnvac(skipList);
void skipListPrint(skipList);
void skipListPrintLayer(skipList);
void skipListDestroy(skipList);
// dirList.c
dirList dirListCreate(char *);
dirList dirListInsert(dirList, char *);
dirList dirListSearch(dirList, char *);
void dirListPrint(dirList);
void dirListDestroy(dirList);
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
void virusListPrint(virusList);
skipList virusListSkipExists(virusList, char *, citizenRecord);
void virusListDestroy(virusList);
// bloomList.c
bloomList bloomListCreate(char *, char *);
bloomList bloomListInsert(bloomList list, char *, char *);
int bloomListSearch(bloomList list, char *, char *);
void bloomListPrint(bloomList list);
void bloomListDestroy(bloomList list);
// processList.c
processList processListCreate(char *, int);
processList processListInsert(processList, char *, int);
int processListSearch(processList, char *);
void processListPrint(processList);
void processListDestroy(processList);
// dirList.c
reqList reqListCreate(char *, char *, char *, int);
reqList reqListInsert(reqList, char *, char *, char *, int);
void reqListSearch(reqList, char *, char *, char *, char *, int *, int *);
void reqListPrint(reqList);
void reqListDestroy(reqList);


dirList newFiles(dirList, dirList, char *);

void setupSIGUSR1(void);
void SIGUSR1Handler(int, siginfo_t *, void *);
void setupSIGCHLD(void);
void SIGCHLDHandler(int, siginfo_t *, void *);
void setupSIGINT_SIGQUIT(void);
void SIGINT_SIGQUITHandler(int, siginfo_t *, void *);
void setupSIGINT_SIGQUIT_PARENT(void);