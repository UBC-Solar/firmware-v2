/**
 * This file "declares" the registers used in the module.
 * Due to how the module is set up with the intent to be 
 * part of an embedded application, this header file must 
 * be included and must consist of the register macros.
 * However, the tests run locally and we cannot determine 
 * which memory location will be free at compile time. 
 * As a result, global variables are used in place of 
 * register macros. 
 *
 * Since hardware mocks are required for this test suite, 
 * and the hardware mock will use the same struct and register
 * definitions the global register pointers are declared (marked 
 * as extern,) not defined, to prevent collisions during the 
 * linking stage. This also means that the register pointers will 
 * have to be defined in the unit test file.
 * 
 */

#include <stdint.h>

#ifndef STM32F10X_H
#define STM32F10X_H

/**
 * Struct declarations for each register.
 * Even though the functions tested will only modify and use USART_Mock,
 * any struct referenced to in the source file must be declared here.
 * 
 */
typedef struct
{
	uint32_t APB1ENR;
	uint32_t APB2ENR;
} RCC_Mock;

typedef struct
{
	uint32_t CRL;
	uint32_t CRH;
	uint32_t IDR;
	uint32_t ODR;
	uint32_t BSRR;
	uint32_t BRR;
} GPIO_Mock;

typedef struct
{
	uint16_t SR;
	uint16_t DR;
	uint16_t CR1;
	uint16_t CR2;
	uint16_t CR3;
	uint16_t BRR;
} USART_Mock;

// Required register declarations in place of register macros
extern RCC_Mock* RCC;
extern GPIO_Mock* GPIOA;
extern USART_Mock* USART2;

#endif



