#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define TESTING 1

#if TESTING
#define STATUS_LED	PD6
#define TOGGLE_STATUS (PORTD ^= _BV(STATUS_LED))
#endif

#define ERROR_LED	PB3
#define TOGGLE_ERROR (PORTB ^= _BV(ERROR_LED))

#define SPI_S_CLK	PINB2
#define SPI_S_DO	PORTA0
#define SPI_S_DI	PINA1

#define SS_F_BELOW	PINB6
#define SS_F_LEFT	PINB4

// modified in ISRs
volatile static uint8_t spi_s_data_out	= 0;
volatile static uint8_t s_data_out_pos	= 8;
volatile static uint8_t spi_s_data_in	= 0;
volatile static uint8_t s_data_in_pos	= 8;
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

/**
 *  Initializes pins and registers for software SPI-slave implementation
 */
void initSoftSPI(void) {
	
	// SPI signal directions
	DDRB &= ~_BV(DDB2);				// set clock as input
	DDRA &= ~_BV(DDRA1);			// set DI as input
	DDRB &= ~_BV(DDB6);				// set slave selects as inputs
	DDRB &= ~_BV(DDB4);
	DDRA |=  _BV(DDRA0);			// set DO as output
	
	// SPI pull-up resistors
	PORTB |= _BV(PORTB6);			// slave select from below
	PORTB |= _BV(PORTB4);			// slave select from left
	
	// previous port values associated with pull-ups
	prev_pin_b |= _BV(SS_F_BELOW);	// initialized to high
	prev_pin_b |= _BV(SS_F_LEFT);	// "slave select" pulls low
	
	// enable pin change interrupts (PCI)
	GIMSK |= _BV(PCIE);				// PORTB
	PCMSK |= _BV(PCINT6);			// slave select from below
	PCMSK |= _BV(PCINT4);			// slave select from left
	PCMSK |= _BV(PCINT2);			// clock
	
	spi_s_data_out = 6;				// initial value
	sei();							// enable interrupts
}

/**
 *  Interrupt Service Routine for Pin Change in PCIE0 (PORTB).
	Triggered on a "slave select" pin (SS_FROM_LEFT or SS_FROM_BELOW) -
	initiating or concluding a software SPI slave transmission.
 */
ISR(PCINT_vect) {
	// isolate pin changes
	uint8_t changed = PINB ^ prev_pin_b;
	
	// soft SPI slave select triggers
	if (changed & _BV(SS_F_BELOW)) {
		
		// trigger from below
		if (prev_pin_b & _BV(SS_F_BELOW)) {
			// transmission started, remove SS_F_LEFT interrupt, put DO MSB
			trig_f_below = 1;
			PCMSK &= ~_BV(PCINT4);
			if (s_data_out_pos--) {
				PORTA = (PORTA & ~_BV(SPI_S_DO)) | ((spi_s_data_out >> s_data_out_pos) & _BV(SPI_S_DO));
			}
		} else {
			// end of receive
			rx_completed = 1;
		}
	} else if (changed & _BV(SS_F_LEFT)) {
		
		// trigger from left,
		if (prev_pin_b & _BV(SS_F_LEFT)) {
			// transmission started, remove SS_F_BELOW interrupt, put DO MSB
			trig_f_left = 1;
			PCMSK &= ~_BV(PCINT6);
			if (s_data_out_pos--) {
				PORTA = (PORTA & ~_BV(SPI_S_DO)) | ((spi_s_data_out >> s_data_out_pos) & _BV(SPI_S_DO));
			}
		} else {
			// end of receive
			rx_completed = 1;
		}
	} else if (changed & _BV(SPI_S_CLK)) {
		
		// SPI mode 0: CPOL = 0, CPHA = 0
		if (prev_pin_b & _BV(SPI_S_CLK)) {
			// HIGH to LOW - update
			if (s_data_out_pos--) {
				PORTA = (PORTA & ~_BV(SPI_S_DO)) | ((spi_s_data_out >> s_data_out_pos) & _BV(SPI_S_DO));
			}
		} else {
			TOGGLE_ERROR;

			// LOW to HIGH - sample
			if (s_data_in_pos--) {
				spi_s_data_in |= (!!(PINA & _BV(SPI_S_DI)) << s_data_in_pos);
			}
		}
	}
	

	// set i2c address if vector byte received
	if (rx_completed) {
//		TOGGLE_ERROR;
		
		// reset positions
		s_data_out_pos = 8;
		s_data_in_pos = 8;
		
		// assign address from transmitted byte (vector)
		// position structure: bits [2:0] are x-coord, [6:3] are y-coord
		// set x-coord [2:0], increment if triggered from the left
		i2c_addr |= (spi_s_data_in & 0x07) + (trig_f_left);
		
		// set y-coord [6:3], increment if triggered from below
		i2c_addr |= (spi_s_data_in & 0x78) + (trig_f_below << 3);
		
		showAddress();
		
		// disable pin change interrupts (PCI)
//		PCMSK &= ~_BV(PCINT2);			// clk pin
//		PCMSK &= ~_BV(PCINT6);			// slave select from below
//		PCMSK &= ~_BV(PCINT4);			// slave select from left
//		GIMSK &= ~_BV(PCIE);			// PORTB
//		return;
	}
	
	// update history
	prev_pin_b = PINB;
}

void showAddress(void) {
	//	TOGGLE_ERROR;
	PORTD &= 0xf0;
	PORTD |= (i2c_addr & 0x3);
	PORTD |= ((i2c_addr & (0x3 << 3)) >> 1);
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

int main(void) {
	initIO();
	initSoftSPI();
	startupSequence();
	
	while (1) {
		_delay_ms(100);
		
		TOGGLE_STATUS;
		
//		// switch LED based on SPI input
//		if (rx_completed == 1) {
//			showAddress();
//			rx_completed = 0;
//		}
	}
	return 0; // never reached
}
