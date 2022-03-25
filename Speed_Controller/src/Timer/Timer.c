#include "Timer.h"

#define TRUE 1
#define FALSE 0

static volatile int8_t timeoutFlag = FALSE;

/**
 * Initializes a timer in one pulse mode to trigger a timing interrupt after a set interval
 *
 * Assumes APB2 timer clock of 36MHz
 * 
 * @param period Period for triggering the timing interrupt in ms (must be < 65535 / 10)
 */
void TimerInit(unsigned int period)
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; // Enable TIM2 clock

	TIM2->PSC = TIM2_PRESCALER; // Processor speed is 36MHz; set prescaler value for a clock frequency of 10kHz

	TIM2->CR1 &= ~TIM_CR1_DIR;	// Set counter to upcount
	TIM2->CR1 &= ~TIM_CR1_CKD; 	// Set the timer to use default clock division
	TIM2->CR1 |= TIM_CR1_OPM; 	// Set the timer to use one pulse mode

	TIM2->ARR = 10 * period - 1;// Set autoreload value to set the period

	TIM2->DIER |= TIM_DIER_UIE;	// Enable timing interrupt
	NVIC_EnableIRQ(TIM2_IRQn);	// Setup interrupt handler

	DBGMCU->CR |= DBGMCU_CR_DBG_TIM2_STOP; // Halt TIM2 upon debug halt

	TIM2->CR1 |= TIM_CR1_CEN; 	// Enable TIM2
}

/**
 * Restarts the counter on the timer
 */
void RestartTimer(void)
{
	TIM2->CNT = 0;
	TIM2->CR1 |= TIM_CR1_CEN; // Re-enable TIM2
}

/**
 * Stops the counter on the timer
 */
void StopTimer(void)
{
	TIM2->CR1 &= ~TIM_CR1_CEN; // Disable TIM2
}

/**
 * Get the timer timeout flag to see if the timer period has elapsed
 * 
 * @returns The state of the timeout flag
 */
uint8_t GetTimeoutFlag(void)
{
	return timeoutFlag;
}

/**
 * Clear the timer timeout flag 
 */
void ClearTimeoutFlag(void)
{
	timeoutFlag = FALSE;
}

/**
 * Interrupt handler for Timer 2
 */
void TIM2_IRQHandler(void)
{
	if (TIM2->SR & TIM_SR_UIF) // If update interrupt flag is set
	{
		// Set a flag for action here
		timeoutFlag = TRUE;

		TIM2->SR &= ~(TIM_SR_UIF); // Clear the update interrupt flag
	}
}
