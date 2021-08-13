#include "functions.h"


void initialize(pool_t *pool, int cyclicBufferSize) {

	pool->start = 0;
	pool->end = -1;
	pool->count = 0;

	pool->data = (char **) malloc(cyclicBufferSize * sizeof(char *));
	for(int buffPos = 0 ; buffPos < cyclicBufferSize ; buffPos++)
		pool->data[buffPos] = NULL;

}

void poolPut(pool_t *pool, char *data) {
	// Lock mutex
	pthread_mutex_lock(&mtx);
	// Wait until there is room in the pool
	while(pool->count >= cyclicBufferSize)
		pthread_cond_wait(&cond_nonfull, &mtx);
	// Change end of pool
	pool->end = (pool->end + 1) % cyclicBufferSize;
	// Add new data to the end
	pool->data[pool->end] = (char *) malloc(strlen(data) + 1);
	strcpy(pool->data[pool->end], data);
	// Increase the pool count
	pool->count++;
	// Unlock mutex
	pthread_mutex_unlock(&mtx);

}

char *poolGet(pool_t *pool) {
	// Lock mutex
	pthread_mutex_lock(&mtx);
	// Wait til there is data in the buffer
	while(pool->count <= 0 && done == 0)
		pthread_cond_wait(&cond_nonempty, &mtx);

	char *data = NULL;
	// Get data from the pool start index
	if(pool->count > 0 || done != 1) {

		data = pool->data[pool->start];
		// Shift starting position
		pool->start = (pool->start + 1) % cyclicBufferSize;
		// Decrement counter of pool items
		pool->count--;

	}
	// Unlock mutex
	pthread_mutex_unlock(&mtx);
	// Return item
	return data;

}

void *consumer(void *ptr) {

	while (pathIndex < pathCount || dir != NULL || pool.count > 0) {
		// Get next pool item
		char *item = poolGet(&pool);

		if(item) {

			insertData(item, &virusDataList, &citizenDataList);
			free(item);

		}
		// Signal that the pool is not full since we got an item
		pthread_cond_signal(&cond_nonfull);
		// Give kernel chance for swap
		// usleep(0);

	}
	// pthread_exit(NULL); // https://stackoverflow.com/questions/9951891/c-cleanup-unused-threads -- Used return NULL cause of leak
	return NULL;

}