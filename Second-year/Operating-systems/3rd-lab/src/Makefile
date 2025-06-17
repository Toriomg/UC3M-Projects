CC=gcc
CFLAGS=-g -Wall -Werror
OBJ= queue factory_manager
LIBS= -pthread

all:  $(OBJ)
	@echo "***************************"
	@echo "Compilation successfully!"
	@echo ""

queue: queue.c
	$(CC) -c queue.c

factory_manager:	factory_manager.c
	$(CC) $(CFLAGS) $(LIBS) -o factory  factory_manager.c process_manager.c queue.c


clean:
	rm -f factory process *.o
	@echo "***************************"
	@echo "Deleted files!"
	@echo  ""

