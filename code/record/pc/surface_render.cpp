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

#define BAUD_RATE 256000
#define COUNT 500
#define window_width 1240
#define window_height 800
#define ALL_CURVES_LEN 100
#define NUMPOINTS 65
#define DEGREE 5
unsigned char buf[8];
int port;
double coeff[DEGREE];
double allcurves[ALL_CURVES_LEN][NUMPOINTS];
double curvedata[2][COUNT];
int indexInAllCurves = 0;
FILE *surface_outf;

/* Rendering constants */
#define xinterval 1.5f
#define xstart (xinterval * (ALL_CURVES_LEN) / 2 * -1)
#define yscale 1000.0f
#define zscale -1.5f

double VOLTAGES[NUMPOINTS]  = { 0, 0.12085189123076923, 0.24170378246153845,0.3625556736923077, 0.4834075649230769, 0.6042594561538461, 0.7251113473846154, 0.8459632386153846, 0.9668151298461538, 1.087667021076923, 1.2085189123076923, 1.3293708035384615, 1.4502226947692307, 1.571074586, 1.6919264772307692, 1.8127783684615384, 1.9336302596923076, 2.054482150923077, 2.175334042153846, 2.2961859333846153, 2.4170378246153845, 2.5378897158461537, 2.658741607076923, 2.779593498307692, 2.9004453895384614, 3.0212972807692307, 3.142149172, 3.263001063230769, 3.3838529544615383, 3.5047048456923076, 3.625556736923077, 3.746408628153846, 3.8672605193846152, 3.9881124106153845, 4.108964301846154, 4.229816193076923, 4.350668084307692, 4.471519975538461, 4.592371866769231, 4.713223758, 4.834075649230769, 4.954927540461538, 5.0757794316923075, 5.196631322923077, 5.317483214153846, 5.438335105384615, 5.559186996615384, 5.680038887846154, 5.800890779076923, 5.921742670307692, 6.042594561538461, 6.1634464527692305, 6.284298344, 6.405150235230769, 6.526002126461538, 6.646854017692307, 6.767705908923077, 6.888557800153846, 7.009409691384615, 7.130261582615384, 7.251113473846154, 7.371965365076923, 7.492817256307692, 7.613669147538461, 7.7345210387692305 };

