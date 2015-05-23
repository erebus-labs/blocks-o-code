//
//  abc_daisy.c
//  a block of code
//
//

#include "abc_daisy.h"

void initSlaveHandshake(void);
static inline void initSPISlave(void);
static inline void initSPIMaster(void);

static inline void disableSPISlave(void);
static inline void disableSPIMaster(void);

void toggleUntilMasterResponse(void);
uint8_t slaveSelectToggler(uint8_t signal);
static inline void resetSlaveState(void);
static inline void resetMasterState(void);

static inline void formatVector(uint8_t data);
static inline void showAddress(void);

static inline void serviceSpiSlaveTransmission(uint8_t changedPins);

static inline void sendVector(uint8_t newBlockDirection);

// keep track of current program state
typedef enum {
	PowerOn,
	StartSlaveHandshake,
	SlaveHandshakeA,
	SlaveHandshakeB,
	SpiSlave,
	StartMasterHandshake,
	MasterHandshakeA,
	MasterHandshakeB,
	SpiMaster,
	GlobalBusCommand
	// ...
} ProgramState;

// modified in ISRs
static volatile ProgramState cur_state	= PowerOn;

static volatile uint8_t spi_s_data_in	= 0;
static volatile uint8_t s_data_in_pos	= 8;

static volatile uint8_t spi_m_data_out	= 0;
static volatile uint8_t m_data_out_pos	= 8;

static volatile uint8_t prev_pin_b		= 0;
static volatile uint8_t trig_f_below	= 0;
static volatile uint8_t trig_f_left		= 0;
static volatile uint8_t rx_completed	= 0;
static volatile uint8_t globalAddress	= 0;

static volatile uint8_t s_toggle_left	= 0;
static volatile uint8_t master_sel_bit  = 0;
static volatile uint8_t slave_sel_bit   = 0;

static const	uint8_t clk_ms			= 2;

void debugBlink(void);

void debugBlink(void) {
	_delay_ms(100);
	TOGGLE_STATUS();
	_delay_ms(150);
	TOGGLE_STATUS();
	_delay_ms(150);
}

/**
 *  Blocking function returns when a SPI transmission provides vector position
 */
uint8_t waitForVector(void) {
	
	// initialize pins and registers
	initSlaveHandshake();
	
	// spin until master brings s_di AND s_clk LOW
	uint8_t wait = 0;
	uint8_t toggle = 0;
	while ((SPI_S_PIN_REG & _BV(SPI_S_CLK)) || (SPI_S_DI_REG & _BV(SPI_S_DI))) {
		++wait;
		if (wait == 0) {
			++toggle;
			if (toggle % 200 == 0) {
				TOGGLE_STATUS();
			}
		}
	}
	
	// enable interrupts
	sei();
	
	// spin until rx has received vector and assigned it to the global address
	while (!rx_completed) {
		
		// initial handshake state, toggles slave select lines until response
		if (cur_state == StartSlaveHandshake) {
			toggleUntilMasterResponse();
		}
	}
	
	return globalAddress;
}

/**
 *  Function to query for attached neighbor blocks.
 *
 *  @return Single unsigned byte with bits [7:6] set if a block
	is present above and to the right of this block, respectively.
	Bits [5:0] will be zeros.
 */
uint8_t adjacentBlocks(void){
	uint8_t adjacent = 0;
	adjacent |= (SPI_M_PIN_REG & _BV(SS_T_ABOVE)) ? (0) : (0b10000000);
	adjacent |= (SPI_M_PIN_REG & _BV(SS_T_RIGHT)) ? (0) : (0b01000000);
	return adjacent;
}

void forwardChain(void) {
	initSPIMaster();
}

void sendDaisyChainHorizontal(void) {
//	DDRA |= _BV(PA4);
//	PORTA |= _BV(PORTA4);
	STATUS_PORT |= _BV(STATUS_LED);

	s_toggle_left = 1;
	initSPIMaster();
}

void sendDaisyChainVertical(void) {
	s_toggle_left = 0;
	initSPIMaster();
}

/**
 *  Sets up master communication and sends vector.
 *
 *  @return Flag to indicate if it forwarded the vector
 */
