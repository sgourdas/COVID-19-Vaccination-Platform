#include "functions.h"

#define YES 1;
#define NO 0;

int bloomSize;

int main(int argc, char **argv) {

	int c = 0;
	srand(time(NULL));

	char *citizenRecordsFile;

	if(configure(argc, argv, &citizenRecordsFile)) {	// Get arguments

		printf("Incorrect arguments.\n");

		return -1;

	}

	FILE *fp = fopen(citizenRecordsFile, "r");
	int ageBuffer, charsRead;
	char buffer[300], a0[30], a1[30], a2[30], a3[30], a4[30], a5[30], a6[30];
	countryList countryDataList = NULL, tempList;
	virusList virusDataList = NULL;
	hashTable table = HTCreate();
	citizenRecord citizen;

	while(fgets(buffer, 200, fp) != NULL) {	// Until you read the entire file

		charsRead = sscanf(buffer, "%s %s %s %s %d %s %s %s", a0, a1, a2, a3, &ageBuffer, a4, a5, a6);

		if(virusDataList != NULL) {

			citizen = HTGet(table, a0);

			if((citizen != NULL) && (compareCitizens(citizen, a0, a1, a2, a3, ageBuffer))) {

				if(charsRead == 7)
					printf("ERROR IN RECORD %s %s %s %s %d %s %s\n", a0, a1, a2, a3, ageBuffer, a4, a5);
				else 
					printf("ERROR IN RECORD %s %s %s %s %d %s %s %s\n", a0, a1, a2, a3, ageBuffer, a4, a5, a6);

				continue;

			}

			if(((citizen != NULL) && (virusListSkipExists(virusDataList, a4, citizen) != NULL)) || ((!strcmp(a5, "NO")) && (charsRead > 7))) {	// If id exists, or we have NO followed by date in file

				if(charsRead == 7)
					printf("ERROR IN RECORD %s %s %s %s %d %s %s\n", a0, a1, a2, a3, ageBuffer, a4, a5);
				else 
					printf("ERROR IN RECORD %s %s %s %s %d %s %s %s\n", a0, a1, a2, a3, ageBuffer, a4, a5, a6);

				continue;

			}

		}
		c++;
		// Make sure country exists on country list and return its pointer
		if(countryDataList == NULL) {
		
			countryDataList = countryListCreate(a3);
			tempList = countryDataList;
		
		} else {

			tempList = countryListInsert(countryDataList, a3);
		
		}
		// Insert entry on to citizen hashtable
		citizen = HTInsert(table, a0, a1, a2, tempList->country, ageBuffer);
		// Add to skiplist
		if(virusDataList == NULL) {

			if(!strcmp(a5, "YES"))
				virusDataList = virusListCreate(a4, citizen, a6);
			else
				virusDataList = virusListCreate(a4, citizen, NULL);
		
		} else {

			if(!strcmp(a5, "YES"))
				virusListInsert(virusDataList, a4, citizen, a6);
			else
				virusListInsert(virusDataList, a4, citizen, NULL);
		
		}

	}
	printf("%d\n", c);
	fclose(fp);

	char *temp, cmd[1000];

	do {	// Get commands from user
	
		printf("vaccineMonitor> "); 
		// Read the Command
		scanf("%[^\n]%*c", cmd);
		// Get the first word of the complete command, to determine the category
		temp = subStr(cmd, 1, ' ');
		/* Decide what the Command is and act accordingly 
			-- every single one of these outside ifs 
			correspond to a different in-game command     */
		if(!strcmp(temp, "/vaccineStatusBloom")) {

	 		if(partCount(cmd) == 3) { 
				// Assign each parameter of insert command to a variable for insertion
				char *a2 = subStr(cmd, 2, ' ');
				char *a3 = subStr(cmd, 3, ' ');
				// Try to insert the entry to the database and print the according message based on return 
				citizen = HTGet(table, a2);

				if(citizen == NULL) {

					printf("ERROR: CITIZEN %s DOES NOT EXIST\n", a2);

					free(a2);
					free(a3);

					continue;

				}

				if(virusListBloomSearch(virusDataList, a3, citizen))
					printf("NOT VACCINATED\n");
				else
					printf("MAYBE\n");
				// Free all space reserved
				free(a2);
				free(a3);

			} else {

				printf("Correct usage: </vaccineStatusBloom citizenID virusName>\n");

			}

		} else if(!strcmp(temp, "/vaccineStatus")) {
				
			if(partCount(cmd) == 3) { 
				
				char *a2 = subStr(cmd, 2, ' ');
				char *a3 = subStr(cmd, 3, ' ');

				citizen = HTGet(table, a2);

				if(citizen == NULL) {	// Citizen does not exist in database, so skip rest

					printf("ERROR: CITIZEN %s DOES NOT EXIST\n", a2);

					free(a2);
					free(a3);

					continue;

				}

				skipList result;

				if((result = virusListSkipSearch(virusDataList, a3, citizen)) != NULL)
					printf("VACCINATED ON %s\n", result->dateVaccinated);
				else
					printf("NOT VACCINATED\n");

				free(a2);
				free(a3);

			} else if(partCount(cmd) == 2) {

				char *a2 = subStr(cmd, 2, ' ');

				citizen = HTGet(table, a2);

				if(citizen == NULL) {

					printf("ERROR: CITIZEN %s DOES NOT EXIST\n", a2);

					free(a2);

					continue;

				}

				virusListSkipSearch(virusDataList, NULL, citizen);

				free(a2);

			} else {

				printf("Correct usage: </vaccineStatus citizenID [virusName]>\n");

			}

		} else if(!strcmp(temp, "/populationStatus")) {

			if(partCount(cmd) == 2) { 
				
				char *a2 = subStr(cmd, 2, ' ');

				popList popData = virusListPopSearch(virusDataList, NULL, a2, "01-01-1000", "01-01-9000");

				popListPrint(popData);

				popListDestroy(popData);

				free(a2);

			} else if(partCount(cmd) == 3) {

				char *a2 = subStr(cmd, 2, ' ');
				char *a3 = subStr(cmd, 3, ' ');

				popList popData = virusListPopSearch(virusDataList, NULL, a2, "01-01-1000", "01-01-9000");

				popListPrint(popData);

				popListDestroy(popData);

				free(a2);
				free(a3);

			} else if(partCount(cmd) == 5) {
				
				char *a2 = subStr(cmd, 2, ' ');
				char *a3 = subStr(cmd, 3, ' ');
				char *a4 = subStr(cmd, 4, ' ');
				char *a5 = subStr(cmd, 5, ' ');

				popList popData = virusListPopSearch(virusDataList, a2, a3, a4, a5);

				popListPrint(popData);

				popListDestroy(popData);

				free(a2);
				free(a3);
				free(a4);
				free(a5);

			} else if(partCount(cmd) == 4) {

				char *a2 = subStr(cmd, 2, ' ');
				char *a3 = subStr(cmd, 3, ' ');
				char *a4 = subStr(cmd, 4, ' ');

				popList popData = virusListPopSearch(virusDataList, NULL, a2, a3, a4);

				popListPrint(popData);

				popListDestroy(popData);

				free(a2);
				free(a3);
				free(a4);

			} else {

				printf("Correct usage: <● /populationStatus [country] virusName date1 date2>\n");

			}

		} else if(!strcmp(temp, "/popStatusByAge")) {

			if(partCount(cmd) == 2) { 
				
				char *a2 = subStr(cmd, 2, ' ');

				ageList ageDataList = virusListAgeSearch(virusDataList, NULL, a2, "01-01-1000", "01-01-9000");

				ageListPrint(ageDataList);

				ageListDestroy(ageDataList);

				free(a2);

			} else if(partCount(cmd) == 3) {

				char *a2 = subStr(cmd, 2, ' ');
				char *a3 = subStr(cmd, 3, ' ');

				ageList ageDataList = virusListAgeSearch(virusDataList, a2, a3, "01-01-1000", "01-01-9000");

				ageListPrint(ageDataList);

				ageListDestroy(ageDataList);

				free(a2);
				free(a3);

			} else if(partCount(cmd) == 5) { 
				
				char *a2 = subStr(cmd, 2, ' ');
				char *a3 = subStr(cmd, 3, ' ');
				char *a4 = subStr(cmd, 4, ' ');
				char *a5 = subStr(cmd, 5, ' ');

				ageList ageDataList = virusListAgeSearch(virusDataList, a2, a3, a4, a5);

				ageListPrint(ageDataList);

				ageListDestroy(ageDataList);

				free(a2);
				free(a3);
				free(a4);
				free(a5);

			} else if(partCount(cmd) == 4) {

				char *a2 = subStr(cmd, 2, ' ');
				char *a3 = subStr(cmd, 3, ' ');
				char *a4 = subStr(cmd, 4, ' ');

				ageList ageDataList = virusListAgeSearch(virusDataList, NULL, a2, a3, a4);

				ageListPrint(ageDataList);

				ageListDestroy(ageDataList);

				free(a2);
				free(a3);
				free(a4);

			} else {

				printf("Correct usage: </popStatusByAge [country] virusName date1 date2>\n");

			}

		} else if(!strcmp(temp, "/insertCitizenRecord")) {	// add list return

			if(partCount(cmd) == 9) { 
				
				char *a2 = subStr(cmd, 2, ' ');
				char *a3 = subStr(cmd, 3, ' ');
				char *a4 = subStr(cmd, 4, ' ');
				char *a5 = subStr(cmd, 5, ' ');
				char *a6 = subStr(cmd, 6, ' ');
				char *a7 = subStr(cmd, 7, ' ');
				char *a8 = subStr(cmd, 8, ' ');
				char *a9 = subStr(cmd, 9, ' ');

				if(!strcmp(a8, "NO")) {	// NO with date

					free(a2);
					free(a3);
					free(a4);
					free(a5);
					free(a6);
					free(a7);
					free(a8);
					free(a9);

					continue;

				}

				citizen = HTGet(table, a2);

				if((citizen != NULL) && (compareCitizens(citizen, a0, a1, a2, a3, ageBuffer))) {

					printf("WRONG DATA INSERTED\n");

					continue;

				}

				if(citizen == NULL) {	// ID not exists
					// Insert entry
					if(countryDataList == NULL) {
			
						countryDataList = countryListCreate(a5);
						tempList = countryDataList;
					
					} else {

						tempList = countryListInsert(countryDataList, a5);
					
					}

					citizen = HTInsert(table, a1, a2, a3, tempList->country, atoi(a4));

					if(virusDataList == NULL)
						virusDataList = virusListCreate(a7, citizen, a9);
					else
						virusListInsert(virusDataList, a7, citizen, a9);

				} else {	// Entry exists
					// Use bloom search first so we can maybe save some time
					if(virusListBloomSearch(virusDataList, a7, HTGet(table, a2)) != 0) {	// Is not vaccinated
						//vaccinateNow	
						virusListVac(virusDataList, a7, citizen, a9);

					} else {

						skipList result;

						if((result = virusListSkipSearch(virusDataList, a7, HTGet(table, a2))) != NULL)
							printf("ERROR: CITIZEN %s ALREADY VACCINATED ON %s\n", result->citizen->citizenID, result->dateVaccinated);
						else
							virusListVac(virusDataList, a7, citizen, a9); //vaccinateNow


					}

				}

				free(a2);
				free(a3);
				free(a4);
				free(a5);
				free(a6);
				free(a7);
				free(a8);
				free(a9);

			} else if(partCount(cmd) == 8) {

				char *a2 = subStr(cmd, 2, ' ');
				char *a3 = subStr(cmd, 3, ' ');
				char *a4 = subStr(cmd, 4, ' ');
				char *a5 = subStr(cmd, 5, ' ');
				char *a6 = subStr(cmd, 6, ' ');
				char *a7 = subStr(cmd, 7, ' ');
				char *a8 = subStr(cmd, 8, ' ');

				citizen = HTGet(table, a2);

				if((citizen != NULL) && (compareCitizens(citizen, a0, a1, a2, a3, ageBuffer))) {

					printf("WRONG DATA INSERTED\n");

					continue;

				}

				if(citizen == NULL) {	// ID not exists

					if(countryDataList == NULL) {
			
						countryDataList = countryListCreate(a5);
						tempList = countryDataList;
					
					} else {

						tempList = countryListInsert(countryDataList, a5);
					
					}

					citizen = HTInsert(table, a2, a3, a4, tempList->country, atoi(a6));

					if(virusDataList == NULL)
						virusDataList = virusListCreate(a7, citizen, NULL);
					else
						virusListInsert(virusDataList, a7, citizen, NULL);

				} else {

					printf("ERROR: CITIZEN %s ALREADY EXISTS\n", a2);
					
				}

				free(a2);
				free(a3);
				free(a4);
				free(a5);
				free(a6);
				free(a7);
				free(a8);

			} else {

				printf("Correct usage: </insertCitizenRecord citizenID firstName lastName country age virusName YES/NO [date]>\n");

			}

		} else if(!strcmp(temp, "/vaccinateNow")) {

			if(partCount(cmd) == 7) { 

				char *a2 = subStr(cmd, 2, ' ');
				char *a3 = subStr(cmd, 3, ' ');
				char *a4 = subStr(cmd, 4, ' ');
				char *a5 = subStr(cmd, 5, ' ');
				char *a6 = subStr(cmd, 6, ' ');
				char *a7 = subStr(cmd, 7, ' ');

				citizen = HTGet(table, a2);

				if((citizen != NULL) && (compareCitizens(citizen, a2, a3, a4, a5, atoi(a6)))) {

					printf("WRONG DATA INSERTED\n");
					printf("%s %s %s %s %d!\n", citizen->citizenID, citizen->firstName, citizen->lastName, citizen->country, citizen->age);
					printf("%s %s %s %s %d!\n", a0, a1, a2, a3, ageBuffer);
					continue;

				}

				if(citizen == NULL) {	// ID not exists

					char *date = getDate();

					if(countryDataList == NULL) {
			
						countryDataList = countryListCreate(a5);
						tempList = countryDataList;
					
					} else {

						tempList = countryListInsert(countryDataList, a5);
					
					}

					citizen = HTInsert(table, a2, a3, a4, tempList->country, atoi(a6));

					if(virusDataList == NULL)
						virusDataList = virusListCreate(a7, citizen, date);
					else
						virusListInsert(virusDataList, a7, citizen, date);

					free(date);

				} else {
					// Use bloom search first so we can maybe save some time
					if(virusListBloomSearch(virusDataList, a7, HTGet(table, a2)) != 0) {	// Is not vaccinated

						char *date = getDate();
						
						virusListVac(virusDataList, a7, citizen, date);

						free(date);

					} else {	// MAYBE vaccinated, so we have to make sure usign skip list

						skipList result;

						if((result = virusListSkipSearch(virusDataList, a7, HTGet(table, a2))) != NULL){
						
							printf("ERROR: CITIZEN %s ALREADY VACCINATED ON %s\n", result->citizen->citizenID, result->dateVaccinated);
						
						} else {

							char *date = getDate();
						
							virusListVac(virusDataList, a7, citizen, date);

							free(date);
						
						}

					}

				}

				free(a2);
				free(a3);
				free(a4);
				free(a5);
				free(a6);
				free(a7);

			} else {

				printf("Correct usage: </vaccinateNow citizenID firstName lastName country age virusName>\n");

			}

		} else if(!strcmp(temp, "/list-nonVaccinated-Persons")) {

			if(partCount(cmd) == 2) { 
				
				char *a2 = subStr(cmd, 2, ' ');

				virusListUnvac(virusDataList, a2);

				free(a2);

			} else {

				printf("Correct usage: </list-nonVaccinated-Persons virusName>\n");

			}

		} else if(!strcmp(temp, "/exit")) {
			// Free all allocated data of structs and exit
			countryListDestroy(countryDataList);
			virusListDestroy(virusDataList);
			HTDestroy(table);

			free(temp);

			return 0;

		} else { printf("Uknown Command!\n"); }
		// Free the space used for command argument
		free(temp);

	} while(1);

	return 1;

}