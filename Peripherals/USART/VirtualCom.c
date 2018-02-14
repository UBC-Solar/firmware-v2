/**
 * Function implementations for enabling virtual com logging.
 */

#include "stm32f10x.h"
#include "VirtualCom.h"

/**
 * Initializes virtual com.
 */
void virtualComInit()
{
	// Enable GPIOA clock, AFIO
	RCC->APB2ENR |= 0x5UL;
	GPIOA->CRL = 0x4B00UL;
	
	// Enable USART2 clock
	RCC->APB1ENR |= (0x1UL << 17);
	USART2->CR2 = 0;
	USART2->CR3= 0;
	USART2->CR1 |= 0xC;
	
	// By Default, set baudrate to 9600
	USART2->BRR = 0xEA6UL;
	
	// Enable USART2
	USART2->CR1 |= 0x1 << 13;
	
}

/**
 * Sends ASCII character through USART. 
 *
 * Param: 
 * 	- c: transmitted character
 */
void putChar(char c)
{
	// Write data to data register
	USART2->DR = 0xFF & ((uint8_t) c);
	
	// Poll until character is transmitted
	while (!((USART2->SR >> 7) & 0x1));
}

/**
 * Sends string through USART.
 *
 * Param:
 *	- str: transmitted string
 */
 void sendString(char* str)
 {
	 int i;
	 
	 for (i = 0; str[i] != '\0'; i++)
	 {
		 putChar(str[i]);
	 }
 }

/**
* Sends 32-bit unsigned integer through USART (with padded zeros)
* Number will be represented in hexadecimal.
*  
* Param:
* -	i: transmitted 32-bit unsigned integer
*/
void sendInt(uint32_t i)
{
	int index;
	
	putChar(0x30);
	putChar(0x58);
	
	for (index = 7; index >= 0; index--)
	{
		putChar("0123456789ABCDEF"[(i >> (index*4)) & 0xFUL]);
	}
}
 
 /**
 * Sends a new line through USART
 */
void sendLine(void)
 {
	 putChar(0xA);
	 putChar(0xD);
 }
