//
// Daisy Chain Raster for the A Block of Code Project
//

#include "abc_daisy.h"
#include "i2c_slave.h"

typedef enum {
	None,
	A_Var,
	Colon,
	Three,
	Print
} BlockFunc;

static BlockFunc function = Three;

static volatile uint8_t sendUpFlag		= 0;
static volatile uint8_t sendRightFlag	= 0;

static void initIO(void);
static void startupSequence(void);

static uint8_t getMyData(void);
static uint8_t sendRight(void);
static uint8_t sendUp(void);
static void advanceVector(void);

static uint8_t getMyData(void) {
	return adjacentBlocks() | function;
}

static uint8_t sendRight(void) {
	++sendRightFlag;
	return getMyData();
}

static uint8_t sendUp(void) {
	++sendUpFlag;
	return getMyData();
}

static void advanceVector(void) {
	if (sendUpFlag) {
		sendVertical();
		sendUpFlag = 0;
	}
	if (sendRightFlag) {
		sendRight();
		sendRightFlag = 0;
	}
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
	
	forwardChain();
	
	sei();
	
	// continuously poll i2c for commands
	while (loop_i2c()) {
		advanceVector();
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