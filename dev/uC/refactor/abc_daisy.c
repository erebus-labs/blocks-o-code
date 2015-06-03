//
//  abc_daisy.c
//  a block of code
//
//

#include "abc_daisy.h"

static inline void initSlaveHandshake(void);
static inline void initSPISlave(void);
static inline void initSPIMaster(void);

void toggleUntilMasterResponse(void);
void slaveSelectToggler(uint8_t signal);

static inline void disableSPISlave(void);
static inline void disableSPIMaster(void);

static inline void formatVector(uint8_t data);

/**
 *  DEBUG function to show received x,y coords on breadboard LEDs
 */

static inline void serviceSpiSlaveTransmission(uint8_t changedPins);

static inline void sendVector(uint8_t newBlockDirection);

static inline void disableSPISlave(void);
static inline void disableSPIMaster(void);

static void updateAdjacentBlocks(void);
static void serviceTrigger(void);

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
	Idle,
	ResetSlave,
	ResetMaster,
	// ...
} ProgramState;

// modified in ISRs
static volatile ProgramState cur_state	= PowerOn;
static volatile uint8_t stateTriggered	= 0;
static volatile uint8_t adjacentBits	= 0;

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

static uint8_t adjacentUpdated = 0;

static const	uint8_t clk_ms			= 3;

/**
 *  Blocking function returns when a SPI transmission provides vector position
 */
uint8_t waitForVector(void) {

	cur_state = PowerOn;
	stateTriggered = 0;
	
	// initialize pins and registers
	initSlaveHandshake();
	
	if (!adjacentUpdated) {
		updateAdjacentBlocks();
		adjacentUpdated = 1;
	}
	
	// spin until rx has received vector and assigned it to the global address
	while (!rx_completed) {
		serviceTrigger();
	}
	
	return globalAddress;
}


void updateChain(void) {
	serviceTrigger();
}

/**
 *  Function to query for attached neighbor blocks.
 *
 *  @return Single unsigned byte with bits [7:6] set if a block
	is present above and to the right of this block, respectively.
	Bits [5:0] will be zeros.
 */
uint8_t adjacentBlocks(void) {
	return adjacentBits;
}

void sendDaisyChainHorizontal(void) {
	STATUS_PORT |= _BV(STATUS_LED);
	s_toggle_left = 1;
	initSPIMaster();
}

void sendDaisyChainVertical(void) {
	ERROR_PORT |= _BV(ERROR_LED);
	s_toggle_left = 0;
	initSPIMaster();
}

void updateAdjacentBlocks(void) {
	adjacentBits  = (SPI_M_PIN_REG & _BV(SS_T_ABOVE)) ? (0) : (0b10000000);
	adjacentBits |= (SPI_M_PIN_REG & _BV(SS_T_RIGHT)) ? (0) : (0b01000000);
}

/**
 *  Spins until it receives signal from a master to continue handshake
 */
void toggleUntilMasterResponse(void) {
	
	// start signalling blocks for update
	s_toggle_left = 0;
	uint8_t signal = (s_toggle_left) ? (SS_F_LEFT) : (SS_F_BELOW);
	
	// spin while slave signals low
	// until handshake start has been recognized 
	uint8_t repeat = 1;
	while (repeat && (((~(SPI_S_PIN_REG)) & _BV(SPI_S_CLK))
					  && ((~(SPI_S_DI_REG)) & _BV(SPI_S_DI)))) {
		
		TOGGLE_ERROR();
		
		// toggles slave select lines until response
		slaveSelectToggler(signal);
		
		// master process may be busy, give time to finish and send interrupt signal
		if (!stateTriggered) {
			// bring SS_F_LEFT or SS_F_BELOW back down LOW
			SPI_S_PORT &= ~_BV(signal);
			_delay_ms(clk_ms);
		}
		
		// disable interrupts for scope, prevents disruptive late attempts
//		ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
//		cli();
		if (!stateTriggered) {
			
			// toggle to other input select
			s_toggle_left = !s_toggle_left;
			
			// reassign signal from toggled select
			signal = (s_toggle_left) ? (SS_F_LEFT) : (SS_F_BELOW);
		} else {
			// handshake started
			repeat = 0;
		}
//		sei();
//		}
	}
}

