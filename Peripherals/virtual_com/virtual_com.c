/**
 * Function implementations for enabling virtual com logging.
 */
#include "virtual_com.h"

/**
 * Initializes virtual com.
 */
void VirtualComInit()
{
	// Enable GPIOA clock, AFIO
	RCC->APB2ENR |= 0x5UL;
	GPIOA->CRL &= ~(0xFF00UL);
	GPIOA->CRL |= 0x4B00UL;
	
	// Enable USART2 clock
	RCC->APB1ENR |= (0x1UL << 17);
	USART2->CR2 = 0;
	USART2->CR3 = 0;
	USART2->CR1 |= 0xC;
	
	// By Default, set baudrate to 9600
	USART2->BRR = 0xEA6UL;
	
	// Enable USART2
	USART2->CR1 |= 0x1 << 13;
	
}

/**
 * Sends ASCII character through USART. 
 *
 * @Param c: transmitted character
 */
void SendChar(char c)
{
	// Write data to data register
	USART2->DR = 0xFF & ((uint8_t) c);
	
	// Poll until character is transmitted
	while (!((USART2->SR >> 7) & 0x1));
	
	// For testing only (to mock hardware)
	#ifdef TEST
	USART2->SR = 0;
	#endif
}

/**
 * Sends string through USART.
 *
 * @Param str: transmitted string
 */
 void SendString(char* str)
 {
	int i;
	
	// Send each character separately
	for (i = 0; str[i] != '\0'; i++)
	{
		SendChar(str[i]);
	}
 }

/**
 * Sends 32-bit unsigned integer through USART (with padded zeros)
 * Number will be represented in hexadecimal.
 * 
 * @Param i: transmitted 32-bit unsigned integer
 */
void SendInt(uint32_t i)
{
	int index;
	
	SendChar(0x30);
	SendChar(0x58);
	
	for (index = 7; index >= 0; index--)
	{
		// Divide integer into half-bytes and send them separately
		SendChar("0123456789ABCDEF"[(i >> (index*4)) & 0xFUL]);
	}
}
 
 /**
 * Sends a new line through USART
 */
void SendLine(void)
 {
	 SendChar(0xA); // Sends a line break
	 SendChar(0xD); // Moves cursor back to start of line
 }