/*
uint8_t passOnVector(void) {
	
	// initialize device for master communication
	initSPIMaster();
	
	PORTA = 0;
	PORTA |= _BV(PA2);

	uint8_t opposite = (s_toggle_left) ? (SS_T_RIGHT) : (SS_T_ABOVE);
	if (!(SPI_M_PIN_REG & _BV(opposite))) {
		PORTA |= _BV(PA3);
		
		// SPI master signal directions
		SPI_M_DO_DDR |= _BV(SPI_M_CLK_DIR);		// set master clock as output
		SPI_M_DO_DDR |= _BV(SPI_M_DO_DIR);		// set master DO as output
		
		// Bring m_clk and m_do low to start handshake for opposing block
		SPI_M_DO_REG &= ~_BV(SPI_M_DO);
		SPI_M_DO_REG &= ~_BV(SPI_M_CLK);
	}
	
	return 0;
}
*/

/**
 *  Spins until it receives signal from a master to continue handshake
 */
void toggleUntilMasterResponse(void) {
	
	// start signalling left block for update
	s_toggle_left = 1;
	uint8_t signal = (s_toggle_left) ? (SS_F_LEFT) : (SS_F_BELOW);
	
	// spin until handshake start has been recognized
	uint8_t repeat = 1;
	while (repeat) {
		
		// toggles slave select lines until response
		slaveSelectToggler(signal);
		
		// disable interrupts globally, prevents disruptive late attempts
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
			if (cur_state == StartSlaveHandshake) {
				// bring SS_F_LEFT or SS_F_BELOW back down LOW
				SPI_S_PORT &= ~_BV(signal);
				
				// toggle to other input select
				s_toggle_left = !s_toggle_left;
				
				// reassign signal from toggled select
				signal = (s_toggle_left) ? (SS_F_LEFT) : (SS_F_BELOW);
			} else {
				// handshake started
				repeat = 0;
			}
		}
	}
}
/**
 *  Signal master block through slave select line to trigger slave handshake.
 *
 *  @param signal Either SS_F_LEFT or SS_F_BELOW, toggling back and forth
 *
 *  @return Current state will update from handshake, remain if no response
 */
uint8_t slaveSelectToggler(uint8_t signal) {
	
	// bring SS_F_LEFT or SS_F_BELOW to HIGH
	SPI_S_PORT |= _BV(signal);
	
	// master process may be busy, give time to finish and send interrupt signal
	_delay_ms(clk_ms);
	
	// interrupts will advance program state
	return cur_state;
}

/**
 *  Interrupt Service Routine for Pin Change Interrupts (PCIE0 and PCIE1).
 *	Interrupts automatically disabled in function and re-enabled on return.
 */
