CC = gcc
CFLAGS = -std=c11

.PHONY: all clean

all: p2

p2: project2.c project2.h student2.c
	gcc -g project2.c project2.h student2.c -o p2

clean:
	rm -f p2