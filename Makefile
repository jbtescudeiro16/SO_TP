CC = gcc
CFLAGS = -Wall -g 

all: sdstore sdstored

sdstored: bin/sdstored
sdstore: bin/sdstore


obj/sdstored.o: src/sdstored.c
	$(CC) $(CFLAGS) -c src/sdstored.c -o obj/sdstored.o

bin/sdstored: obj/sdstored.o
	$(CC) $(CFLAGS) obj/sdstored.o -o bin/sdstored

obj/sdstore.o : src/sdstore.c
	$(CC) $(CFLAGS) -c src/sdstore.c -o obj/sdstore.o

bin/sdstore: obj/sdstore.o
	$(CC) $(CFLAGS) obj/sdstore.o -o bin/sdstore


clean:
	rm obj/* bin/*

