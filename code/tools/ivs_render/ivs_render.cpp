/*
	Make sure you have GSL installed (GNU Scientific Library)
	$ brew install Gsl
*/

#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <gsl/gsl_multifit.h>
#include "Vector.h"
#define __STDC_FORMAT_MACROS

using namespace std;

#define window_width 1240
#define window_height 800
#define NUMPOINTS 65

/* Rendering constants */
int all_curves_len;
double xinterval;
double xstart;
double yscale=10000.0f;
double zscale= -1.5f;

double *allcurves;
int indexInAllCurves = 0;
FILE *surface_inf;

double VOLTAGES[NUMPOINTS]  = { 0, 0.12085189123076923, 0.24170378246153845,0.3625556736923077, 0.4834075649230769, 0.6042594561538461, 0.7251113473846154, 0.8459632386153846, 0.9668151298461538, 1.087667021076923, 1.2085189123076923, 1.3293708035384615, 1.4502226947692307, 1.571074586, 1.6919264772307692, 1.8127783684615384, 1.9336302596923076, 2.054482150923077, 2.175334042153846, 2.2961859333846153, 2.4170378246153845, 2.5378897158461537, 2.658741607076923, 2.779593498307692, 2.9004453895384614, 3.0212972807692307, 3.142149172, 3.263001063230769, 3.3838529544615383, 3.5047048456923076, 3.625556736923077, 3.746408628153846, 3.8672605193846152, 3.9881124106153845, 4.108964301846154, 4.229816193076923, 4.350668084307692, 4.471519975538461, 4.592371866769231, 4.713223758, 4.834075649230769, 4.954927540461538, 5.0757794316923075, 5.196631322923077, 5.317483214153846, 5.438335105384615, 5.559186996615384, 5.680038887846154, 5.800890779076923, 5.921742670307692, 6.042594561538461, 6.1634464527692305, 6.284298344, 6.405150235230769, 6.526002126461538, 6.646854017692307, 6.767705908923077, 6.888557800153846, 7.009409691384615, 7.130261582615384, 7.251113473846154, 7.371965365076923, 7.492817256307692, 7.613669147538461, 7.7345210387692305 };

