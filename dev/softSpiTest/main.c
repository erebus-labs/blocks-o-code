#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define TESTING 1

#if TESTING
#define STATUS_LED	PD6
#define TOGGLE_STATUS (PORTD ^= _BV(STATUS_LED))
#endif

#define ERROR_LED	PD5
#define TOGGLE_ERROR (PORTD ^= _BV(ERROR_LED))

#ifndef PCIE0
#define PCIE0 5
#endif

#ifndef PCIE1
#define PCIE1 3
#endif

#define SPI_S_CLK	PINA1
#define SPI_S_DO	PORTA0
#define SPI_S_DI	PIND3

#define SS_F_BELOW	PINB6
#define SS_F_LEFT	PINB4


/* ATtiny2313a modifications for programming*/
#ifndef DDA1
#define DDA1 1
#endif

#ifndef DDA0
#define DDA0 0
#endif

#ifndef PCMSK1
#define PCMSK1 _SFR_IO8(0x004)
#endif

#ifndef PCINT_B_vect
#define PCINT_B_vect _VECTOR(11)
#endif

#ifndef PCINT_A_vect
#define PCINT_A_vect _VECTOR(19)
#endif

#ifndef PCINT9
#define PCINT9 1
#endif

#ifndef WDTCSR
#define WDTCSR _SFR_IO8(0x21)
#endif
/* End modifications */


// modified in ISRs
volatile static uint8_t spi_s_data_out	= 0;
volatile static uint8_t s_data_out_pos	= 8;
volatile static uint8_t spi_s_data_in	= 0;
volatile static uint8_t s_data_in_pos	= 8;
volatile static uint8_t prev_pin_b		= 0;
volatile static uint8_t prev_pin_a		= 0;
volatile static uint8_t trig_f_below	= 0;
volatile static uint8_t trig_f_left		= 0;
volatile static uint8_t tx_completed	= 0;
volatile static uint8_t i2c_addr		= 0;

volatile static uint8_t irc_counter		= 0;

static void initSoftSPI(void);
static void showAddress(void);
static void initIO(void);
static void startupSequence(void);

/**
 *  Initializes pins and registers for software SPI-slave implementation
 */
void initSoftSPI(void) {
	
	// SPI signal directions
	DDRA &= ~_BV(DDA1);				// set clock as input
	DDRD &= ~_BV(DDD3);				// set DI as input
	DDRB &= ~_BV(DDB6);				// set slave selects as inputs
	DDRB &= ~_BV(DDB4);
	DDRA |=  _BV(DDA0);				// set DO as output
	
	// SPI pull-up resistors
//	PORTA |= _BV(PORTA1);			// clock idles high (SPI CPOL = 1)
	PORTB |= _BV(PORTB6);			// slave select from below
	PORTB |= _BV(PORTB4);			// slave select from left
	
	// previous port values associated with pull-ups
//	prev_pin_a |= _BV(SPI_S_CLK);
	prev_pin_b |= _BV(SS_F_BELOW);	// initialized to high
	prev_pin_b |= _BV(SS_F_LEFT);	// "slave select" pulls low
	
	// enable pin change interrupts (PCI)
	GIMSK  |= _BV(PCIE1);			// PORTA
	GIMSK  |= _BV(PCIE0);			// PORTB
	PCMSK1 |= _BV(PCINT9);			// clk pin
	PCMSK  |= _BV(PCINT6);			// slave select from below
	PCMSK  |= _BV(PCINT4);			// slave select from left
 
	// Disable Watchdog Timer
	//	_WDR();
	// Clear WDRF in MCUSR
	MCUSR = 0x00;
	// Write one to WDCE and WDE
	WDTCSR |= (1<<WDCE) | (1<<WDE);
	// Turn off WDT
	WDTCSR = 0x00;
	
	spi_s_data_out = 6;				// initial value
	sei();							// enable interrupts
}

/**
 *  Interrupt Service Routine for Pin Change in PCIE0 (PORTB). 
	Triggered on a "slave select" pin (SS_FROM_LEFT or SS_FROM_BELOW) - 
	initiating or concluding a software SPI slave transmission.
 */
