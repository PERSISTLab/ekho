UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
# Linux
GL=-lglut -lGL -lGLU
GSL=-lgsl -lgslcblas
OSTYPE=0
endif
ifeq ($(UNAME), Darwin)
# Mac
GL=-framework Foundation -framework glut -framework opengl
GSL=-L/usr/local/Cellar/gsl/1.16/lib -lgsl -lgslcblas
OSTYPE=1
endif

CC=g++
CFLAGS=-g -Wall -pedantic -lm

all:
	make clean
	make record

Vector.o: Vector.cpp Vector.h
	$(CC) -g -c Vector.cpp

record: Vector.o
	$(CC) -DOSTYPE=$(OSTYPE) surface_render.cpp Vector.o $(GL) $(CFLAGS) $(GSL)  -o surface_render

clean:
	rm -rf surface_render surface_render.dSYM Vector.o