/* Lights and color */
GLfloat LightAmbient[] = { 0.05f, 0.05f, 0.05f, 1.0f };
GLfloat LightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat LightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat LightPosition[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat cyan[] = { 0.f, .8f, .8f, 1.f };


Vector3d calc_normal(Vector3d vertex[]) {
	Vector3d normal;
	for (int i = 0; i < 4; i++) {
	    int j = (i + 1) % 4;
	    normal.x += (vertex[i].y - vertex[j].y) * (vertex[i].z + vertex[j].z);
	    normal.y += (vertex[i].z - vertex[j].z) * (vertex[i].x + vertex[j].x);
	    normal.z += (vertex[i].x - vertex[j].x) * (vertex[i].y + vertex[j].y);
	}

	// |v| = sqrt(x^2 + y^2 + z^2)
	float mag = (float) sqrtf(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
	normal.x /= mag;
	normal.y /= mag;
	normal.z /= mag;
	return normal;
}

/* OpenGL display / idle stuff */
Vector3d rectpoints[4];
void display(void) {
   	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
	glLoadIdentity(); // Reset The View
	glTranslatef(-1*xstart, -35.0f, -100.0f); // Move Left And Into The Screen
	glRotatef(40.0f, 1.0f, 0.0f, 0.0f); 
	glColor3f(0.1f, 0.4f, 0.9f);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, LightSpecular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, cyan);

	// Paint most recent curve to last curve, keep pointer to
	// Index in array, then go back wards (modulus) till wrap around
	glBegin(GL_QUADS);
	for (int x = 0; x < all_curves_len-1;x++) {
	    for (int y = 0; y < NUMPOINTS - 1; y++) {
			printf("%lf,",allcurves[(x + 1) * (NUMPOINTS + 1) + y + 1]);
			Vector3d topright(xinterval * (x + 1),
				(float) allcurves[(x + 1) * (NUMPOINTS + 1) + y + 1] * yscale,
				(NUMPOINTS - y) * zscale);
				
			Vector3d topleft(xinterval * x,
				(float) allcurves[x * (NUMPOINTS + 1) + y + 1] * yscale,
				(NUMPOINTS - y) * zscale);
				
			Vector3d bottomleft(xinterval * x,
				(float) allcurves[x * (NUMPOINTS + 1) + y + 2] * yscale,
				(NUMPOINTS - y - 1) * zscale);
				
			Vector3d bottomright(xinterval * (x + 1),
				(float) allcurves[(x+1) * (NUMPOINTS + 1) + y + 2] * yscale,
				(NUMPOINTS - y - 1) * zscale);
				
			rectpoints[0] = topright;
			rectpoints[1] = topleft;
			rectpoints[2] = bottomleft;
			rectpoints[3] = bottomright;
			Vector3d normal = calc_normal(rectpoints);

			glNormal3f(normal.x, normal.y, normal.z);
			glVertex3f(topright.x, topright.y, topright.z); // Top right
			glVertex3f(topleft.x, topleft.y, topleft.z); // Top left
			glVertex3f(bottomleft.x, bottomleft.y, bottomleft.z); // Bottom left
			glVertex3f(bottomright.x, bottomright.y, bottomright.z); // Bottom right
	    }
		printf("\n");
	}
	glEnd();
    glutSwapBuffers();
}


void initGL() {
	
	int width=window_width;
	int height=window_height;
	glViewport(0, 0, window_width, window_height); // Reset The Current Viewport
	glMatrixMode(GL_PROJECTION); // Select The Projection Matrix
	glLoadIdentity(); // Reset The Projection Matrix
	gluPerspective(45.0f, ((float) width) / ((float) height), 0.1f, 1000.0f); // Calculate The Aspect Ratio Of The Window
	glMatrixMode(GL_MODELVIEW); // Select The Modelview Matrix
	glLoadIdentity(); // Reset The Modelview Matrix

	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_SMOOTH); // Enables Smooth Shading
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // Black Background
	glClearDepth(1.0f); // Depth Buffer Setup

	glEnable(GL_DEPTH_TEST); // Enables Depth Testing
	glDepthFunc(GL_LEQUAL); // The Type Of Depth IntMath To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); 
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, LightAmbient);
	glMaterialf(GL_FRONT, GL_SHININESS, 100.0f);
	glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient); // Setup The Ambient Light
	glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse); // Setup The
	glLightfv(GL_LIGHT1, GL_SPECULAR, LightSpecular);
	glLightfv(GL_LIGHT1, GL_POSITION, LightPosition); // Position
	glEnable(GL_LIGHT1); // Enable Light One
	glEnable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE); 
}


void reshape(int width, int height) {
//	initGL();
}

void idle(void) {
	glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y) {
	/*  Exit on ESC */
	if (key == 27) {
		exit(0);
	}
	if (key == 'a') {
		exit(0);
	}
	if (key == 'd') {
		exit(0);
	}
}

int main(int argc, char **argv) {
	if (argc < 2) {
		fprintf(stderr, "Usage:   surface_render <ivs_file>\n");
		exit(0);
	}
	struct stat st;
	stat(argv[1], &st);
	long size = st.st_size;
	all_curves_len=size / (sizeof(double) * (NUMPOINTS + 1));
	printf("Num size=%ld curves=%d\n", size, all_curves_len);
	int num_expected_doubles = all_curves_len * (NUMPOINTS + 1);
	xinterval=1.5f * 100 / all_curves_len;
	xstart=(xinterval * (all_curves_len) / 2);
	allcurves=(double*)malloc(sizeof(double) * num_expected_doubles);
	
	surface_inf = fopen(argv[1], "r");
	fseek(surface_inf, 0, SEEK_SET);
	int count = fread(allcurves, sizeof(double), num_expected_doubles, surface_inf);
	printf("count=%d num_expected_doubles=%d\n", count, num_expected_doubles);
	if(count != num_expected_doubles) {
		fprintf(stderr, "could not read entire surface into memory\n");
		free(allcurves);
		exit(0);
	}
	glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(window_width, window_height);

    //create the window, the argument is the title
    glutCreateWindow("Ekho IV-Surface View");
	// Init viewport
	initGL();
	//pass the callbacks
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardUpFunc(keyboard);
    glutIdleFunc(idle);
	
	// Start loop
	glutMainLoop();
	free(allcurves);
	fclose(surface_inf);
	return 0;
}
