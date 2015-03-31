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

# added -Wno-deprecated because on Mac 10.10 GLUT functions are all deprecated
CFLAGS=-g -Wall -pedantic -lm -Wno-deprecated

all: rtplot

rtplot: rtplot.c
	$(CC) -DOSTYPE=$(OSTYPE)  rtplot.c $(CFLAGS) $(GL) -o rtplot

test_plot: test_plot.c
	$(CC) -DOSTYPE=$(OSTYPE)  test_plot.c $(CFLAGS) $(GL) -o test_plot

clean:
	rm -rf rtplot test_plot *.o *.dSYM

