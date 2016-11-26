CC = gcc
EXEC = client tracker
SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)

all : $(EXEC)

client:	q1-2_client.o 
	$(CC) -o client q1-2_client.o 

q1-2_client.o: q1-2_client.c
	$(CC) -o q1-2_client.o -c q1-2_client.c -W -Wall 

tracker: q1-2_tracker.o 
	$(CC) -o tracker q1-2_tracker.o 

q1-2_tracker.o: q1-2_tracker.c
	$(CC) -o q1-2_tracker.o -c q1-2_tracker.c -W -Wall 

clean:
	rm -f $(EXEC) *.o

  

