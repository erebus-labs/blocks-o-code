

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "i2c_slave.h"

#define STATUS_LED		PA1
#define STATUS_PORT		PORTA
#define TOGGLE_STATUS	(STATUS_PORT ^= _BV(STATUS_LED))

void initIO(void) {
	DDRA |= _BV(STATUS_LED);
	setup_i2c(42);
	sei();
}

int main(void) {
	initIO();

	while (loop_i2c()) {
//		TOGGLE_STATUS;
//		_delay_ms(25);
	}
	return 0; // never reached
}
