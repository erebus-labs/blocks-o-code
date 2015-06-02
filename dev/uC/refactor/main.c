/**
 *	main.c
 *	A Block of Code
 *	PSU Capstone with Erebus Labs
 *
 *	Spring 2015
 *	v1.1
 */

#include "abc_daisy.h"		// daisy chain locationing
#include "i2c_slave.h"		// global bus identification and communication
#include "func_select.h"	// function selection input

#define INPUT_BLOCK	0
#define POTENTIOMETER 0

// hard-coded test values for prototyping

#if POTENTIOMETER
#else
typedef enum {
	None,
	A_Var,
	Colon,
	Three,
	Print
} BlockFunc;

static const BlockFunc function = A_Var;
#endif


typedef enum {
	ValueTokens,
	OperatorTokens,
	ControlTokens,
	StatementTokens
} TokenCategories;

static const TokenCategories category = ValueTokens;

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
	AuxiliaryRead,
	
	ReadCommandCount	// Unused, always last
} ReadCommands;

static volatile uint16_t workQueue	= 0;

static void initIO(void);
static void startupSequence(void);
static uint8_t GlobalBusCommand(uint8_t command);
static uint8_t ABCReadData(void);
static void serviceWorkqueue(void);
static void initTimer(void);

#if INPUT_BLOCK
static uint8_t ABCAuxiliary(void);
#endif

int main(void) {
	// initialization
	initIO();
	startupSequence();
	startupSequence();
	
	// spin until position received from connected block (or main processor)
	uint8_t addr = waitForVector();
//	uint8_t addr = 42;
	
	// assign function pointer to use custom data-collector function
	GetData_ptr GlobalFunc_ptr = GlobalBusCommand;
	
	// assign received vector to i2c initialization
	setup_i2c(addr, GlobalFunc_ptr);
	sei();
	
//	STATUS_PORT &= ~_BV(STATUS_LED);
	
	// continuously poll i2c for commands
	while (loop_i2c()) {
		serviceWorkqueue();
		updateChain();
		
#if POTENTIOMETER
		ADCSRA |= (1<<ADSC);
#endif
		
		// timer led blink?
	}
	return 0; // never reached
}

void initIO(void) {

#if POTENTIOMETER
	// start adc
	setup_adc();
#endif
	
	// set output pins
	DDRA |= _BV(STATUS_LED);
	DDRA |= _BV(ERROR_LED);
}

void initTimer(void) {
	
	// set timer1 presaler to 128
	TCCR1B |= _BV(CS13);
//	OCR1B
}

void startupSequence(void) {
	TOGGLE_STATUS();
	_delay_ms(100);
	TOGGLE_ERROR();
	_delay_ms(100);
	TOGGLE_STATUS();
	_delay_ms(100);
	TOGGLE_ERROR();
	_delay_ms(100);
}

static uint8_t GlobalBusCommand(uint8_t command) {
	if ((command < ReadCommandCount) && (command < 16)) {
		workQueue |= _BV(command);
	}
	
	// provide input value back to lexer
#if INPUT_BLOCK
	if (command == AuxiliaryRead) {
		return ABCAuxiliary();
	}
#endif
	
	// supply standard block data back to lexer
	return ABCReadData();
}

#if INPUT_BLOCK
static uint8_t ABCAuxiliary(void) {
	return 0b00000000 & read_adc();
}
#endif

static uint8_t ABCReadData(void) {
#if INPUT_BLOCK
	return (adjacentBlocks() | (category << 4) | 0b00001111);
#elif POTENTIOMETER
	return (adjacentBlocks() | (category << 4) | read_adc());
#else
	return (adjacentBlocks() | (category << 4) | function);
#endif
}

static void serviceWorkqueue(void) {
	// Read
	if (workQueue & _BV(ReadDataCommand)) {
		// may need delay for ADC
		workQueue &= ~_BV(ReadDataCommand);
	}
	
	// Send Vertical
	if (workQueue & _BV(SendVerticalCommand)) {
		sendDaisyChainVertical();
		workQueue &= ~_BV(SendVerticalCommand);
	}
	
	// Send Horizontal
	if (workQueue & _BV(SendHorizontalCommand)) {
		STATUS_PORT |= _BV(STATUS_LED);
		sendDaisyChainHorizontal();
		workQueue &= ~_BV(SendHorizontalCommand);
	}
	
	// Status On
	if (workQueue & _BV(StatusLedOnCommand)) {
		STATUS_PORT |= _BV(STATUS_LED);
		workQueue &= ~_BV(StatusLedOnCommand);
	}
	
	// Status Blink
	if (workQueue & _BV(StatusLedBlinkCommand)) {
		// may move to timer
		STATUS_PORT &= ~_BV(STATUS_LED);
		TOGGLE_STATUS();
		_delay_ms(50);
		TOGGLE_STATUS();
		_delay_ms(50);
		workQueue &= ~_BV(StatusLedBlinkCommand);
	}
	
	// Status Off
	if (workQueue & _BV(StatusLedOffCommand)) {
		STATUS_PORT &= ~_BV(STATUS_LED);
		workQueue &= ~_BV(StatusLedOffCommand);
	}
	
	// Error On
	if (workQueue & _BV(ErrorLedOnCommand)) {
		ERROR_PORT |= _BV(ERROR_LED);
		workQueue &= ~_BV(ErrorLedOnCommand);
	}
	
	// Error Blink
	if (workQueue & _BV(ErrorLedBlinkCommand)) {
		// may move to timer
		ERROR_PORT &= ~_BV(ERROR_LED);
		TOGGLE_ERROR();
		_delay_ms(50);
		TOGGLE_ERROR();
		_delay_ms(50);
		workQueue &= ~_BV(ErrorLedBlinkCommand);
	}
	
	// Error Off
	if (workQueue & _BV(ErrorLedOffCommand)) {
		ERROR_PORT &= ~_BV(ERROR_LED);
		workQueue &= ~_BV(ErrorLedOffCommand);
	}
	
	// Auxiliary Read
	if (workQueue & _BV(AuxiliaryRead)) {
		// do any other work as necessary...
		workQueue &= ~_BV(AuxiliaryRead);
	}
}
