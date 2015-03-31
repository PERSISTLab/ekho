#include <ADC.h>
#include <ADC_Module.h>
#include <RingBuffer.h>
#include <RingBufferDMA.h>
#include <SPI.h>

ADC *adc = new ADC(); // adc object
byte buf[6];
int SSpin = 10;   //SPI Slave Select / Chip Select
void setPot(int reg, int level) {
  digitalWrite(SSpin, LOW);
  SPI.transfer(reg);
  SPI.transfer(level);
  digitalWrite(SSpin, HIGH);
}

void setup() {
  Serial.begin(115200);

  pinMode(A3, INPUT);

  ////// ADC1 /////
  adc->setAveraging(32, ADC_1); // set number of averages
  adc->setResolution(12, ADC_1); // set bits of resolution
  adc->setConversionSpeed(ADC_HIGH_SPEED, ADC_1); // change the conversion speed
  adc->setSamplingSpeed(ADC_HIGH_SPEED, ADC_1); // change the sampling speed
  
  SPI.begin();
  delay(1000);
  setPot(0,0); 
  delayMicroseconds(28);
  
  // Let everything settle
  delay(1000);
}

uint16_t current=0;
uint16_t voltage=0;
uint32_t avgvoltage = 0;
uint16_t i = 0;
uint16_t countvolt = 0;
void loop() {
  if(Serial.available()) {
     Serial.read();
     uint16_t outv = avgvoltage / countvolt;
     buf[0] = (outv >> 8) & 0xff;
     buf[1] = outv & 0xff;
     Serial.write(buf, 2);
     avgvoltage = 0;
     countvolt = 0;
  }
  voltage = adc->analogRead(A3, ADC_1);
  avgvoltage = avgvoltage + voltage;
  countvolt = countvolt + 1;
//  for(int i = 0; i< 128; i++) {
    setPot(0,55); 
    delayMicroseconds(100);  
  //}
}



