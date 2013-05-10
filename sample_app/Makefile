CC = gcc
CFLAGS = -Wall -ggdb -fopenmp
SOURCES = lin_sys_sol.c
EXECUTABLE = lin_sys_sol
LIBS = -lm

all: add
	$(CC) $(CFLAGS) $(SOURCES) $(LIBS) -o $(EXECUTABLE)

add:
	$(CC) -Wall -g add.c -o add
	
clean:
	rm -rf *.o

very_clean:
	rm -rf *.o $(EXECUTABLE)
