#include "functions.h"

int configure(int argc, char **argv, int *numMonitors, int *socketBufferSize, int *cyclicBufferSize, int *sizeOfBloom, char **inputDir, int *numThreads) {		// Function for -i argument

	if(argc == 13) {

		/*
			argv choices:
				–m numMonitors 
				-b socketBufferSize
				-c cyclicBufferSize 
				-s sizeOfBloom 
				-i input_dir
				-t numThreads
		*/
		// Go through all arguments
		for (int i = 1 ; i < argc ; i++) {
			// Check if we have -c as argument
			if(!strcmp("-m", argv[i])) { /* if we have -c */

				*numMonitors = atoi(argv[i + 1]);

			} else if(!strcmp("-b", argv[i])) { /* if we have -c */

				*socketBufferSize = atoi(argv[i + 1]);

			} else if(!strcmp("-c", argv[i])) { /* if we have -c */

				*cyclicBufferSize = atoi(argv[i + 1]);

			} else if(!strcmp("-s", argv[i])) { /* if we have -c */

				*sizeOfBloom = atoi(argv[i + 1]);

			} else if(!strcmp("-i", argv[i])) { /* if we have -c */

				*inputDir = argv[i + 1];

			} else if(!strcmp("-t", argv[i])) { /* if we have -c */

				*numThreads = atoi(argv[i + 1]);

			}

		}

		return 0;

	}

	return -1;

}

