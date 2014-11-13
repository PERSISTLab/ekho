all:
	make clean
	make record

Vector.o: Vector.cpp Vector.h
	g++ -g -c Vector.cpp

record: Vector.o
	g++ surface_render.cpp Vector.o -framework Foundation -framework glut -framework opengl -g -Wall -pedantic -L/usr/local/Cellar/gsl/1.16/lib -lgsl -lgslcblas -lm  -o surface_render

clean:
	rm -rf surface_render surface_render.dSYM Vector.o

