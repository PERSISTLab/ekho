#include "rtplot.h"
GLfloat color1[3] = {1.0,1.0,1.0};
void draw_data() {	
	/*  Draw something here */
	glPointSize(4.0);
	glColor3fv( color1 );	
	glBegin(GL_POINTS);
	glVertex2f(1.0, 0.0005);	
	glEnd();	
}

void idle_impl() {
	// Dont do anything in this one
}	

int main(int argc, char** argv)
{
	run(argc, argv, 4.0, 0.001, 1000, 500);
	return 0;
}

