// For Teensy 3.6 with Multi Ekho board
#include <Wire.h>
#include <ADC.h>
#include <ADC_Module.h>
#include <RingBuffer.h>
#include <RingBufferDMA.h>

#define AD5282_ADDRESS 0b0101100
#define AD5282_RDAC1 0b00000000
#define AD5282_RDAC2 0b10000000
ADC *adc = new ADC(); // adc object
uint16_t current, voltage;
byte response[6];

void set_rdac(uint8_t num, uint8_t val) {
  Wire.beginTransmission(AD5282_ADDRESS);
  Wire.write(num);            
  Wire.write(val);             
  Wire.endTransmission();     
}

void reset_digpot() {
  set_rdac(AD5282_RDAC1, 0);
  set_rdac(AD5282_RDAC2, 0);
}

void setup() {
  Serial.begin(115200);
  Wire.begin(); // join i2c bus (address optional for master)
  delay(100);
  reset_digpot();
  analogWriteResolution(12); 
  analogWrite(A14, 1);

  ////// ADC0 /////
  adc->setAveraging(1); // set number of averages
  adc->setResolution(12); // set bits of resolution
  adc->setConversionSpeed(ADC_CONVERSION_SPEED::HIGH_SPEED); // change the conversion speed 
  adc->setSamplingSpeed(ADC_SAMPLING_SPEED::HIGH_SPEED); // change the sampling speed

  ////// ADC1 /////
  //adc->setAveraging(1, ADC_1); // set number of averages
  //adc->setResolution(12, ADC_1); // set bits of resolution
  //adc->setConversionSpeed(ADC_CONVERSION_SPEED::HIGH_SPEED, ADC_1); // change the conversion speed 
  //adc->setSamplingSpeed(ADC_SAMPLING_SPEED::HIGH_SPEED, ADC_1); // change the sampling speed
}
#define NUM_POINTS 46
uint8_t index_smart = 1;
uint8_t POINTS[NUM_POINTS] = {0, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 4, 4, 5, 6, 7, 7, 8, 9, 11, 12, 14, 16, 18, 20, 23, 26, 29, 33, 38, 43, 48, 55, 62, 70, 79, 90, 102, 116, 131, 148, 168, 191, 216, 245, 255};
void loop() {
  // Get IV point
  //current = adc->analogRead(A9, ADC_0); // read a new value, will return ADC_ERROR_VALUE if the comparison is false.
  //voltage = adc->analogRead(A3, ADC_1);
  current = adc->analogRead(A0, ADC_0); // read a new value, will return ADC_ERROR_VALUE if the comparison is false.
  voltage = adc->analogRead(A1, ADC_0);
  response[0] = (byte) current & 0xff;
  response[1] = (byte) (current >> 8) & 0xff;
  response[2] = (byte) voltage & 0xff;
  response[3] = (byte) (voltage >> 8) & 0xff;    
  response[4] = 0xFF;
  response[5] = 0xFF;
  Serial.write(response, 6);    
  //Serial.print(voltage);
  //Serial.print(",");
  //Serial.println(current);
  
  set_rdac(AD5282_RDAC1, index_smart);
  set_rdac(AD5282_RDAC2, index_smart++);
  delayMicroseconds(5);    
}
