#ifndef TIMER_H
#define TIMER_H

#include "stm32f103xb.h"

// Processor speed is 36MHz; set prescaler value for a clock frequency of 10kHz
#define TIM2_PRESCALER (3600 - 1)

void TimerInit(unsigned int period);
void RestartTimer(void);
void StopTimer(void);
uint8_t GetTimeoutFlag(void);
void ClearTimeoutFlag(void);
// void TIM2_IRQHandler(void) is defined in this module

#endif /* TIMER_H */
