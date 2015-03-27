//
// Daisy Chain utility for the A Block of Code Project
// Greg Stromire
//

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define TESTING 1

/**
 *	Microcontroller selection
 */
#define MCU_2313
//#define MCU_461

/**
 *	Using the ATtiny2313 model microcontrollers
 */
#ifdef MCU_2313

// Testing LED (Debug)
#if TESTING
#define STATUS_LED		PD6
#define STATUS_PORT		PORTD
#define TOGGLE_STATUS	(STATUS_PORT ^= _BV(STATUS_LED))
#endif

// Error LED
#define ERROR_LED		PD5
#define ERROR_PORT		PORTD
#define TOGGLE_ERROR	(ERROR_PORT ^= _BV(ERROR_LED))

// SPI Slave Pins
#define SPI_S_DI_REG	PINA
#define SPI_S_DI		PINA0
#define SPI_S_PIN_REG	PINB
#define SPI_S_CLK		PINB2
#define SS_F_BELOW		PINB1
#define SS_F_LEFT		PINB0
#define SPI_S_DDR		DDRB
#define SPI_S_CLK_DIR	DDB2
#define SPI_F_B_DIR		DDB1
#define SPI_F_L_DIR		DDB0
#define SPI_S_DI_DDR	DDRA
#define SPI_S_DI_DIR	DDRA1
#define SPI_S_PORT		PORTB
#define SPI_S_INT_PORT	PCIE
#define SPI_S_CLK_PCINT	PCINT2
#define SPI_F_B_PCINT	PCINT1
#define SPI_F_L_PCINT	PCINT0
//#define SPI_S_DO		PORTA0
//#define SPI_S_DO_DDR	DDRA
//#define SPI_S_DO_DIR	DDRA0


// SPI Master Pins
#define SPI_M_CLK		PORTB6
#define SPI_M_DO		PORTA1
#define SS_T_ABOVE		PORTB4
#define SS_T_RIGHT		PORTB3
//#define SPI_M_DI	PINA0

/**
 *	Using the ATtiny461 model microcontrollers
 */
#elif MCU_461

#endif

// keep track of current program state
typedef enum {
	PowerOn,
	StartHandshake,
	EndHandshake,
	SpiSlave,
	SpiMaster,
	TwoWireTransmission
//	...
} ProgramState;

// modified in ISRs
volatile static ProgramState cur_state	= PowerOn;
volatile static uint8_t hs_started		= 0;
volatile static uint8_t hs_ended		= 0;

volatile static uint8_t spi_s_data_in	= 0;
volatile static uint8_t s_data_in_pos	= 8;

#ifdef SPI_S_DO
volatile static uint8_t spi_s_data_out	= 0;
volatile static uint8_t s_data_out_pos	= 8;
#endif

volatile static uint8_t prev_pin_b		= 0;
volatile static uint8_t prev_pin_d		= 0;
volatile static uint8_t trig_f_below	= 0;
volatile static uint8_t trig_f_left		= 0;
volatile static uint8_t rx_completed	= 0;
volatile static uint8_t i2c_addr		= 0;

volatile static uint8_t irc_count		= 0;

static void initSoftSPI(void);
static void showAddress(void);
static void initIO(void);
static void startupSequence(void);

static void initHandshake(void);

static void serviceHandshakeStart(void);
static void serviceHandshakeEnd(void);
static void serviceSpiSlaveTransmission(uint8_t changedPins);
static void waitForCompletedHandshake(void);

/**
 *	Initializes pins and registers for handshake protocol
 */
void initHandshake(void){
	
	cur_state = StartHandshake;

	// SPI signal directions
	SPI_S_DDR	 &= ~_BV(SPI_S_CLK_DIR);	// set slave clock as input
	SPI_S_DI_DDR &= ~_BV(SPI_S_DI_DIR);		// set slave DI as input
	
	// set slave selects temporarily as outputs to "announce" new block presence
	SPI_S_DDR	 |= _BV(SPI_F_B_DIR);
	SPI_S_DDR	 |= _BV(SPI_F_L_DIR);
	
	// start slave selects high
	SPI_S_PORT	 |= _BV(SS_F_BELOW);
	SPI_S_PORT	 |= _BV(SS_F_LEFT);
	
	// enable pin change interrupts (PCI)
	GIMSK |= _BV(SPI_S_INT_PORT);			// SPI slave interrupt port
	PCMSK |= _BV(SPI_S_CLK_PCINT);			// SPI slave clock
	
	sei();									// enable interrupts
}

/**
 *	Initializes pins and registers for software SPI-slave implementation
 */