/**
 *  Signal master block through slave select line to trigger slave handshake.
 *
 *  @param signal Either SS_F_LEFT or SS_F_BELOW, toggling back and forth
 *
 *  @return Current state will update from handshake, remain if no response
 */
void slaveSelectToggler(uint8_t signal) {
	
	// bring SS_F_LEFT or SS_F_BELOW to HIGH
	SPI_S_PORT |= _BV(signal);
	
	// master process may be busy, give time to finish and send interrupt signal
	_delay_ms(clk_ms);
}


static void serviceTrigger(void) {


	switch (cur_state) {
		case PowerOn:
			// poll until master brings s_clk AND s_di LOW
			if (((~(SPI_S_PIN_REG)) & _BV(SPI_S_CLK))
			&& ((~(SPI_S_DI_REG)) & _BV(SPI_S_DI))) {
				
				TOGGLE_STATUS();
				
				// advance program state
				cur_state = StartSlaveHandshake;
				stateTriggered = 0;

				// enable interrupts to prepare for responses
				sei();
			}
			break;
			
		case StartSlaveHandshake:
			toggleUntilMasterResponse();
			
			if (stateTriggered) {
				
				// advance handshake
				stateTriggered = 0;
				cur_state = SlaveHandshakeA;
				
				// wait for signals to register
				_delay_ms(clk_ms);
				
				uint8_t sel_pin = (s_toggle_left) ? (SS_F_LEFT) : (SS_F_BELOW);

				// master clock triggers the select signal
				// bring SS_F_LEFT or SS_F_BELOW back down LOW
				SPI_S_PORT &= ~_BV(sel_pin);
			} else {
				waitForVector();
			}
			break;
			
		case SlaveHandshakeA:
			
			if (stateTriggered) {
    
				// advance handshake
				stateTriggered = 0;
				cur_state = SlaveHandshakeB;

				// wait for signals to register
				_delay_ms(clk_ms);

//				ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
//				cli();
				// determine currently responding signal
				uint8_t sel_dir = (s_toggle_left) ? (SPI_F_L_DIR) : (SPI_F_B_DIR);
				uint8_t sel_pin = (s_toggle_left) ? (SS_F_LEFT) : (SS_F_BELOW);

				// master clock triggers the select signal
				// bring SS_F_LEFT or SS_F_BELOW to HIGH
				SPI_S_PORT |= _BV(sel_pin);
				
				// previous port value associated with pull-up
				prev_pin_b |= _BV(sel_pin);			// initialize high
				
				// release select line for master to retain after a short delay
//					_delay_ms(5);
				SPI_S_DDR  &= ~_BV(sel_dir);		// set responding as input
			
				// pull-up resistor
				SPI_S_PORT |= _BV(sel_pin);			// slave select pull-up
//				}
//				sei();
			}
			
			break;
		
		case SlaveHandshakeB:
			
			if (stateTriggered) {
				// complete handshake and initialize SPI slave for RX
				stateTriggered = 0;
				initSPISlave();
				
				// wait for signals to register
//				_delay_ms(clk_ms);
			}
			
			break;
			
		case SpiSlave:
			
			// stateTriggered set to 1 when SPI rx has completed
			if (stateTriggered) {
				
				stateTriggered = 0;
				
				// reset positions
				s_data_in_pos = 8;
				
				// structure vector from SPI transferred data
				formatVector(spi_s_data_in);
				
				disableSPISlave();
				rx_completed = 1;
				
			} else if (rx_completed) {
				initSPIMaster();

				// m_sel lines in use, return to prevent adjacent block update
				return;
			}
			
			break;
			
		case StartMasterHandshake:
			
			if (stateTriggered) {
				// advance handshake
				stateTriggered = 0;
				cur_state = MasterHandshakeA;
			
				// handshake initiated, send response signals
				SPI_M_DO_REG |= _BV(SPI_M_DO);
				SPI_M_DO_REG |= _BV(SPI_M_CLK);
			}
			
			// m_sel lines in use, return to prevent adjacent block update
			return;
			
		case MasterHandshakeA:
			
			if (stateTriggered) {
				// advance handshake
				stateTriggered = 0;
				cur_state = MasterHandshakeB;
				
				// handshake continued, send response signals
				SPI_M_DO_REG &= ~_BV(SPI_M_DO);
				SPI_M_DO_REG &= ~_BV(SPI_M_CLK);
			}
			// m_sel lines in use, return to prevent adjacent block update
			return;
			
		case MasterHandshakeB:
			
			if (stateTriggered) {

				// swap slave signal to output then send vector
				cur_state = SpiMaster;
				
				// let slave device release signal, long delay for capacitance?
				_delay_ms(10);
				
				// set corresponding slave select as output and bring low
				SPI_M_DDR	  |= master_sel_bit;
				SPI_M_PORT	  &= ~master_sel_bit;
				
				// pulse the clock with select line low to complete handshake
				// give some delay to allow master to sink the current
				_delay_ms(20);
				SPI_M_DO_REG  |= _BV(SPI_M_CLK);
				_delay_ms(clk_ms);
				SPI_M_DO_REG  &= ~_BV(SPI_M_CLK);
				_delay_ms(clk_ms);
			}
			
			// m_sel lines in use, return to prevent adjacent block update
			return;
			
		case SpiMaster:
			
			// stateTriggered not auto-cleared from previous state completion,
			// should continue through this state
			if (stateTriggered) {
				
				stateTriggered = 0;
				cur_state = Idle;
				
				// send current vector to requesting block
				sendVector(master_sel_bit);
				
				// stop listening
				disableSPIMaster();
				
				// get ready for commands from main processor board
				_delay_ms(clk_ms);
			}
			
			// m_sel lines in use, return to prevent adjacent block update
			return;
			
		case Idle:
			break;
			
		case ResetSlave:
			_delay_ms(30);
			waitForVector();
			break;
			
		case ResetMaster:
			_delay_ms(30);
			initSPIMaster();
			
			// m_sel lines in use, return to prevent adjacent block update
			return;

			
		default:
			break;
	}
//	updateAdjacentBlocks();
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
				
				// trigger and exit switch statement
				stateTriggered = 1;
				break;
			}
			
			// unexpected input for state, reset handshake
			cur_state = ResetSlave;
			break;
			
		//
		case SlaveHandshakeA:
			if ((changed & _BV(SPI_S_CLK))			// master clock interrupt
			&& (prev_pin_b & _BV(SPI_S_CLK))) {		// HIGH to LOW: change state
				
				// trigger and exit switch statement
				stateTriggered = 1;
				break;
			}
			
			// unexpected input for state, reset handshake
			cur_state = ResetSlave;
			break;
			
		case SlaveHandshakeB:
			if ((changed & _BV(SPI_S_CLK))			// master clock interrupt
			&& !(prev_pin_b & _BV(SPI_S_CLK))		// LOW to HIGH: sample data
			&& !(SPI_S_DI_REG & _BV(SPI_S_DI))		// data-in is LOW
			&& !(SPI_S_PIN_REG & _BV(sel_pin))) {	// SEL is LOW
				
				// trigger and exit switch statement
				stateTriggered = 1;
				break;
			}
			
			// unexpected input for state, reset handshake
			cur_state = ResetSlave;
			break;
			
		// soft SPI slave select triggers transmission
		case SpiSlave:
			serviceSpiSlaveTransmission(changed);
			
			break;
			
		// starting SPI Master handshake
		case StartMasterHandshake:
			if ((changed & _BV(SS_T_ABOVE))			// slave SEL to-above intrpt
			&& !(prev_pin_b & _BV(SS_T_ABOVE))) {	// LOW to HIGH: start hndshk

				stateTriggered = 1;

				// save signaling slave select pin
				master_sel_bit = _BV(SS_T_ABOVE);

				// disable the other slave select interrupt
				SPI_PCMSK &= ~_BV(SPI_T_R_PCINT);
			} else
			if ((changed & _BV(SS_T_RIGHT))			// slave SEL to-right intrpt
			&& !(prev_pin_b & _BV(SS_T_RIGHT))) {	// LOW to HIGH: start hndshk
				
				stateTriggered = 1;
				
				// save signaling slave select pin
				master_sel_bit = _BV(SS_T_RIGHT);
				
				// disable the other slave select interrupt
				SPI_PCMSK &= ~_BV(SPI_T_A_PCINT);	// slave sel to-above intrpt
			}
			
			break;
			
		// advancing SPI Master handshake
		case MasterHandshakeA:
			if ((changed & master_sel_bit)			// compare to saved SEL pin
			&& (prev_pin_b & master_sel_bit)) {		// HIGH to LOW
				
				stateTriggered = 1;
				
				break;
			}
			
			cur_state = ResetMaster;
			break;
			
		// completing SPI Master handshake
		case MasterHandshakeB:
			if ((changed & master_sel_bit)			// compare to saved SEL pin
			&& !(prev_pin_b & master_sel_bit)) {	// LOW to HIGH
				// LOW to HIGH - handshake complete
				stateTriggered = 1;
				
				break;
			}
			
			cur_state = ResetMaster;
			break;
			
		default:
			break;
	}
	
	// update history
	prev_pin_b = current;
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

	// set slave selects temporarily as outputs to "announce" new block presence
	SPI_S_DDR	 |= _BV(SPI_F_B_DIR);
	SPI_S_DDR	 |= _BV(SPI_F_L_DIR);
	
	// start slave selects low
	SPI_S_PORT	 &= ~_BV(SS_F_BELOW);
	SPI_S_PORT	 &= ~_BV(SS_F_LEFT);
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
}

