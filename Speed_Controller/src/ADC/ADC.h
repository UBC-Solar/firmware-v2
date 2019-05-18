#include "stm32f10x.h"

#ifndef ADC_MODULE
#define ADC_MODULE

void ADCInit(void);

uint16_t ReadADC(void);

#endif
