

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "i2c_slave.h"

#define STATUS_LED		PA1
#define STATUS_PORT		PORTA
#define TOGGLE_STATUS	(STATUS_PORT ^= _BV(STATUS_LED))

#define i2c_addr
#define block_func
#define x_pos
#define y_pos

void initIO(void) {
	DDRA |= _BV(STATUS_LED);
	setup_i2c(i2c_addr,block_func,x_pos,y);
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
