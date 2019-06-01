#include "stm32f10x.h"

#ifndef ADC_MODULE
#define ADC_MODULE

#define ADC_MAX 0xFFF  //TODO: CHANGE THIS (CALIBRATE) 


void ADCInit(void);

uint16_t ReadADC(void);

#endif
