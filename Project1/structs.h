typedef struct {

	char *citizenID;
	char *firstName;
	char *lastName;
	char *country;
	int age;

} citizenRc;

typedef citizenRc *citizenRecord;
// countryList.c
typedef struct clst {

	char *country;
	int population;
	struct clst *link;

} countryListNode;

typedef countryListNode *countryList;
// hashtable.c
typedef struct NodeTag {

	citizenRecord citizen;
	struct NodeTag *link;

} hashNodeType;

typedef hashNodeType *hashNode;

typedef hashNode *hashTable;
// skipList.c
typedef struct slst {

	struct slst *top;
	struct slst *bot;
	struct slst *link;
	citizenRecord citizen;
	char *dateVaccinated;

} skipListNode;

typedef skipListNode *skipList;
// virusList.c
typedef struct vlst {

	char *virus;
	char *bloom;
	skipList nonVaccinated;
	skipList vaccinated;
	struct vlst *link;

} virusListNode;

typedef virusListNode *virusList;
//
typedef struct alst {

	char *country;
	int vaccinatedA;
	int vaccinatedB;
	int vaccinatedC;
	int vaccinatedD;
	int nonVaccinatedA;
	int nonVaccinatedB;
	int nonVaccinatedC;
	int nonVaccinatedD;
	int vaccinatedRangeA;
	int vaccinatedRangeB;
	int vaccinatedRangeC;
	int vaccinatedRangeD;
	struct alst *link;

} ageListNode;

typedef ageListNode *ageList;
// popList.c
typedef struct plst {

	char *country;
	int vaccinated;
	int notVaccinated;
	int vaccinatedRange;
	struct plst *link;

} popListNode;

typedef popListNode *popList;