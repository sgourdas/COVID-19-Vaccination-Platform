HEADER  = structs.h functions.h
SOURCE	= main.c bloom.c util.c hash.c skipList.c bloomList.c processList.c reqList.c dirList.c countryList.c
OBJS 	= main.o bloom.o util.o hash.o skipList.o bloomList.o processList.o reqList.o dirList.o countryList.o
EXEC    = travelMonitor
EXEC2   = Monitor
CC	    = gcc
CFLAGS  = -g -Wall -Werror


make: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(EXEC)
	$(CC) $(CFLAGS) monitor.c hash.c util.c dirList.c virusList.c skipList.c bloom.c countryList.c -o $(EXEC2)

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
	$(CC) -c $(CFLAGS) countryList.c

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

countryList.o: countryList.c
	$(CC) -c $(CFLAGS) countryList.c

clean:
	rm -f $(OBJS) $(EXEC) $(EXEC2)

run:
	./travelMonitor -m 3 -b 1024 -s 1024 -i dir

val:
	valgrind --trace-children=yes ./travelMonitor -m 3 -b 256 -s 240 -i dir

valfull:
	valgrind --leak-check=full --trace-children=yes ./travelMonitor -m 3 -b 256 -s 240 -i dir