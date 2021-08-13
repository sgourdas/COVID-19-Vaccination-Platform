#include "functions.h"

int bloomSize;
int bufferSize;
int SIGUSR1flag = 0;
int SIGINT_SIGQUITflag = 0;

int main(int argc, char **argv) {

	setupSIGUSR1();
	setupSIGINT_SIGQUIT();

	int accepted = 0, rejected = 0;

	int fd[2], buffIndex, stringIndex = 0;
	char tempbuff[FIRST_MSG];

	fd[0] = open(argv[1], O_RDONLY);
	fd[1] = open(argv[2], O_WRONLY);

	if(read(fd[0], tempbuff, FIRST_MSG) < 0)
		return -1;

	char buff[bufferSize = atoi(tempbuff)];
	char stringBuff[99999];
	dirList directories = NULL;
	int flag = 0;
	// Get directories, to analyze, in a list
	while(read(fd[0], buff, bufferSize)) {

		for(buffIndex = 0 ; (buffIndex < bufferSize) && (buff[buffIndex] != '.') ; buffIndex++) {

			stringBuff[stringIndex++] = buff[buffIndex];

			if((buff[buffIndex] == '~') || (buff[buffIndex] == '|')) {

				stringBuff[stringIndex - 1] = '\0';

				if(directories == NULL)
					directories = dirListCreate(stringBuff);
				else
					directories = dirListInsert(directories, stringBuff);

				stringIndex = 0;

				if(buff[buffIndex] == '|') {

					buffIndex++;

					flag = 1;

					break;

				}

			}

		}

		if(flag)
			break;

	}

	char *strbuff;
	strbuff = readString('~', fd[0], buff, &buffIndex, bufferSize, 0);
	bloomSize = atoi(strbuff);
	free(strbuff);

	int regenFlag = 0;
	if(bloomSize < 0) {

		regenFlag = 1;
		bloomSize = -bloomSize;

	}

	char *inputDir;
	inputDir = readString('~', fd[0], buff, &buffIndex, bufferSize, 0);

	FILE *fp;
	dirList dirs, files = NULL;
	DIR *d;
	struct dirent *dir;
	char *sep = "/";
	char fileBuff[99999];

	char buffer[300], id[30], fn[30], ln[30], country[30], virus[30], vacc[30], date[30];
	int ageBuffer, charsRead;
	countryList countryDataList = NULL;
	virusList virusDataList = NULL;
	citizenRecord citizen;
	skipList tmpSkip;
	// Parse all directories
	for(dirs = directories ; dirs != NULL; dirs = dirs->link) {

		// Make path string
		strcpy(stringBuff, inputDir);
		strcat(stringBuff, sep);
		strcat(stringBuff, dirs->dir);
		// Get directory pointer
		d = opendir(stringBuff);
		// List directory files
		while((dir = readdir(d)) != NULL) {

			if(!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, ".."))
				continue;

			if(files == NULL)
				files = dirListCreate(dir->d_name);
			else
				files = dirListInsert(files, dir->d_name);
			// Create file path
			strcpy(fileBuff, stringBuff);
			strcat(fileBuff, sep);
			strcat(fileBuff, dir->d_name);
			// Open file and read contents
			fp = fopen(fileBuff, "r");

			while(fgets(buffer, 200, fp) != NULL) {	// Until you read the entire file

				charsRead = sscanf(buffer, "%s %s %s %s %d %s %s %s", id, fn, ln, country, &ageBuffer, virus, vacc, date);
				citizen = citizenCreate(id, fn, ln, country, ageBuffer);

				if(virusDataList != NULL) {

					tmpSkip = virusListSkipSearch(virusDataList, virus, citizen);
					// If citizen has been inserted before, keep that one, else keep the new one that we created
					if(tmpSkip != NULL) {

						free(citizen);

						citizen = tmpSkip->citizen;

					}
					// Invalid data
					if((citizen != NULL) && (compareCitizens(citizen, id, fn, ln, country, ageBuffer))) {

						if(charsRead == 7)
							printf("ERROR IN RECORD %s %s %s %s %d %s %s\n", id, fn, ln, country, ageBuffer, virus, vacc);
						else 
							printf("ERROR IN RECORD %s %s %s %s %d %s %s %s\n", id, fn, ln, country, ageBuffer, virus, vacc, date);

						continue;

					}
					// Citizen exists
					if(((citizen != NULL) && (virusListSkipExists(virusDataList, virus, citizen) != NULL)) || ((!strcmp(vacc, "NO")) && (charsRead > 7))) {	// If id exists, or we have NO followed by date in file

						if(charsRead == 7)
							printf("ERROR IN RECORD %s %s %s %s %d %s %s\n", id, fn, ln, country, ageBuffer, virus, vacc);
						else 
							printf("ERROR IN RECORD %s %s %s %s %d %s %s %s\n", id, fn, ln, country, ageBuffer, virus, vacc, date);

						continue;

					}

				}
				// Make sure country exists on country list and return its pointer
				if(countryDataList == NULL)
					countryDataList = countryListCreate(country);
				else
					countryListInsert(countryDataList, country);
				// Add to skiplist
				if(virusDataList == NULL) {

					if(!strcmp(vacc, "YES"))
						virusDataList = virusListCreate(virus, citizen, date);
					else
						virusDataList = virusListCreate(virus, citizen, NULL);
				
				} else {

					if(!strcmp(vacc, "YES"))
						virusListInsert(virusDataList, virus, citizen, date);
					else
						virusListInsert(virusDataList, virus, citizen, NULL);
				
				}

			}

			fclose(fp);

		}
        // Close directory pointer
        closedir(d);

	}

	char *bloomBuff = (char *) malloc(bloomSize + 1);
	bloomBuff[bloomSize] = '~';
	// Empty buffer from reading stage
	for(buffIndex = 0 ; buffIndex < bufferSize ; buffIndex++)
		buff[buffIndex] = '.';
	// If this isnt a resurrected child
	if(regenFlag == 0) {
		// Parse all viruses with their corresponding bloom filter and write them on the buffer	
		for(virusList virusListParse = virusDataList ; virusListParse != NULL ; virusListParse = virusListParse->link) {

			strcpy(stringBuff, virusListParse->virus);
			strcat(stringBuff, "~");
			writeString2(stringBuff, buff, bufferSize, fd[1], 0);

			memcpy(bloomBuff, virusListParse->bloom, bloomSize);
			writeString2(bloomBuff, buff, bufferSize, fd[1], bloomSize);

		}
		writeString2("|", buff, bufferSize, fd[1], 0);

		if(buff[0] != '.')
			write(fd[1], buff, bufferSize);
		// Empty buffer from reading stage
		for(buffIndex = 0 ; buffIndex < bufferSize ; buffIndex++)
			buff[buffIndex] = '.';

	} else {

		printf("travelMonitor> "); 
		fflush(stdout);

	}
	////
	///
	//
	struct pollfd poller;
	int retval;

	poller.fd = fd[0];
	poller.events = POLLIN;
	// printf("child generated with pid: %d\n", getpid());
	// Check for requests
	while(1) {

		if(SIGUSR1flag == 1) {

			SIGUSR1flag = 0;
			// Get new files added, in a list
			dirList new = newFiles(directories, files, inputDir);
			dirList tmplist = new;
			// If list is empty
			if(new == NULL) {

				// printf("There are no new files added.\n");

				char *nonebuff = (char *) malloc(strlen("none~") + 1);
				strcpy(nonebuff, "none~");
				writeString2(nonebuff, buff, bufferSize, fd[1], 0);

				if(buff[0] != '.')
					write(fd[1], buff, bufferSize);
				// Empty buffer from reading stage
				for(buffIndex = 0 ; buffIndex < bufferSize ; buffIndex++)
					buff[buffIndex] = '.';

				free(nonebuff);

				continue;
			
			}
			// Read new data
			while(tmplist != NULL) {

				char dirbuff[99999];
				int i;
				for(i = 0 ; i < strlen(tmplist->dir) && tmplist->dir[i] != '-' ; i++)
					dirbuff[i] = tmplist->dir[i];
				dirbuff[i] = '\0';

				strcpy(stringBuff, inputDir);
				strcat(stringBuff, "/");
				strcat(stringBuff, dirbuff);
				strcat(stringBuff, "/");
				strcat(stringBuff, tmplist->dir);

				fp = fopen(stringBuff, "r");

				while(fgets(buffer, 200, fp) != NULL) {	// Until you read the entire file

					charsRead = sscanf(buffer, "%s %s %s %s %d %s %s %s", id, fn, ln, country, &ageBuffer, virus, vacc, date);
					citizen = citizenCreate(id, fn, ln, country, ageBuffer);

					if(virusDataList != NULL) {

						tmpSkip = virusListSkipSearch(virusDataList, virus, citizen);
						// If citizen has been inserted before, keep that one, else keep the new one that we created
						if(tmpSkip != NULL) {

							free(citizen);

							citizen = tmpSkip->citizen;

						}
						// Invalid data
						if((citizen != NULL) && (compareCitizens(citizen, id, fn, ln, country, ageBuffer))) {

							if(charsRead == 7)
								printf("ERROR IN RECORD %s %s %s %s %d %s %s\n", id, fn, ln, country, ageBuffer, virus, vacc);
							else 
								printf("ERROR IN RECORD %s %s %s %s %d %s %s %s\n", id, fn, ln, country, ageBuffer, virus, vacc, date);

							continue;

						}
						// Citizen exists
						if(((citizen != NULL) && (virusListSkipExists(virusDataList, virus, citizen) != NULL)) || ((!strcmp(vacc, "NO")) && (charsRead > 7))) {	// If id exists, or we have NO followed by date in file

							if(charsRead == 7)
								printf("ERROR IN RECORD %s %s %s %s %d %s %s\n", id, fn, ln, country, ageBuffer, virus, vacc);
							else 
								printf("ERROR IN RECORD %s %s %s %s %d %s %s %s\n", id, fn, ln, country, ageBuffer, virus, vacc, date);

							continue;

						}

					}
					// Make sure country exists on country list and return its pointer
					if(countryDataList == NULL)
						countryDataList = countryListCreate(country);
					else
						countryListInsert(countryDataList, country);
					// Add to skiplist
					if(virusDataList == NULL) {

						if(!strcmp(vacc, "YES"))
							virusDataList = virusListCreate(virus, citizen, date);
						else
							virusDataList = virusListCreate(virus, citizen, NULL);
					
					} else {

						if(!strcmp(vacc, "YES"))
							virusListInsert(virusDataList, virus, citizen, date);
						else
							virusListInsert(virusDataList, virus, citizen, NULL);
					
					}

				}

				fclose(fp);

				tmplist = tmplist->link;


			}
			// Send new bloom filters

			// Empty buffer from reading stage
			for(buffIndex = 0 ; buffIndex < bufferSize ; buffIndex++)
				buff[buffIndex] = '.';
			// Parse all viruses with their corresponding bloom filter and write them on the buffer
			for(virusList virusListParse = virusDataList ; virusListParse != NULL ; virusListParse = virusListParse->link) {

				strcpy(stringBuff, virusListParse->virus);
				strcat(stringBuff, "~");

				writeString2(stringBuff, buff, bufferSize, fd[1], 0);

				memcpy(bloomBuff, virusListParse->bloom, bloomSize);

				writeString2(bloomBuff, buff, bufferSize, fd[1], 1);

			}

			writeString2("|", buff, bufferSize, fd[1], 0);

			if(buff[0] != '.')
				write(fd[1], buff, bufferSize);
			// Empty buffer from reading stage
			for(buffIndex = 0 ; buffIndex < bufferSize ; buffIndex++)
				buff[buffIndex] = '.';
			// Update files list with new files
			tmplist = files;

			while(tmplist->link != NULL)
				tmplist = tmplist->link;

			tmplist->link = new;

		} else if(SIGINT_SIGQUITflag != 0) {

			SIGINT_SIGQUITflag = 0;

			char logfile[100];
			char *pidString = itoa2(getpid());
			strcpy(logfile, "log_file.");
			strcat(logfile, pidString);

			FILE *fp = fopen(logfile, "w");

			for(dirList countriestmp = directories ; countriestmp != NULL ; countriestmp = countriestmp->link)
				fprintf(fp, "%s\n", countriestmp->dir);

			fprintf(fp, "TOTAL TRAVEL REQUESTS %d\n", accepted + rejected);
			fprintf(fp, "ACCEPTED %d\n", accepted );
			fprintf(fp, "REJECTED %d\n", rejected);

			fclose(fp);
			free(pidString);

		}

		retval = poll(&poller, 1, 0);

		if(retval < 0)
			return -1;

		if((poller.revents & POLLIN) == POLLIN) {
			// Read message
			char *type = readString('~', fd[0], buff, &buffIndex, bufferSize, 0);
			// Get message type
			if(!strcmp(type, "request")) {
				// Get id and virus following request type
				char *id = readString('~', fd[0], buff, &buffIndex, bufferSize, 0);
				char *virus = readString('~', fd[0], buff, &buffIndex, bufferSize, 0);

				citizenRecord ctmp = citizenCreate(id, "tmp", "tmp", "tmp", 1);
				// Send query to skip list
				skipList res = virusListSkipExists(virusDataList, virus, ctmp);

				for(buffIndex = 0 ; buffIndex < bufferSize ; buffIndex++)
					buff[buffIndex] = '.';

				char tmp[99999];
				// Act based on skip list result
				if(res == NULL) {

					strcpy(tmp, "NO");

					rejected++;

				} else {
				
					strcpy(tmp, "YES");
					strcat(tmp, res->dateVaccinated);

					accepted++;

				}
				strcat(tmp, "~");
				// Write message to parent
				writeString2(tmp, buff, bufferSize, fd[1], 0);

				if(buff[0] != '.')
					write(fd[1], buff, bufferSize);
				// Empty buffer from reading stage
				for(buffIndex = 0 ; buffIndex < bufferSize ; buffIndex++)
					buff[buffIndex] = '.';

				free(id);
				free(virus);
				free(ctmp);

			} else if(!strcmp(type, "search")) {
				// Read id given after search
				char *id = readString('~', fd[0], buff, &buffIndex, bufferSize, 0);

				for(buffIndex = 0 ; buffIndex < bufferSize ; buffIndex++)
					buff[buffIndex] = '.';

				citizenRecord dummyCitizen = citizenCreate(id, "tmp", "tmp", "tmp", -1);
				skipList result;
				char answer[9999];
				int flag = 0; 

				free(id);
				// For all nodes get status of id
				for(virusList viruses = virusDataList ; viruses != NULL ; viruses = viruses->link) {

					strcpy(answer, viruses->virus);

					result = skipListGet(viruses->vaccinated, dummyCitizen);
					// Write back to father based on result
					if(result != NULL) {

						if(flag == 0) {

							flag = 1;

							writeString2("yes~", buff, bufferSize, fd[1], 0);

							char tmp[9999];

							strcpy(tmp, result->citizen->firstName);
							strcat(tmp, "~");
							writeString2(tmp, buff, bufferSize, fd[1], 0);

							strcpy(tmp, result->citizen->lastName);
							strcat(tmp, "~");
							writeString2(tmp, buff, bufferSize, fd[1], 0);

							strcpy(tmp, result->citizen->country);
							strcat(tmp, "~");
							writeString2(tmp, buff, bufferSize, fd[1], 0);

							char *t = itoa2(result->citizen->age);
							strcpy(tmp, t);
							strcat(tmp, "~");
							writeString2(tmp, buff, bufferSize, fd[1], 0);
							free(t);

						}

						strcat(answer, " VACCINATED ON ");
						strcat(answer, result->dateVaccinated);
						strcat(answer, "~");

						writeString2(answer, buff, bufferSize, fd[1], 0);

						continue;

					}

					result = skipListGet(viruses->nonVaccinated, dummyCitizen);

					if(result != NULL) {

						if(flag == 0) {

							flag = 1;

							writeString2("yes~", buff, bufferSize, fd[1], 0);

							char tmp[9999];

							strcpy(tmp, result->citizen->firstName);
							strcat(tmp, "~");
							writeString2(tmp, buff, bufferSize, fd[1], 0);

							strcpy(tmp, result->citizen->lastName);
							strcat(tmp, "~");
							writeString2(tmp, buff, bufferSize, fd[1], 0);

							strcpy(tmp, result->citizen->country);
							strcat(tmp, "~");
							writeString2(tmp, buff, bufferSize, fd[1], 0);

							char *t = itoa2(result->citizen->age);
							strcpy(tmp, t);
							strcat(tmp, "~");
							writeString2(tmp, buff, bufferSize, fd[1], 0);
							free(t);

						}

						strcat(answer, " NOT YET VACCINATED");
						strcat(answer, "~");

						writeString2(answer, buff, bufferSize, fd[1], 0);

					}

				}
				// If we have not raised the flag means id isnt vaccinated for any virus
				if(flag == 0)
					writeString2("no~", buff, bufferSize, fd[1], 0);

				writeString2("|", buff, bufferSize, fd[1], 0);

				if(buff[0] != '.')
					write(fd[1], buff, bufferSize);
				// Empty buffer from reading stage
				for(buffIndex = 0 ; buffIndex < bufferSize ; buffIndex++)
					buff[buffIndex] = '.';

				free(dummyCitizen);

			}

		}

	}

	close(fd[0]);
	close(fd[1]);

	dirListDestroy(directories);
	countryListDestroy(countryDataList);
	virusListDestroy(virusDataList);
	dirListDestroy(files);

	free(bloomBuff);
	free(inputDir);

	return 0;

}

