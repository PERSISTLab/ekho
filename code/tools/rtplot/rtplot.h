#if OSTYPE
	// Mac 
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
	#include <GLUT/glut.h>
#else
	// Linux
	#include <GL/glut.h>
	#include <GL/gl.h>
	#include <GL/glu.h>
	#include <GL/glext.h>
#endif

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <mach/mach.h>
#include <mach/mach_time.h>

#define __STDC_FORMAT_MACROS

double orig_xsize;
double orig_ysize;
double xsize;
double ysize;
double x_plot_offset;
double y_plot_offset;
double window_width;
double window_height;

void draw_data();
void idle_impl();

void draw_text_labels() {
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
//	gluOrtho2D(0.0, 1000, 0.0, 500);
	glOrtho(0,xsize+xsize/10, 0,ysize+ysize/10, -0.2,0.2);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	
	glColor3d(1.0, 1.0, 1.0);
	void * font = GLUT_BITMAP_8_BY_13;	
	char buf[200];
	for(int i=0;i<11;i++) {
		sprintf(buf, "%.1fV", (xsize / 10) * i);
		glRasterPos2f((xsize / 10)* i + x_plot_offset, 0);
		for (int j=0;j<strlen(buf);j++)
		{
			  glutBitmapCharacter(font, buf[j]);
		}
	}
	for(int i=1;i<11;i++) {
		sprintf(buf, "%.4fA", (ysize / 10) * i);
		glRasterPos2f(0, (ysize / 10) * i + y_plot_offset);
		for (int j=0;j<strlen(buf);j++)
		{
			  glutBitmapCharacter(font, buf[j]);
		}
	}

	glMatrixMode(GL_PROJECTION); //swapped this with...
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW); //...this
	glPopMatrix();
}

void display ()
{
	int i;

	/*  Clear the image */
	glClear(GL_COLOR_BUFFER_BIT);
	/*  Reset previous transforms */
	glLoadIdentity();
	glPushMatrix();
	glTranslatef(x_plot_offset, y_plot_offset, 0.0);
	/* Draw graph lines */
	glLineWidth(.6); 
	glColor3f(0.2, 0.2, 0.2);
	for(i=0;i<11;i++) {
		glBegin(GL_LINES);
		glVertex2f(0.0, (ysize / 10) * i);
		glVertex2f(xsize, (ysize / 10) * i);
		glEnd();		
	}
	
	for(i=0;i<11;i++) {
		glBegin(GL_LINES);
		glVertex2f((xsize / 10) * i, 0);
		glVertex2f((xsize / 10) * i, ysize);
		glEnd();		
	}
	
	/*  Draw something here */
	draw_data();
	glPopMatrix();
	
	// Draw axis labels
	draw_text_labels();

	/*  Flush and swap */
	glFlush();
	glutSwapBuffers();
}

void reshape(int width,int height)
{
	window_width=width;
	window_height=height;
	glViewport(0,0, width,height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,xsize+xsize/10, 0,ysize+ysize/10, -0.2,0.2);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void keyboard(unsigned char key, int x, int y) {
	/*  Exit on ESC */
	if (key == 27) {
		exit(0);
	}
	/* Zoom-in */
	if (key == 'z') {
		xsize+=orig_xsize*0.5;
		ysize+=orig_ysize*0.5;
		reshape(window_width, window_height);
	}
	/* Zoom-out */
	if (key == 'x') {
		xsize-=orig_xsize*0.5;
		ysize-=orig_ysize*0.5;
		reshape(window_width, window_height);
	}

}

void idle()
{
	idle_impl();
	glutPostRedisplay();
}
int scale=10;
void run(int argc, char** argv, double _xsize, double _ysize, double _window_width, double _window_height) {
	
	xsize=orig_xsize=_xsize;
	ysize=orig_ysize=_ysize;
	x_plot_offset=xsize/40.0;
	y_plot_offset=ysize/20.0;
	window_width=_window_width;
 	window_height=_window_height;
	glutInit( &argc, argv );	
	glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE ); // Display Mode
	glutInitWindowSize( window_width, window_height); // If glutFullScreen wasn't called this is the window size
	glutCreateWindow( "OpenGL Graph Component" ); // Window Title (argv[0] for current directory as title)
	glutDisplayFunc( display );  // Matching Earlier Functions To Their Counterparts
	glutReshapeFunc( reshape );
	glutKeyboardUpFunc( keyboard );
	glutIdleFunc(idle);
	glutMainLoop();
}
