/*
 * GccApplication1.c
 * a test for adc use on attiny461
 * Created: 5/13/2015 10:21:39 PM
 *  Author: Jacob Mickiewicz
 */ 


#include <avr/io.h>

uint8_t get_adc(void)
{
	return (ADCH);
}

void setup_adc(void)
{
	ADMUX = (1<<ADLAR)|(1<<MUX0); //set 8 MSB of 10 bit adc to ADCH, set ADC to PA1
	//ADCSRB = (1<<ADTS2); //set to make a measurement every counter0 overflow (ADTS0&ADTS1) for match A
	//TCCR0A = (1<<0) //CTC0 reset the counter on match
	//TCCR0B = (1<<CS01); //turn on counter0 using sys clock
	//OCR0A = 0x10; //counter0 match A for 250KHz from 8M sys clock (max resolution)
	ADCSRA = (1<<ADEN); // enable ADC with sys clock/2 (max speed) |(1<<ADATE) to set periodic measurement
	ADCSRA |= (1<<ADSC); //Start reading
}

int main(void)
{
	uint8_t ADC_value;
	
	DDRB = 0b11111111; // all outputs ; only for testing
	setup_adc();
	
    while(1)
    {
		
        ADC_value = get_adc(); 
		PORTB = ADC_value;	//output to leds for testing
		ADCSRA |= (1<<ADSC); //start another reading
    }
}