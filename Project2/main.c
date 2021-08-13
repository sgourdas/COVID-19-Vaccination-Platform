#include "functions.h"

#define YES 1;
#define NO 0;

int bufferSize;
int bloomSize;
int SIGCHLDflag = 0;
int SIGINT_SIGQUITflag = 0;
// TODO unlink and close pipes at end
int main(int argc, char **argv) {

	setupSIGINT_SIGQUIT_PARENT();
	setupSIGCHLD();

	int accepted = 0 , rejected = 0;

	int numMonitors;
	char *inputDir;

	if(configure(argc, argv, &numMonitors, &inputDir)) {	// Get arguments

		printf("Incorrect arguments.\n");

		return -1;

	}

	char *index = NULL;
	int *childPIDS = (int *) malloc(numMonitors * sizeof(int));
	int **fd = (int **) malloc(numMonitors * sizeof(int *));
	// pipeF is for parent to child -- pipeS is for child to parent
	char **pipeF = (char **) malloc(numMonitors * sizeof(char *));
	char **pipeS = (char **) malloc(numMonitors * sizeof(char *));
	// Create pipes and processes
	for(int i = 0 ; i < numMonitors ; i++) {

		fd[i] = (int *) malloc(2 * sizeof(int));
		// Creating pipes and storing names
		index = itoa2(i);
		pipeF[i] = (char *) malloc(strlen("pipeF") + strlen(index) + 1); 
		strcpy(pipeF[i], "pipeF");
		strcat(pipeF[i], index);
		unlink(pipeF[i]);
		if(mkfifo(pipeF[i], 0666) < 0)
			return -1;
		pipeS[i] = (char *) malloc(strlen("pipeF") + strlen(index) + 1); 
		strcpy(pipeS[i], "pipeS");
		strcat(pipeS[i], index);
		unlink(pipeS[i]);
		if(mkfifo(pipeS[i], 0666) < 0)
			return -1;

		free(index);

		if((childPIDS[i] = fork()) < 0)
			return -1;

		if(childPIDS[i] == 0) {
			// Overlay process with monitor executable, passing the correct arguments
			// char *temp = itoa2(bufferSize);
			execl("./Monitor", "./Monitor", pipeF[i], pipeS[i], NULL);	
			// This will only run if the exec fails
			return -1;

		}

	}
	// Initialize variables
	int n;
	struct dirent **dirlist;
	char **buff = (char **) malloc(numMonitors * sizeof(char *));
	char tempBuff[FIRST_MSG];

	memset(tempBuff, 0, FIRST_MSG);
	itoa3(bufferSize, tempBuff);
	// Open pipes and write buffersize to children
	for(int i = 0 ; i < numMonitors ; i++) {
		// Open pipes
		if((fd[i][0] = open(pipeF[i], O_WRONLY)) < 0)
			return -1;
		// Open pipes
		if((fd[i][1] = open(pipeS[i], O_RDONLY)) < 0)
			return -1;
		// Allocate buffer for each process
		buff[i] = (char *) malloc(bufferSize);
		// Init buffers
		for(int j = 0 ; j < bufferSize ; j++)
		    buff[i][j] = '.';
		// Write buffer, for process to get the bufferSize
		write(fd[i][0], tempBuff, FIRST_MSG);

	}
	// Scan/List directories
	if((n = scandir(inputDir, &dirlist, 0, alphasort)) < 0)
		return -1;

	char currDir[1024];
	memset(currDir, 0, 1024);
	int buffIndex, processIndex;
	processList processes = NULL;
	dirList countries = NULL;
	// For every subdirectory
	for(int i =  0 ; i < n ; i++) {

		processIndex = i % numMonitors;
		// Save current directory name
		strcpy(currDir, dirlist[i]->d_name);
		free(dirlist[i]);
		// Skip uneeded directories
		if(!strcmp(currDir, ".") || !strcmp(currDir, ".."))
			continue;

		if(countries == NULL)
			countries = dirListCreate(currDir);
		else
			countries = dirListInsert(countries, currDir);

		if(processes == NULL)
			processes = processListCreate(currDir, processIndex);
		else 
			processes = processListInsert(processes, currDir, processIndex);

		strcat(currDir, "~");
		writeString(currDir, processIndex, fd, numMonitors, buff, bufferSize, 0);

  	}

  	free(dirlist);
  	// Insert | character to signal end of directories to parse
  	char *term = "|";
  	writeString(term, -1, fd, numMonitors, buff, bufferSize, 0);
  	// Write bloom filter size
  	char *tempBloom = itoa2(bloomSize);
  	strcat(tempBloom, "~");
  	writeString(tempBloom, -1, fd, numMonitors, buff, bufferSize, 0);
  	free(tempBloom);
  	// Write input directory name
  	char tmp[99999];
  	strcpy(tmp, inputDir);
  	strcat(tmp, "~");
  	writeString(tmp, -1, fd, numMonitors, buff, bufferSize, 0);

  	fd_set master, master2;
  	int maxFD = -1;
  	struct timeval tv;

  	tv.tv_sec = 0;
	tv.tv_usec = 0;
  	FD_ZERO(&master);

  	bloomList *blooms = (bloomList *) malloc(numMonitors * sizeof(bloomList));
  	int *buffIndexes = (int *) malloc(numMonitors * sizeof(int));
  	// Write buffer, if not empty/written already
  	for(int i = 0 ; i < numMonitors ; i++) {

	  	if(buff[i][0] != '.')
	    	write(fd[i][0], buff[i], bufferSize);

	    for(buffIndex = 0 ; buffIndex < bufferSize ; buffIndex++) 
	        buff[i][buffIndex] = '.';

	    FD_SET(fd[i][1], &master);

	    if(fd[i][1] > maxFD)
	    	maxFD = fd[i][1];

	    blooms[i] = NULL;

	    buffIndexes[i] = 0;

	}
	
	char *v, *b;
	int *done = (int *) malloc(numMonitors * sizeof(int));
	for(int processIndex = 0 ; processIndex < numMonitors ; processIndex++)
		done[processIndex] = 0;
	int ready = 0;
	// Read bloom filters from children
	while(ready != numMonitors) {
		
		master2 = master;
		// Check which are ready
		if(select(maxFD + 1 , &master2, NULL, NULL, &tv) < 0)
			return -1;
		// For every child process
		for(int processIndex = 0 ; processIndex < numMonitors ; processIndex++) {
			// Check if ready to read
			if(FD_ISSET(fd[processIndex][1], &master2)) {
				// Skip if this stage has ended
				if(done[processIndex] == 1)
					continue;

				if(buffIndexes[processIndex] >= bufferSize)	// Buffer might have been entirely read on last iteration so reset index to enter while loop
					buffIndexes[processIndex] = 0;
				// Read entire buffer
				while((buffIndexes[processIndex] < bufferSize) && (buff[processIndex][buffIndexes[processIndex]] != '|')) {
					// Read virus followed by the bloom filter
					v = readString('~', fd[processIndex][1], buff[processIndex], &buffIndexes[processIndex], bufferSize, 0);
					if(!strcmp(v, "|"))	{ // This is a really rare case where the | terminating char is at index 0 of a buffer so this reads it

						free(v);

						break;

					}
					b = readString('~', fd[processIndex][1], buff[processIndex], &buffIndexes[processIndex], bufferSize, bloomSize);
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
	for(int processIndex = 0 ; processIndex < numMonitors ; processIndex++)
		buffIndexes[processIndex]++;

	char *temp, cmd[1000];
	reqList requests = NULL;

	struct pollfd commandPoller;
	commandPoller.fd = 0; // stdin poll
	commandPoller.events = POLLIN;
	// printf("parent pid: %d\n", getpid());
	int printFlag = 0;
	// Get commands from user and handle signals gotten
	do {
		// If a child process got killed
		if(SIGCHLDflag != 0) {
			// Find the child that was affacted, based on pid
			for(int i = 0 ; i < numMonitors ; i++) {

				if(SIGCHLDflag == childPIDS[i]) {

					SIGCHLDflag = 0;

					printf("\n");
					// Fork a new child
					if((childPIDS[i] = fork()) < 0)
						return -1;
					// Overlay new child
					if(childPIDS[i] == 0) {
						// Overlay process with monitor executable, passing the correct arguments
						execl("./Monitor", "./Monitor", pipeF[i], pipeS[i], NULL);	
						// This will only run if the exec fails
						return -1;

					}
					// Open pipes
					if((fd[i][0] = open(pipeF[i], O_WRONLY)) < 0)
						return -1;
					// Open pipes
					if((fd[i][1] = open(pipeS[i], O_RDONLY)) < 0)
						return -1;
					// Init buffers
					for(int j = 0 ; j < bufferSize ; j++)
					    buff[i][j] = '.';

					write(fd[i][0], tempBuff, FIRST_MSG);

					for(dirList tmpcountries = countries ; tmpcountries != NULL ; tmpcountries = tmpcountries->link ) {

						if(processListSearch(processes, tmpcountries->dir) == i) {

							char tmpCountry[99999];
							strcpy(tmpCountry, tmpcountries->dir);
							strcat(tmpCountry, "~");
							writeString(tmpCountry, i, fd, numMonitors, buff, bufferSize, 0);

						}

					}

  					writeString("|", i, fd, numMonitors, buff, bufferSize, 0);
  					// Write bloom filter size
				  	char *tempBloom = itoa2(-bloomSize);
				  	strcat(tempBloom, "~");
				  	writeString(tempBloom, -1, fd, numMonitors, buff, bufferSize, 0);
				  	free(tempBloom);
				  	// Write input directory name
				  	char tmp[99999];
				  	strcpy(tmp, inputDir);
				  	strcat(tmp, "~");
				  	writeString(tmp, -1, fd, numMonitors, buff, bufferSize, 0);

				  	if(buff[i][0] != '.')
	    				write(fd[i][0], buff[i], bufferSize);

					break;

				}

			}

		} else if(SIGINT_SIGQUITflag != 0) {

			SIGINT_SIGQUITflag = 0;

			for(int i = 0 ; i < numMonitors ; i++)
				kill(childPIDS[i], SIGKILL);	// Send SIGKILL to all children

			int status;

			while(wait(&status) > 0); // Wait for all children to finish
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

				close(fd[i][0]);
				close(fd[i][1]);

				unlink(pipeF[i]);
				unlink(pipeS[i]);

				free(fd[i]);
				free(pipeF[i]);
				free(pipeS[i]);
				free(buff[i]);

				bloomListDestroy(blooms[i]);

			}

			free(childPIDS);
			free(fd);
			free(pipeF);
			free(pipeS);
			free(buff);
			free(blooms);
			free(buffIndexes);
			free(done);

			processListDestroy(processes);
		  	dirListDestroy(countries);
			reqListDestroy(requests);

			return 0;

		}

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

				if(partCount(cmd) == 6) { 
					// Assign each parameter of insert command to a variable for insertion
					char *id = subStr(cmd, 2, ' ');
					char *date = subStr(cmd, 3, ' ');
					char *countryFrom = subStr(cmd, 4, ' ');
					char *countryTo = subStr(cmd, 5, ' ');
					char *virus = subStr(cmd, 6, ' ');
					
					int r = bloomListSearch(blooms[processListSearch(processes, countryFrom)], virus, id);
					int req;
					// Act accordingl to bloom filter answer
					if(r) {	// Bloom no is definitive no

						printf("REQUEST REJECTED – YOU ARE NOT VACCINATED\n");

						req = 0;
						rejected++;

					} else {	// Check vaccination date

					  	int processID = processListSearch(processes, countryFrom);

					  	for(buffIndex = 0 ; buffIndex < bufferSize ; buffIndex++)
	    					buff[processID][buffIndex] = '.';
	    				// Send request type
						writeString("request~", processID, fd, numMonitors, buff, bufferSize, 0);

					  	char tmp[99999];
					  	strcpy(tmp, id);
					  	strcat(tmp, "~");
					  	// Send id
					  	writeString(tmp, processID, fd, numMonitors, buff, bufferSize, 0);

					  	strcpy(tmp, virus);
					  	strcat(tmp, "~");
					  	// Send virus
					  	writeString(tmp, processID, fd, numMonitors, buff, bufferSize, 0);

					  	if(buff[processID][0] != '.')
					  		write(fd[processID][0], buff[processID], bufferSize);

					  	for(buffIndex = 0 ; buffIndex < bufferSize ; buffIndex++)
	    					buff[processID][buffIndex] = '.';
	    				//
						char *res;
						int retval;
						struct pollfd poller;
						poller.fd = fd[processID][1];
						poller.events = POLLIN;
						// Poll for answer
						while(1) {

		    				retval = poll(&poller, 1, 0);

							if(retval < 0)
								return -1;

							if((poller.revents & POLLIN) == POLLIN) {
								// Get result
						  		res = readString('~', fd[processID][1], buff[processID], &buffIndexes[processID], bufferSize, 0);

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

				if(partCount(cmd) == 2) { 

					char *country = subStr(cmd, 2, ' ');
					// Identify process corresponding to this couuntry
					int processID = processListSearch(processes, country);
					// Send SIGUSR1 to that process
					kill(childPIDS[processID], SIGUSR1);
					
					ready = 0;

					for(buffIndex = 0 ; buffIndex < bufferSize ; buffIndex++)
	    				buff[processID][buffIndex] = '.';

					int retval;
					struct pollfd poller;
					poller.fd = fd[processID][1];
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

								v = readString('~', fd[processID][1], buff[processID], &buffIndexes[processID], bufferSize, 0);

								if(!strcmp(v, "none"))
									break;


								if(!strcmp(v, "|"))	{ // This is a really rare case where the | terminating char is at index 0 of a buffer so this reads it

									free(v);
									v = NULL;

									break;

								}
								b = readString('~', fd[processID][1], buff[processID], &buffIndexes[processID], bufferSize, bloomSize);

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

				if(partCount(cmd) == 2) { 

					char *id = subStr(cmd, 2, ' ');

					for(int i = 0 ; i < numMonitors ; i++)
					    for(buffIndex = 0 ; buffIndex < bufferSize ; buffIndex++) 
					        buff[i][buffIndex] = '.';
					// Send request type
					writeString("search~", -1, fd, numMonitors, buff, bufferSize, 0);
					// Send id we are interested in
				  	char tmp[99999];
				  	strcpy(tmp, id);
				  	strcat(tmp, "~");

				  	writeString(tmp, -1, fd, numMonitors, buff, bufferSize, 0);
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
					    	write(fd[i][0], buff[i], bufferSize);

					    for(buffIndex = 0 ; buffIndex < bufferSize ; buffIndex++) 
					        buff[i][buffIndex] = '.';

					    buffIndexes[i] = 0;

					    FD_SET(fd[i][1], &master);

					    if(fd[i][1] > maxFD)
					    	maxFD = fd[i][1];

					}
					// Read answers
					char *answer;
					int done = 0;
					int processID = -1;
					int flag = 0;
					// Get result from everyone until you find a yes
					while(done != numMonitors) {
						
						master2 = master;

						if(select(maxFD + 1 , &master2, NULL, NULL, &tv) < 0)
							return -1;

						for(int processIndex = 0 ; processIndex < numMonitors ; processIndex++) {

							if(FD_ISSET(fd[processIndex][1], &master2)) {

								if(buff[processIndex][0] == 'n')
									continue;

								answer = readString('~', fd[processIndex][1], buff[processIndex], &buffIndexes[processIndex], bufferSize, 0);

								if(!strcmp(answer, "yes")) {

									// Clear all other buffers and break loop
									for(int i = 0 ; i < numMonitors ; i++) 
										if(i != processIndex)
											for(buffIndex = 0 ; buffIndex < bufferSize ; buffIndex++) 
							        			buff[i][buffIndex] = '.';

									flag = 1; 

									free(answer);

							       	break;

							    }

							    free(answer);

								done++;

							}

						}

						if(flag)
							break;

					}

					if((done == numMonitors) && flag != 0) {	// If everyone answered no

						printf("Identifier not available\n");

						continue;

					}

					processID = processIndex;

					int dataRead = 0;
					char *name, *surname, *country, *age, *res;
					// Get results and print formatted message
					printf("%s", id);
					// If the rest of the info was in the buffer previously
					while((buffIndexes[processID] < bufferSize) && (buff[processID][buffIndexes[processID]] != '.') && (buff[processID][buffIndexes[processID]] != '|')) {

						switch(dataRead) {

							case 0:
					  			name = readString('~', fd[processID][1], buff[processID], &buffIndexes[processID], bufferSize, 0);
					  			printf(" %s", name);
					  			free(name);
					  			break;
					  		case 1:
					  			surname = readString('~', fd[processID][1], buff[processID], &buffIndexes[processID], bufferSize, 0);
					  			printf(" %s", surname);
					  			free(surname);
					  			break;
					  		case 2:
					  			country = readString('~', fd[processID][1], buff[processID], &buffIndexes[processID], bufferSize, 0);
					  			printf(" %s\n", country);
					  			free(country);
					  			break;
					  		case 3:
					  			age = readString('~', fd[processID][1], buff[processID], &buffIndexes[processID], bufferSize, 0);
					  			printf("AGE %s\n", age);
					  			free(age);
					  			break;
					  		default:
					  			res = readString('~', fd[processID][1], buff[processID], &buffIndexes[processID], bufferSize, 0);
					  			printf("%s\n", res);
					  			free(res);

				  		}

				  		dataRead++;

				  	}
					//
					int retval;
					struct pollfd poller;
					poller.fd = fd[processID][1];
					poller.events = POLLIN;
					// Check if you have leftover buffers to read and do the same
					while(buff[processID][buffIndexes[processID]] != '|') {

	    				retval = poll(&poller, 1, 0);

						if(retval < 0)
							return -1;

						if((poller.revents & POLLIN) == POLLIN) {

							while((buffIndexes[processID] < bufferSize) && (buff[processID][buffIndexes[processID]] != '|')) {
								// Switch for correct output format
								switch(dataRead) {

									case '0':
							  			name = readString('~', fd[processID][1], buff[processID], &buffIndexes[processID], bufferSize, 0);
							  			printf(" %s", name);
							  			free(name);
							  			break;
							  		case '1':
							  			surname = readString('~', fd[processID][1], buff[processID], &buffIndexes[processID], bufferSize, 0);
							  			printf(" %s", surname);
							  			free(surname);
							  			break;
							  		case '2':
							  			country = readString('~', fd[processID][1], buff[processID], &buffIndexes[processID], bufferSize, 0);
							  			printf(" %s\n", country);
							  			free(country);
							  			break;
							  		case '3':
							  			age = readString('~', fd[processID][1], buff[processID], &buffIndexes[processID], bufferSize, 0);
							  			printf("AGE %s\n", age);
							  			free(age);
							  			break;
							  		default:
							  			res = readString('~', fd[processID][1], buff[processID], &buffIndexes[processID], bufferSize, 0);
							  			printf(" %s\n", res);
							  			free(res);

						  		}

						  		dataRead++;

						  	}
					  	
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

				kill(getpid(), SIGINT);

			} else { printf("Uknown Command!\n"); }
			// Free the space used for command argument
			free(temp);

		}

	} while(1);

	return 1;

}

void setupSIGCHLD(void) {

	struct sigaction sa;

	memset(&sa, 0, sizeof(sa));

	sa.sa_sigaction = &SIGCHLDHandler;
	sa.sa_flags = SA_RESTART | SA_SIGINFO;	// SA_RESTART to protect from syscall interruptions
	
	if(sigaction(SIGCHLD, &sa, NULL) == -1)
		exit(1);

}

void SIGCHLDHandler(int signum, siginfo_t *info, void *ucontext) {

	pid_t p;
    int status;

    while((p = waitpid(-1, &status, WNOHANG)) != -1) {

    	SIGCHLDflag = p;
    	// printf("child died: %d\n", p);
    	return;

    }

}

void setupSIGINT_SIGQUIT_PARENT(void) {

	struct sigaction sa;

	memset(&sa, 0, sizeof(sa));

	sa.sa_sigaction = &SIGINT_SIGQUITHandler;
	sa.sa_flags = SA_RESTART | SA_SIGINFO;	// SA_RESTART to protect from syscall interruptions

	if(sigaction(SIGQUIT, &sa, NULL) == -1)
		exit(1);

	if(sigaction(SIGINT, &sa, NULL) == -1)
		exit(1);

}

void SIGINT_SIGQUITHandler(int signum, siginfo_t *info, void *ucontext) {

    SIGINT_SIGQUITflag = 1;

}