/*
	Make sure you have GSL installed (GNU Scientific Library)
	$ brew install Gsl
*/

#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
// This makes PRIu64 work in printf
#define __STDC_FORMAT_MACROS 
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <gsl/gsl_multifit.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include "Vector.h"


using namespace std;

#define COUNT 500
#define window_width 1240
#define window_height 800
#define ALL_CURVES_LEN 50
#define NUMPOINTS 65
#define DEGREE 5
#define RESISTANCE 410
#define DAC_DIVISION 2.5
#define SHOW_PROGRESS 1

unsigned char buf[8];
int port;
int daqport;
double coeff[DEGREE];
double allcurves[ALL_CURVES_LEN][NUMPOINTS];
double allcurves_path[ALL_CURVES_LEN];
double curvedata[2][COUNT];
int indexInAllCurves = 0;
int indexInBufCurves = 0;
uint64_t time_per_frame;
double dac_map[4097];
double dac_map_mult[4097];
FILE *surface_inf;

/* Rendering constants */
#define xinterval 3.0f
#define xstart (xinterval * (ALL_CURVES_LEN) / 2 * -1)
#define yscale 10000.0f
#define zscale -1.5f
#define time_per_frame 55

double VOLTAGES[NUMPOINTS]  = { 0, 0.12085189123076923, 0.24170378246153845,0.3625556736923077, 0.4834075649230769, 0.6042594561538461, 0.7251113473846154, 0.8459632386153846, 0.9668151298461538, 1.087667021076923, 1.2085189123076923, 1.3293708035384615, 1.4502226947692307, 1.571074586, 1.6919264772307692, 1.8127783684615384, 1.9336302596923076, 2.054482150923077, 2.175334042153846, 2.2961859333846153, 2.4170378246153845, 2.5378897158461537, 2.658741607076923, 2.779593498307692, 2.9004453895384614, 3.0212972807692307, 3.142149172, 3.263001063230769, 3.3838529544615383, 3.5047048456923076, 3.625556736923077, 3.746408628153846, 3.8672605193846152, 3.9881124106153845, 4.108964301846154, 4.229816193076923, 4.350668084307692, 4.471519975538461, 4.592371866769231, 4.713223758, 4.834075649230769, 4.954927540461538, 5.0757794316923075, 5.196631322923077, 5.317483214153846, 5.438335105384615, 5.559186996615384, 5.680038887846154, 5.800890779076923, 5.921742670307692, 6.042594561538461, 6.1634464527692305, 6.284298344, 6.405150235230769, 6.526002126461538, 6.646854017692307, 6.767705908923077, 6.888557800153846, 7.009409691384615, 7.130261582615384, 7.251113473846154, 7.371965365076923, 7.492817256307692, 7.613669147538461, 7.7345210387692305 };

uint16_t dacvalues[65];

