#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <poll.h>
    
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>

#include "structs.h"

#define BASE_PORT 10000

enum {FALSE, TRUE};
// Struct sizes
extern int bloomSize;
extern int bufferSize;
extern int cyclicBufferSize;
// Multithread variables
extern pool_t pool;
extern pthread_mutex_t mtx;
extern pthread_cond_t cond_nonempty;
extern pthread_cond_t cond_nonfull;
extern struct dirent *dir;
extern int pathIndex;
extern int pathCount;
extern int done;
// Structs;
extern virusList virusDataList;
extern citizenList citizenDataList;

// bloom.c
void bloomInsert(char *, char *);
int bloomExists(char *, char *);
void setBit(char *, int, int);
int checkBit(char *, int);
// hash.c
unsigned long hash_i(unsigned char *, unsigned int);
// util.c
int configure(int, char **, int *, int *, int *, int *, char **, int *);
int configure2(int, char **, int *, int *, int *, int *, int *, char ***);
char *subStr(char *, int, char);
int partCount(char *);
int compareDates(char *, char *);
int checkDates(char *, char *);
int compareCitizens(citizenRecord, char *, char *, char *, char*, int);
char *itoa2(int);
void itoa3(int, char *);
void writeString(char *, int, int *, int, char **, int, int);
void writeString2(char *, char *, int, int, int);
char *readString(char, int, char *, int *, int, int);
citizenRecord citizenCreate(char *, char *, char *, char *, int);
void citizenDestroy(citizenRecord);
char **fillArgs(int, int, int, int, int, char **, int);
void destroyArgs(char **);
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
char **processListGet(processList, int, int *);
void processListPrint(processList);
void processListDestroy(processList);
// dirList.c
reqList reqListCreate(char *, char *, char *, int);
reqList reqListInsert(reqList, char *, char *, char *, int);
void reqListSearch(reqList, char *, char *, char *, char *, int *, int *);
void reqListPrint(reqList);
void reqListDestroy(reqList);
// citizenList.c
citizenList citizenListCreate(citizenRecord);
citizenList citizenListInsert(citizenList, citizenRecord);
void citizenListDestroy(citizenList);
// monitor.c
void insertData(char *, virusList *, citizenList *);
dirList newFiles(char **, int, dirList);
// pool .c
void initialize(pool_t *, int);
void poolPut(pool_t *, char *);
char *poolGet(pool_t *);
void *consumer(void *);