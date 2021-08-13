HEADER  = structs.h functions.h
SOURCE	= main.c bloom.c util.c hash.c skipList.c bloomList.c processList.c reqList.c dirList.c 
OBJS 	= main.o bloom.o util.o hash.o skipList.o bloomList.o processList.o reqList.o dirList.o
EXEC    = travelMonitorClient
EXEC2   = monitorServer
CC	    = gcc
CFLAGS  = -g -Wall -Werror -lpthread -pthread


make: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(EXEC)
	$(CC) $(CFLAGS) monitor.c hash.c util.c dirList.c virusList.c skipList.c bloom.c pool.c citizenList.c -o $(EXEC2)

main.o: main.c
	$(CC) -c $(CFLAGS) main.c

util.o: util.c
	$(CC) -c $(CFLAGS) util.c

bloom.o: bloom.c
	$(CC) -c $(CFLAGS) bloom.c

virusList.o: virusList.c
	$(CC) -c $(CFLAGS) virusList.c

hash.o: hash.c
	$(CC) -c $(CFLAGS) hash.c

skipList.o: skipList.c
	$(CC) -c $(CFLAGS) skipList.c

bloomList.o: bloomList.c
	$(CC) -c $(CFLAGS) bloomList.c

processList.o: processList.c
	$(CC) -c $(CFLAGS) processList.c

reqList.o: reqList.c
	$(CC) -c $(CFLAGS) reqList.c

dirList.o: dirList.c
	$(CC) -c $(CFLAGS) dirList.c

pool.o: pool.c
	$(CC) -c $(CFLAGS) pool.c

clean:
	rm -f $(OBJS) $(EXEC) $(EXEC2) pool.o

run:
	./$(EXEC) -m 2 -b 1024 -c 10 -s 3072 -i dir -t 4

val:
	valgrind --trace-children=yes ./$(EXEC) -m 2 -b 1024 -c 10 -s 3072 -i dir -t 4

valfull:
	valgrind --leak-check=full --show-leak-kinds=all --trace-children=yes ./$(EXEC) -m 1 -b 1024 -c 10 -s 3072 -i dir -t 50