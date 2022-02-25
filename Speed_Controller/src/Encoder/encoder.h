#ifndef ENCODER_H
#define ENCODER_H

#include "stm32f103xb.h"

#define PEDAL_MAX 0xD0
#define PEDAL_MIN 0x0F
#define PEDAL_OVERLOAD 0xFF

/**
 * Initializes the encoder for the accelerator pedal
 */
void EncoderInit(void);

/**
 * Reads a value from the encoder
 */
uint16_t EncoderRead(void);

#endif /* ENCODER_H */
