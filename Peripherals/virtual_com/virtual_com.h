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
 *
 */

#include "stm32f10x.h"

/**
 * Initializes virtual com.
 */
void VirtualComInit(void);

/**
 * Sends ASCII character through USART. 
 *
 * @Param c: transmitted character
 */
void SendChar(char c);

/**
 * Sends string through USART.
 *
 * @Param str: transmitted string
 */
 void SendString(char* c);
 
/**
 * Sends 32-bit unsigned integer through USART (with padded zeros)
 * Number will be represented in hexadecimal.
 * 
 * @Param i: transmitted 32-bit unsigned integer
 */
void SendInt(uint32_t i);

/**
 * Sends a new line through USART
 */
void SendLine(void);
