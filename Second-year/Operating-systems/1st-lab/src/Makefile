CC = gcc
CFLAGS = -Wall -Werror -Wextra -O2

PROG1 = crear
PROG2 = combine


SRC1 = crear.c
SRC2 = combine.c


OBJ1 = $(SRC1:.c=.o)
OBJ2 = $(SRC2:.c=.o)

all: $(PROG1) $(PROG2) 

$(PROG1): $(OBJ1)
	$(CC) $(CFLAGS) -o $@ $^

$(PROG2): $(OBJ2)
	$(CC) $(CFLAGS) -o $@ $^


%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(PROG1) $(PROG2) $(OBJ1) $(OBJ2) 

distclean: clean
	rm -f *~
