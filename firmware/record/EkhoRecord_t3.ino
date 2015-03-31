#include <ADC.h>
#include <ADC_Module.h>
#include <RingBuffer.h>
#include <RingBufferDMA.h>
#include <SPI.h>


//By Default, 11 = MOSI / SDO, 12 = MISO / SDI, 13 = CLK / SCLK
int SSpin = 10;   //SPI Slave Select / Chip Select
ADC *adc = new ADC(); // adc object

// Make this as large as possible
byte response[6];
boolean start;
uint16_t current;
uint16_t voltage;

void setup() {
  Serial.begin(115200);

  //Again, the other SPI pins are configured automatically
  pinMode(SSpin, OUTPUT);
  pinMode(A9, INPUT); //pin 23 single ended
  pinMode(A3, INPUT);
  
  ////// ADC0 /////
  adc->setAveraging(1); // set number of averages
  adc->setResolution(12); // set bits of resolution
  adc->setConversionSpeed(ADC_HIGH_SPEED); // change the conversion speed 
  adc->setSamplingSpeed(ADC_HIGH_SPEED); // change the sampling speed

  ////// ADC1 /////
  adc->setAveraging(1, ADC_1); // set number of averages
  adc->setResolution(12, ADC_1); // set bits of resolution
  adc->setConversionSpeed(ADC_HIGH_SPEED, ADC_1); // change the conversion speed
  adc->setSamplingSpeed(ADC_HIGH_SPEED, ADC_1); // change the sampling speed

  //Initialize SPI
  SPI.begin();
  delay(1000);
  setPot(0,0); 
  delayMicroseconds(28);
  
  while(Serial.available() == 0) {}
  
}


void setPot(int reg, int level) {
  digitalWrite(SSpin, LOW);
  SPI.transfer(reg);
  SPI.transfer(level);
  digitalWrite(SSpin, HIGH);
}

void printLine() {
  Serial.print( response[0]);
  Serial.print(",");
  Serial.print( response[1]);
  Serial.print(",");
  Serial.print( response[2]);
  Serial.print(",");  
  Serial.println( response[3]);
  Serial.print(current);
  Serial.print(",");
  Serial.println(voltage);   
}

void loop() {
  for(int i = 0; i< 128; i++) {
    current = adc->analogRead(A9, ADC_0); // read a new value, will return ADC_ERROR_VALUE if the comparison is false.
    voltage = adc->analogRead(A3, ADC_1);
   
    response[0] = (byte) current & 0xff;
    response[1] = (byte) (current >> 8) & 0xff;
    response[2] = (byte) voltage & 0xff;
    response[3] = (byte) (voltage >> 8) & 0xff;    
    response[4] = 255;
    response[5] = 255;
    //printLine();     
    Serial.write(response, 6);
    setPot(0,i); 
    delayMicroseconds(50);
  }
}



