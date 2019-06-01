#include "stm32f10x.h"

#define TIM2_PRESCALER 7199

/** Interrupt handler for Timer 2.
 */
void TIM2_IRQHandler(void);

/** Initializes a timer to trigger a timing interrupt periodically
 * @param period: period for triggering the timing interrupt
 */
void TimerInit(int period);

/** Resets the timer counter to a starting value
 */
void TimerReset(void);