ISR(PCINT_vect) {
	
	// isolate changed pins
	uint8_t current = SPI_S_PIN_REG;
	uint8_t changed = current ^ prev_pin_b;
	uint8_t sel_pin = (s_toggle_left) ? (SS_F_LEFT) : (SS_F_BELOW);
	
	switch (cur_state) {
			
		// handshake initiated (toggling)
		case StartSlaveHandshake:
			if ((changed & _BV(SPI_S_CLK))			// master clock interrupt
			&& !(prev_pin_b & _BV(SPI_S_CLK))		// LOW to HIGH: sample data
			&& (SPI_S_DI_REG & _BV(SPI_S_DI))) {	// data-in is HIGH
				
				// advance handshake
				cur_state = SlaveHandshakeA;
				
				_delay_ms(clk_ms);
//				PORTA |= _BV(PA2);

				// master clock triggers the select signal
				// bring SS_F_LEFT or SS_F_BELOW back down LOW
				SPI_S_PORT &= ~_BV(sel_pin);
				
				// exit switch statement
				break;
			}
			
			// unexpected input for state, reset handshake
			resetSlaveState();
			break;
			
		//
		case SlaveHandshakeA:
			if ((changed & _BV(SPI_S_CLK))			// master clock interrupt
			&& (prev_pin_b & _BV(SPI_S_CLK))) {		// HIGH to LOW: change state
				
				// advance handshake
				cur_state = SlaveHandshakeB;

				_delay_ms(clk_ms);
//				PORTA |= _BV(PA3);

				// determine currently responding signal
				uint8_t sel_dir = (s_toggle_left) ? (SPI_F_L_DIR) : (SPI_F_B_DIR);
				
				// master clock triggers the select signal
				// bring SS_F_LEFT or SS_F_BELOW to HIGH
				SPI_S_PORT |= _BV(sel_pin);
				
				// release select line for master to retain after a short delay
				_delay_ms(5);
				SPI_S_DDR  &= ~_BV(sel_dir);		// set responding as input
				
				// pull-up resistor
				SPI_S_PORT |= _BV(sel_pin);			// slave select pull-up
				
				// previous port value associated with pull-up
				prev_pin_b |= _BV(sel_pin);			// initialize high
				
				// exit switch statement
				break;
			}
			
			// unexpected input for state, reset handshake
			resetSlaveState();
			break;
			
		case SlaveHandshakeB:
			if ((changed & _BV(SPI_S_CLK))			// master clock interrupt
			&& !(prev_pin_b & _BV(SPI_S_CLK))		// LOW to HIGH: sample data
			&& !(SPI_S_DI_REG & _BV(SPI_S_DI))		// data-in is LOW
			&& !(SPI_S_PIN_REG & _BV(sel_pin))) {	// SEL is LOW
				
				// complete handshake and initialize SPI slave for RX
				initSPISlave();
				
				_delay_ms(clk_ms);
//				PORTA |= _BV(PA4);
				
				// exit switch statement
				break;
			}
			
			// unexpected input for state, reset handshake
			resetSlaveState();
			break;
			
		// soft SPI slave select triggers transmission
		case SpiSlave:
			serviceSpiSlaveTransmission(changed);
			
			if (rx_completed) {
				
				// reset positions
				s_data_in_pos = 8;
				
				// structure vector from SPI transferred data
				formatVector(spi_s_data_in);
				
				showAddress();
				
				disableSPISlave();
			}
			break;
			
		// starting SPI Master handshake
		case StartMasterHandshake:
			if ((changed & _BV(SS_T_ABOVE))			// slave SEL to-above intrpt
			&& !(prev_pin_b & _BV(SS_T_ABOVE))) {	// LOW to HIGH: start hndshk

				// advance handshake
				cur_state = MasterHandshakeA;

				// save signaling slave select pin
				master_sel_bit = _BV(SS_T_ABOVE);

				// disable the other slave select interrupt
				SPI_PCMSK &= ~_BV(SPI_T_R_PCINT);
			} else
			if ((changed & _BV(SS_T_RIGHT))			// slave SEL to-right intrpt
			&& !(prev_pin_b & _BV(SS_T_RIGHT))) {	// LOW to HIGH: start hndshk
				
				// advance handshake
				cur_state = MasterHandshakeA;
				
				// save signaling slave select pin
				master_sel_bit = _BV(SS_T_RIGHT);
				
				// disable the other slave select interrupt
				SPI_PCMSK &= ~_BV(SPI_T_A_PCINT);	// slave sel to-above intrpt
			}
			
			// handshake started, send response signals
			if (cur_state == MasterHandshakeA) {
				
//				PORTA |= _BV(PA3);

//				PORTA &= ~_BV(PA2);
				
				SPI_M_DO_REG |= _BV(SPI_M_DO);
				SPI_M_DO_REG |= _BV(SPI_M_CLK);
			}
			break;
			
		// advancing SPI Master handshake
		case MasterHandshakeA:
			if ((changed & master_sel_bit)			// compare to saved SEL pin
			&& (prev_pin_b & master_sel_bit)) {		// HIGH to LOW
				
				// advance handshake
				cur_state = MasterHandshakeB;
				
//				PORTA |= _BV(PA2);
				
				// handshake continued, send response signals
				SPI_M_DO_REG &= ~_BV(SPI_M_DO);
				SPI_M_DO_REG &= ~_BV(SPI_M_CLK);
				
				break;
			}
			
			resetMasterState();
			break;
			
		// completing SPI Master handshake
		case MasterHandshakeB:
			if ((changed & master_sel_bit)			// compare to saved SEL pin
			&& !(prev_pin_b & master_sel_bit)) {	// LOW to HIGH
				// LOW to HIGH - handshake complete
				// swap slave signal to output then send vector
				cur_state = SpiMaster;
				
				// let slave device release signal, long delay for capacitance?
				_delay_ms(10);
//				PORTA |= _BV(PA4);
				
				// set corresponding slave select as output and bring low
				uint8_t s_bit = master_sel_bit;
				SPI_M_DDR	  |= s_bit;
				SPI_M_PORT	  &= ~s_bit;
				
				// pulse the clock with select line low to complete handshake
				// give some delay to allow master to sink the current
				_delay_ms(20);
				SPI_M_DO_REG  |= _BV(SPI_M_CLK);
				_delay_ms(clk_ms);
				SPI_M_DO_REG  &= ~_BV(SPI_M_CLK);
				_delay_ms(clk_ms);
				
				// send current vector to requesting block
				sendVector(s_bit);
				
				// stop listening
				disableSPIMaster();
				
				// get ready for commands from main processor board
				_delay_ms(clk_ms);
				cur_state = GlobalBusCommand;
				
				// allow blocks to finish
//				_delay_ms(10);
				
//				PORTA &= ~_BV(PA4);
				
				break;
			}
			
			resetMasterState();
			break;
			
		case GlobalBusCommand:
			break;
		default:
			break;
	}
	
	// update history
	prev_pin_b = current;
}