/**
 *	Initializes device for master transmission. It does so selectively, 
	i.e. if this block received its vector from the left, it will continue
	passing the message to the right. If it received from below, it will send
	above. Also, if it received from below, but it doesn't have a block located 
	above, then it is considered 'at the top' and will try to send to the right.
 */
static inline void initSPIMaster(void) {

	cur_state = StartMasterHandshake;
	stateTriggered = 0;
	master_sel_bit  = 0;
	
	// set slave select lines (back) to inputs to "listen" for block presence
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
	} else {
		opposite = SS_T_ABOVE;
		SPI_PCMSK |= _BV(SPI_T_A_PCINT);
	}

	// if the opposing block has brought the associated select line LOW,
	// respond by bringing m_do and m_clk LOW to initiate handshake
	if (~SPI_M_PIN_REG & _BV(opposite)) {
		
		// SPI interrupt port
		GIMSK		 |= _BV(SPI_M_INT_PORT);	// master port interrupt enable
		
		// SPI master signal directions
		SPI_M_DO_DDR |= _BV(SPI_M_CLK_DIR);		// set master clock as output
		SPI_M_DO_DDR |= _BV(SPI_M_DO_DIR);		// set master DO as output
		
		// Bring m_clk and m_do LOW to start handshake for opposing block
		SPI_M_DO_REG &= ~_BV(SPI_M_DO);
		SPI_M_DO_REG &= ~_BV(SPI_M_CLK);
		
		// allow lines to register changes and interrupt
		uint8_t wait = 55;
		while ((!stateTriggered) && wait) {
			_delay_ms(1);
			--wait;
		}
		
		// TIMEOUT: no handshake initiated, release lines
		if (!stateTriggered) {
			disableSPIMaster();
			cur_state = Idle;
		}
	}
}


/**
 *	Interrupt Service Functions
 */

/**
 *	Services signals relating to receiving vector over software SPI.
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
			stateTriggered = 1;
		} else {								// LOW to HIGH
			// transmission started, remove SS_F_LEFT interrupt
			trig_f_below = 1;
			SPI_PCMSK &= ~_BV(SPI_F_L_PCINT);
		}
	} else if (changedPins & _BV(SS_F_LEFT)) {
		
		// trigger from left
		if (prev_pin_b & _BV(SS_F_LEFT)) {		// HIGH to LOW
			// end of receive
			stateTriggered = 1;
			
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
	
	// provide small delay to allow end of tx to be recognized
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
}

/**
 *  DEBUG function to show received x,y coords on breadboard LEDs
 */
static inline void showAddress(void) {
	PORTA &= 0b11000011;
	PORTA |= ((globalAddress << 2) & 0b00001100);
	PORTA |= ((globalAddress << 1) & 0b00110000);
}

