#include "encoder.h"

/**
 * Initializes the encoder for the accelerator pedal
 */
void EncoderInit(void)
{
	// Initialize AFIO Clock
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;

	// TIM 1 Enable
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;

	// Initialize GPIOA Clock
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
	// Configure pins A8 and A9 as inputs
	GPIOA->CRH &= ~(GPIO_CRH_CNF9 | GPIO_CRH_MODE9 | GPIO_CRH_CNF8 | GPIO_CRH_MODE8);
	GPIOA->CRH |= (0x1 << GPIO_CRH_CNF9_Pos) | (0x0 << GPIO_CRH_MODE9_Pos) | 
	              (0x1 << GPIO_CRH_CNF8_Pos) | (0x0 << GPIO_CRH_MODE8_Pos);

	// TIM 1 no pin remap
	AFIO->MAPR &= ~AFIO_MAPR_TIM1_REMAP;

	TIM1->CCMR1 |= 0x1 << TIM_CCMR1_CC1S_Pos;	// Map capture/compare channel 1 to TI1 (Timer Input 1)
	TIM1->CCMR1 |= 0x1 << TIM_CCMR1_CC2S_Pos; 	// Map capture/compare channel 2 to TI2 (Timer Input 2)

	// Enable capture/compare channels 1 and 2, other channels disabled and all polarities default
	TIM1->CCER = (TIM_CCER_CC2E | TIM_CCER_CC1E);

	TIM1->SMCR |= 0x3 << TIM_SMCR_SMS_Pos; // Set timer to encoder mode 3 (clocked by both TI1FP1 and TI2FP2)

	TIM1->CR1 |= TIM_CR1_CEN; // Enable timer
}

/**
 * Reads a value from the encoder
 */
uint16_t EncoderRead(void)
{
	uint16_t encoderReading = TIM1->CNT;

	if (encoderReading < PEDAL_MIN)
	{
		encoderReading = 0;
	}
	else if (encoderReading < PEDAL_MAX)
	{
		encoderReading -= PEDAL_MIN;
	}
	else if (encoderReading < PEDAL_OVERLOAD)
	{
		encoderReading = PEDAL_MAX - PEDAL_MIN;
	}
	else
	{
		encoderReading = 0;
	}

	return encoderReading;
}
