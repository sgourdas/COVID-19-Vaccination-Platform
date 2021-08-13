#include "functions.h"

#define YES 1;
#define NO 0;

int bufferSize;
int bloomSize;

int main(int argc, char **argv) {

	int numMonitors = -1, socketBufferSize = -1, cyclicBufferSize = -1, numThreads = -1;
	char *inputDir;

	if(configure(argc, argv, &numMonitors, &socketBufferSize, &cyclicBufferSize, &bloomSize, &inputDir, &numThreads)) {	// Get arguments

		printf("Incorrect arguments.\n");

		return -1;

	}
	bufferSize = socketBufferSize;
	/* Read directories and store */
	// Initialize variables
	int n, processIndex;
	struct dirent **dirlist;
	// Scan/List directories
	if((n = scandir(inputDir, &dirlist, 0, alphasort)) < 0)
		return -1;

	char currDir[256], baseDir[256], dirPath[512];
	processList processes = NULL;
	processList processesCountries = NULL;
	dirList countries = NULL;
  	strcpy(baseDir, inputDir);
  	strcat(baseDir, "/");
	// For every subdirectory
	for(int i =  0 ; i < n ; i++) {

		processIndex = i % numMonitors;
		// Save current directory name
		strcpy(currDir, dirlist[i]->d_name);
		free(dirlist[i]);
		// Skip uneeded directories
		if(!strcmp(currDir, ".") || !strcmp(currDir, ".."))
			continue;

		strcpy(dirPath, baseDir);
		strcat(dirPath, currDir);

		if(countries == NULL)
			countries = dirListCreate(currDir);
		else
			countries = dirListInsert(countries, currDir);

		if(processesCountries == NULL)
			processesCountries = processListCreate(currDir, processIndex);
		else 
			processesCountries = processListInsert(processesCountries, currDir, processIndex);

		if(processes == NULL)
			processes = processListCreate(dirPath, processIndex);
		else 
			processes = processListInsert(processes, dirPath, processIndex);

  	}

  	free(dirlist);

	int *childPIDS = (int *) malloc(numMonitors * sizeof(int));
	int arraySize = -1;
	char **pathsArray;
	// Create pipes and processes
	for(processIndex = 0 ; processIndex < numMonitors ; processIndex++) {
		// Get this process's paths
		pathsArray = processListGet(processes, processIndex, &arraySize);

		if(pathsArray == 0) {

			printf("ERROR: More processes than countries given.");

			for(int i = 0 ; i < processIndex ; i++)
				kill(childPIDS[i], SIGKILL);

			free(childPIDS);

			dirListDestroy(countries);
			processListDestroy(processesCountries);
			processListDestroy(processes);

			return 0;

		}
		// Fit all args in an array
		char **args = fillArgs(BASE_PORT + processIndex, numThreads, socketBufferSize, cyclicBufferSize, bloomSize, pathsArray, arraySize);

		if((childPIDS[processIndex] = fork()) < 0)
			return -1;
		// Overlay process with monitor executable, passing the correct arguments
		if(childPIDS[processIndex] == 0) {

			execv("./monitorServer", args);	
			// This will only run if the exec fails
			return -1;

		}

		free(pathsArray);
		destroyArgs(args);

	}

	int *sockets = (int *) malloc(numMonitors * sizeof(int));

	char host[1024];
	host[1023] = '\0';
	gethostname(host, 1023);

    struct sockaddr_in server;
    struct sockaddr *serverptr = (struct sockaddr *) &server;
    struct hostent *localhost = gethostbyname(host);

    server.sin_family = AF_INET;       /* Internet domain */
    memcpy(&server.sin_addr, localhost->h_addr, localhost->h_length);

	for(processIndex = 0 ; processIndex < numMonitors ; processIndex++) {

		if((sockets[processIndex] = socket(PF_INET, SOCK_STREAM, 0)) < 0)
			return -1;
		// Make port insantly reusable
		setsockopt(sockets[processIndex], SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

	    server.sin_port = htons(BASE_PORT + processIndex);      /* The given port */
		// Try connecting until you succeed
		while(connect(sockets[processIndex], serverptr, sizeof(server)) < 0);

	}

  	fd_set master, master2;
  	int maxFD = -1, buffIndex;
  	struct timeval tv;

  	tv.tv_sec = 0;
	tv.tv_usec = 0;
  	FD_ZERO(&master);

  	char **buff = (char **) malloc(numMonitors * sizeof(char *));
  	int *buffIndexes = (int *) malloc(numMonitors * sizeof(int));
  	bloomList *blooms = (bloomList *) malloc(numMonitors * sizeof(bloomList));
  	// Write buffer, if not empty/written already
  	for(processIndex = 0 ; processIndex < numMonitors ; processIndex++) {
	  	// Allocate buffer for each process
		buff[processIndex] = (char *) malloc(bufferSize);
		// Initialize buffer as empty
	    for(buffIndex = 0 ; buffIndex < bufferSize ; buffIndex++) 
	        buff[processIndex][buffIndex] = '.';

	    FD_SET(sockets[processIndex], &master);

	    if(sockets[processIndex] > maxFD)
	    	maxFD = sockets[processIndex];

	    blooms[processIndex] = NULL;

	    buffIndexes[processIndex] = 0;

	}
	
	char *v, *b;
	int ready = 0;
	int *done = (int *) malloc(numMonitors * sizeof(int));
	for(int processIndex = 0 ; processIndex < numMonitors ; processIndex++)
		done[processIndex] = 0;
	// Read bloom filters from children
	while(ready != numMonitors) {
		
		master2 = master;
		// Check which are ready
		if(select(maxFD + 1 , &master2, NULL, NULL, &tv) < 0)
			return -1;
		// For every child process
		for(int processIndex = 0 ; processIndex < numMonitors ; processIndex++) {
			// Check if ready to read
			if(FD_ISSET(sockets[processIndex], &master2)) {
				// Skip if this stage has ended
				if(done[processIndex] == 1)
					continue;

				if(buffIndexes[processIndex] >= bufferSize)	// Buffer might have been entirely read on last iteration so reset index to enter while loop
					buffIndexes[processIndex] = 0;
				// Read entire buffer
				while((buffIndexes[processIndex] < bufferSize) && (buff[processIndex][buffIndexes[processIndex]] != '|')) {
					// Read virus followed by the bloom filter
					v = readString('~', sockets[processIndex], buff[processIndex], &buffIndexes[processIndex], bufferSize, 0);
					if(!strcmp(v, "|"))	{ // This is a really rare case where the | terminating char is at index 0 of a buffer so this reads it

						free(v);

						break;

					}
					b = readString('~', sockets[processIndex], buff[processIndex], &buffIndexes[processIndex], bufferSize, bloomSize);
					// Add to bloom filter list
					if(blooms[processIndex] == NULL)
						blooms[processIndex] = bloomListCreate(v, b);
					else
						blooms[processIndex] = bloomListInsert(blooms[processIndex], v, b);

					free(v);

				}
				// Mark one more process as done if we encounter "|" character
				if(buffIndexes[processIndex] < bufferSize) {

					if(buff[processIndex][buffIndexes[processIndex]] == '|') {

						done[processIndex] = 1;
						ready++;

						continue;

					}

				}

			}

		}

	}
	// Pass | character on buffer 
	for(processIndex = 0 ; processIndex < numMonitors ; processIndex++)
		buffIndexes[processIndex]++;

	// return -2;
	char *temp, cmd[1000];
	reqList requests = NULL;

	struct pollfd commandPoller;
	commandPoller.fd = 0; // stdin poll
	commandPoller.events = POLLIN;
	// printf("parent pid: %d\n", getpid());
	int printFlag = 0, accepted = 0, rejected = 0;
	// Get commands from user and handle signals gotten
	do {

		if(printFlag == 0) {

			printf("travelMonitor> "); 
			fflush(stdout);
			printFlag = 1;

		}

		if(poll(&commandPoller, 1, 0) < 0)
			return -1;

		if((commandPoller.revents & POLLIN) == POLLIN) {

			printFlag = 0;
			// Read the Command
			scanf("%[^\n]%*c", cmd);
			// Get the first word of the complete command, to determine the category
			temp = subStr(cmd, 1, ' ');
			/* Decide what the Command is and act accordingly 
				-- every single one of these outside ifs 
				correspond to a different in-game command     */
			if(!strcmp(temp, "/travelRequest")) {
				// /travelRequest 7216 26-10-1000 Afghanistan Greece Tuberculosis
				// /travelRequest 7216 26-10-2005 Afghanistan Greece Tuberculosis
				if(partCount(cmd) == 6) { 
					// Assign each parameter of insert command to a variable for insertion
					char *id = subStr(cmd, 2, ' ');
					char *date = subStr(cmd, 3, ' ');
					char *countryFrom = subStr(cmd, 4, ' ');
					char *countryTo = subStr(cmd, 5, ' ');
					char *virus = subStr(cmd, 6, ' ');

					int r = bloomListSearch(blooms[processListSearch(processesCountries, countryFrom)], virus, id);
					int req;
					// Act accordingl to bloom filter answer
					if(r) {	// Bloom no is definitive no

						printf("REQUEST REJECTED – YOU ARE NOT VACCINATED\n");

						req = 0;
						rejected++;

					} else {	// Check vaccination date

					  	int processID = processListSearch(processesCountries, countryFrom);

					  	for(buffIndex = 0 ; buffIndex < bufferSize ; buffIndex++)
	    					buff[processID][buffIndex] = '.';
	    				// Send request type
						writeString("request~", processID, sockets, numMonitors, buff, bufferSize, 0);

					  	char tmp[99999];
					  	strcpy(tmp, id);
					  	strcat(tmp, "~");
					  	// Send id
					  	writeString(tmp, processID, sockets, numMonitors, buff, bufferSize, 0);

					  	strcpy(tmp, virus);
					  	strcat(tmp, "~");
					  	// Send virus
					  	writeString(tmp, processID, sockets, numMonitors, buff, bufferSize, 0);

					  	if(buff[processID][0] != '.')
					  		write(sockets[processID], buff[processID], bufferSize);

					  	for(buffIndex = 0 ; buffIndex < bufferSize ; buffIndex++)
	    					buff[processID][buffIndex] = '.';
	    				//
						char *res;
						int retval;
						struct pollfd poller;
						poller.fd = sockets[processID];
						poller.events = POLLIN;
						// Poll for answer
						while(1) {

		    				retval = poll(&poller, 1, 0);

							if(retval < 0)
								return -1;

							if((poller.revents & POLLIN) == POLLIN) {
								// Get result
						  		res = readString('~', sockets[processID], buff[processID], &buffIndexes[processID], bufferSize, 0);

						  		break;
						  	
						  	}

					  	}

					  	// If answer starts with no, citizen has not been vaccinated
					  	if(res[0] == 'N' && res[1] == 'O') {

					  		printf("REQUEST REJECTED – YOU ARE NOT VACCINATED\n");

					  		req = 0;
					  		rejected++;

					  	} else {	// Answer starts with yes

					  		char dateVac[11];
					  		// Copy date over
					  		memcpy(dateVac, &res[3], 11);
					  		// Check 6 month period and proceed accordingly
					  		if(!checkDates(dateVac, date)) {	

					  			printf("REQUEST ACCEPTED – HAPPY TRAVELS\n");

					  			req = 1;
					  			accepted++;

					  		} else {

					  			printf("REQUEST REJECTED – YOU WILL NEED ANOTHER VACCINATION BEFORE TRAVEL DATE\n");

					  			req = 0;
					  			rejected++;

					  		}

					  	}

					  	free(res);

					}
					// Build requests list for stats
					if(requests == NULL)
				  		requests = reqListCreate(date, virus, countryTo, req);
				  	else
				  		requests = reqListInsert(requests, date, virus, countryTo, req);

					// Free all space reserved
					free(id);
					free(date);
					free(countryFrom);
					free(countryTo);
					free(virus);

				} else {

					printf("Correct usage: </travelRequest citizenID date countryFrom countryTo virusName>\n");

				}

			} else if(!strcmp(temp, "/travelStats")) {

				if(partCount(cmd) == 5) { // Has country

					char *virus = subStr(cmd, 2, ' ');
					char *date1 = subStr(cmd, 3, ' ');
					char *date2 = subStr(cmd, 4, ' ');
					char *country = subStr(cmd, 5, ' ');

					int acc = 0, rej = 0;

					reqListSearch(requests, date1, date2, virus, country, &acc, &rej);

					printf("TOTAL REQUESTS %d\n", acc + rej);
					printf("ACCEPTED %d\n", acc);
					printf("REJECTED %d\n", rej);

					accepted += acc;
					rejected += rej;

					free(virus);
					free(date1);
					free(date2);
					free(country);

				} else if(partCount(cmd) == 4) { // No country

					char *virus = subStr(cmd, 2, ' ');
					char *date1 = subStr(cmd, 3, ' ');
					char *date2 = subStr(cmd, 4, ' ');

					int acc = 0, rej = 0;

					reqListSearch(requests, date1, date2, virus, NULL, &acc, &rej);

					printf("TOTAL REQUESTS %d\n", acc + rej);
					printf("ACCEPTED %d\n", acc);
					printf("REJECTED %d\n", rej);

					accepted += acc;
					rejected += rej;

					free(virus);
					free(date1);
					free(date2);

				} else {

					printf("Correct usage: </travelStats virusName date1 date2 [country]>\n");

				}

			} else if(!strcmp(temp, "/addVaccinationRecords")) {
				// /addVaccinationRecords Afghanistan
				if(partCount(cmd) == 2) { 
					// Get arguments of cmd
					char *country = subStr(cmd, 2, ' ');
					// Identify process corresponding to this couuntry
					int processID = processListSearch(processesCountries, country);
					// Clear buffer
					for(buffIndex = 0 ; buffIndex < bufferSize ; buffIndex++) 
						buff[processID][buffIndex] = '.';
					// Send request type
					writeString("add~", processID, sockets, numMonitors, buff, bufferSize, 0);
				  	// Write buffer, if not empty/written already
					if(buff[processID][0] != '.')
						write(sockets[processID], buff[processID], bufferSize);

					for(buffIndex = 0 ; buffIndex < bufferSize ; buffIndex++) 
						buff[processID][buffIndex] = '.';
					// Start reading data
					ready = 0;

					for(buffIndex = 0 ; buffIndex < bufferSize ; buffIndex++)
	    				buff[processID][buffIndex] = '.';

					int retval;
					struct pollfd poller;
					poller.fd = sockets[processID];
					poller.events = POLLIN;
					// Read new blooms -- almost same process as above
					while(1) {

	    				retval = poll(&poller, 1, 0);

						if(retval < 0)
							return -1;

						if((poller.revents & POLLIN) == POLLIN) {

							if(buffIndexes[processID] >= bufferSize)	// Buffer might have been entirely read on last iteration so reset index to enter while loop
								buffIndexes[processID] = 0;

							while((buffIndexes[processID] < bufferSize) && (buff[processID][buffIndexes[processID]] != '|')) {

								v = readString('~', sockets[processID], buff[processID], &buffIndexes[processID], bufferSize, 0);

								if(!strcmp(v, "none"))
									break;

								if(!strcmp(v, "|"))	{ // This is a really rare case where the | terminating char is at index 0 of a buffer so this reads it

									free(v);
									v = NULL;

									break;

								}
								b = readString('~', sockets[processID], buff[processID], &buffIndexes[processID], bufferSize, bloomSize);

								if(blooms[processID] == NULL)
									blooms[processID] = bloomListCreate(v, b);
								else
									blooms[processID] = bloomListInsert(blooms[processID], v, b);

								free(v);
								v = NULL;

							}

							if(v != NULL && !strcmp(v, "none")) 
								break;

							if(buff[processID][buffIndexes[processID]] == '|')
								break;

						}

					}

					if(v != NULL && !strcmp(v, "none")) {

						free(v);
						printf("There are no new files added.\n");

					} else {
						// Pass | character on buffer 
						buffIndexes[processID]++;

					}

					free(country);

				} else {

					printf("Correct usage: </addVaccinationRecords country>\n");

				}

			} else if(!strcmp(temp, "/searchVaccinationStatus")) {
				// /searchVaccinationStatus 20211
				if(partCount(cmd) == 2) { 

					char *id = subStr(cmd, 2, ' ');

					for(int i = 0 ; i < numMonitors ; i++)
					    for(buffIndex = 0 ; buffIndex < bufferSize ; buffIndex++) 
					        buff[i][buffIndex] = '.';
					// Send request type
					writeString("search~", -1, sockets, numMonitors, buff, bufferSize, 0);
					// Send id we are interested in
				  	char tmp[99999];
				  	strcpy(tmp, id);
				  	strcat(tmp, "~");

				  	writeString(tmp, -1, sockets, numMonitors, buff, bufferSize, 0);
					// Find the process responsible for this id
					fd_set master, master2;
				  	int maxFD = -1;
				  	struct timeval tv;

				  	tv.tv_sec = 0;
					tv.tv_usec = 0;
				  	FD_ZERO(&master);
				  	// Write buffer, if not empty/written already
				  	for(int i = 0 ; i < numMonitors ; i++) {

					  	if(buff[i][0] != '.')
					    	write(sockets[i], buff[i], bufferSize);

					    for(buffIndex = 0 ; buffIndex < bufferSize ; buffIndex++) 
					        buff[i][buffIndex] = '.';

					    buffIndexes[i] = 0;

					    FD_SET(sockets[i], &master);

					    if(sockets[i] > maxFD)
					    	maxFD = sockets[i];

					}
					// Read answers
					char *answer;
					int done = 0;
					int processID = -1;
					// Get result from everyone until you find a yes
					while(done != numMonitors) {
						
						master2 = master;

						if(select(maxFD + 1 , &master2, NULL, NULL, &tv) < 0)
							return -1;

						for(processIndex = 0 ; processIndex < numMonitors ; processIndex++) {

							if(FD_ISSET(sockets[processIndex], &master2)) {

								if(buff[processIndex][0] == 'n')
									continue;	// This doesnt go on first readString

								answer = readString('~', sockets[processIndex], buff[processIndex], &buffIndexes[processIndex], bufferSize, 0);

								if(!strcmp(answer, "yes")) {

									// Clear all other buffers and break loop
									for(int i = 0 ; i < numMonitors ; i++) 
										if(i != processIndex)
											for(buffIndex = 0 ; buffIndex < bufferSize ; buffIndex++) 
							        			buff[i][buffIndex] = '.';

									processID = processIndex;

							    }

							    free(answer);

								done++;

							}

						}

					}

					if(processID == -1) {	// If everyone answered no

						printf("Identifier not available\n");

						free(temp);
						free(id);

						continue;

					}
					
					int dataRead = 0;
					char *name, *surname, *country, *age, *res;
					// Get results and print formatted message
					printf("%s", id);
					// If the rest of the info was in the buffer previously
					while((buffIndexes[processID] < bufferSize) && (buff[processID][buffIndexes[processID]] != '.') && (buff[processID][buffIndexes[processID]] != '|')) {

						switch(dataRead) {

							case 0:
					  			name = readString('~', sockets[processID], buff[processID], &buffIndexes[processID], bufferSize, 0);
					  			printf(" %s", name);
					  			free(name);
					  			break;
					  		case 1:
					  			surname = readString('~', sockets[processID], buff[processID], &buffIndexes[processID], bufferSize, 0);
					  			printf(" %s", surname);
					  			free(surname);
					  			break;
					  		case 2:
					  			country = readString('~', sockets[processID], buff[processID], &buffIndexes[processID], bufferSize, 0);
					  			printf(" %s\n", country);
					  			free(country);
					  			break;
					  		case 3:
					  			age = readString('~', sockets[processID], buff[processID], &buffIndexes[processID], bufferSize, 0);
					  			printf("AGE %s\n", age);
					  			free(age);
					  			break;
					  		default:
					  			res = readString('~', sockets[processID], buff[processID], &buffIndexes[processID], bufferSize, 0);
					  			printf("%s\n", res);
					  			free(res);

				  		}

				  		dataRead++;

				  	}
					//
					int retval;
					struct pollfd poller;
					poller.fd = sockets[processID];
					poller.events = POLLIN;

					if(buffIndexes[processID] >= bufferSize) {

				  		buffIndexes[processID] = 0;
				  		read(sockets[processID], buff[processID], bufferSize);

				  	}
					// Check if you have leftover buffers to read and do the same
					while(buff[processID][buffIndexes[processID]] != '|') {

	    				retval = poll(&poller, 1, 0);

						if(retval < 0)
							return -1;

						if((poller.revents & POLLIN) == POLLIN) {

							while((buffIndexes[processID] < bufferSize) && (buff[processID][buffIndexes[processID]] != '|')) {
								// Switch for correct output format
								switch(dataRead) {

									case 0:
							  			name = readString('~', sockets[processID], buff[processID], &buffIndexes[processID], bufferSize, 0);
							  			printf("%s", name);
							  			free(name);
							  			break;
							  		case 1:
							  			surname = readString('~', sockets[processID], buff[processID], &buffIndexes[processID], bufferSize, 0);
							  			printf(" %s", surname);
							  			free(surname);
							  			break;
							  		case 2:
							  			country = readString('~', sockets[processID], buff[processID], &buffIndexes[processID], bufferSize, 0);
							  			printf(" %s\n", country);
							  			free(country);
							  			break;
							  		case 3:
							  			age = readString('~', sockets[processID], buff[processID], &buffIndexes[processID], bufferSize, 0);
							  			printf("AGE %s\n", age);
							  			free(age);
							  			break;
							  		default:
							  			res = readString('~', sockets[processID], buff[processID], &buffIndexes[processID], bufferSize, 0);
							  			printf("%s\n", res);
							  			free(res);

						  		}

						  		dataRead++;

						  	}
					  	
					  	}

					  	if(buffIndexes[processID] >= bufferSize) {

					  		buffIndexes[processID] = 0;
					  		read(sockets[processID], buff[processID], bufferSize);

					  	}

				  	}
				  	// Read expected | and move buffer
				  	if(buff[processID][buffIndexes[processID]] == '|')
				  		buffIndexes[processID]++;

					free(id);

				} else {

					printf("Correct usage: </searchVaccinationStatus citizenID>\n");

				}

			} else if(!strcmp(temp, "/exit")) {
				// Clear buffer
				for(int i = 0 ; i < numMonitors ; i++)
				    for(buffIndex = 0 ; buffIndex < bufferSize ; buffIndex++) 
				        buff[i][buffIndex] = '.';
				// Send request type
				writeString("exit~", -1, sockets, numMonitors, buff, bufferSize, 0);
			  	// Write buffer, if not empty/written already
			  	for(int i = 0 ; i < numMonitors ; i++) {

				  	if(buff[i][0] != '.')
				    	write(sockets[i], buff[i], bufferSize);

				    for(buffIndex = 0 ; buffIndex < bufferSize ; buffIndex++) 
				        buff[i][buffIndex] = '.';

				}
				// END OF SEND EXIT

				// for(int i = 0 ; i < numMonitors ; i++)
				// kill(childPIDS[i], SIGKILL);	// Send SIGKILL to all children

				// int status;
				// while(wait(&status) > 0); // Wait for all children to finish
				// Create log file
				char logfile[100];
				char *pidString = itoa2(getpid());
				strcpy(logfile, "log_file.");
				strcat(logfile, pidString);

				FILE *fp = fopen(logfile, "w");
				// Print countries
				for(dirList countriestmp = countries ; countriestmp != NULL ; countriestmp = countriestmp->link)
					fprintf(fp, "%s\n", countriestmp->dir);
				// Print request counts
				fprintf(fp, "TOTAL TRAVEL REQUESTS %d\n", accepted + rejected);
				fprintf(fp, "ACCEPTED %d\n", accepted);
				fprintf(fp, "REJECTED %d\n", rejected);

				fclose(fp);
				free(pidString);

				// Free all memory and handle as SIGQUIT
				for(int i = 0 ; i < numMonitors ; i++) {

					free(buff[i]);

					bloomListDestroy(blooms[i]);

					close(sockets[i]);

				}

				free(childPIDS);
				free(sockets);
				free(buff);
				free(blooms);
				free(buffIndexes);
				free(done);
				free(temp);

				processListDestroy(processes);
				processListDestroy(processesCountries);
			  	dirListDestroy(countries);
				reqListDestroy(requests);

				return 0;

			} else { printf("Uknown Command!\n"); }
			// Free the space used for command argument
			free(temp);

		}

	} while(1);

	return 1;

}