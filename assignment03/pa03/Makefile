CC ?= cc
CFLAGS ?= -Wall -Wextra -std=c17 -g

PROGRAMS = filetool
OBJECTS = main.o file_ops.o

.PHONY: all clean

all: $(PROGRAMS)

filetool: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS)

main.o: main.c file_ops.h
	$(CC) $(CFLAGS) -c main.c

file_ops.o: file_ops.c file_ops.h
	$(CC) $(CFLAGS) -c file_ops.c
	
clean:
	rm -f $(PROGRAMS) $(OBJECTS)

