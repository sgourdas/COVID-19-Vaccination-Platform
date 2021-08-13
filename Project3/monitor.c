#include "functions.h"
// Struct sizes
int bloomSize;
int bufferSize;
int cyclicBufferSize;
// Multithread variables
pool_t pool;
pthread_mutex_t mtx;
pthread_mutex_t citizen_mtx;
pthread_mutex_t skip_mtx;
pthread_mutex_t countries_mtx;
pthread_cond_t cond_nonempty;
pthread_cond_t cond_nonfull;
struct dirent *dir;
int pathIndex = -1, pathCount = -1, done = -1;
// Structs
virusList virusDataList = NULL;
citizenList citizenDataList = NULL;

int main(int argc, char **argv) {

	int port, numThreads, socketBufferSize;
	char **paths = NULL;
	pathCount = configure2(argc, argv, &port, &numThreads, &socketBufferSize, &cyclicBufferSize, &bloomSize, &paths);

	if(pathCount == -1)	// Problem: more processes than countries
		return -1;
	// int accepted = 0, rejected = 0;
	bufferSize = socketBufferSize;
	dirList files = NULL;
	DIR *d;
	char *sep = "/", fileBuff[99999];
	// Initialize vars
	pthread_mutex_init(&mtx, 0);
	pthread_mutex_init(&citizen_mtx, 0);
	pthread_mutex_init(&countries_mtx, 0);
	pthread_mutex_init(&skip_mtx, 0);
	pthread_cond_init(&cond_nonempty, NULL);
	pthread_cond_init(&cond_nonfull, NULL);
	initialize(&pool, cyclicBufferSize);

	pthread_t *consumers = (pthread_t *) malloc(numThreads * sizeof(pthread_t));
	done = 0;
	for(int consumerIndex = 0 ; consumerIndex < numThreads ; consumerIndex++)
		pthread_create(&consumers[consumerIndex], 0, consumer, NULL);

	char dirBuff[99999];
	// Parse all directories
	for(pathIndex = 0 ; pathIndex < pathCount ; pathIndex++) {
		// Get directory pointer
		d = opendir(paths[pathIndex]);
		// List directory files
		while((dir = readdir(d)) != NULL) {
			// Skip these
			if(!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, ".."))
				continue;

			strcpy(dirBuff, paths[pathIndex]);
			strcat(dirBuff, "/");
			strcat(dirBuff, dir->d_name);
			// Store files for later
			if(files == NULL)
				files = dirListCreate(dirBuff);
			else
				files = dirListInsert(files, dirBuff);
			// Create file path
			strcpy(fileBuff, paths[pathIndex]);
			strcat(fileBuff, sep);
			strcat(fileBuff, dir->d_name);
			// Put file path on the cyclic buffer - pool
			poolPut(&pool, fileBuff);
			// Signal that the buffer is not empty since we just added an item
			pthread_cond_signal(&cond_nonempty);
			// Give kernel a chance to swap
			usleep(0);

		}
        // Close directory pointer
        closedir(d);

	}

	done = 1;
	// sleep(3);
	pthread_cond_broadcast(&cond_nonempty);

	for(int consumerIndex = 0 ; consumerIndex < numThreads ; consumerIndex++)
		pthread_join(consumers[consumerIndex], NULL);

	int sock;
	if((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0)
		return -1;
	// Make port insantly reusable
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

	char host[1024];
	host[1023] = '\0';
	gethostname(host, 1023);

    struct sockaddr_in server, client;
    struct sockaddr *serverptr = (struct sockaddr *) &server;
    struct sockaddr *clientptr = (struct sockaddr *) &client;
    struct hostent *localhost = gethostbyname(host);

    server.sin_family = AF_INET;       /* Internet domain */
    memcpy(&server.sin_addr, localhost->h_addr, localhost->h_length);
    server.sin_port = htons(port);      /* The given port */
    /* Bind socket to address */
    if(bind(sock, serverptr, sizeof(server)) < 0)
		return -1;

	if(listen(sock, 1) < 0)
		return -1;

	socklen_t clientlen = sizeof(client), newsock;
	if ((newsock = accept(sock, clientptr, &clientlen)) < 0)
		return -1;

	char *buff = (char *) malloc(bufferSize * sizeof(char));
	char *bloomBuff = (char *) malloc(bloomSize + 1);
	char stringBuff[99999];
	bloomBuff[bloomSize] = '~';
	int buffIndex;
	// Empty buffer from reading stage
	for(buffIndex = 0 ; buffIndex < bufferSize ; buffIndex++)
		buff[buffIndex] = '.';

	// Parse all viruses with their corresponding bloom filter and write them on the buffer	
	for(virusList virusListParse = virusDataList ; virusListParse != NULL ; virusListParse = virusListParse->link) {

		strcpy(stringBuff, virusListParse->virus);
		strcat(stringBuff, "~");
		writeString2(stringBuff, buff, bufferSize, newsock, 0);

		memcpy(bloomBuff, virusListParse->bloom, bloomSize);
		writeString2(bloomBuff, buff, bufferSize, newsock, bloomSize);

	}

	writeString2("|", buff, bufferSize, newsock, 0);

	if(buff[0] != '.')
		write(newsock, buff, bufferSize);
	// Empty buffer from writing stage
	for(buffIndex = 0 ; buffIndex < bufferSize ; buffIndex++)
		buff[buffIndex] = '.';

	// ////
	// ///
	// //
	struct pollfd poller;
	int retval;

	poller.fd = newsock;
	poller.events = POLLIN;

	int accepted = 0, rejected =0;
	// Check for requests
	while(1) {

		retval = poll(&poller, 1, 0);

		if(retval < 0)
			return -1;

		if((poller.revents & POLLIN) == POLLIN) {
			// Read message
			char *type = readString('~', newsock, buff, &buffIndex, bufferSize, 0);
			// Get message type
			if(!strcmp(type, "request")) {
				// Get id and virus following request type
				char *id = readString('~', newsock, buff, &buffIndex, bufferSize, 0);
				char *virus = readString('~', newsock, buff, &buffIndex, bufferSize, 0);

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
				writeString2(tmp, buff, bufferSize, newsock, 0);

				if(buff[0] != '.')
					write(newsock, buff, bufferSize);
				// Empty buffer from reading stage
				for(buffIndex = 0 ; buffIndex < bufferSize ; buffIndex++)
					buff[buffIndex] = '.';

				free(id);
				free(virus);
				citizenDestroy(ctmp);

			} else if(!strcmp(type, "search")) {
				// Read id given after search
				char *id = readString('~', newsock, buff, &buffIndex, bufferSize, 0);

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

							writeString2("yes~", buff, bufferSize, newsock, 0);

							char tmp[9999];

							strcpy(tmp, result->citizen->firstName);
							strcat(tmp, "~");
							writeString2(tmp, buff, bufferSize, newsock, 0);

							strcpy(tmp, result->citizen->lastName);
							strcat(tmp, "~");
							writeString2(tmp, buff, bufferSize, newsock, 0);

							strcpy(tmp, result->citizen->country);
							strcat(tmp, "~");
							writeString2(tmp, buff, bufferSize, newsock, 0);

							char *t = itoa2(result->citizen->age);
							strcpy(tmp, t);
							strcat(tmp, "~");
							writeString2(tmp, buff, bufferSize, newsock, 0);
							free(t);

						}

						strcat(answer, " VACCINATED ON ");
						strcat(answer, result->dateVaccinated);
						strcat(answer, "~");
						writeString2(answer, buff, bufferSize, newsock, 0);

						continue;

					}

					result = skipListGet(viruses->nonVaccinated, dummyCitizen);

					if(result != NULL) {

						if(flag == 0) {

							flag = 1;

							writeString2("yes~", buff, bufferSize, newsock, 0);

							char tmp[9999];

							strcpy(tmp, result->citizen->firstName);
							strcat(tmp, "~");
							writeString2(tmp, buff, bufferSize, newsock, 0);
							
							strcpy(tmp, result->citizen->lastName);
							strcat(tmp, "~");
							writeString2(tmp, buff, bufferSize, newsock, 0);

							strcpy(tmp, result->citizen->country);
							strcat(tmp, "~");
							writeString2(tmp, buff, bufferSize, newsock, 0);

							char *t = itoa2(result->citizen->age);
							strcpy(tmp, t);
							strcat(tmp, "~");
							writeString2(tmp, buff, bufferSize, newsock, 0);
							free(t);

						}

						strcat(answer, " NOT YET VACCINATED");
						strcat(answer, "~");

						writeString2(answer, buff, bufferSize, newsock, 0);

					}

				}
				// If we have not raised the flag means id isnt vaccinated for any virus
				if(flag == 0)
					writeString2("no~", buff, bufferSize, newsock, 0);

				writeString2("|", buff, bufferSize, newsock, 0);

				if(buff[0] != '.')
					write(newsock, buff, bufferSize);
				// Empty buffer from reading stage
				for(buffIndex = 0 ; buffIndex < bufferSize ; buffIndex++)
					buff[buffIndex] = '.';

				citizenDestroy(dummyCitizen);

			} else if(!strcmp(type, "add")) {
				// Get new files added, in a list
				dirList new = newFiles(paths, pathCount, files);
				dirList tmplist = new;
				// If list is empty
				if(new == NULL) {

					for(buffIndex = 0 ; buffIndex < bufferSize ; buffIndex++)
						buff[buffIndex] = '.';

					char *nonebuff = (char *) malloc(strlen("none~") + 1);
					strcpy(nonebuff, "none~");
					writeString2(nonebuff, buff, bufferSize, newsock, 0);

					if(buff[0] != '.')
						write(newsock, buff, bufferSize);
					// Empty buffer from reading stage
					for(buffIndex = 0 ; buffIndex < bufferSize ; buffIndex++)
						buff[buffIndex] = '.';

					free(nonebuff);
					free(type);

					continue;
				
				}
				// Read new data
				for( ; tmplist != NULL ; tmplist = tmplist->link) {

					done = 0;
					for(int consumerIndex = 0 ; consumerIndex < numThreads ; consumerIndex++)
						pthread_create(&consumers[consumerIndex], 0, consumer, NULL);
					// Put file path on the cyclic buffer - pool
					poolPut(&pool, tmplist->dir);
					// Signal that the buffer is not empty since we just added an item
					pthread_cond_signal(&cond_nonempty);
					// Give kernel a chance to swap
					usleep(0);
					done = 1;

					pthread_cond_broadcast(&cond_nonempty);

					for(int consumerIndex = 0 ; consumerIndex < numThreads ; consumerIndex++)
						pthread_join(consumers[consumerIndex], NULL);

					pthread_cond_destroy(&cond_nonempty);
					pthread_cond_destroy(&cond_nonfull);
					pthread_mutex_destroy(&mtx);

				}
				// Send new bloom filters
				// Empty buffer from reading stage
				for(buffIndex = 0 ; buffIndex < bufferSize ; buffIndex++)
					buff[buffIndex] = '.';
				// Parse all viruses with their corresponding bloom filter and write them on the buffer
				for(virusList virusListParse = virusDataList ; virusListParse != NULL ; virusListParse = virusListParse->link) {

					strcpy(stringBuff, virusListParse->virus);
					strcat(stringBuff, "~");
					writeString2(stringBuff, buff, bufferSize, newsock, 0);

					memcpy(bloomBuff, virusListParse->bloom, bloomSize);
					writeString2(bloomBuff, buff, bufferSize, newsock, bloomSize);

				}

				writeString2("|", buff, bufferSize, newsock, 0);

				if(buff[0] != '.')
					write(newsock, buff, bufferSize);
				// Empty buffer from reading stage
				for(buffIndex = 0 ; buffIndex < bufferSize ; buffIndex++)
					buff[buffIndex] = '.';
				// Update files list with new files
				tmplist = files;

				while(tmplist->link != NULL)
					tmplist = tmplist->link;

				tmplist->link = new;

			} else if(!strcmp(type, "exit")) {

				char logfile[100];
				char *pidString = itoa2(getpid());
				strcpy(logfile, "log_file.");
				strcat(logfile, pidString);

				FILE *fp = fopen(logfile, "w");

				char *countrytmp;
				for(int i = 0 ; i < pathCount ; i++) {

					countrytmp = subStr(paths[i], 2, '/');
					fprintf(fp, "%s\n", countrytmp);
					free(countrytmp);

				}

				fprintf(fp, "TOTAL TRAVEL REQUESTS %d\n", accepted + rejected);
				fprintf(fp, "ACCEPTED %d\n", accepted );
				fprintf(fp, "REJECTED %d\n", rejected);

				fclose(fp);

				free(pidString);
				free(type);
				free(buff);
				free(bloomBuff);
				for(int i = 0 ; i < pathCount ; i++)
					free(paths[i]);
				free(paths);
				free(consumers);
				free(pool.data);

				virusListDestroy(virusDataList);
				dirListDestroy(files);
				citizenListDestroy(citizenDataList);

				pthread_cond_destroy(&cond_nonempty);
				pthread_cond_destroy(&cond_nonfull);
				pthread_mutex_destroy(&mtx);

				close(sock);
				close(newsock);

				return 0;

			}

			free(type);

		}

	}

	return -1;

}

