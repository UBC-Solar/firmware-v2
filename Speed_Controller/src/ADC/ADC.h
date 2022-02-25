#ifndef ADC_H
#define ADC_H

#include "stm32f103xb.h"

#define ADC_ZERO_THRESHOLD 0x300

/**
 * Initializes an ADC for the regenerative braking toggle
 */
void ADCInit(void);

/**
 * Reads a value from the ADC
 */
uint16_t ReadADC(void);

#endif /* ADC_H */