/* Lights and color */
GLfloat LightAmbient[] = { 0.05f, 0.05f, 0.05f, 1.0f };
GLfloat LightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat LightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat LightPosition[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat cyan[] = { 0.f, .8f, .8f, 1.f };
GLfloat red[] = { 1.f, .0f, .0f, 1.f };

/* Utility methods */
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
	glTranslatef(0.0f, -35.0f, -100.0f); // Move Left And Into The Screen
	glRotatef(40.0f, 1.0f, 0.0f, 0.0f); 
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, LightSpecular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, cyan);
  
	glColor3f(0.3f, 0.7f, 0.0f);
	glBegin(GL_QUAD_STRIP);
		for (int i = indexInAllCurves, x = ALL_CURVES_LEN; x > 0; i = (i + 1) % ALL_CURVES_LEN, x--) {
	    	int nextCurvendx = (i + 1) % ALL_CURVES_LEN;
			if(x != 1) {
				int j = allcurves_path[i];
				Vector3d topright(xstart + xinterval * (x + 1),
					(float) allcurves[i][j] * yscale + 1.0f,
					(NUMPOINTS - j) * zscale);
				Vector3d topleft(xstart + xinterval * x,
					(float) allcurves[nextCurvendx][j] * yscale + 1.0f,
					(NUMPOINTS - j) * zscale);
				Vector3d bottomleft(xstart + xinterval * x,
					(float) allcurves[nextCurvendx][j + 1] * yscale + 1.0f,
					(NUMPOINTS - j - 1) * zscale);
				Vector3d bottomright(xstart + xinterval
					* (x + 1), (float) allcurves[i][j + 1] * yscale + 1.0f,
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
	
#if SHOW_PROGRESS	
	// Paint most recent curve to last curve, keep pointer to
	// Index in array, then go back wards (modulus) till wrap around
	glColor3f(0.1f, 0.4f, 0.9f);
	glBegin(GL_QUADS);
	for (int i = indexInAllCurves, x = ALL_CURVES_LEN; x > 0; i = (i + 1) % ALL_CURVES_LEN, x--) {
	    int nextCurvendx = (i + 1) % ALL_CURVES_LEN;
	    if(x != 1){
		    for (int j = 0; j < NUMPOINTS - 1; j++) {
					Vector3d topright(xstart + xinterval * (x + 1),
						(float) allcurves[i][j] * yscale ,
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
	}	
	glEnd();
#endif

  glutSwapBuffers();
}

double dac_mult_lookup(double dacvalue) {
	int low = 0;
	int high = 4095;
	int mid = 0;
	
	// edge cases
	if(dacvalue < dac_map[0]) return dac_map_mult[0];
	if(dacvalue > dac_map[4095]) return dac_map_mult[4095];
	
	// binary search
	while (low <= high) {
		mid = (low + high) / 2;
		if(dac_map[mid] >= dacvalue){
			high = mid-1;
		} else {
			low = mid + 1;
		}
	}
	return dac_map_mult[low];
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

unsigned char teensybufo[NUMPOINTS * 2];
int shown = 0;
void timer(int unused)
{
	int n = fread(&allcurves[indexInAllCurves], sizeof(double), NUMPOINTS, surface_inf);

	// Get DAC values for newest curve to play out
	for(int i =0;i<NUMPOINTS;i++) {
		// Convert to dac voltage value
		double dacValueV = ((RESISTANCE * allcurves[indexInAllCurves][i] + VOLTAGES[i]));
		// Use dacmap to convert to actual pre-amplified voltage we should play out with Teensy
		dacValueV = dacValueV * dac_mult_lookup(dacValueV);
		
		// Now convert dac voltage to 12-bit digital 
		dacvalues[i] = (uint16_t)(dacValueV /  (3.28 / 4096));
	  	printf("%d,", dacvalues[i]);	
	}
	
	
#if SHOW_PROGRESS		
	// Get back last voltages played out for last curve from DAQ teensy 
	n = write(daqport, teensybufo, 1);
	if(n < 1) {
		fprintf(stderr, "error asking for update point from daqport\n");
		exit(0);
	}
	n = read(daqport, teensybufo, 2);
	
	if (n < 2) {
		fprintf(stderr, "error reading update point from daqport\n");
		exit(0);
	}
	usleep(10);
#endif
	// Send curve to teensy
	n = write(port, dacvalues, NUMPOINTS * 2);
	printf("sent=%d\n", n);
	if (n < NUMPOINTS * 2) {
		fprintf(stderr, "error writing dac curve to port\n");
		exit(0);
	}

#if SHOW_PROGRESS	
	// Display progress along emulation curve
	uint16_t retADC  = (teensybufo[0] << 8) + teensybufo[1];
	double retvoltage = retADC * (3.28 / 4096) * 4;
	printf(":adc is %" PRIu16 ",=%lf\n", retADC, retvoltage);
	int i=0;
	for(i=1;i<NUMPOINTS;i++) {
		if(retvoltage >= VOLTAGES[i-1] && retvoltage <= VOLTAGES[i])
			break;
	}
	allcurves_path[indexInAllCurves % ALL_CURVES_LEN] = i;
#endif
	
	indexInAllCurves = (indexInAllCurves + 1) % ALL_CURVES_LEN;
	
    glutPostRedisplay();
    glutTimerFunc(time_per_frame, timer, 0);

}

/* Serial init and configuration */
void init_serial(int argc, char **argv) {
	struct termios settings;

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
	
	
#if SHOW_PROGRESS		
	// Open serial daqport
	daqport = open(argv[2], O_RDWR);
	if (daqport < 0) {
		fprintf(stderr, "Unable to open %s\n", argv[2]);
		exit(0);
	}

	// Configure the daqport
	tcgetattr(daqport, &settings);
	cfmakeraw(&settings);
	tcsetattr(daqport, TCSANOW, &settings);
#endif	
}

void loadmap(char mapfname[], double maparray[]) {
	// Load DAC map
	FILE * dacFile=fopen(mapfname, "r");
	char line[128];
	int ndx=0;
	while(fgets(line, 128, dacFile) != NULL) {
		maparray[ndx++] = atof(line);
	}
	// Edge case
	maparray[ndx]=maparray[ndx-1];
	fclose(dacFile);
}

int main(int argc, char **argv) {
	if (argc < 2) {
		fprintf(stderr, "Usage:   emulate <emulate port> <daq port>(optional) <surface file (optional)>\n");
		exit(0);
	}
	// Sleep for a second
	usleep(1000000);
	
	// Load file
	if(argc > 3) {
		surface_inf=fopen(argv[3], "r");
	} else {
		surface_inf=fopen("surface.raw", "r");
	}
	
	// Load DAC maps
	char dm_literal[] = "febprofile/dacmap.dat";
	loadmap(dm_literal, dac_map);
	char dmm_literal[] = "febprofile/dacmapmult.dat";
	loadmap(dmm_literal, dac_map_mult);
	
	// Now init serial to emulation teensy, and setup OpenGL surface rendering
	init_serial(argc, argv);
	glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(window_width, window_height);

    //create the window, the argument is the title
    glutCreateWindow("Ekho IV-Emulate View");
	// Init viewport
	initGL();
    //pass the callbacks
    glutDisplayFunc(display);
   	glutTimerFunc(time_per_frame, timer, 0);
	
	// Start loop
    glutMainLoop();
	fclose(surface_inf);
	close(port);
	return 0;
}
