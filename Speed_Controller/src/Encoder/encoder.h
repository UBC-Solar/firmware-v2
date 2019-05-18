#include "stm32f10x.h"

#ifndef ENCODER_MODULE
#define ENCODER_MODULE

#define PEDAL_MAX 0xE0

void EncoderInit(void);


uint16_t EncoderRead(void);

#endif