/* Lights and color */
GLfloat LightAmbient[] = { 0.05f, 0.05f, 0.05f, 1.0f };
GLfloat LightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat LightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat LightPosition[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat cyan[] = { 0.f, .8f, .8f, 1.f };

/* Utility methods */
bool polynomialfit(int obs, int degree, double *dx, double *dy, double *store) /* n, p */
{
  gsl_multifit_linear_workspace *ws;
  gsl_matrix *cov, *X;
  gsl_vector *y, *c;
  double chisq;
 
  int i, j;
 
  X = gsl_matrix_alloc(obs, degree);
  y = gsl_vector_alloc(obs);
  c = gsl_vector_alloc(degree);
  cov = gsl_matrix_alloc(degree, degree);
 
  for(i=0; i < obs; i++) {
    gsl_matrix_set(X, i, 0, 1.0);
    for(j=0; j < degree; j++) {
      gsl_matrix_set(X, i, j, pow(dx[i], j));
    }
    gsl_vector_set(y, i, dy[i]);
  }
 
  ws = gsl_multifit_linear_alloc(obs, degree);
  gsl_multifit_linear(X, y, c, cov, &chisq, ws);
 
  /* store result ... */
  for(i=0; i < degree; i++)
  {
    store[i] = gsl_vector_get(c, i);
  }
 
  gsl_multifit_linear_free(ws); 
  gsl_vector_free(c);
  return true; /* we do not "analyse" the result (cov matrix mainly)
		  to know if the fit is "good" */
}

double polycurve(double voltage, double *coeffs, int degree) {
	double ret = coeffs[1] * voltage + coeffs[0];
	int i;
	for(i=2;i<degree;i++) {
		ret+= coeffs[i] * gsl_pow_int(voltage,i);
	}
	return ret;
}

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

void sort(double array[2][COUNT]) {
	for (int i = 1; i < COUNT; i++) {
		int j = i;
		while (j > 0 && array[0][j - 1] > array[0][j]) {
			double tempx = array[0][j - 1];
			double tempy = array[1][j - 1];
			// x
			array[0][j - 1] = array[0][j];
			array[0][j] = tempx;
			// y
			array[1][j - 1] = array[1][j];
			array[1][j] = tempy;
			j = j - 1;
		}
	}
}

double clamp_current(double current) {
	return fmin(fmax(current, 0), 0.1f);
}

void create_curves_with_regression() {
	double tox[3];
	double toy[3];
	
	// Clear curve first to all zeros
	for (int j = 0; j < NUMPOINTS - 1; j++) {
		allcurves[indexInAllCurves][j] = 0;
	}

	// Sort curve data cloud on Voltage
	sort(curvedata);

	// Get bounds for polyline
	double leftmostpoint = curvedata[0][0];
	double rightmostpoint = curvedata[0][COUNT - 1];

	// Get poly regression of all points
	polynomialfit(COUNT, DEGREE, curvedata[0], curvedata[1], coeff);
	int lastIndex = 63;
	int firstIndex = 0;
	for (int j = 0; j < NUMPOINTS - 1; j++) {
		if (VOLTAGES[j] < leftmostpoint) {
			// Get the index where the regression starts in allcurves
			firstIndex = j + 1;
		}
		if (VOLTAGES[j] > leftmostpoint && VOLTAGES[j] < rightmostpoint) {
			allcurves[indexInAllCurves][j] = clamp_current(polycurve(VOLTAGES[j], coeff, DEGREE));
		}
		if (VOLTAGES[j] > rightmostpoint) {
			lastIndex = j;
			break;
		}
	}
	
	// Draw a line through left part past point cloud, using points already
	// in curve
	memcpy(tox, &VOLTAGES[firstIndex+1], sizeof(double) * 3);
	memcpy(toy, &allcurves[indexInAllCurves][firstIndex+1], sizeof(double) * 3);
	polynomialfit(3, 2, tox, toy, coeff); 
	for (int j = 0; j < firstIndex; j++) {
		allcurves[indexInAllCurves][j] = clamp_current(polycurve(VOLTAGES[j], coeff, 2));
	}
	
	// Draw a line through right part past point cloud, using points already
	// in curve
	// 2-degree since it will most likely curve into zero
	memcpy(tox, &VOLTAGES[lastIndex - 3], sizeof(double) * 3);
	memcpy(toy, &allcurves[indexInAllCurves][lastIndex - 3],sizeof(double) * 3);	
	polynomialfit(3, 2, tox, toy, coeff);

	// We only want a negative slope between two points, if not negative
	// then render it at zero height
	for (int j = lastIndex; j < NUMPOINTS - 1; j++) {
		if (coeff[1] < 0) {
			allcurves[indexInAllCurves][j] = clamp_current(polycurve(VOLTAGES[j], coeff, 2));
		} else {
			allcurves[indexInAllCurves][j] = 0;
		}
	}
}

/* OpenGL display / idle stuff */
Vector3d rectpoints[4];
void display(void) {
   	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
	glLoadIdentity(); // Reset The View
	glTranslatef(0.0f, -35.0f, -100.0f); // Move Left And Into The Screen
	glRotatef(40.0f, 1.0f, 0.0f, 0.0f); 
	glColor3f(0.1f, 0.4f, 0.9f);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, LightSpecular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, cyan);

	// Paint most recent curve to last curve, keep pointer to
	// Index in array, then go back wards (modulus) till wrap around
	glBegin(GL_QUADS);
	for (int i = indexInAllCurves, x = ALL_CURVES_LEN; x > 0; i = (i + 1) % ALL_CURVES_LEN, x--) {
	    int nextCurvendx = (i + 1) % ALL_CURVES_LEN;
	    if(x != 1)
	    for (int j = 0; j < NUMPOINTS - 1; j++) {
			Vector3d topright(xstart + xinterval * (x + 1),
				(float) allcurves[i][j] * yscale,
				(NUMPOINTS - j) * zscale);
			Vector3d topleft(xstart + xinterval * x,
				(float) allcurves[nextCurvendx][j] * yscale,
				(NUMPOINTS - j) * zscale);
			Vector3d bottomleft(xstart + xinterval * x,
				(float) allcurves[nextCurvendx][j + 1] * yscale,
				(NUMPOINTS - j - 1) * zscale);
			Vector3d bottomright(xstart + xinterval
				* (x + 1), (float) allcurves[i][j + 1] * yscale,
				(NUMPOINTS - j - 1) * zscale);
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
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // Black Background
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
	int n;
	int points = 0;
   	while (1) {
		memset(buf, 0, sizeof(unsigned char) * 6);
		n = read(port, buf, 6);
		if (n < 6) {
			fprintf(stderr, "error reading from port, read %d\n", n);
			//break;
		}
		unsigned int check = ((buf[5] << 8) + buf[4]);
		if (check == 0xFFFF) {
			double current = (((buf[1]) << 8) + buf[0]) * (3.3f / 4096.0f);
			double voltage = (((buf[3]) << 8) + buf[2]) * (3.3f / 4096.0f);
			current = current / 100;
			voltage = voltage * 4;
			curvedata[0][points] = voltage;
			curvedata[1][points] = current;
			if (points++ == COUNT - 1) {
				create_curves_with_regression();
				
				// Pipe curve output to next program
/*				for(int i = 0;i<NUMPOINTS-1;i++) {
					printf("%lf,", allcurves[indexInAllCurves][i]);
				}
				printf("%lf\n", allcurves[indexInAllCurves][NUMPOINTS-1]); */
//				fwrite(&allcurves[indexInAllCurves], sizeof(double), NUMPOINTS, stdout);
				fwrite(&allcurves[indexInAllCurves], sizeof(double), NUMPOINTS, surface_outf);
				// Prepare for next curve
				indexInAllCurves = (indexInAllCurves + 1) % ALL_CURVES_LEN;
				points = 0;
				
				// Update display
				break;
			}
		} else {
			n = read(port, buf, 1);
			if (n < 1) {
				fprintf(stderr, "error reading from port\n");
				break;
			}
		}
	}
	glutPostRedisplay();
}

/* Serial init and configuration */
void init_serial(int argc, char **argv) {
	struct termios settings;
	if (argc < 2) {
		fprintf(stderr, "Usage:   serial_listen <port>\n");
		exit(0);
	}

	// Open the serial port
	port = open(argv[1], O_RDWR);
	if (port < 0) {
		fprintf(stderr, "Unable to open %s\n", argv[1]);
		exit(0);
	}

	// Configure the port
	tcgetattr(port, &settings);
	cfmakeraw(&settings);
	tcsetattr(port, TCSANOW, &settings);	
}

int main(int argc, char **argv) {
	surface_outf = fopen("surface.raw", "w");
	init_serial(argc, argv);
	int n = write(port, buf, 1);
	if (n < 1) {
		fprintf(stderr, "error writing to port\n");
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
    glutIdleFunc(idle);
	
	// Start loop
    glutMainLoop();
	fclose(surface_outf);
	close(port);
	return 0;
}
