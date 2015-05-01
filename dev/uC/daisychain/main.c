//
// Daisy Chain utility for the A Block of Code Project
// Greg Stromire
//

#include "abc_daisy.h"

static void initIO(void);
static void startupSequence(void);

int main(void) {
	
//	cli();
	
	// initialization
	initIO();
	startupSequence();
	startupSequence();
	_delay_ms(100);
	
//	sei();
	
	// spin until position received from connected block
	waitForVector();
	
	while (1) {
//		_delay_ms(500);
//		TOGGLE_STATUS;
	}
	return 0; // never reached
}

void initIO(void) {
	// set output pins
#ifdef MCU_2313
	DDRA |= _BV(PD0);
	DDRA |= _BV(PD1);
	DDRA |= _BV(PD2);
	DDRA |= _BV(PD3);
	DDRA |= _BV(STATUS_LED);
	DDRA |= _BV(ERROR_LED);
#endif
#ifdef MCU_461
	DDRA |= _BV(PA1);
	DDRA |= _BV(PA2);
	DDRA |= _BV(PA4);
	DDRA |= _BV(PA5);
	DDRA |= _BV(STATUS_LED);
	DDRA |= _BV(ERROR_LED);
#endif
}

void startupSequence(void) {
#ifdef MCU_2313
	for(int i = 0; i < 4; ++i){
		_delay_ms(25);
		PORTD |= _BV(PD0 + i);
	}
	for(int i = 0; i < 4; ++i){
		_delay_ms(25);
		PORTD &= ~_BV(PD0 + i);
	}
#endif
#ifdef MCU_461
	PORTA |= _BV(PA2);
	_delay_ms(25);
	PORTA |= _BV(PA3);
	_delay_ms(25);
	PORTA |= _BV(PA4);
	_delay_ms(25);
	PORTA |= _BV(PA5);
	_delay_ms(25);
	PORTA &= ~_BV(PA2);
	_delay_ms(25);
	PORTA &= ~_BV(PA3);
	_delay_ms(25);
	PORTA &= ~_BV(PA4);
	_delay_ms(25);
	PORTA &= ~_BV(PA5);
	_delay_ms(25);
	PORTA &= 0b11000000;
#endif
}