int configure2(int argc, char **argv, int *port, int *numThreads, int *socketBufferSize, int *cyclicBufferSize, int *bloomSize, char ***paths) {		// Function for -i argument

	if(argc > 10) {
		/*
			argv choices:
				–p port 
				-t numThreads
				-b socketBufferSize 
				-c cyclicBufferSize 
				-s sizeOfBloom
				paths...
		*/
		// Go through all arguments
		for (int i = 1 ; i < argc ; i++) {

			// Check if we have -c as argument
			if(!strcmp("-p", argv[i])) { /* if we have -c */

				*port = atoi(argv[i + 1]);

			} else if(!strcmp("-t", argv[i])) { /* if we have -c */

				*numThreads = atoi(argv[i + 1]);

			} else if(!strcmp("-b", argv[i])) { /* if we have -c */

				*socketBufferSize = atoi(argv[i + 1]);

			} else if(!strcmp("-c", argv[i])) { /* if we have -c */

				*cyclicBufferSize = atoi(argv[i + 1]);

			} else if(!strcmp("-s", argv[i])) { /* if we have -c */

				*bloomSize = atoi(argv[i + 1]);

			} else if(i > 10) { /* if we have -c */

				if(argc == 11) // No paths
					*paths = NULL;
				// Make a char array equal to the number of paths
				*paths = (char **) malloc((argc - 11) * sizeof(char *));
				// Parse rest args (paths)
				for( ; i < argc ; i++) {

					(*paths)[i - 11] = (char *) malloc(strlen(argv[i]) + 1);
					strcpy((*paths)[i - 11], argv[i]);
				
				}
				printf("\n");
				return argc - 11;

			}

		}

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

int checkDates(char *date1, char *date2) {	// Compares dates of format xx-xx-xxxx and returns value based on 6 month difference
    // date2 must be the goal date, date 1 must be the vaccination date
    char temp[11], temp2[11];

    strcpy(temp, date1);
    strcpy(temp2, date2);

    const char s[2] = "-";
    char *d1, *m1, *y1, *d2, *m2, *y2;
    // Seperate date month and year based on - locations
    d1 = strtok(temp, s);
    m1 = strtok(NULL, s);
    y1 = strtok(NULL, s);

    d2 = strtok(temp2, s);
    m2 = strtok(NULL, s);
    y2 = strtok(NULL, s);
    // Compare year first, then month, then date
    if(atoi(y1) == atoi(y2) - 1) { 				// 1 year difference (second one being higher)

    	if(12 - atoi(m1) + atoi(m2) == 6) {		// 6 month difference

    		if(atoi(d1) >= atoi(d2))
    			return 0;


    	} else if((12 - atoi(m1)) + (atoi(m2)) < 6) {	// less than 6 month diff

    		return 0;

    	}

    } else if(atoi(y1) == atoi(y2)) {			// same year

    	if(atoi(m2) - atoi(m1) == 6) {			// 6 month difference

    		if(atoi(d1) > atoi(d2))
    			return 0;


    	} else if(atoi(m2) - atoi(m1) == 0) {

    		if(atoi(d1) <= atoi(d2))
    			return 0;

    	} else if(atoi(m2) - atoi(m1) < 6) {	// less than 6 month diff


    		return 0;

    	}

    }

    return -1;
    
}

int compareCitizens(citizenRecord citizen1, char *a0, char *a1, char *a2, char *a3, int a4) {	// When same, returns 0, else 1

	if(!strcmp(citizen1->citizenID, a0))
		if(!strcmp(citizen1->firstName, a1))
			if(!strcmp(citizen1->lastName, a2))
				if(!strcmp(citizen1->country, a3))
					return citizen1->age != a4;

	return 1;

}

// Returns the num parameter as a heap allocated string
char *itoa2(int num) {
        
        char *str = (char *) malloc(20 * sizeof(char));
        sprintf(str, "%d", num);

        return str;

}
// Converts num to string and copies it to a pre-allocated space
void itoa3(int num, char *str) {
        
        sprintf(str, "%d", num);

}

void writeString(char *string, int process, int *fd, int numMonitors, char **buff, int bufferSize, int bloomMode) {
	// -1 means copy for all processes
	if(process != -1)
		numMonitors = process + 1; 
	else	// copy for all
		process = 0;
  	// For all processes
  	for(int processIndex = process ; processIndex < numMonitors ; processIndex++)		
  		writeString2(string, buff[processIndex], bufferSize, fd[processIndex], bloomMode);

}

void writeString2(char *string, char *buff, int bufferSize, int fd, int bloomMode) {

	int buffIndex = 0, stringIndex = 0;
	// Find the next available space on the buffer (if there is one) and stop
	if(buff[bufferSize - 1] != '.') {	// Big edge case
	
		buffIndex = bufferSize;	// Setup to write and clean
	
	} else {
	
		for(buffIndex = 0 ; buffIndex < bufferSize ; buffIndex++)
			if(buff[buffIndex] == '.')
				break;

	}

	while(1) {
		// Break if we find a seperator at any time (bloomMode == 0) or after we have copied the side of bloom (bloomMode == 1)
		// Commented code is added for ease of reading but is not needed
		// if(bloomMode == 0) {

		// 	if(string[stringIndex] == '~' || string[stringIndex] == '|')
		// 		break;

		// } else {

		if(stringIndex >= bloomMode)
			if(string[stringIndex] == '~' || string[stringIndex] == '|')
				break;

		// }
		// Check for full buffer
		if(buffIndex >= bufferSize) {	// Buffer is full here
			// Write buffer
			write(fd, buff, bufferSize);
			// Empty buffer -- Reset with "." value
			for(buffIndex = 0 ; buffIndex < bufferSize ; buffIndex++)
        		buff[buffIndex] = '.';
        	// Reset index
        	buffIndex = 0;

		}
		// Copy character from string to buffer
		buff[buffIndex] = string[stringIndex];

		stringIndex++;
		buffIndex++;

	}
	// Final check for full buffer, before writing the whole string
	if(buffIndex >= bufferSize) {

		write(fd, buff, bufferSize);

		for(buffIndex = 0 ; buffIndex < bufferSize ; buffIndex++)
    		buff[buffIndex] = '.';

    	buffIndex = 0;

	}
	// Write final terminating character
	buff[buffIndex] = string[stringIndex];

}

char *readString(char seperator, int fd, char *buff, int *buffIndex, int bufferSize, int bloomMode) {

	char *stringBuff = (char *) malloc(99999);
	int stringIndex = 0, ready = -1;
	// Read until you break from a seperator -- do while to read information left over from previous iterations
	do {
		// printf("%s ###\n", buff);
		// While you encounter content (non "." characters) and you are not out of bounds 
		for( ; ((*buffIndex) < bufferSize) && (buff[*buffIndex] != '.') ; (*buffIndex)++) {

			ready = (bloomMode == 0) || ((bloomMode != 0) && (stringIndex >= bloomMode));
			// Check if we are ready to return string based on mode
			if(ready) {
				// Check for seperator encounter -- This is information seperator
				if(buff[*buffIndex] == seperator) {
					// Set this as the index place for the string
					stringBuff[stringIndex] = '\0';
					// Pass terminator on index
					(*buffIndex)++;
					// Return read string
					char *returnal = (char *) malloc(stringIndex + 1);
					memcpy(returnal, stringBuff, stringIndex + 1);
					free(stringBuff);

					return returnal; 
				
				} else if(buff[*buffIndex] == '|') {
					
					stringBuff[0] = '|';
					stringBuff[1] = '\0';

					char *returnal = (char *) malloc(2);
					strcpy(returnal, stringBuff);
					free(stringBuff);
					// Return read string
					return returnal; 
				
				}

			}
			// Copy character over from buffer
			stringBuff[stringIndex++] = buff[*buffIndex];

		}
		// Out of content - Reset index and read next buffer
		*buffIndex = 0;

	} while(read(fd, buff, bufferSize));

	return NULL;

}

citizenRecord citizenCreate(char *id, char *fn, char *ln, char *country, int age) {	// Creates citizenRecord struct
	// Allocate space and copy values
	citizenRecord cit = (citizenRecord) malloc(sizeof(citizenRc));
	cit->citizenID = malloc(strlen(id) + 1);
	strcpy(cit->citizenID, id);
	cit->firstName = malloc(strlen(fn) + 1);
	strcpy(cit->firstName, fn);
	cit->lastName = malloc(strlen(ln) + 1);
	strcpy(cit->lastName, ln);
	cit->country = malloc(strlen(country) + 1);
	strcpy(cit->country, country);
	cit->age = age;

	return cit;

}

void citizenDestroy(citizenRecord cit) {
	// Free all space aquired from heap
	free(cit->citizenID);
	free(cit->firstName);
	free(cit->lastName);
	free(cit->country);
	free(cit);

}

char **fillArgs(int port, int numThreads, int socketBufferSize, int cyclicBufferSize, int bloomSize, char **paths, int arraySize) {

	char **args = malloc((arraySize + 11 + 1) * sizeof(char *));

	args[0] = malloc(strlen("./Monitor") + 1);
	strcpy(args[0], "./Monitor");

	args[1] = malloc(strlen("-p") + 1);
	strcpy(args[1], "-p");

	args[2] = itoa2(port);

	args[3] = malloc(strlen("-t") + 1);
	strcpy(args[3], "-t");

	args[4] = itoa2(numThreads);

	args[5] = malloc(strlen("-b") + 1);
	strcpy(args[5], "-b");

	args[6] = itoa2(socketBufferSize);

	args[7] = malloc(strlen("-c") + 1);
	strcpy(args[7], "-c");

	args[8] = itoa2(cyclicBufferSize);

	args[9] = malloc(strlen("-s") + 1);
	strcpy(args[9], "-s");

	args[10] = itoa2(bloomSize);

	int i;
	for(i = 0 ; i < arraySize ; i++)
		args[11 + i] = paths[i];

	args[11 + i] = NULL;

	return args;

}

void destroyArgs(char **args) {

	free(args[0]);
	free(args[1]);
	free(args[2]);
	free(args[3]);
	free(args[4]);
	free(args[5]);
	free(args[6]);
	free(args[7]);
	free(args[8]);
	free(args[9]);
	free(args[10]);
	free(args);
	
}