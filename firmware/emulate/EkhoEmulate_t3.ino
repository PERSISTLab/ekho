#include <ADC.h>
#include <ADC_Module.h>
#include <RingBuffer.h>
#include <RingBufferDMA.h>

ADC *adc = new ADC();
uint16_t current_curve[65];
const int statPin=21;
int16_t current=0;
int16_t voltage=0;
unsigned char buf[512];
uint16_t bcount = 0;
uint16_t i = 0;

uint16_t interpolate_f(uint16_t _adc_14_value, uint16_t _shift_bits) 
{
	uint16_t left = _adc_14_value >> _shift_bits;
	return	current_curve[left] -
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
	digitalWrite(statPin, LOW);
	delay(2000);
}

void loop() {
	// Calculate output value (TODO: dont use float math, use a table)
	voltage = adc->analogRead(A3, ADC_1);
	//					  = voltage * (3.3 / 4096) * 4;
	double actual_voltage = voltage * 0.00322265;
	double actual_voltage_digital = actual_voltage / 0.0019178;
	uint16_t avd = (uint16_t)actual_voltage_digital;
	
	// Start reading in a new curve if necessary
	while(Serial.available()) {
		if(bcount == 0) {
			digitalWrite(statPin, HIGH);
		}
		buf[bcount++] = Serial.read();
		if(bcount > 129) {
			digitalWrite(statPin, LOW);
			for(i=0;i<130;i+=2) {
				current_curve[i / 2] = (buf[i] << 8) + buf[i+1];
			}
			bcount = 0; 
			break;
		}
	}

	// Write to DAC
	analogWrite(A14, interpolate_f(avd, 6));
}



