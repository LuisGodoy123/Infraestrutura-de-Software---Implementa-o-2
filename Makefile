CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c11 -fopenmp -pthread

all: mandelbrot

mandelbrot: mandelbrot.c mandelbrot.h
	$(CC) $(CFLAGS) -o mandelbrot mandelbrot.c

clean:
	rm -f mandelbrot
