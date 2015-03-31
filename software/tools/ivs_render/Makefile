CC=clang++
BREW=-I/usr/local/include -L/usr/local/lib
GSL=-I/usr/local/Cellar/gsl/1.16/include  -lgsl -lgslcblas

ifeq ("$(shell uname)", "Darwin")
	GL= -framework Foundation -framework GLUT -framework OpenGL
else
	GL= -lglut -lGL -lGLU -lm
endif

# added -Wno-deprecated because on Mac 10.10 GLUT functions are all deprecated
CFLAGS=-g -Wall -pedantic -lm -Wno-deprecated

all: ivs_render

ivs_render: Vector.o ivs_render.cpp
	$(CC) $(CFLAGS) ivs_render.cpp -o ivs_render Vector.o $(GL) $(BREW) $(GSL)

Vector.o: Vector.cpp Vector.h
	$(CC) -g -c Vector.cpp

clean:
	rm -rf ivs_render *.dSYM *.o