void initSoftSPI(void) {
	cur_state = SpiSlave;

	// SPI signal directions
	SPI_S_DDR	 &= ~_BV(SPI_S_CLK_DIR);	// set slave clock as input
	SPI_S_DI_DDR &= ~_BV(SPI_S_DI_DIR);		// set slave DI as input
	SPI_S_DDR	 &= ~_BV(SPI_F_B_DIR);		// set slave selects as inputs
	SPI_S_DDR	 &= ~_BV(SPI_F_L_DIR);
	
	// SPI pull-up resistors
	SPI_S_PORT |= _BV(SS_F_BELOW);			// slave select from below
	SPI_S_PORT |= _BV(SS_F_LEFT);			// slave select from left
	
	// previous port values associated with pull-ups
	prev_pin_b |= _BV(SS_F_BELOW);			// initialized to high
	prev_pin_b |= _BV(SS_F_LEFT);			// "slave select" pulls low
	
	// enable pin change interrupts (PCI)
	GIMSK |= _BV(SPI_S_INT_PORT);			// SPI slave interrupt port
	PCMSK |= _BV(SPI_S_CLK_PCINT);			// SPI slave clock
	PCMSK |= _BV(SPI_F_B_PCINT);			// slave select from below
	PCMSK |= _BV(SPI_F_L_PCINT);			// slave select from left
	
#ifdef SPI_S_DO
	SPI_S_DO_DDR |=  _BV(SPI_S_DO_DIR);		// set slave DO as output
	spi_s_data_out = 6;						// initial value
#endif
	
	sei();									// enable interrupts
}

void serviceHandshakeStart(void) {
	if (prev_pin_b & _BV(SPI_S_CLK)) {
		// HIGH to LOW transition - change state
		
		if (hs_started) {
			cur_state = EndHandshake;
			
			// set slave selects as inputs
			SPI_S_DDR	&= ~_BV(SPI_F_B_DIR);
			SPI_S_DDR	&= ~_BV(SPI_F_L_DIR);
			
			// SPI pull-up resistors
			SPI_S_PORT	|= _BV(SS_F_BELOW);			// slave select from below
			SPI_S_PORT	|= _BV(SS_F_LEFT);			// slave select from left
			
			// previous port values associated with pull-ups
			prev_pin_b	|= _BV(SS_F_BELOW);			// initialized to high
			prev_pin_b	|= _BV(SS_F_LEFT);			// "slave select" pulls low
		}
		
	} else {
		// LOW to HIGH transition - sample
		
		// if data-in is high, handshake has started
		if (!!(SPI_S_DI_REG & _BV(SPI_S_DI))) {
			hs_started = 1;
		}
	}
}

void serviceHandshakeEnd(void) {
	if (prev_pin_b & _BV(SPI_S_CLK)) {
		// HIGH to LOW transition - change state
		
		if (hs_ended) {
			initSoftSPI();
		}
	} else {
		// LOW to HIGH transition - sample
		
		// if data-in is low, handshake has ended
		if (!(SPI_S_DI_REG & _BV(SPI_S_DI))) {
			hs_ended = 1;
		}
	}
}

void serviceSpiSlaveTransmission(uint8_t changedPins) {
	
	if (changedPins & _BV(SS_F_BELOW)) {
		
		// trigger from below
		if (prev_pin_b & _BV(SS_F_BELOW)) {
			// transmission started, remove SS_F_LEFT interrupt
			trig_f_below = 1;
			PCMSK &= ~_BV(SPI_F_L_PCINT);
			
			// put MSB on slave data out if implemented
#ifdef SPI_S_DO
			if (s_data_out_pos--) {
				PORTA = (PORTA & ~_BV(SPI_S_DO)) | ((spi_s_data_out >> s_data_out_pos) & _BV(SPI_S_DO));
			}
#endif
			
		} else {
			// end of receive
			rx_completed = 1;
		}
	} else if (changedPins & _BV(SS_F_LEFT)) {
		
		// trigger from left
		if (prev_pin_b & _BV(SS_F_LEFT)) {
			// transmission started, remove SS_F_BELOW interrupt
			trig_f_left = 1;
			PCMSK &= ~_BV(SPI_F_B_PCINT);
			
			// put MSB on slave data out if implemented
#ifdef SPI_S_DO
			if (s_data_out_pos--) {
				PORTA = (PORTA & ~_BV(SPI_S_DO)) | ((spi_s_data_out >> s_data_out_pos) & _BV(SPI_S_DO));
			}
#endif
			
		} else {
			// end of receive
			rx_completed = 1;
		}
	} else if (changedPins & _BV(SPI_S_CLK)) {
		
		// SPI mode 0: CPOL = 0, CPHA = 0
		if (prev_pin_b & _BV(SPI_S_CLK)) {
			
			// HIGH to LOW - update (if implemented)
#ifdef SPI_S_DO
			if (s_data_out_pos--) {
				PORTA = (PORTA & ~_BV(SPI_S_DO)) | ((spi_s_data_out >> s_data_out_pos) & _BV(SPI_S_DO));
			}
#endif
			
		} else {
			// LOW to HIGH - sample
			if (s_data_in_pos--) {
				spi_s_data_in |= (!!(SPI_S_DI_REG & _BV(SPI_S_DI)) << s_data_in_pos);
			}
		}
	}
}