/**
 *  Resets to intial Slave Handshake state (toggling select line) to try again
 */
static inline void resetSlaveState(void) {
	// wait for other block(s) to finish work
	_delay_ms(15);
	waitForVector();
}

static inline void disableSPISlave(void) {
	// change slave select lines back to outputs to "announce" block presence
	SPI_S_DDR	|= _BV(SPI_F_B_DIR);
	SPI_S_DDR	|= _BV(SPI_F_L_DIR);
	
	// return slave selects LOW
	SPI_S_PORT	&= ~_BV(SS_F_BELOW);
	SPI_S_PORT	&= ~_BV(SS_F_LEFT);
	
	// disable pin change interrupts (PCI)
	GIMSK		&= ~_BV(SPI_S_INT_PORT);	// SPI interrupt port
	GIMSK		&= ~_BV(SPI_CLK_INT_PRT);	// SPI slave clock interrupt port
	SPI_PCMSK	&= ~_BV(SPI_S_CLK_PCINT);	// SPI slave clock
	SPI_PCMSK	&= ~_BV(SPI_F_B_PCINT);		// slave select from below
	SPI_PCMSK	&= ~_BV(SPI_F_L_PCINT);		// slave select from left
}

static inline void resetMasterState(void) {
	// wait for other block(s) to finish work
	_delay_ms(15);
	initSPIMaster();
}

static inline void disableSPIMaster(void) {
	
	// disable interrupts
	SPI_PCMSK	 |= _BV(SPI_T_R_PCINT);
	SPI_PCMSK	 |= _BV(SPI_T_A_PCINT);
	GIMSK		 &= ~_BV(SPI_M_INT_PORT);

	// change slave select lines back to inputs to "listen" block presence
	// and enable pull-up resistors
	SPI_M_DDR	 &= ~_BV(SPI_T_A_DIR);		// to-above line
	SPI_M_DDR	 &= ~_BV(SPI_T_R_DIR);		// to-right line
	SPI_M_PORT	 |= _BV(SPI_T_A_DIR);		// pull-up resistors
	SPI_M_PORT	 |= _BV(SPI_T_R_DIR);
	
	// change to tri state (Hi-Z) inputs to 'detatch' from bus
	SPI_M_DO_DDR &= ~_BV(SPI_M_CLK_DIR);
	SPI_M_DO_DDR &= ~_BV(SPI_M_DO_DIR);
	SPI_M_DO_REG &= ~_BV(SPI_M_DO);
	SPI_M_DO_REG &= ~_BV(SPI_M_CLK);
}

/**
 *	Initialization Functions
 */

/**
 *	Initializes pins and registers for slave handshake protocol
 */
void initSlaveHandshake(void) {
	
	cur_state = StartSlaveHandshake;
	
	// set slave selects temporarily as outputs to "announce" new block presence
	SPI_S_DDR	 |= _BV(SPI_F_B_DIR);
	SPI_S_DDR	 |= _BV(SPI_F_L_DIR);
	
	// start slave selects low
	SPI_S_PORT	 &= ~_BV(SS_F_BELOW);
	SPI_S_PORT	 &= ~_BV(SS_F_LEFT);
	
	// SPI slave signal directions and enable pull-up resistors
	SPI_S_DDR	 &= ~_BV(SPI_S_CLK_DIR);	// set slave clock as input
	SPI_S_DI_DDR &= ~_BV(SPI_S_DI_DIR);		// set slave DI as input
	SPI_S_PORT	 |= _BV(SPI_S_CLK);			// pull-up resistors
	SPI_M_DO_REG |= _BV(SPI_S_DI);
	
	// set slave-selects temporarily as inputs to "listen"
	// for new block presence and enable pull-up resistors
	SPI_M_DDR	 &= ~_BV(SPI_T_A_DIR);		// to-above line
	SPI_M_DDR	 &= ~_BV(SPI_T_R_DIR);		// to-right line
	SPI_M_PORT	 |= _BV(SPI_T_A_DIR);		// pull-up resistors
	SPI_M_PORT	 |= _BV(SPI_T_R_DIR);
	
	// only the slave clock signal can interrupt
	PCMSK0 = 0;
	PCMSK1 = 0;
	
	// enable pin change interrupts (PCI)
	GIMSK		 |= _BV(SPI_CLK_INT_PRT);	// SPI slave clock interrupt port
	SPI_PCMSK	  = _BV(SPI_S_CLK_PCINT);	// SPI slave clock only
	
//	PORTA &= 0b11000011;
}

