//
//  abc_daisy.h
//  a block of code
//
//

#ifndef __daisy__abc_daisy__
#define __daisy__abc_daisy__

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <util/atomic.h>

#define TESTING 1

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
		is present above and to the right of this block, respectively.
		Bits [5:0] will be zeros.
 */
uint8_t adjacentBlocks(void);

void forwardChain(void);

void sendHorizontal(void);

void sendVertical(void);

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

//#define SPI_M_DI	PINA0

#endif /* defined(__MCU_461__) */

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

#endif /* defined(__MCU_2313__) */

#endif /* defined(__daisy__abc_daisy__) */
