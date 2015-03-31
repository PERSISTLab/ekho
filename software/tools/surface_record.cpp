/*
	Make sure you have GSL installed (GNU Scientific Library)
	$ brew install Gsl
	For old EKHO GAIN is determined by the A0 A1 pins, note that they are pulled down by default:
	
			A1 		A0 		Gain
			Low 	Low 	 1
			Low 	High 	 10
			High 	Low 	 100
			High 	High 	 1000	

Generates two files, *.ivs and *.ivp


*/

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
#include <mach/mach.h>
#include <mach/mach_time.h>

#if AUDIO
#include <portaudio.h>
#endif

#define __STDC_FORMAT_MACROS

using namespace std;

#define BAUD_RATE 256000
#define COUNT 500
#define window_width 1240
#define window_height 800
#define ALL_CURVES_LEN 100
#define NUMPOINTS 65
#define DEGREE 4
#define SENSE_RESISTANCE 10
#define CS_GAIN 100
#define VS_GAIN 1

unsigned char buf[8];
int port;
double coeff[DEGREE];
double allcurves[ALL_CURVES_LEN][NUMPOINTS];
double curvedata[2][COUNT];
int indexInAllCurves = 0;
FILE *surface_outf;
FILE *rawpairs_outf;
double cs_map[4097];
double cs_map_mult[4097];
double vs_map[4097];
double vs_map_mult[4097];

double VOLTAGES[NUMPOINTS]  = { 0, 0.12085189123076923, 0.24170378246153845,0.3625556736923077, 0.4834075649230769, 0.6042594561538461, 0.7251113473846154, 0.8459632386153846, 0.9668151298461538, 1.087667021076923, 1.2085189123076923, 1.3293708035384615, 1.4502226947692307, 1.571074586, 1.6919264772307692, 1.8127783684615384, 1.9336302596923076, 2.054482150923077, 2.175334042153846, 2.2961859333846153, 2.4170378246153845, 2.5378897158461537, 2.658741607076923, 2.779593498307692, 2.9004453895384614, 3.0212972807692307, 3.142149172, 3.263001063230769, 3.3838529544615383, 3.5047048456923076, 3.625556736923077, 3.746408628153846, 3.8672605193846152, 3.9881124106153845, 4.108964301846154, 4.229816193076923, 4.350668084307692, 4.471519975538461, 4.592371866769231, 4.713223758, 4.834075649230769, 4.954927540461538, 5.0757794316923075, 5.196631322923077, 5.317483214153846, 5.438335105384615, 5.559186996615384, 5.680038887846154, 5.800890779076923, 5.921742670307692, 6.042594561538461, 6.1634464527692305, 6.284298344, 6.405150235230769, 6.526002126461538, 6.646854017692307, 6.767705908923077, 6.888557800153846, 7.009409691384615, 7.130261582615384, 7.251113473846154, 7.371965365076923, 7.492817256307692, 7.613669147538461, 7.7345210387692305 };


#if AUDIO
/** Sine wave generation */
#define PI 3.14159265358979323846264338327950288
#define SAMPLE_RATE 44100
#define TABLE_SIZE SAMPLE_RATE
#define FRAMES_PER_BUFFER 256

typedef struct _testData {
  float sine[TABLE_SIZE];
  int phase;
} TestData;

static int paCallback( const void *inputBuffer,
			 void *outputBuffer, unsigned long framesPerBuffer,
			 const PaStreamCallbackTimeInfo* timeInfo,
			 PaStreamCallbackFlags statusFlags, void *userData ) 
{
	TestData *data = (TestData*) userData;
	float *out = (float*) outputBuffer;
	float sample;
	int i;

	for (i = 0; i < framesPerBuffer; i++) {
		sample = data->sine[data->phase++];
		*out++ = sample; /* left */
		*out++ = sample; /* right */
		if (data->phase >= TABLE_SIZE)
			data->phase -= TABLE_SIZE;
	}
	return paContinue;
}
/** End sine wave generation */
#endif

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

