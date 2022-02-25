/**
 * Function implementations for enabling virtual com logging.
 */

#include "virtual_com.h"

// Baud = fclk / (16 * (mantissa + fraction / 16))
typedef struct {
	uint16_t mantissa;
	uint8_t fraction;
} BaudParams_t;

static const BaudParams_t baudParams[] = {
	{234, 6},	// 9600
	{117, 3}, 	// 19200
	{39,  1},	// 57600
	{19,  8}	// 115200 (actually 115384)
};

/**
 * Initializes USART2 peripheral for virtual COM
 *
 * Assumes APB1 clock (PCLK1) of 36MHz
 *
 * @param baud Baud rate for serial communication
 */
void VirtualComInit(BaudRate_t baud)
{
	// Enable GPIOA and AFIO clocks
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN;
	// Set up UART pins A2 and A3:
	// Pin A3 as imput
	// Pin A2 as alternate function output push-pull (highest speed)
	GPIOA->CRL &= ~(GPIO_CRL_CNF3 | GPIO_CRL_MODE3 | GPIO_CRL_CNF2 | GPIO_CRL_MODE2);
	GPIOA->CRL |= 	(0x1 << GPIO_CRL_CNF3_Pos) | (0x0 << GPIO_CRL_MODE3_Pos) | 
					(0x2 << GPIO_CRL_CNF2_Pos) | (0x3 << GPIO_CRL_MODE2_Pos);

	// Enable USART2 clock
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

	USART2->CR2 = 0;
	USART2->CR3 = 0;
	USART2->CR1 |= USART_CR1_TE | USART_CR1_RE; // Enable both receive and transmit

	// Set baudrate
	// Baud = fclk / (16 * (mantissa + fraction / 16))
	USART2->BRR = 	(baudParams[baud].mantissa << USART_BRR_DIV_Mantissa_Pos) |
					(baudParams[baud].fraction << USART_BRR_DIV_Fraction_Pos);

	// Enable USART2
	USART2->CR1 |= USART_CR1_UE;
}

/**
 * Sends ASCII character through USART
 *
 * @param c transmitted character
 */
void SendChar(char c)
{
	// Write data to data register
	USART2->DR = 0xFF & ((uint8_t)c);

	// Poll until character is transmitted
	while (!(USART2->SR & USART_SR_TXE));

// For testing only (to mock hardware)
#ifdef TEST
	USART2->SR = 0;
#endif
}

/**
 * Sends string through USART
 *
 * @param str transmitted string
 */
void SendString(char *str)
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
 * Number will be represented in hexadecimal
 *
 * @param i transmitted 32-bit unsigned integer
 */
void SendInt(uint32_t i)
{
	int index;

	SendChar('0');
	SendChar('x');

	for (index = 7; index >= 0; index--)
	{
		// Divide integer into half-bytes and send them separately
		SendChar("0123456789ABCDEF"[(i >> (index * 4)) & 0xFUL]);
	}
}

/**
 * Sends a CRLF new line through USART
 */
void SendLine(void)
{
	SendChar('\r');
	SendChar('\n');
}

/**
 * Sends a carriage return through USART, moving cursor back to start of current line
 */
void SendCR(void)
{
	SendChar('\r');
}
