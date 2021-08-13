HEADER  = structs.h functions.h
SOURCE	= main.c bloom.c virusList.c hashtable.c util.c hash.c countryList.c skipList.c ageList.c popList.c
OBJS 	= main.o bloom.o virusList.o hashtable.o util.o hash.o countryList.o skipList.o ageList.o popList.o
EXEC   = vaccineMonitor
CC	    = gcc
CFLAGS  = -g -Wall -Werror


make: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(EXEC)

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

hashtable.o: hashtable.c
	$(CC) -c $(CFLAGS) hashtable.c

countryList.o: countryList.c
	$(CC) -c $(CFLAGS) countryList.c

skipList.o: skipList.c
	$(CC) -c $(CFLAGS) skipList.c

ageList.o: ageList.c
	$(CC) -c $(CFLAGS) ageList.c

popList.o: popList.c
	$(CC) -c $(CFLAGS) popList.c

clean:
	rm -f $(OBJS) $(EXEC)

run:
	./vaccineMonitor -c input -b 102400