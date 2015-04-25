//
//  abc_daisy.c
//  a block of code
//
//

#include "abc_daisy.h"

void initSPISlave(void);
void initSPIMaster(void);

void serviceSlaveHandshakeStart(void);
void serviceHandshakeEnd(void);
void serviceSpiSlaveTransmission(uint8_t changedPins);
void serviceMasterHandshake(uint8_t changedPins);

void sendVector(uint8_t newBlockDirection);

// keep track of current program state
typedef enum {
	PowerOn,
	StartSlaveHandshake,
	SlaveHandshakeA,
	SlaveHandshakeB,
	SlaveHandshakeC,
	SlaveHandshakeD,
	SlaveHandshakeE,
	SlaveHandshakeF,
	MidSlaveHandshake,
	SpiSlave,
	SpiMasterHandshake,
	SpiMaster,
	//	...
} ProgramState;

// modified in ISRs
static volatile ProgramState cur_state	= PowerOn;

static volatile uint8_t hs_started		= 0;
static volatile uint8_t hs_ended		= 0;

static volatile uint8_t spi_s_data_in	= 0;
static volatile uint8_t s_data_in_pos	= 8;

static volatile uint8_t spi_m_data_out	= 0;
static volatile uint8_t m_data_out_pos	= 8;

static volatile uint8_t prev_pin_b		= 0;
static volatile uint8_t prev_pin_a		= 0;
static volatile uint8_t trig_f_below	= 0;
static volatile uint8_t trig_f_left		= 0;
static volatile uint8_t rx_completed	= 0;
static volatile uint8_t tx_completed	= 0;
static volatile uint8_t i2c_addr		= 0;

static volatile uint8_t irc_count		= 0;

static volatile uint8_t signal_f_left	= 1;

static const	uint8_t m_clk_ms		= 2;

void debugBlink(void);

void debugBlink(void) {
	_delay_ms(100);
	TOGGLE_STATUS;
	_delay_ms(1000);
	TOGGLE_STATUS;
	_delay_ms(1000);
}

/**
 *	Initializes pins and registers for slave handshake protocol
 */
void initSlaveHandshake(void) {
	
	cur_state = StartSlaveHandshake;
	
	// SPI signal directions
	SPI_S_DDR	 &= ~_BV(SPI_S_CLK_DIR);	// set slave clock as input
	SPI_S_DI_DDR &= ~_BV(SPI_S_DI_DIR);		// set slave DI as input
	
	// set slave selects temporarily as outputs to "announce" new block presence
	SPI_S_DDR	 |= _BV(SPI_F_B_DIR);
	SPI_S_DDR	 |= _BV(SPI_F_L_DIR);
	
	// start slave selects low
	SPI_S_PORT	 &= ~_BV(SS_F_BELOW);
	SPI_S_PORT	 &= ~_BV(SS_F_LEFT);
	
	PCMSK0 = 0;
	PCMSK1 = 0;
	
	// enable pin change interrupts (PCI)
	GIMSK		 |= _BV(SPI_CLK_INT_PRT);	// SPI slave clock interrupt port
	SPI_PCMSK	  = _BV(SPI_S_CLK_PCINT);	// SPI slave clock
	
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
	
//	sei();									// enable interrupts
}

void resetState(void) {
	cur_state = StartSlaveHandshake;
	hs_started = 0;
	waitForCompletedHandshake();
}

/**
 *
 */
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


