/*
ADC based function select
A Block of Code 5/21/15

returns top 4 bits from adc connected to PA1 in bits [3:0], [7:4] will be zeros
*/
#include "func_select.h"
#include <avr/io.h>

void setup_adc(void)
{
	ADMUX = (1<<ADLAR)|(1<<MUX0); //set 8 MSB of 10 bit adc to ADCH,// set ADC to PA1
	OCR0A = 0x10; //counter0 match A for 250KHz from 8M sys clock (max resolution)
	ADCSRB = (1<<ADTS2); //set to make a measurement every counter0 overflow (ADTS0&ADTS1) for match A
	TCCR0A = (1<<0); //CTC0 reset the counter on match
	TCCR0B = (1<<CS01); //turn on counter0 using sys clock
	ADCSRA = (1<<ADEN)|(1<<ADATE); // enable ADC with sys clock/2 (max speed),  //set periodic measurement
	ADCSRA |= (1<<ADSC); //Start reading
}
uint8_t read_adc(void)
{
	return((ADCH & 0xF0)>>4); //return top 4 bits in the bottom 4 bits
}