//
// Daisy Chain Raster for the A Block of Code Project
//

#include "abc_daisy.h"		// daisy chain locationing
#include "i2c_slave.h"		// global bus identification
#include "func_select.h"	// function selection input

// test values
typedef enum {
	None,
	A_Var,
	Colon,
	Three,
	Print
} BlockFunc;

static BlockFunc function = A_Var;

/**
	Read Commands from the global bus. Used as bit masks to add tasks to the 
	work queue, serviced in main loop.
 */
typedef enum {
	ReadDataCommand,
	SendVerticalCommand,
	SendHorizontalCommand,
	StatusLedOnCommand,
	StatusLedBlinkCommand,
	StatusLedOffCommand,
	ErrorLedOnCommand,
	ErrorLedBlinkCommand,
	ErrorLedOffCommand,
	
	ReadCommandCount	// Unused, always last
} ReadCommands;

static volatile uint16_t workQueue	= 0;

static void initIO(void);
static void startupSequence(void);
static void serviceWorkqueue(void);
static uint8_t GlobalBusCommand(uint8_t command);
static uint8_t ABCReadData(void);

uint8_t ABCReadData(void) {
	return (adjacentBlocks() | function); // read_adc();
}

uint8_t GlobalBusCommand(uint8_t command) {
	if ((command < ReadCommandCount) && (command < 16)) {
		workQueue |= _BV(command);
	}
	return ABCReadData();
}

int main(void) {
	// initialization
	initIO();
	startupSequence();
	startupSequence();
	
	// spin until position received from connected block (or main processor)
	uint8_t addr = waitForVector();
	
	// assign function pointer to use custom data-collector function
	GetData_ptr GlobalFunc_ptr = GlobalBusCommand;
	
	// assign received vector to i2c initialization
	setup_i2c(addr, GlobalFunc_ptr);
	
	// advance daisy chain as much as can be assumed
	forwardChain();
	
	STATUS_PORT &= ~_BV(STATUS_LED);
	// enable interrupts
	sei();
	
	// continuously poll i2c for commands
	while (loop_i2c()) {
		serviceWorkqueue();
		_delay_ms(200);
		TOGGLE_STATUS();
	}
	return 0; // never reached
}

void initIO(void) {
	// start adc
//	setup_adc();
	
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

static void serviceWorkqueue(void) {
	
	// Read
	if (workQueue & _BV(ReadDataCommand)) {
		// may need delay for ADC
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
			workQueue &= ~_BV(ReadDataCommand);
		}
	}
	
	// Send Vertical
	if (workQueue & _BV(SendVerticalCommand)) {
		sendDaisyChainVertical();
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
			workQueue &= ~_BV(SendVerticalCommand);
		}
	}
	
	// Send Horizontal
	if (workQueue & _BV(SendHorizontalCommand)) {
		STATUS_PORT |= _BV(STATUS_LED);
		sendDaisyChainHorizontal();
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
			workQueue &= ~_BV(SendHorizontalCommand);
		}
	}
	
	// Status On
	if (workQueue & _BV(StatusLedOnCommand)) {
		STATUS_PORT |= _BV(STATUS_LED);
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
			workQueue &= ~_BV(StatusLedOnCommand);
		}
	}
	
	// Status Blink
	if (workQueue & _BV(StatusLedBlinkCommand)) {
		// may move to timer
		STATUS_PORT &= ~_BV(STATUS_LED);
		TOGGLE_STATUS();
		_delay_ms(50);
		TOGGLE_STATUS();
		_delay_ms(50);
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
			workQueue &= ~_BV(StatusLedBlinkCommand);
		}
	}
	
	// Status Off
	if (workQueue & _BV(StatusLedOffCommand)) {
		STATUS_PORT &= ~_BV(STATUS_LED);
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
			workQueue &= ~_BV(StatusLedOffCommand);
		}
	}
	
	// Error On
	if (workQueue & _BV(ErrorLedOnCommand)) {
		ERROR_PORT |= _BV(ERROR_LED);
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
			workQueue &= ~_BV(ErrorLedOnCommand);
		}
	}
	
	// Error Blink
	if (workQueue & _BV(ErrorLedBlinkCommand)) {
		// may move to timer
		ERROR_PORT &= ~_BV(ERROR_LED);
		TOGGLE_ERROR();
		_delay_ms(50);
		TOGGLE_ERROR();
		_delay_ms(50);
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
			workQueue &= ~_BV(ErrorLedBlinkCommand);
		}
	}
	
	// Error Off
	if (workQueue & _BV(ErrorLedOffCommand)) {
		ERROR_PORT &= ~_BV(ERROR_LED);
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
			workQueue &= ~_BV(ErrorLedOffCommand);
		}
	}
}
