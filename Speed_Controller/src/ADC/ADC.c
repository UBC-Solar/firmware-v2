#include "ADC.h"

void ADCInit(void){
	
	// Init PA0 to analog input (0x0)
	GPIOA->CRL &= ~(0xFUL);
	
	// Configure ADC2
	// Enable ADC clock
	RCC->APB2ENR |= RCC_APB2ENR_ADC2EN;
	
	// Enable config registers
	// By default, CR1 is set correctly
	ADC2->CR2 |= ADC_CR2_CONT;
	
	// TODO: check sample time to see diff
	ADC2->SMPR2 |= 0x7UL;
	
	// Start ADC
	ADC2->CR2 |= ADC_CR2_ADON;
	ADC2->CR2    |= 0x8UL;
	while ((ADC2->CR2 >> 3) & 0x1UL);
	ADC2->CR2    |= 0x4UL;
	while ((ADC2->CR2 >> 2) & 0x1UL);
	ADC2->CR2 |= 0x1UL;
	
}

uint16_t ReadADC(void){
	
	return 0xFFFF & ADC2->DR;
}

