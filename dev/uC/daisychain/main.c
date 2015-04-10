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

//#define MCU_2313
#define MCU_461

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
#define SPI_S_DI_DIR	DDRA0
#define SPI_S_PORT		PORTB
#define SPI_S_INT_PORT	PCIE
#define SPI_CLK_INT_PRT	PCIE
#define SPI_S_CLK_PCINT	PCINT2
#define SPI_F_B_PCINT	PCINT1
#define SPI_F_L_PCINT	PCINT0
//#define SPI_S_DO		PORTA0
//#define SPI_S_DO_DDR	DDRA
//#define SPI_S_DO_DIR	DDRA0

// SPI Master Pins
#define SPI_M_DO_REG	PORTA
#define SPI_M_DO		PORTA1
#define SPI_M_PIN_REG	PINB
#define SPI_M_CLK		PINB6
#define SS_T_ABOVE		PINB4
#define SS_T_RIGHT		PINB3
#define SPI_M_DDR		DDRB
#define SPI_M_CLK_DIR	DDB6
#define SPI_T_A_DIR		DDB4
#define SPI_T_R_DIR		DDB3
#define SPI_M_DO_DDR	DDRA
#define SPI_M_DO_DIR	DDRA1
#define SPI_M_PORT		PORTB
#define SPI_M_INT_PORT	PCIE
#define SPI_T_A_PCINT	PCINT4
#define SPI_T_R_PCINT	PCINT3
//#define SPI_S_DI		PORTA0
//#define SPI_S_DI_DDR	DDRA
//#define SPI_S_DI_DIR	DDRA0


//#define SPI_M_DI	PINA0

#endif

/**
 *	Using the ATtiny461 model microcontrollers
 */
#ifdef MCU_461

// Testing LED (Debug)
#if TESTING
#define STATUS_LED		PA1
#define STATUS_PORT		PORTA
#define TOGGLE_STATUS	(STATUS_PORT ^= _BV(STATUS_LED))
#endif

// Error LED
#define ERROR_LED		PA3
#define ERROR_PORT		PORTA
#define TOGGLE_ERROR	(ERROR_PORT ^= _BV(ERROR_LED))

// SPI Slave Pins
#define SPI_S_DI_REG	PINA
#define SPI_S_DI		PINA0
#define SPI_S_PIN_REG	PINB
#define SPI_S_CLK		PINB4
#define SS_F_BELOW		PINB3
#define SS_F_LEFT		PINB1
#define SPI_S_DDR		DDRB
#define SPI_S_CLK_DIR	DDB4
#define SPI_F_B_DIR		DDB3
#define SPI_F_L_DIR		DDB1
#define SPI_S_DI_DDR	DDRA
#define SPI_S_DI_DIR	DDA0
#define SPI_S_PORT		PORTB
#define SPI_S_INT_PORT	PCIE0
#define SPI_CLK_INT_PRT	PCIE1
#define SPI_S_CLK_PCINT	PCINT12
#define SPI_F_B_PCINT	PCINT11
#define SPI_F_L_PCINT	PCINT9
#define SPI_PCMSK		PCMSK1
//#define SPI_S_DO		PORTA0
//#define SPI_S_DO_DDR	DDRA
//#define SPI_S_DO_DIR	DDRA0

// SPI Master Pins
#define SPI_M_DO_REG	PORTA
#define SPI_M_DO		PORTA7
#define SPI_M_PIN_REG	PINB
#define SPI_M_CLK		PORTA6
#define SS_T_ABOVE		PINB6
#define SS_T_RIGHT		PINB5
#define SPI_M_DDR		DDRB
#define SPI_M_CLK_DIR	DDA6
#define SPI_T_A_DIR		DDB6
#define SPI_T_R_DIR		DDB5
#define SPI_M_DO_DDR	DDRA
#define SPI_M_DO_DIR	DDA7
#define SPI_M_PORT		PORTB
#define SPI_M_INT_PORT	PCIE1
#define SPI_T_A_PCINT	PCINT14
#define SPI_T_R_PCINT	PCINT13
//#define SPI_S_DI		PORTA0
//#define SPI_S_DI_DDR	DDRA
//#define SPI_S_DI_DIR	DDRA0

