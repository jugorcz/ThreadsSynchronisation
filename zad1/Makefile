CC = gcc

all: main clean
	./main configuration.txt

main: main.o
	gcc -o main main.o -lpthread -pthread

main.o: main.c
	gcc -c main.c

clean:
	rm -f *.o

cleanAll:
	rm -f *.o
	rm main