void serviceSpiSlaveTransmission(uint8_t changedPins) {
	
//	TOGGLE_STATUS;
//	_delay_ms(100);
//	TOGGLE_STATUS;
//	_delay_ms(100);
	
	if (changedPins & _BV(SS_F_BELOW)) {
		
		// trigger from below
		if (prev_pin_b & _BV(SS_F_BELOW)) {
			// transmission started, remove SS_F_LEFT interrupt
			trig_f_below = 1;
			SPI_PCMSK &= ~_BV(SPI_F_L_PCINT);
			
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
			
		} else {
			// end of receive
			rx_completed = 1;
		}
	} else if (changedPins & _BV(SPI_S_CLK)) {
		
		// SPI mode 0: CPOL = 0, CPHA = 0
		if (prev_pin_b & _BV(SPI_S_CLK)) {
			// HIGH to LOW - update (if implemented)
			
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
	
	
//	TOGGLE_STATUS;
//	_delay_ms(m_clk_ms);
//	TOGGLE_STATUS;
//	_delay_ms(m_clk_ms);
//	TOGGLE_STATUS;
//	_delay_ms(m_clk_ms);
//	TOGGLE_STATUS;
//	_delay_ms(m_clk_ms);
	
	
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
	m_data_out_pos = 8;
	
	// pull HIGH to signal end of tx
	SPI_M_PORT |= newBlockDirection;
}

/**
 *  Interrupt Service Routine for Pin Change in PCIE (PORTB).
 */
ISR(PCINT_vect) {
	// isolate changed pins
	uint8_t changed = SPI_S_PIN_REG ^ prev_pin_b;
	uint8_t sel_pin = (signal_f_left) ? (SS_F_LEFT) : (SS_F_BELOW);

	// turn off LED
	STATUS_PORT &= ~_BV(STATUS_LED);
	
//	uint8_t pinbChanged = PINB ^ prev_pin_b;
//	uint8_t pinaChanged = PINA ^ prev_pin_a;
	
	// update history
	prev_pin_b = SPI_S_PIN_REG;
//	prev_pin_a = PINA;
	
//	if (pinaChanged & _BV(PA0)) {
//		TOGGLE_STATUS;
//		_delay_ms(1000);
//		TOGGLE_STATUS;
//		_delay_ms(1000);
//	}
	
	switch (cur_state) {
			
		// handshake initiated
		case StartSlaveHandshake:
			if ((changed & _BV(SPI_S_CLK))			// master clock transition
			&& !(prev_pin_b & _BV(SPI_S_CLK))		// LOW to HIGH: sample data
			&& (SPI_S_DI_REG & _BV(SPI_S_DI))) {	// data-in is HIGH
				
				// advance handshake
				cur_state = SlaveHandshakeA;
				
				// master clock triggers the select signal
				// bring SS_F_LEFT or SS_F_BELOW back down LOW
				SPI_S_PORT &= ~_BV(sel_pin);
				
				debugBlink();
				
				// exit switch statement
				break;
			}
			
			// unexpected input for state, reset handshake
//			resetState();
			break;
		
		//
		case SlaveHandshakeA:
			if ((changed & _BV(SPI_S_CLK))			// master clock transition
			&& (prev_pin_b & _BV(SPI_S_CLK))) {		// HIGH to LOW: change state
				
				// advance handshake
				cur_state = SlaveHandshakeB;
				
				// master clock triggers the select signal
				// bring SS_F_LEFT or SS_F_BELOW to HIGH
				SPI_S_PORT |= _BV(sel_pin);
				
				debugBlink();
				
				// exit switch statement
				break;
			}
			
			// unexpected input for state, reset handshake
//			resetState();
			break;
			
		case SlaveHandshakeB:
			if ((changed & _BV(SPI_S_CLK))			// master clock transition
			&& !(prev_pin_b & _BV(SPI_S_CLK))		// LOW to HIGH: sample data
			&& !(SPI_S_DI_REG & _BV(SPI_S_DI))) {	// data-in is LOW
				
				// advance handshake
				cur_state = SlaveHandshakeC;
				
				// master clock triggers the select signal
				// release select line for master to retain
				SPI_S_DDR &= ~_BV(SPI_F_B_DIR);		// set slave sels as inputs
				SPI_S_DDR &= ~_BV(SPI_F_L_DIR);
				
				debugBlink();
				
				// exit switch statement
				break;
			}
			
			// unexpected input for state, reset handshake
//			resetState();
			break;
			
			
		case SlaveHandshakeC:
			if (changed & _BV(SPI_S_CLK)) {			// master clock transition
				
				// clock rising and falling edges are allowed here,
				// but only the rising edge signals the next handshake stage
				
				if (!(prev_pin_b & _BV(SPI_S_CLK))	// LOW to HIGH: sample data
				&& !(SPI_S_DI_REG & _BV(SPI_S_DI))){// data-in is LOW
					
					// advance handshake
					cur_state = SlaveHandshakeD;
					
					/**
					 *  The master block will have retained control of select line at this point. No action necessary, just confirm clock and data signals.
					 */
					
					debugBlink();
				}
					
				// exit switch statement
				break;
			}
			
			// unexpected input for state, reset handshake
//			resetState();
			break;
			
			
		case SlaveHandshakeD:
			if ((changed & _BV(SPI_S_CLK))			// master clock transition
			&& (prev_pin_b & _BV(SPI_S_CLK))		// HIGH to LOW: sample SEL
			&& !(SPI_S_PIN_REG & _BV(sel_pin))) {	// SEL is LOW
				
				// advance handshake
				cur_state = SlaveHandshakeE;
				
				/**
				 *  The master block will have retained control of select line at this point. No action necessary, just confirm clock and select signals.
				 */
				
				debugBlink();
				
				// exit switch statement
				break;
			}
			
			// unexpected input for state, reset handshake
//			resetState();
			break;
			
		case SlaveHandshakeE:
			if ((changed & _BV(SPI_S_CLK))			// master clock transition
			&& !(prev_pin_b & _BV(SPI_S_CLK))		// LOW to HIGH: sample data
			&& (SPI_S_DI_REG & _BV(SPI_S_DI))) {	// data-in is HIGH
				
				// advance handshake
				cur_state = SlaveHandshakeF;
				
				/**
				 *  The master block will have retained control of select line at this point. No action necessary, just confirm clock and data signals.
				 */
				
				debugBlink();
				
				// exit switch statement
				break;
			}
			
			// unexpected input for state, reset handshake
//			resetState();
			break;
			
			
		case SlaveHandshakeF:
			if ((changed & _BV(SPI_S_CLK))			// master clock transition
			&& (prev_pin_b & _BV(SPI_S_CLK))		// HIGH to LOW: sample SEL
			&& (SPI_S_PIN_REG & _BV(sel_pin))) {	// SEL is HIGH
			
				// complete handshake
//				cur_state = SpiSlave;
				
				// initialize SPI slave for RX
//				initSPISlave();
				
				debugBlink();
				
				// exit switch statement
				break;
			}
			
			// unexpected input for state, reset handshake
//			resetState();
			break;

		
		// soft SPI slave select triggers
		case SpiSlave:

//			serviceSpiSlaveTransmission(changed);
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
		s_data_in_pos = 8;
		
		// assign address from transmitted byte (vector)
		// position structure: bits [2:0] are x-coord, [6:3] are y-coord
		// set x-coord [2:0], increment if triggered from the left
		i2c_addr |= (spi_s_data_in & 0b00000111) + (trig_f_left);
		
		// set y-coord [6:3], increment if triggered from below
		i2c_addr |= (spi_s_data_in & 0b01111000) + (trig_f_below << 3);
		
		showAddress();
		
//		initSPIMaster();
		
		// disable pin change interrupts (PCI)
		//		PCMSK &= ~_BV(PCINT2);			// clk pin
		//		PCMSK &= ~_BV(PCINT6);			// slave select from below
		//		PCMSK &= ~_BV(PCINT4);			// slave select from left
		//		GIMSK &= ~_BV(PCIE);			// PORTB
		//		return;
	}
}

void showAddress(void) {
	//	TOGGLE_ERROR;

//	TOGGLE_STATUS;
//	_delay_ms(1000);
//	TOGGLE_STATUS;
//	_delay_ms(1000);
//	TOGGLE_STATUS;
//	_delay_ms(1000);
//	TOGGLE_STATUS;
//	_delay_ms(1000);

	
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


/**
 *  Spins until receives signal from a master to continue handshake
 */
void waitForCompletedHandshake(void) {
	uint8_t cycle = 0;
	
//	TOGGLE_STATUS;
//	_delay_ms(200);
//	TOGGLE_STATUS;
//	_delay_ms(200);
	
	// bring SS_F_LEFT or SS_F_BELOW to HIGH
	SPI_S_PORT |= _BV((signal_f_left) ? (SS_F_LEFT) : (SS_F_BELOW));
	
	// master processor may be busy
	_delay_ms(m_clk_ms);
	
	// spin until handshake start has been recognized
	while (cur_state == StartSlaveHandshake) { // && hs_started == 0) {
		
//		TOGGLE_STATUS;
//		_delay_ms(400);
//		TOGGLE_STATUS;
//		_delay_ms(400);
//		TOGGLE_STATUS;
//		_delay_ms(400);
//		TOGGLE_STATUS;
//		_delay_ms(400);
		
		// disable clock pin change interrupt
		SPI_PCMSK &= ~_BV(SPI_S_CLK_PCINT);
		
		// bring SS_F_LEFT or SS_F_BELOW back down LOW
		SPI_S_PORT &= ~_BV((signal_f_left) ? (SS_F_LEFT) : (SS_F_BELOW));
		
		// give more time to finish
		_delay_ms(m_clk_ms);
		
		// re-eneable clock pin change interrupt
		SPI_PCMSK |= _BV(SPI_S_CLK_PCINT);
		
		// toggle to other input select
		signal_f_left = !signal_f_left;
		
		// bring SS_F_LEFT or SS_F_BELOW to HIGH
		SPI_S_PORT |= _BV((signal_f_left) ? (SS_F_LEFT) : (SS_F_BELOW));
		
		// master processor may be busy
		_delay_ms(m_clk_ms);
		
		cycle++;
		if (!(cycle % 10)) {
//			TOGGLE_STATUS;
			cycle = 0;
		}
	}
}