#endif

// keep track of current program state
typedef enum {
	PowerOn,
	StartHandshake,
	EndHandshake,
	SpiSlave,
	SpiMasterHandshake,
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

volatile static uint8_t spi_m_data_out	= 0;
volatile static uint8_t m_data_out_pos	= 9;

#ifdef SPI_S_DO
volatile static uint8_t spi_s_data_out	= 0;
volatile static uint8_t s_data_out_pos	= 8;
#endif

volatile static uint8_t prev_pin_b		= 0;
volatile static uint8_t prev_pin_d		= 0;
volatile static uint8_t trig_f_below	= 0;
volatile static uint8_t trig_f_left		= 0;
volatile static uint8_t rx_completed	= 0;
volatile static uint8_t tx_completed	= 0;
volatile static uint8_t i2c_addr		= 0;

volatile static uint8_t irc_count		= 0;

static const uint8_t m_clk_ms			= 2;

static void initSPISlave(void);
static void initSPIMaster(void);
static void showAddress(void);
static void initIO(void);
static void startupSequence(void);

static void initHandshake(void);

static void serviceHandshakeStart(void);
static void serviceHandshakeEnd(void);
static void serviceSpiSlaveTransmission(uint8_t changedPins);
static void serviceMasterHandshake(uint8_t changedPins);

static void waitForCompletedHandshake(void);
static void sendVector(uint8_t newBlockDirection);

/**
 *	Initializes pins and registers for handshake protocol
 */
void initHandshake(void) {
	
	cur_state = StartHandshake;
	
	// SPI signal directions
	SPI_S_DDR	 &= ~_BV(SPI_S_CLK_DIR);	// set slave clock as input
	SPI_S_DI_DDR &= ~_BV(SPI_S_DI_DIR);		// set slave DI as input
	
	// set slave selects temporarily as outputs to "announce" new block presence
	SPI_S_DDR	 |= _BV(SPI_F_B_DIR);
	SPI_S_DDR	 |= _BV(SPI_F_L_DIR);
	
	// start slave selects low
	SPI_S_PORT	 &= ~_BV(SS_F_BELOW);
	SPI_S_PORT	 &= ~_BV(SS_F_LEFT);
	
	// enable pin change interrupts (PCI)
	GIMSK		 |= _BV(SPI_S_INT_PORT);	// SPI slave interrupt port
	GIMSK		 |= _BV(SPI_CLK_INT_PRT);	// SPI slave clock interrupt port
	SPI_PCMSK	 |= _BV(SPI_S_CLK_PCINT);	// SPI slave clock
	
	sei();									// enable interrupts
}

/**
 *	Initializes pins and registers for software SPI-slave implementation
 */
void initSPISlave(void) {
	
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
	GIMSK	  |= _BV(SPI_S_INT_PORT);		// SPI slave interrupt port
	GIMSK	  |= _BV(SPI_CLK_INT_PRT);		// SPI slave clock interrupt port
	SPI_PCMSK |= _BV(SPI_S_CLK_PCINT);		// SPI slave clock
	SPI_PCMSK |= _BV(SPI_F_B_PCINT);		// slave select from below
	SPI_PCMSK |= _BV(SPI_F_L_PCINT);		// slave select from left
	
#ifdef SPI_S_DO
	SPI_S_DO_DDR |=  _BV(SPI_S_DO_DIR);		// set slave DO as output
	spi_s_data_out = 6;						// initial value
#endif
	
	//	sei();									// enable interrupts
}

void initSPIMaster(void) {
	
	// disable interrupts for setup
//	cli();
	
	cur_state = SpiMasterHandshake;
	
	// SPI master signal directions
	SPI_M_DO_DDR |= _BV(SPI_M_CLK_DIR);		// set master clock as output
	SPI_M_DO_DDR |= _BV(SPI_M_DO_DIR);		// set master DO as output
	
	// set slave-selects temporarily as inputs to "listen"
	// for new block presence
	SPI_M_DDR	 &= ~_BV(SPI_T_A_DIR);
	SPI_M_DDR	 &= ~_BV(SPI_T_R_DIR);
	
	// enable pin change interrupts (PCI)
	GIMSK		 |= _BV(SPI_M_INT_PORT);	// SPI interrupt port
	SPI_PCMSK	 |= _BV(SPI_T_A_PCINT);		// SPI slave select to-right intrpt
	SPI_PCMSK	 |= _BV(SPI_T_R_PCINT);		// SPI slave select to-above intrpt
	
	// re-enable interrupts
//	sei();
}

void serviceHandshakeStart(void) {
	if (prev_pin_b & _BV(SPI_S_CLK)) {
		// HIGH to LOW transition - change state
		
		/*
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
		 */
		
	} else {
		// LOW to HIGH transition - sample
		
		// if data-in is high, handshake has started
		if (!!(SPI_S_DI_REG & _BV(SPI_S_DI))) {
			hs_started = 1;
		} else {
			// if data-in is low, handshake is completing
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
		}
	}
}

void serviceHandshakeEnd(void) {
	if (prev_pin_b & _BV(SPI_S_CLK)) {
		// HIGH to LOW transition - change state
		
		if (hs_ended) {
			initSPISlave();
		}
	} else {
		// LOW to HIGH transition - sample
		
		// if data-in is high, handshake has ended
		if (!!(SPI_S_DI_REG & _BV(SPI_S_DI))) {
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
			SPI_PCMSK &= ~_BV(SPI_F_L_PCINT);
			
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
			SPI_PCMSK &= ~_BV(SPI_F_B_PCINT);
			
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

void serviceMasterHandshake(uint8_t changedPins) {
	
	uint8_t from_above = 0;
	uint8_t from_right = 0;
	
	if (changedPins & _BV(SS_T_ABOVE)) {
		if (prev_pin_b & _BV(SS_T_ABOVE)) {
			// HIGH to LOW - end handshake
			
		} else {
			// LOW to HIGH - start handshake
			
			from_above = 1;
		}
	} else if (changedPins & _BV(SS_T_RIGHT)) {
		if (prev_pin_b & _BV(SS_T_RIGHT)) {
			// HIGH to LOW - end handshake
			
		} else {
			// LOW to HIGH - start handshake
			
			from_right = 1;
		}
		
	}
	
	// start handshake
	if (from_above || from_right) {
		
		SPI_M_DO_REG |= _BV(SPI_M_DO);
		SPI_M_DO_REG |= _BV(SPI_M_CLK);
		_delay_ms(m_clk_ms);
		SPI_M_DO_REG &= ~_BV(SPI_M_CLK);
		SPI_M_DO_REG &= ~_BV(SPI_M_DO);
		_delay_ms(m_clk_ms);
		
		SPI_M_DO_REG |= _BV(SPI_M_CLK);
		_delay_ms(m_clk_ms);
		SPI_M_DO_REG &= ~_BV(SPI_M_CLK);
		_delay_ms(m_clk_ms);
		
		SPI_M_DO_REG |= _BV(SPI_M_DO);
		SPI_M_DO_REG |= _BV(SPI_M_CLK);
		
		// set corresponding slave select as output
		uint8_t slave = _BV((from_above) ? (SPI_T_A_DIR) : (SPI_T_R_DIR));
		SPI_M_DDR	 |= slave;
		
		// pull HIGH to prepare for tx
		SPI_M_PORT	 |= slave;
		_delay_ms(m_clk_ms);
		
		SPI_M_DO_REG  &= ~_BV(SPI_M_CLK);
		SPI_M_DO_REG &= ~_BV(SPI_M_DO);
		_delay_ms(m_clk_ms);
		
		sendVector(slave);
	}
}

void sendVector(uint8_t newBlockDirection) {
	
	//	cur_state = SpiMaster;
	
	
	TOGGLE_STATUS;
	_delay_ms(m_clk_ms);
	TOGGLE_STATUS;
	_delay_ms(m_clk_ms);
	TOGGLE_STATUS;
	_delay_ms(m_clk_ms);
	TOGGLE_STATUS;
	_delay_ms(m_clk_ms);
	
	
	// pull slave select LOW to signal tx start
	SPI_M_PORT &= ~newBlockDirection;
	
	// put bits on data out line
	while (m_data_out_pos--) {
		SPI_M_DO_REG = (SPI_M_DO_REG & ~_BV(SPI_M_DO)) | (((i2c_addr >> m_data_out_pos) & 0b00000001) << SPI_M_DO);
		
		// pulse clock
		SPI_M_DO_REG |= _BV(SPI_M_CLK);
		_delay_ms(m_clk_ms);
		SPI_M_DO_REG &= ~_BV(SPI_M_CLK);
		_delay_ms(m_clk_ms);
	}
	
	// reset position
	m_data_out_pos = 9;
	
	// pull HIGH to signal end of tx
	SPI_M_PORT |= newBlockDirection;
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
			
			// starting SPI Master handshake
		case SpiMasterHandshake:
			serviceMasterHandshake(changed);
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
		
		initSPIMaster();
		
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
//	TOGGLE_ERROR;

#ifdef MCU_2313
	PORTD &= 0b11110000;
	PORTD |= (i2c_addr & 0b00000011);
	PORTD |= ((i2c_addr & (0b00000011 << 3)) >> 1);
//	spi_s_data_in = 0;
//	i2c_addr = 0;
//	rx_completed = 0;
#endif
#ifdef MCU_461
	PORTA &= 0b11001001;
	PORTA |= ((i2c_addr << 2) & 0b00001100);
	PORTA |= ((i2c_addr << 1) & 0b00110000);
//	spi_s_data_in = 0;
//	i2c_addr = 0;
//	rx_completed = 0;
#endif
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
	DDRA |= _BV(PA1);
	DDRA |= _BV(PA2);
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
#endif
}

void waitForCompletedHandshake(void) {
	uint8_t left = 1;
	uint8_t cycle = 0;
	
	
	// bring SS_F_LEFT or SS_F_BELOW to HIGH
	SPI_S_PORT |= _BV((left) ? (SS_F_LEFT) : (SS_F_BELOW));
	
	// master processor may be busy
	_delay_ms(m_clk_ms);
	
	// spin until handshake has been recognized
	while (cur_state == StartHandshake && !hs_started) {
		
		// disable clock pin change interrupt
		SPI_PCMSK &= ~_BV(SPI_S_CLK_PCINT);
		
		// bring SS_F_LEFT or SS_F_BELOW back down LOW
		SPI_S_PORT &= ~_BV((left) ? (SS_F_LEFT) : (SS_F_BELOW));
		
		// give more time to finish
		_delay_ms(m_clk_ms);
		
		// re-eneable clock pin change interrupt
		SPI_PCMSK |= _BV(SPI_S_CLK_PCINT);
		
		// toggle to other input select
		left = !left;
		
		// bring SS_F_LEFT or SS_F_BELOW to HIGH
		SPI_S_PORT |= _BV((left) ? (SS_F_LEFT) : (SS_F_BELOW));
		
		// master processor may be busy
		_delay_ms(m_clk_ms);
		
		cycle++;
		if (!(cycle % 10)) {
			TOGGLE_STATUS;
//			cycle = 0;
		}
	}
}

int main(void) {
	
	initIO();
//	initSPISlave();
	startupSequence();
	
	initHandshake();
	waitForCompletedHandshake();
	
	PORTA &= 0b11000000;

	
	while (1) {
		_delay_ms(150);
		
		TOGGLE_STATUS;
	}
	return 0; // never reached
}
