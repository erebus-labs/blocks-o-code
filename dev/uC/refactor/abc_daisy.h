/**
 *	abc_daisy.h
 *	A Block of Code
 *	PSU Capstone with Erebus Labs
 *	Spring 2015
 *	v1.1
 *
 *	This file defines the interface for the daisychain block identification and
 *	related functions.
 */

#ifndef __daisy__abc_daisy__
#define __daisy__abc_daisy__

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <util/atomic.h>

/**
 *  Blocking function returns when a SPI transmission provides vector position
 *
 *  @return Formatted address byte
 */
uint8_t waitForVector(void);

/**
 *  Function to query for attached neighbor blocks.
 *
 *  @return Single unsigned byte with bits [7:6] set if a block
 *		is present above and to the right of this block, respectively.
 *		Bits [5:0] will be zeros.
 */
uint8_t adjacentBlocks(void);

/**
 *  Function to respond to daisychain events
 */
void updateChain(void);

/**
 *  Functions to advance daisychain directionally
 */
void sendDaisyChainHorizontal(void);
void sendDaisyChainVertical(void);


/**
 *	A Block of Code utilizes the ATtiny461 model microcontrollers
 */
#if defined( __AVR_ATtiny261__ ) | \
	defined( __AVR_ATtiny461__ ) | \
	defined( __AVR_ATtiny861__ )

// Status LED
#define STATUS_LED		PA4
#define STATUS_PORT		PORTA
#define TOGGLE_STATUS()	(STATUS_PORT ^= _BV(STATUS_LED))

// Error LED
#define ERROR_LED		PA5
#define ERROR_PORT		PORTA
#define TOGGLE_ERROR()	(ERROR_PORT ^= _BV(ERROR_LED))

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

#endif /* defined(__AVR_ATtiny461__) */

#endif /* defined(__daisy__abc_daisy__) */