/**
 *	Initializes pins and registers for software SPI-slave implementation
 */
static inline void initSPISlave(void) {
	
	cur_state = SpiSlave;
	
	// SPI signal directions
	SPI_S_DDR	 &= ~_BV(SPI_S_CLK_DIR);	// set slave clock as input
	SPI_S_DI_DDR &= ~_BV(SPI_S_DI_DIR);		// set slave DI as input
	
	// change only currently active select line to input
	SPI_S_DDR	 &= ~_BV((s_toggle_left) ? (SPI_F_L_DIR) : (SPI_F_B_DIR));
	
	// previous pin values associated with select line,
	// initialized to low, "slave select" pulls high
	prev_pin_b	 &= ~_BV((s_toggle_left) ? (SS_F_LEFT) : (SS_F_BELOW));
	
	// enable pin change interrupts (PCI)
	GIMSK		 |= _BV(SPI_S_INT_PORT);	// SPI interrupt port
	GIMSK		 |= _BV(SPI_CLK_INT_PRT);	// SPI slave clock interrupt port
	SPI_PCMSK	 |= _BV(SPI_S_CLK_PCINT);	// SPI slave clock
	SPI_PCMSK	 |= _BV(SPI_F_B_PCINT);		// slave select from below
	SPI_PCMSK	 |= _BV(SPI_F_L_PCINT);		// slave select from left
	
//	sei();									// enable interrupts
}

/**
 *	Initializes device for master transmission. It does so selectively, 
	i.e. if this block received its vector from the left, it will continue
	passing the message to the right. If it received from below, it will send
	above. Also, if it received from below, but it doesn't have a block located 
	above, then it is considered 'at the top' and will try to send to the right.
 */
static inline void initSPIMaster(void) {
	
	// disable interrupts for setup
//	cli();
	
	cur_state = StartMasterHandshake;

	master_sel_bit  = 0;
	
	// return slave select lines back to inputs to "listen" block presence
	// and enable pull-up resistors
	SPI_M_DDR	 &= ~_BV(SPI_T_A_DIR);		// to-above line
	SPI_M_DDR	 &= ~_BV(SPI_T_R_DIR);		// to-right line
	SPI_M_PORT	 |= _BV(SS_T_ABOVE);		// pull-up resistors
	SPI_M_PORT	 |= _BV(SS_T_RIGHT);
	
	// If received from left, or if considered 'at the top' (i.e. in the first,
	// leftmost column and received from below but no block exists above), then
	// send to the right. Otherwise send to the above block.
	uint8_t opposite = 0;
	if (s_toggle_left || (!(globalAddress & 0b111) && (SPI_M_PIN_REG & _BV(SS_T_ABOVE)))){
		opposite = SS_T_RIGHT;
		SPI_PCMSK |= _BV(SPI_T_R_PCINT);
//		PORTA |= _BV(PA3);
	} else {
		opposite = SS_T_ABOVE;
		SPI_PCMSK |= _BV(SPI_T_A_PCINT);
//		PORTA |= _BV(PA3);
	}

	// if the opposing block has brought the associated select line LOW,
	// respond by bringing m_do and m_clk LOW to initiate handshake
	if (!(SPI_M_PIN_REG & _BV(opposite))) {
		
//		PORTA = 0;
//		PORTA |= _BV(PA4);
		
		// SPI master signal directions
		SPI_M_DO_DDR |= _BV(SPI_M_CLK_DIR);		// set master clock as output
		SPI_M_DO_DDR |= _BV(SPI_M_DO_DIR);		// set master DO as output
		
		// Bring m_clk and m_do LOW to start handshake for opposing block
		SPI_M_DO_REG &= ~_BV(SPI_M_DO);
		SPI_M_DO_REG &= ~_BV(SPI_M_CLK);
		
		// SPI interrupt port
		GIMSK		 |= _BV(SPI_M_INT_PORT);	// master port interrupt enable
		
		// allow lines to register changes and interrupt
		uint8_t wait = 10;
		while ((cur_state == StartMasterHandshake) && wait) {
			_delay_ms(1);
			--wait;
		}
		
		// no handshake initiated, release lines
//		if (cur_state == StartMasterHandshake) {
//			disableSPIMaster();
//		}
	}

	
	// re-enable interrupts
//	sei();
}