void insertData(char *file, virusList *virusDataList, citizenList *citizenDataList) {

	char buffer[300], id[30], fn[30], ln[30], country[30], virus[30], vacc[30], date[30];
	int ageBuffer, charsRead;
	citizenRecord citizen;
	skipList tmpSkip;
	// Open file and read contents

	FILE *fp = fopen(file, "r");

	while(fgets(buffer, 200, fp) != NULL) {	// Until you read the entire file

		charsRead = sscanf(buffer, "%s %s %s %s %d %s %s %s", id, fn, ln, country, &ageBuffer, virus, vacc, date);
		citizen = citizenCreate(id, fn, ln, country, ageBuffer);

		pthread_mutex_lock(&skip_mtx);		// Lock 0
		int cond = (*virusDataList != NULL);
		pthread_mutex_unlock(&skip_mtx);	// Unlock 0
		if(cond) {

			pthread_mutex_lock(&skip_mtx); 		// Lock 1
			tmpSkip = virusListSkipSearch(*virusDataList, virus, citizen);
			pthread_mutex_unlock(&skip_mtx);	// Unlock 1
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
			pthread_mutex_lock(&skip_mtx);		// Lock 2
			skipList stemp = virusListSkipExists(*virusDataList, virus, citizen);
			pthread_mutex_unlock(&skip_mtx);	// Unlock 2
			// Citizen exists
			if(((citizen != NULL) && (stemp != NULL)) || ((!strcmp(vacc, "NO")) && (charsRead > 7))) {	// If id exists, or we have NO followed by date in file

				if(charsRead == 7)
					printf("ERROR IN RECORD %s %s %s %s %d %s %s\n", id, fn, ln, country, ageBuffer, virus, vacc);
				else 
					printf("ERROR IN RECORD %s %s %s %s %d %s %s %s\n", id, fn, ln, country, ageBuffer, virus, vacc, date);

				continue;

			}

		}

		pthread_mutex_lock(&citizen_mtx);	// Lock 3
		if(*citizenDataList)
			*citizenDataList = citizenListInsert(*citizenDataList, citizen);
		else
			*citizenDataList = citizenListCreate(citizen);
		pthread_mutex_unlock(&citizen_mtx);	// Unlock 3
		// Add to skiplist
		pthread_mutex_lock(&skip_mtx);	// Lock 4
		if(*virusDataList == NULL) {

			if(!strcmp(vacc, "YES"))
				*virusDataList = virusListCreate(virus, citizen, date);
			else
				*virusDataList = virusListCreate(virus, citizen, NULL);
		
		} else {

			if(!strcmp(vacc, "YES"))
				virusListInsert(*virusDataList, virus, citizen, date);
			else
				virusListInsert(*virusDataList, virus, citizen, NULL);
		
		}
		pthread_mutex_unlock(&skip_mtx);	// Unlock 4

	}

	fclose(fp);

}

dirList newFiles(char **paths, int pathsSize, dirList files) {

	struct dirent *dir;
	DIR *d;
	dirList newFiles = NULL;
	char dirBuff[99999];
	// Parse all directories
	for(int pathIndex = 0 ; pathIndex < pathsSize ; pathIndex++) {
		// Get directory pointer
		d = opendir(paths[pathIndex]);
		// List directory files
		while((dir = readdir(d)) != NULL) {

			if(!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, ".."))
				continue;

			strcpy(dirBuff, paths[pathIndex]);
			strcat(dirBuff, "/");
			strcat(dirBuff, dir->d_name);

			if(dirListSearch(files, dirBuff) == NULL) {

				if(newFiles == NULL)
					newFiles = dirListCreate(dirBuff);
				else
					newFiles = dirListInsert(newFiles, dirBuff);

			}

		}

		closedir(d);

	}

	return newFiles;

}