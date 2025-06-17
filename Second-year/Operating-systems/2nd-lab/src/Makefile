# NO TOCAR / NOT MODIFIED ME ##
CC=gcc
FLAGS=-Wno-implicit-function-declaration
CFLAGS=-I. -g
###############################

# MODIFIED ME ##

OBJ1 = scripter.o
OBJ2 = mygrep.o

all: scripter mygrep

%.o: %.c 
	$(CC) $(FLAGS) -c -o $@ $< $(CFLAGS)

scripter: $(OBJ1)
	$(CC) $(FLAGS) -L. -o $@ $< $(LIBS)

mygrep: $(OBJ2)
	$(CC) $(FLAGS) -L. -o $@ $< $(LIBS)

clean:
	rm -f ./scripter.o ./scripter ./mygrep.o ./mygrep
