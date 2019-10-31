#include "encoder.h"
#include "virtual_com.h"

/**
 * Initializes the encoder for the accelerator pedal
 */
void EncoderInit (void) {
	
	//Initialize AFIO Clock
	RCC->APB2ENR |= 0x1;
	
	//TIM 1 Enable
	RCC->APB2ENR |= 0x1 << 11;
	
	//Initialize GPIOA Clock
	RCC->APB2ENR |= 0x1 << 2;	
	GPIOA->CRH &= 0;
	GPIOA->CRH |= 0x00000044;
	
	//TIM 1 No Remap
	AFIO->MAPR &= ~(0x11 << 6);
	
	TIM1->CCMR1 |= 0x01;	//Map Channel 1 to TI1
	TIM1->CCMR1 |= 0x01 << 8; //Map Channel 2 to TI2
	
	TIM1->CCER &= ~(0x1 << 1);
	TIM1->CCER &= ~(0x1 << 3); 
	
	TIM1->CCER &= ~(0x1 << 5);
	TIM1->CCER &= ~(0x1 << 7);
	
	TIM1->CCER |= 0x1;
	TIM1->CCER |= 0x1 << 4;
	
	TIM1->SMCR |= 0x011;
	
	TIM1->CR1 |= 0x1;	
	
}

/**
 * Reads a value from the encoder
 */
uint16_t EncoderRead (void) {
    
    uint16_t Encoder_Reading = TIM1->CNT;
    
    if (Encoder_Reading < PEDAL_MIN)
    {
        Encoder_Reading = 0;
    }
    else if (Encoder_Reading < PEDAL_MAX)
    {
        Encoder_Reading = Encoder_Reading - PEDAL_MIN;
    }
    else if (Encoder_Reading < PEDAL_OVERLOAD)
    {
        Encoder_Reading = PEDAL_MAX - PEDAL_MIN;
    }
    else
    {
        Encoder_Reading = 0;
    }
    
    
    return Encoder_Reading;
    
}
