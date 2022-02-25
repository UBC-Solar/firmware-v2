#include "ADC.h"
#if ADC_DEBUG
#include "virtual_com.h"
#endif /* ADC_DEBUG */

// 0 - 300 hex pot

/**
 * Initializes an ADC for the regenerative braking control
 */
void ADCInit(void)
{
	// Init PA0 to analog input (CNF and MODE bits all 0)
	GPIOA->CRL &= ~(GPIO_CRL_CNF0 | GPIO_CRL_MODE0);

	// Configure ADC2
	// Enable ADC clock
	RCC->APB2ENR |= RCC_APB2ENR_ADC2EN;

	// Set config registers
	// By default, CR1 is set correctly

	// Enable continuous conversion mode
	ADC2->CR2 |= ADC_CR2_CONT;

	// Set ADC2 Channel 0 to longest sampling time (239.5 cycles)
	ADC2->SMPR2 |= 0x7UL << ADC_SMPR2_SMP0_Pos;

	ADC2->CR2 |= ADC_CR2_ADON;	 		// Power up ADC
	ADC2->CR2 |= ADC_CR2_RSTCAL; 		// Reset ADC calibration
	while (ADC2->CR2 & ADC_CR2_RSTCAL);	// Wait for completion of calibration reset
	ADC2->CR2 |= ADC_CR2_CAL; 			// Initialize ADC calibration
	while (ADC2->CR2 & ADC_CR2_CAL);	// Wait for completion of calibration
	ADC2->CR2 |= ADC_CR2_ADON; 			// Start ADC Conversion
}

/**
 * Reads a value from the ADC
 * 
 * If ADC_DEBUG is on (true), prints the reading to the virtual COM as well
 * If ADC_REVERSE_READING is on (true), the reading is negated and offset by ADC_ZERO_THRESHOLD
 * 
 * @returns Either the raw ADC reading or (ADC_ZERO_THRESHOLD - raw reading) depending on ADC_REVERSE_READING 
 */
uint16_t ReadADC(void)
{
	uint16_t ADC_reading = ADC2->DR;

#if ADC_DEBUG
	SendString("    Raw ADC Reading:");
	SendInt(ADC_reading);
#endif /* ADC_DEBUG */

#if ADC_REVERSE_READING
	if (ADC_reading > ADC_ZERO_THRESHOLD)
	{
		ADC_reading = 0x0;
	}
	else
	{
		ADC_reading = ADC_ZERO_THRESHOLD - ADC_reading;
	}
#endif /* ADC_REVERSE_READING */

	return ADC_reading;
}
