#include "functions.h"

int configure(int argc, char **argv, char **citizenRecordsFile) {		// Function for -i argument

	if(argc > 1) {

		/*
			argv choices:
				-c	citizenRecordsFile
				-b	bloomSize
		*/
		// Go through all arguments
		for (int i = 1 ; i < argc ; i++) {
			// Check if we have -c as argument
			if(!strcmp("-c", argv[i])) { /* if we have -c */

				*citizenRecordsFile = argv[i + 1];


			} else if(!strcmp("-b", argv[i])) { /* if we have -c */

				bloomSize = atoi(argv[i + 1]);


			}

		}

		return 0;

	}

	return -1;

}

char *subStr(char *str1, int part, char seperator) { 		/*    subStr is a function that copies the part
													     		substring of str1 to str2 and returns str2's 
													  address - "parts" are strings seperated by a 'seperator' char 
													  	  we could have used default ' ' seperator but it is here 
													  					for expandability purposes             */
	int i = 0, j = 0;
	char *str2 = malloc(1000);

	while(part > 1) {

		while(str1[i] != seperator) // Cycle through parts 
			i++;

		i++;
		part--;

	}

	// Here we have reached the part we want to copy

	while(!(str1[i] == seperator || str1[i] == '\0')) {	// Copy chars until we find a seperator or end of string

		str2[j] = str1[i];
		i++;
		j++;

	}
	// Put string ending character at the end, after finishing copying
	str2[j] = '\0';

	return str2;

}

int partCount(char *str) { // partCount function just counts how many parts str has (counts spaces + 1) 

	int parts = 1, i = 0;
	// While we have not reached the end of the string
	while(str[i] != '\0') {
		// Up parts if char was space
		if(str[i] == ' ')
			parts++;
		// Up character index
		i++;

	}

	return parts;

}

int compareDates(char *date1, char *date2) {	// Compares dates of format xx-xx-xxxx
    
    char temp[11], temp2[11];

    strcpy(temp, date1);
    strcpy(temp2, date2);

    const char s[2] = "-";
    char *a1, *a2, *a3, *a4, *a5, *a6;
    // Seperate date month and year based on - locations
    a1 = strtok(temp, s);
    a2 = strtok(NULL, s);
    a3 = strtok(NULL, s);

    a4 = strtok(temp2, s);
    a5 = strtok(NULL, s);
    a6 = strtok(NULL, s);
    // Compare year first, then month, then date
    if(atoi(a3) < atoi(a6))
    	return -1;
    else if(atoi(a3) > atoi(a6))
    	return 1;

    if(atoi(a2) < atoi(a5))
    	return -1;
    else if(atoi(a2) > atoi(a5))
    	return 1;

    if(atoi(a1) < atoi(a4))
    	return -1;
    else if(atoi(a1) > atoi(a4))
    	return 1;

    return 0;
    
}

char *getDate(void) {

	char *date = malloc(11);
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);

	sprintf(date, "%d-%d-%d", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);

	return date;

}

int compareCitizens(citizenRecord citizen1, char *a0, char *a1, char *a2, char *a3, int a4) {	// When same, returns 0, else 1

	if(!strcmp(citizen1->citizenID, a0))
		if(!strcmp(citizen1->firstName, a1))
			if(!strcmp(citizen1->lastName, a2))
				if(!strcmp(citizen1->country, a3))
					return citizen1->age != a4;

	return 1;

}