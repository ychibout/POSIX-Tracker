CC = gcc
EXEC = client1 tracker
SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)

all : $(EXEC)

client1: client1.o 
	$(CC) -o client1 client1.o 

client1.o: client1.c
	$(CC) -o client1.o -c client1.c -W -Wall 

tracker: tracker.o 
	$(CC) -o tracker tracker.o 

tracker.o: tracker.c
	$(CC) -o tracker.o -c tracker.c -W -Wall 

clean:
	rm -f $(EXEC) *.o

  

