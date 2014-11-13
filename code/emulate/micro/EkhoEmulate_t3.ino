#include <ADC.h>
#include <ADC_Module.h>
#include <RingBuffer.h>
#include <RingBufferDMA.h>

ADC *adc = new ADC(); // adc object
byte response[6];
uint16_t current_curve[65];
const int statPin=21;
int16_t interpolate_f(int16_t _adc_14_value, uint16_t _shift_bits) 
{
	int16_t left = _adc_14_value >> _shift_bits;
	return  current_curve[left] -
				((
					(uint32_t)(current_curve[left] - current_curve[left+1]) * 
					(uint32_t)(_adc_14_value - (left << _shift_bits))
				) >> _shift_bits);
};

void setup() {
  Serial.begin(115200);

  pinMode(A3, INPUT);
  pinMode(statPin, OUTPUT);
  
  ////// ADC1 /////
  adc->setAveraging(1, ADC_1); // set number of averages
  adc->setResolution(12, ADC_1); // set bits of resolution
  adc->setConversionSpeed(ADC_HIGH_SPEED, ADC_1); // change the conversion speed
  adc->setSamplingSpeed(ADC_HIGH_SPEED, ADC_1); // change the sampling speed

  // 12-bit DAC
  analogWriteResolution(12); 
  
  // Let everything settle
  delay(1000);
}

void printLine() {
  Serial.print( response[0]);
  Serial.print(",");
  Serial.print( response[1]);
  Serial.print(",");
  Serial.print( response[2]);
  Serial.print(",");  
  Serial.println( response[3]);
}

uint16_t current=0;
uint16_t voltage=0;
unsigned char buf[512];
uint16_t bcount = 0;
uint16_t i = 0;
void loop() {
  digitalWrite(statPin, LOW);
  while(Serial.available()) {
    buf[bcount++] = Serial.read();
    if(bcount > 130) {
      for(i=0;i<130;i+=2) {
        current_curve[i / 2] = (buf[i] << 8) + buf[i+1];
      }
      bcount = 0; 
      break;
    }
  }
  digitalWrite(statPin, HIGH);
  voltage = adc->analogRead(A3, ADC_1);
  analogWrite(A14, interpolate_f(voltage, 6));
  //delayMicroseconds();
}