/**
 *	Interrupt Service Functions
 */

/**
 *	Services signals relating to
 *
 *	SPI mode 0: CPOL = 0, CPHA = 0. Slave Select (SS) lines idle low.
 *
 *  @param changedPins Bit-field of pins that have changed since last interrupt
 */
static inline void serviceSpiSlaveTransmission(uint8_t changedPins) {
	
	if (changedPins & _BV(SS_F_BELOW)) {
		
		// trigger from below
		if (prev_pin_b & _BV(SS_F_BELOW)) {		// HIGH to LOW
			// end of receive
			rx_completed = 1;
		} else {								// LOW to HIGH
			// transmission started, remove SS_F_LEFT interrupt
			trig_f_below = 1;
			SPI_PCMSK &= ~_BV(SPI_F_L_PCINT);
		}
	} else if (changedPins & _BV(SS_F_LEFT)) {
		
		// trigger from left
		if (prev_pin_b & _BV(SS_F_LEFT)) {		// HIGH to LOW
			// end of receive
			rx_completed = 1;
			
		} else {								// LOW to HIGH
			// transmission started, remove SS_F_BELOW interrupt
			trig_f_left = 1;
			SPI_PCMSK &= ~_BV(SPI_F_B_PCINT);
		}
	} else if (changedPins & _BV(SPI_S_CLK)) {
		
		// SPI mode 0
		if (prev_pin_b & _BV(SPI_S_CLK)) {		// HIGH to LOW
			// HIGH to LOW - update data-out (if implemented)
			
		} else {								// LOW to HIGH
			// LOW to HIGH - sample
			if (s_data_in_pos--) {
				spi_s_data_in |= (!!(SPI_S_DI_REG & _BV(SPI_S_DI)) << s_data_in_pos);
			}
		}
	}
}

/**
 *  Data Functions
 *
 */

/**
 *  Sends current position vector on data-out bus (master) to requesting block 
 *	(slave).
 *
 *  @param newBlockDirection Bit position of newly placed block signal
 */
static inline void sendVector(uint8_t slave_bit) {
	
	// pull slave select HIGH to signal tx start
	SPI_M_PORT |= slave_bit;
	
	// put bits on data-out line
	while (m_data_out_pos--) {
		SPI_M_DO_REG = (SPI_M_DO_REG & ~_BV(SPI_M_DO)) | (((globalAddress >> m_data_out_pos) & 0b00000001) << SPI_M_DO);
		
		// pulse clock
		SPI_M_DO_REG |= _BV(SPI_M_CLK);
		_delay_ms(1);
		SPI_M_DO_REG &= ~_BV(SPI_M_CLK);
		_delay_ms(1);
	}
	
	// pull data to default LOW
	SPI_M_DO_REG &= ~_BV(SPI_M_DO);
	
	// reset position
	m_data_out_pos = 8;
	
	// pull LOW to signal end of tx
	SPI_M_PORT &= ~slave_bit;
	
	// need small delay to allow end of tx to be recognized
	_delay_ms(clk_ms);
}

/**
 *  Assign global bus address from daisy chain data
 *
 *  @param data Transmitted vector
 */
static inline void formatVector(uint8_t data) {
	// position structure: bits [2:0] are x-coord, [6:3] are y-coord
	// set x-coord [2:0], increment if triggered from the left
	globalAddress  = (data + trig_f_left) & 0b00000111;
	
	// set y-coord [6:3], increment if triggered from below
	globalAddress |= (data + (trig_f_below << 3)) & 0b01111000;
	
	// old implementation
//	globalAddress = (data & 0b00000111) + (trig_f_left);
//	globalAddress |= (data & 0b01111000) + (trig_f_below << 3);
}

/**
 *  DEBUG function to show received x,y coords on LEDs
 */
static inline void showAddress(void) {
#ifdef MCU_2313
	PORTD &= 0b11110000;
	PORTD |= (globalAddress & 0b00000011);
	PORTD |= ((globalAddress & (0b00000011 << 3)) >> 1);
#endif
#ifdef MCU_461
	PORTA &= 0b11000011;
	PORTA |= ((globalAddress << 2) & 0b00001100);
	PORTA |= ((globalAddress << 1) & 0b00110000);
#endif
}