void SIGUSR1Handler(int signum, siginfo_t *info, void *ucontext) {	// When recieving a SIGUSR1 signal we up our counter

	SIGUSR1flag = 1;

}

void setupSIGUSR1(void) {

	struct sigaction sa;

	memset(&sa, 0, sizeof(sa));

	sa.sa_sigaction = &SIGUSR1Handler;
	sa.sa_flags = SA_RESTART | SA_SIGINFO;	// SA_RESTART to protect from syscall interruptions
	
	if(sigaction(SIGUSR1, &sa, NULL) == -1)
		exit(1);

}

void setupSIGINT_SIGQUIT(void) {

	struct sigaction sa;

	memset(&sa, 0, sizeof(sa));

	sa.sa_sigaction = &SIGINT_SIGQUITHandler;
	sa.sa_flags = SA_RESTART | SA_SIGINFO;	// SA_RESTART to protect from syscall interruptions
	
	if(sigaction(SIGINT, &sa, NULL) == -1)
		exit(1);

	if(sigaction(SIGQUIT, &sa, NULL) == -1)
		exit(1);

}

void SIGINT_SIGQUITHandler(int signum, siginfo_t *info, void *ucontext) {

    SIGINT_SIGQUITflag = 1;

}

dirList newFiles(dirList directories, dirList files, char *inputDir) {

	char stringBuff[99999];
	struct dirent *dir;
	DIR *d;
	dirList dirs, newFiles = NULL;
	// Parse all directories
	for(dirs = directories ; dirs != NULL; dirs = dirs->link) {
		// Make path string
		strcpy(stringBuff, inputDir);
		strcat(stringBuff, "/");
		strcat(stringBuff, dirs->dir);
		// Get directory pointer
		d = opendir(stringBuff);
		// List directory files
		while((dir = readdir(d)) != NULL) {

			if(!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, ".."))
				continue;

			if(dirListSearch(files, dir->d_name) == NULL) {

				if(newFiles == NULL)
					newFiles = dirListCreate(dir->d_name);
				else
					newFiles = dirListInsert(newFiles, dir->d_name);

			}

		}

	}

	return newFiles;

}