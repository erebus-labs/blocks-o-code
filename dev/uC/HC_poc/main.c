

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "i2c_slave.h"

#define STATUS_LED		PA1
#define STATUS_PORT		PORTA
#define TOGGLE_STATUS	(STATUS_PORT ^= _BV(STATUS_LED))

#define i2c_addr		17
#define block_func		36
#define x_pos			0
#define y_pos			2

#define SETUP_BLOCK_D (setup_i2c(4,  36,  0, 2))
#define SETUP_BLOCK_E (setup_i2c(17, 12,  1, 2))
#define SETUP_BLOCK_B (setup_i2c(32, 117, 2, 2))
#define SETUP_BLOCK_C (setup_i2c(42, 19,  0, 1))
#define SETUP_BLOCK_A (setup_i2c(6,  36,  1, 1))

void initIO(void) {
	DDRA |= _BV(STATUS_LED);
//	setup_i2c(i2c_addr,block_func,x_pos,y_pos);
	SETUP_BLOCK_A;
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