void create_curves_with_regression_old() {
	double tox[30];
	double toy[30];
	
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
	memcpy(tox, &VOLTAGES[lastIndex - 10], sizeof(double) * 10);
	memcpy(toy, &allcurves[indexInAllCurves][lastIndex - 10],sizeof(double) * 10);	
	polynomialfit(10, 2, tox, toy, coeff);

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
	
	// We only want positive currents on back half of the curve, so get highest point in curve
	// and draw a straight line back using highest point and point to the right of it to zero
	int indexforhighpoint=NUMPOINTS-1;
	for(int j = NUMPOINTS-1;j>0;j--) {
		if(allcurves[indexInAllCurves][j] > allcurves[indexInAllCurves][indexforhighpoint]) {
			indexforhighpoint = j;
		}
	}
	memcpy(tox, &VOLTAGES[indexforhighpoint], sizeof(double) * 2);
	memcpy(toy, &allcurves[indexInAllCurves][indexforhighpoint],sizeof(double) * 2);	
	polynomialfit(2, 2, tox, toy, coeff);
	for(int j= 0;j<indexforhighpoint;j++) {
		allcurves[indexInAllCurves][j] = clamp_current(polycurve(VOLTAGES[j], coeff, 2));
	}
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

int main(int argc, char *argv[]) {
	char output_file_name[255];
	char output_rawpairs_name[255];
	double record_time_ms = 0.0;
	if (argc < 3) {
		fprintf(stderr, "This program records IV-surfaces with no rendering, and can also generate sine waves to test kinetic.\n\n	Usage:   surface_render <port> <record_time_ms> <surface_file> (optional)<sine_frequency> (optional)<sine_amp>\n	Add in sine frequency and amplitude if also generating tones for capturing surfaces (and compiled with recordaudio)\n");
		exit(0);
	}
	int tempint = atoi(argv[2]);
	if(tempint < 1) {
		fprintf(stderr, "record_time_ms needs to be positive and longer than 1 millisecond\n");
		exit(0);
	}
#if AUDIO
	if (argc < 5) {
		fprintf(stderr, "Add Hz for fourth argument");
		exit(0);
	}
	float sinefrequ = atof(argv[4]);
	float amplitude = atof(argv[5]);	
	int i;
	double t;
	TestData data;
	PaStream *stream;
	PaStreamParameters outputParameters;
	PaError err;
	/* Generate table with sine values at given frequency */
	//float volumeLevelDb = -6.f; //cut amplitude in half; same as 0.5 above
	float VOLUME_REFERENCE = 1.f;
	float volumeMultiplier = (VOLUME_REFERENCE * pow(10, (amplitude / 20.f)));
	for (i = 0; i < TABLE_SIZE; i++) {
		t = (double)i/(double)SAMPLE_RATE;
		data.sine[i] = sin(2 * PI * sinefrequ * t) * volumeMultiplier;
	}
	/* Initialize user data */
	data.phase = 0;
	/* Initialize PortAudio */
	Pa_Initialize();
	/* Set output stream parameters */
	outputParameters.device = Pa_GetDefaultOutputDevice();
	outputParameters.channelCount = 2;
	outputParameters.sampleFormat = paFloat32;
	outputParameters.suggestedLatency = 
	Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
	outputParameters.hostApiSpecificStreamInfo = NULL;
	/* Open audio stream */
	err = Pa_OpenStream( &stream, NULL /* no input */,
		   &outputParameters,
		   SAMPLE_RATE, FRAMES_PER_BUFFER, paNoFlag, 
		   paCallback, &data );
	if (err != paNoError) {
		printf("PortAudio error: open stream: %s\n", Pa_GetErrorText(err));
	}
#endif	

	record_time_ms = (double)tempint / 1000.0;
	if (argc > 3) {
		strcpy(output_file_name, argv[3]);
		strcpy(output_rawpairs_name, argv[3]);	
		strcat (output_file_name,".ivs");
		strcat (output_rawpairs_name,".ivp");
	}
	printf("Saving to %s...\n", output_file_name);
	printf("Saving pairs to %s...\n", output_rawpairs_name);
	surface_outf = fopen(output_file_name, "w");
	rawpairs_outf = fopen(output_rawpairs_name, "w");	
	int cnt = COUNT;
	fwrite(&cnt, sizeof(int), 1, rawpairs_outf);
	
	init_serial(argc, argv);
	int n = write(port, buf, 1);
	if (n < 1) {
		fprintf(stderr, "error writing start to port\n");
		exit(0);
	}
	
	// Load FEB profile TODO
/*	char cs_literal[] = "febprofile/csmap.dat";
	loadmap(cs_literal, cs_map);
	char csm_literal[] = "febprofile/csmapmult.dat";
	loadmap(csm_literal, cs_map_mult);
	char vs_literal[] = "febprofile/vsmap.dat";
	loadmap(vs_literal, vs_map);
	char vsm_literal[] = "febprofile/vsmapmult.dat";
	loadmap(vsm_literal, vs_map_mult); */
	
	// Record surface for specified amount of time
	int points = 0;
	int numcurves = 0;
	double timestamp;
	int cpsavg = 0;
	uint64_t start;
	uint64_t end;
	uint64_t elapsed;
	uint64_t elapsedNano;
	static mach_timebase_info_data_t    sTimebaseInfo;	
	if ( sTimebaseInfo.denom == 0 ) {
		(void) mach_timebase_info(&sTimebaseInfo);
	}
	start = mach_absolute_time();

#if AUDIO
	/* Start audio stream */
	err = Pa_StartStream( stream );
	if (err != paNoError) {
		printf(	 "PortAudio error: start stream: %s\n", Pa_GetErrorText(err));
	}
#endif

	/* Start recording the surface */
   	while (1) {
		memset(buf, 0, sizeof(unsigned char) * 6);
		n = read(port, buf, 6);
		if (n < 6) {
			fprintf(stderr, "error reading from port, read %d\n", n);
			continue;
		}
		unsigned int check = ((buf[5] << 8) + buf[4]);
		//if (check == 0xFEFE) continue;
		if (check == 0xFFFF) {
			double current = (((buf[1]) << 8) + buf[0]) * (3.28f / 4096.0f);
			double voltage = (((buf[3]) << 8) + buf[2]) * (3.28f / 4096.0f);
			current = current / ( SENSE_RESISTANCE * CS_GAIN );
			voltage = voltage * 4;
			curvedata[0][points] = voltage;
			curvedata[1][points] = current;
			if (points++ == COUNT - 1) {
				// Figure out how many nanoseconds elapsed for this curve
				end = mach_absolute_time();
				elapsed = end - start;
				if ( sTimebaseInfo.denom == 0 ) {
					(void) mach_timebase_info(&sTimebaseInfo);
				}
				elapsedNano = elapsed * sTimebaseInfo.numer / sTimebaseInfo.denom;
				//printf("Curves per second : %" PRIu64 "\n", 1000000000 / elapsedNano);
				cpsavg += 1000000000 / elapsedNano;
				// Pipe timestamp to both files
				timestamp +=  elapsedNano / 1000000000.0;
				fwrite(&timestamp, sizeof(double), 1, surface_outf);
				fwrite(&timestamp, sizeof(double), 1, rawpairs_outf);				
				// Write to file all x points
				fwrite(&curvedata[0], sizeof(double), COUNT, rawpairs_outf);
				// Write to file all y points
				fwrite(&curvedata[1], sizeof(double), COUNT, rawpairs_outf);
				
				// Create new curve (this sorts curve)
				create_curves_with_regression_old();
				// Pipe curve output to file
				fwrite(&allcurves[indexInAllCurves], sizeof(double), NUMPOINTS, surface_outf);
				
				// End if over time
				if(timestamp > record_time_ms) {
					#if AUDIO
						Pa_StopStream( stream );
						Pa_CloseStream(stream);
						Pa_Terminate();
					#endif
					printf("=======\nIV-surface \"%s\" created!\nRecorded %d curves for %lf seconds to output file \"%s\" at %d curves per second\n=======\n", output_file_name, numcurves, timestamp, output_file_name, cpsavg / numcurves);
					break;
				}
				// Prepare for next curve
				points = 0;
				numcurves++;
				start = mach_absolute_time();

				// Update display here if we are going to do that
				//indexInAllCurves = (indexInAllCurves + 1) % ALL_CURVES_LEN;
			}
		} else {
			n = read(port, buf, 1);
			if (n < 1) {
				fprintf(stderr, "error reading from port\n");
				break;
			}
		}
	}
	// Tell the TEENSY to end the surface collection
	n = write(port, buf, 1);
	if (n < 1) {
		fprintf(stderr, "error writing end note to port\n");
		exit(0);
	}
	fclose(surface_outf);
	close(port);
	
	return 0;
}
