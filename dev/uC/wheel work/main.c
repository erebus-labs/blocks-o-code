//
// Daisy Chain Raster for the A Block of Code Project
//

#include "abc_daisy.h"
#include "i2c_slave.h"
#include "func_select.h"

/*typedef enum {
	None,
	A_Var,
	Colon,
	Three,
	Print
} BlockFunc;*/

//static BlockFunc function = A_Var;

typedef enum{
	Variables,
	Assignment,
	Control,
	Statement
}WheelSelect;

static WheelSelect wheel = Variables;

static void initIO(void);
static void startupSequence(void);

static uint8_t getMyData(void);
static uint8_t sendRight(void);
static uint8_t sendUp(void);

/*
 @return Single unsigned byte
 bits [7:6] set if a block is present above and to the right of this block, respectively.
 bits [5:4] code for wheel selection; 00 is Variables, 01 is Assignment, 10 is Control, 11 is Statements
 bits [3:0] code for the function on the wheel; 0000 is first, 0001 is the second ... 1111 is the sixteenth 
*/

static uint8_t getMyData(void) {
	return adjacentBlocks() | (wheel<<2) | read_adc();
}

static uint8_t sendRight(void) {
	sendHorizontal();
	return getMyData();
}

static uint8_t sendUp(void) {
	sendVertical();
	return getMyData();
}

int main(void) {
	// initialization
	initIO();
	startupSequence();
	startupSequence();
//	_delay_ms(100);
//	TOGGLE_STATUS;
//	_delay_ms(100);
//	TOGGLE_STATUS;

	// spin until position received from connected block (or BBB)
	uint8_t addr = waitForVector();
	
	// assign function pointer to use custom data-collector function
	getData_ptr gData = getMyData;
	sendHorizontal_ptr sHoriz = sendRight;
	sendHorizontal_ptr sVert = sendUp;
	
	// assign received vector to i2c initialization
	setup_i2c(addr, gData, sHoriz, sVert, 0, 0);
	
	// begin taking adcmesurements
	setup_adc();
	
	forwardChain();
	
	sei();
	
	// continuously poll i2c for commands
	while (loop_i2c()) {
//		_delay_ms(200);
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
	DDRA |= _BV(PA2);
	DDRA |= _BV(PA3);
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