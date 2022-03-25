#ifndef ADC_H
#define ADC_H

#include "stm32f103xb.h"

// Options for ReadADC()
#define ADC_DEBUG 0
#define ADC_REVERSE_READING 0
#define ADC_ZERO_THRESHOLD 0x300

void ADCInit(void);
uint16_t ReadADC(void);

#endif /* ADC_H */
