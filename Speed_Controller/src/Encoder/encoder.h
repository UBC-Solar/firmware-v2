#ifndef ENCODER_H
#define ENCODER_H

#include "stm32f103xb.h"

#define PEDAL_MAX 0x70
#define PEDAL_MIN 0x0F
#define PEDAL_OVERLOAD 0xFF

void EncoderInit(void);
uint16_t EncoderRead(void);

#endif /* ENCODER_H */
