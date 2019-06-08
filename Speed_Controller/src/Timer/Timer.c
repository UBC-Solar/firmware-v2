#include "Timer.h"

int8_t timeoutFlag = FALSE;

/** Interrupt handler for Timer 2.
 */
void TIM2_IRQHandler(void)
{
	
	if (TIM2->SR & 0x1){
		
		//Set a flag for action here.
		timeoutFlag = TRUE;
			
		TIM2->SR &= ~(0x1);		//reset the update interrupt flag
	}
	
}

/** Initializes a timer to trigger a timing interrupt periodically
 * @param period: period for triggering the timing interrupt
 */
void TimerInit(int period){
	
	RCC->APB1ENR |= 0x1;		//enable TIM2
	
	TIM2->PSC = TIM2_PRESCALER;	//Processor speed is ~72MHZ, so set prescaler value for scaling,
								//Clock frequency of 10kHz
	
	TIM2->CR1 &= ~(0x1UL << 4);	//set counter to upcount
	
	TIM2->ARR &= 0;
	TIM2->ARR = 10*period;		//set autoreload to reset every period
	
	TIM2->CR1 &= (0x11UL << 8);	//set the timer to use default clock division

	TIM2->CR1 |= 0x1UL << 3;	//set the timer to use one pulse mode
	
	TIM2->CR1 |= 0x1UL;			//enable TIM2
	
	TIM2->DIER |= 0x1UL;		//enable timing interrupt
	
	NVIC_EnableIRQ(TIM2_IRQn);	//setup interrupt handler
	
}

/**
 * Restarts the counter on the timer
 */
void RestartTimer(void){
	
	TIM2->CR1 &= ~(0x1UL);		//disable TIM2
	
	TIM2->CR1 |= 0x1UL;			//re-enable TIM2
	
}

/**
 * Stops the counter on the timer
 */
void StopTimer(void){
	
	TIM2->CR1 &= ~(0x1UL);		//disable TIM2
	
}

/**
 * Checks the timer status
 */
int8_t CheckTimerStatus(void){
	return timeoutFlag;
}

/**
 * Set the timer status
 */
void SetTimerStatus(int8_t timerStatus){
	timeoutFlag = timerStatus;
}

