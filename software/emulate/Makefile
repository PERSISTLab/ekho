all:
	make clean
	make emulate

Vector.o: Vector.cpp Vector.h
	g++ -g -c Vector.cpp

emulate: Vector.o
	g++ emulate_render.cpp Vector.o -framework Foundation -framework glut -framework opengl -g -Wall -pedantic -L/usr/local/Cellar/gsl/1.16/lib -lgsl -lgslcblas -lm  -o emulate_render

clean:
	rm -rf emulate_render emulate_render.dSYM Vector.o

