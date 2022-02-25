#ifndef TIMER_H
#define TIMER_H

#include "stm32f103xb.h"

/** Interrupt handler for Timer 2.
 */
void TIM2_IRQHandler(void);

/** Initializes a timer to trigger a timing interrupt periodically
 * @param period: period for triggering the timing interrupt in ms
 */
void TimerInit(int period);

/**
 * Restarts the counter on the timer
 */
void RestartTimer(void);

/**
 * Stops the counter on the timer
 */
void StopTimer(void);

#endif /* TIMER_H */
