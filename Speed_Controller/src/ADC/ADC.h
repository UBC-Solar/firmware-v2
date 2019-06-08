#include "stm32f10x.h"

#ifndef ADC_MODULE
#define ADC_MODULE

#define ADC_MAX 0xFFF  //TODO: CHANGE THIS (CALIBRATE) 

/** 
 * Initializes an ADC for the regenerative braking toggle
 */
void ADCInit(void);

/**
 * Reads a value from the ADC
 */
uint16_t ReadADC(void);

#endif
