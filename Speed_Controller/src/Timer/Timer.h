#include "stm32f10x.h"

#define TIM2_PRESCALER 7199

#define TRUE 1
#define FALSE 0

/** Interrupt handler for Timer 2.
 */
void TIM2_IRQHandler(void);


/** Initializes a timer to trigger a timing interrupt periodically
 * @param period: period for triggering the timing interrupt
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

/**
 * Checks the timer status
 */
int8_t CheckTimerStatus(void);

/**
 * Set the timer status
 */
void SetTimerStatus(int8_t timerStatus);
