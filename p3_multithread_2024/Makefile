CC=gcc
FLAGS=-g -Wall -Werror
OBJ= queue store_manager
LIBS= -pthread

all:  $(OBJ)
	@echo "***************************"
	@echo "Compilation successfully!"
	@echo ""

queue: queue.c
	$(CC) -c queue.c

store_manager:	store_manager.c
	$(CC) $(FLAGS) $(LIBS) -o store_manager  store_manager.c queue.c

load:
	ld -o store_manager queue.o

clean:
	rm -f store_manager *.o
	@echo "***************************"
	@echo "Deleted files!"
	@echo  ""

