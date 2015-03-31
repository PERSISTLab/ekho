#include <Wire.h>
#include <SPI.h>
#include <ADC.h>
#include <ADC_Module.h>
#include <RingBuffer.h>
#include <RingBufferDMA.h>

#define DEBUG 0
#define USE_BOTH 0

#define GAIN_1   B00000001
#define GAIN_10  B00000011
#define GAIN_20  B00000101
#define GAIN_30  B00000111
#define GAIN_40  B00001001
#define GAIN_60  B00001011
#define GAIN_80  B00001101
#define GAIN_120 B00001111
#define GAIN_157 B00010001
#define GAIN_DIV4 B00010011
ADC *adc = new ADC(); // adc object
boolean start=false;
byte response[6];
uint16_t current;
uint16_t voltage;
void set_gain(int amp, int gain_set_val) {
  SPI.beginTransaction(SPISettings(5000000, LSBFIRST, SPI_MODE0));
  digitalWrite(amp, LOW);     
  SPI.transfer(gain_set_val);
  digitalWrite(amp, HIGH);
  SPI.endTransaction();
}

#define VOS_TOP B00111110
#define VOS_BOTTOM B00000000

void set_vos(int amp, int gain_set_val) {
  SPI.beginTransaction(SPISettings(5000000, LSBFIRST, SPI_MODE0));
  digitalWrite(amp, LOW);     
  SPI.transfer(gain_set_val);
  digitalWrite(amp, HIGH);
  SPI.endTransaction();
}

int CS_SD = 6;
int CSVS_amp = 20; // chip selects
int CSCS_amp = 23; 
int shutdown_p = 10; // active low shutdown digipot
int ADC_CS = 22; // ADC current set
int ADC_VS = 17; // ADC voltage set
void setup()
{
  SPI.begin();
  Wire.begin();
  Serial.begin(115200);
  

  pinMode(shutdown_p, OUTPUT);
  pinMode(CSCS_amp, OUTPUT);
  pinMode(CS_SD, OUTPUT);
  pinMode(CSVS_amp, OUTPUT);
  
  digitalWrite(shutdown_p,HIGH);
  set_gain(CSVS_amp, GAIN_1);
  set_gain(CSCS_amp, GAIN_120);
  set_vos(CSCS_amp, VOS_BOTTOM);
  
  pinMode(A8, INPUT);
  pinMode(A3, INPUT);
  
  ////// ADC0 /////
  adc->setAveraging(1); // set number of averages
  adc->setResolution(12); // set bits of resolution
  adc->setConversionSpeed(ADC_VERY_HIGH_SPEED); // change the conversion speed 
  adc->setSamplingSpeed(ADC_VERY_HIGH_SPEED); // change the sampling speed

  ////// ADC1 /////
  adc->setAveraging(1, ADC_1); // set number of averages
  adc->setResolution(12, ADC_1); // set bits of resolution
  adc->setConversionSpeed(ADC_VERY_HIGH_SPEED, ADC_1); // change the conversion speed
  adc->setSamplingSpeed(ADC_VERY_HIGH_SPEED, ADC_1); // change the sampling speed
  
  delay(1000);
#if !DEBUG  
  while(Serial.available() == 0) {}
  Serial.read();
#endif
  start=true;
}

bool digpot_send(uint8_t valres) {
  bool verified=true;
  byte sendthis[]= {B00010000, valres};
  Wire.beginTransmission(B0101100);
  Wire.write(sendthis, 2);
  Wire.endTransmission();
  
  sendthis[0] = B10001000;
#if !USE_BOTH  
  sendthis[1] = 0;
#endif  
  Wire.beginTransmission(B0101100);
  Wire.write(sendthis, 2);
  Wire.endTransmission();
}

bool digpot_send_verify(uint8_t valres) {
  bool verified=true;
  byte sendthis[]= {B00010000, valres};
  Wire.beginTransmission(B0101100);
  Wire.write(sendthis, 2);
  Wire.endTransmission();
  Wire.requestFrom(B0101100, 1);
  while(Wire.available()) {
    int num = Wire.receive();
    if(num != valres) verified=false;
  }
  
  sendthis[0] = B10001000;
#if !USE_BOTH  
  sendthis[1] = 0;
#endif  
  Wire.beginTransmission(B0101100);
  Wire.write(sendthis, 2);
  Wire.endTransmission();
  
  Wire.requestFrom(B0101100, 1);
  while(Wire.available()) {
    int num = Wire.receive();
    if(num != valres) verified=false;
  }  
  return verified;
}  
uint8_t res=1;
uint8_t highlow = 0;

void debug(uint8_t i) {
   Serial.print("ndx=");
   Serial.print(i);
   Serial.print(",I=");
   Serial.print(current);
   Serial.print(",V=");
   Serial.println(voltage);   
}
void loop()
{
if(start == true) {
		for(uint8_t i=0;i<64;i++) {
			current = adc->analogRead(A8, ADC_0); // read a new value, will return ADC_ERROR_VALUE if the comparison is false.
			voltage = adc->analogRead(A3, ADC_1);

			response[0] = (byte) current & 0xff;
			response[1] = (byte) (current >> 8) & 0xff;
			response[2] = (byte) voltage & 0xff;
			response[3] = (byte) (voltage >> 8) & 0xff;		
			response[4] = 255;
			response[5] = 255;
			digpot_send(i);
			#if DEBUG
			debug(i);
			#else
			Serial.write(response, 6);
                        #endif 
                        //delay(1);
		}
	
}
}