ISR(PCINT_B_vect) {
	// isolate pin changes
	uint8_t changed = PINB ^ prev_pin_b;
	
	// soft SPI slave select triggers
	if (changed & _BV(SS_F_BELOW)) {
		// trigger from below
		if (prev_pin_b & _BV(SS_F_BELOW)) {
			// transmission started, remove SS_F_LEFT interrupt, put DO MSB
			trig_f_below = 1;
			PCMSK &= ~_BV(PCINT4);
			if (s_data_out_pos) {
				PORTA = (PORTA & ~_BV(SPI_S_DO)) | ((spi_s_data_out >> --s_data_out_pos) & _BV(SPI_S_DO));
			}
		} else {
			// transmission ended
			tx_completed = 1;
		}
	} else if (changed & _BV(SS_F_LEFT)) {
		// trigger from left,
		if (prev_pin_b & _BV(SS_F_LEFT)) {
			// transmission started, remove SS_F_BELOW interrupt, put DO MSB
			trig_f_left = 1;
			PCMSK &= ~_BV(PCINT6);
			if (s_data_out_pos) {
				PORTA = (PORTA & ~_BV(SPI_S_DO)) | ((spi_s_data_out >> --s_data_out_pos) & _BV(SPI_S_DO));
			}
		} else {
			// transmission ended
			tx_completed = 1;
		}
	}

	// set i2c address if vector byte received
	if (tx_completed) {
//		TOGGLE_ERROR;

		// reset positions
		s_data_out_pos = s_data_in_pos = 8;
		
		// assign address from transmitted byte (vector)
		// position structure: bits [2:0] are x-coord, [6:3] are y-coord
		// set x-coord [2:0], increment if triggered from the left
		i2c_addr |= (spi_s_data_in & 0x07) + (trig_f_left);
		
		// set y-coord [6:3], increment if triggered from below
		i2c_addr |= (spi_s_data_in & 0x78) + (trig_f_below << 3);
		
		// disable pin change interrupts (PCI)
		GIMSK  &= ~_BV(PCIE1);			// PORTA
		GIMSK  &= ~_BV(PCIE0);			// PORTB
		PCMSK1 &= ~_BV(PCINT9);			// clk pin
		PCMSK  &= ~_BV(PCINT6);			// slave select from below
		PCMSK  &= ~_BV(PCINT4);			// slave select from left
		return;
	}
	
	// update history
	prev_pin_b = PINB;
}

/**
 *  Interrupt Service Routine for Pin Change in PCIE1 (PORTA).
	Triggered on every clock edge from incoming master-driven SPI SCK.
	Read-in bits on the DI line (and optionally write out bits on the DO).
 */
ISR(PCINT_A_vect) {
	
	// if a trigger has not happened yet, return and wait for a slave select
	if (!(trig_f_below | trig_f_left)) {
		return;
	}
	
	irc_counter++;
	if (irc_counter > 200) {
		TOGGLE_ERROR;
		irc_counter = 0;
	}
	
	// isolate pin changes
	uint8_t changed = PINA ^ prev_pin_a;
	
	// this ISR should only trigger on clock pulses
	// but checking clock pin anyway
	if (changed & _BV(SPI_S_CLK)) {
		// SPI mode 0: CPOL = 0, CPHA = 0
		if (prev_pin_a & _BV(SPI_S_CLK)) {
			// HIGH to LOW - update
			if (s_data_out_pos) {
				PORTA = (PORTA & ~_BV(SPI_S_DO)) | ((spi_s_data_out >> --s_data_out_pos) & _BV(SPI_S_DO));
			}
		} else {
			// LOW to HIGH - sample
			if (s_data_in_pos) {
				spi_s_data_in |= ((PIND & _BV(SPI_S_DI)) << --s_data_in_pos);
			}
		}
	}
	
	// update history
	prev_pin_a = PINA;
}

void showAddress(void) {
//	TOGGLE_ERROR;
	PORTB |= (i2c_addr & 0x3);
	PORTB |= ((i2c_addr & (0x3 << 3)) >> 1);
}

void initIO(void) {
	// set output pins
	DDRB |= _BV(PB0);
	DDRB |= _BV(PB1);
	DDRB |= _BV(PB2);
	DDRB |= _BV(PB3);
	DDRD |= _BV(STATUS_LED);
	DDRD |= _BV(ERROR_LED);
}

void startupSequence(void) {
	for(int i = 0; i < 4; ++i){
		_delay_ms(50);
		PORTB |= _BV(PB0 + i);
	}
	for(int i = 0; i < 4; ++i){
		_delay_ms(50);
		PORTB &= ~_BV(PB0 + i);
	}
}

int main(void) {
	initIO();
	initSoftSPI();
	
	startupSequence();
	
	while (1) {
		for(int i = 0; i < 5; ++i){
			_delay_ms(30);
		}
		
		TOGGLE_STATUS;
		
		// switch LED based on SPI input
		if (tx_completed == 1) {
			showAddress();
			tx_completed = 0;
		}
	}
	return 0; // never reached
}
