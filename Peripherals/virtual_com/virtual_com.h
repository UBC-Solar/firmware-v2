/**
 * This is the header file for functions that utilize the
 * virtual com on the STM32 micro-controller.
 * 
 * These functions will ideally serve as a logging function, 
 * since it requires no additional hardware (unlike other USART 
 * peripherals.)
 * 
 * For now, the virtual com will be polling-based, unidirectional, 
 * with no hardware control (assume logging messages are short.) 
 *
 * If required, this implementation could be changed to interrupt-based, 
 * with a transmitting/receiving queue and hardware control.
 */

#ifndef VIRTUAL_COM_H
#define VIRTUAL_COM_H

#include "stm32f103xb.h"

typedef enum {
	BAUD_9600   = 0,
	BAUD_19200  = 1,
	BAUD_57600  = 2,
	BAUD_115200	= 3	// CAUTION: probably works but clock division isn't bang on - has 0.15% error
} BaudRate_t;

void VirtualComInit(BaudRate_t baud);
void SendChar(char c);
void SendString(char* c);
void SendInt(uint32_t i);
void SendLine(void);
void SendCR(void);

#endif /* VIRTUAL_COM_H */
