typedef struct {

	char *citizenID;
	char *firstName;
	char *lastName;
	char *country;
	int age;

} citizenRc;

typedef citizenRc *citizenRecord;
// skipList.c
typedef struct slst {

	struct slst *top;
	struct slst *bot;
	struct slst *link;
	citizenRecord citizen;
	char *dateVaccinated;

} skipListNode;

typedef skipListNode *skipList;
// dirList.c
typedef struct dlst {

	char *dir;
	struct dlst *link;

} dirListNode;

typedef dirListNode *dirList;
// virusList.c
typedef struct vlst {

	char *virus;
	char *bloom;
	skipList nonVaccinated;
	skipList vaccinated;
	struct vlst *link;

} virusListNode;

typedef virusListNode *virusList;
// countryList.c
typedef struct clst {

	char *country;
	int population;
	struct clst *link;

} countryListNode;

typedef countryListNode *countryList;
// bloomList.c
typedef struct blst {

	char *virus;
	char *bloom;
	struct blst *link;

} bloomListNode;

typedef bloomListNode *bloomList;
// processList.c
typedef struct plst {

	char *country;
	int processID;
	struct plst *link;

} processListNode;

typedef processListNode *processList;
// dirList.c
typedef struct rlst {

	char *date;
	char *virus;
	char *country;
	int accepted;
	struct rlst *link;

} reqListNode;

typedef reqListNode *reqList;