/**
 *  Interrupt Service Routine for Pin Change in PCIE (PORTB).
 */
ISR(PCINT_vect) {
	// isolate changed pins
	uint8_t changed = SPI_S_PIN_REG ^ prev_pin_b;
	
	switch (cur_state) {
		// handshake initiated
		case StartHandshake:
			if (changed & _BV(SPI_S_CLK)) {
				serviceHandshakeStart();
			}
			break;
			
		// handshake completing
		case EndHandshake:
			if (changed & _BV(SPI_S_CLK)) {
				serviceHandshakeEnd();
			}
			break;
			
		// soft SPI slave select triggers
		case SpiSlave:
			serviceSpiSlaveTransmission(changed);
			break;
			
		default:
			break;
	}
	
	// set i2c address if vector byte received
	if (rx_completed) {
		
		// reset positions
#ifdef SPI_S_DO
		s_data_out_pos = 8;
#endif
		
		s_data_in_pos = 8;
		
		// assign address from transmitted byte (vector)
		// position structure: bits [2:0] are x-coord, [6:3] are y-coord
		// set x-coord [2:0], increment if triggered from the left
		i2c_addr |= (spi_s_data_in & 0b00000111) + (trig_f_left);
		
		// set y-coord [6:3], increment if triggered from below
		i2c_addr |= (spi_s_data_in & 0b01111000) + (trig_f_below << 3);
		
		showAddress();
		
		// disable pin change interrupts (PCI)
//		PCMSK &= ~_BV(PCINT2);			// clk pin
//		PCMSK &= ~_BV(PCINT6);			// slave select from below
//		PCMSK &= ~_BV(PCINT4);			// slave select from left
//		GIMSK &= ~_BV(PCIE);			// PORTB
//		return;
	}
	
	// update history
	prev_pin_b = SPI_S_PIN_REG;
}

void showAddress(void) {
	TOGGLE_ERROR;
	PORTD &= 0b11110000;
	PORTD |= (i2c_addr & 0b00000011);
	PORTD |= ((i2c_addr & (0b00000011 << 3)) >> 1);
	spi_s_data_in = 0;
	i2c_addr = 0;
	rx_completed = 0;
}

void initIO(void) {
	// set output pins
	DDRD |= _BV(PD0);
	DDRD |= _BV(PD1);
	DDRD |= _BV(PD2);
	DDRD |= _BV(PD3);
	DDRD |= _BV(STATUS_LED);
	DDRB |= _BV(ERROR_LED);
}

void startupSequence(void) {
	for(int i = 0; i < 4; ++i){
		_delay_ms(25);
		PORTD |= _BV(PD0 + i);
	}
	for(int i = 0; i < 4; ++i){
		_delay_ms(25);
		PORTD &= ~_BV(PD0 + i);
	}
}

void waitForCompletedHandshake(void) {
	uint8_t left = 1;
	
	// bring SS_F_LEFT or SS_F_BELOW to low
	SPI_S_PORT &= ~_BV((left) ? (SS_F_LEFT) : (SS_F_BELOW));
	
	// master processor may be busy
	_delay_ms(50);
	
	// spin until handshake has been recognized
	while (cur_state == StartHandshake && !hs_started) {

		// disable clock pin change interrupt
		PCMSK &= ~_BV(SPI_S_CLK_PCINT);
		
		// bring SS_F_LEFT or SS_F_BELOW back up high
		SPI_S_PORT |= _BV((left) ? (SS_F_LEFT) : (SS_F_BELOW));
		
		// give more time to finish
		_delay_ms(50);
		
		// re-eneable clock pin change interrupt
		PCMSK |= _BV(SPI_S_CLK_PCINT);
		
		// toggle to other input select
		left = !left;
		
		// bring SS_F_LEFT or SS_F_BELOW to low
		SPI_S_PORT &= ~_BV((left) ? (SS_F_LEFT) : (SS_F_BELOW));
		
		// master processor may be busy
		_delay_ms(50);
	}
}

int main(void) {
	
	initIO();
	initSoftSPI();
	startupSequence();

//	initHandshake();
//	waitForCompletedHandshake();
	
	while (1) {
		_delay_ms(100);
		
		TOGGLE_STATUS;
	}
	return 0; // never reached
}
