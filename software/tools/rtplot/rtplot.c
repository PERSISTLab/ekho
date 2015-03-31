#include "rtplot.h"

#define SENSE_RESISTANCE 10
#define CS_GAIN 100
#define VS_GAIN 1
#define COUNT 5000
#define NUMPOINTS 65
unsigned char buf[8];
int port;
double curvedata[2][COUNT];
GLfloat color1[3] = {1.0,1.0,1.0};

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
}

void draw_data()
{
	int i;
	/*  Draw something here */
	glPointSize(4.0);
	glColor3fv( color1 );	
	glBegin(GL_POINTS);
	glVertex2f(0.1, 0.001);
	for(i=0;i<COUNT;i++) {
		glVertex2f(curvedata[0][i], curvedata[1][i]);
	}	
	glEnd();
}
FILE *outfile;
double timestamp=0.0;
uint64_t start;
uint64_t end;
uint64_t elapsed;
uint64_t elapsedNano;
static mach_timebase_info_data_t    sTimebaseInfo;	
void idle_impl()
{
	int n;
	int points = 0;
   	while (1) {
		memset(buf, 0, sizeof(unsigned char) * 6);
		n = read(port, buf, 6);
		if (n < 6) {
			fprintf(stderr, "error reading from port, read %d\n", n);
			continue;
		}
		unsigned int check = ((buf[5] << 8) + buf[4]);
		if (check == 0xFFFF) {
			double current = (((buf[1]) << 8) + buf[0]) * (3.28f / 4096.0f);
			double voltage = (((buf[3]) << 8) + buf[2]) * (3.28f / 4096.0f);
			current = current / ( SENSE_RESISTANCE * CS_GAIN );
			voltage = voltage * 4;
			curvedata[0][points] = voltage;
			curvedata[1][points] = current;
			
			end = mach_absolute_time();
			elapsed = end - start;
			elapsedNano = elapsed * sTimebaseInfo.numer / sTimebaseInfo.denom;
			// Pipe timestamp to both files
			timestamp +=  elapsedNano / 1000000000.0;
			fprintf(outfile, "%.9lf	%.9lf	%.9lf\n", voltage, current, timestamp);
			if (points++ == COUNT - 1) {
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
		start = mach_absolute_time();
	}
}

int main ( int argc, char** argv )   // Create Main Function For Bringing It All Together
{
	outfile = fopen("rawdata.ivp", "w");
	if ( sTimebaseInfo.denom == 0 ) {
		(void) mach_timebase_info(&sTimebaseInfo);
	}
	start = mach_absolute_time();
	if ( sTimebaseInfo.denom == 0 ) {
		(void) mach_timebase_info(&sTimebaseInfo);
	}
	init_serial(argc, argv);
	int n = write(port, buf, 1);
	if (n < 1) {
		fprintf(stderr, "error sending start command to Ekho device\n");
		exit(0);
	}
	run(argc, argv, 4.0, 0.001, 1000, 500);

  return 0;
}

