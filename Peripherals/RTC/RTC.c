#include "RTC.h"

/** Initializes the RTC Clock with the LSE Oscillator on the Nucleo-64 board,
		using a prescalar value defined as a constant RTC_PRESCALAR.
**/
void RTCinit(void)
{
	// Enable the Clocks
	
	RCC->APB1ENR |= 0x1 << 27;		// Enable Backup Interface Clock
	RCC->APB1ENR |= 0x1 << 28;		// Enable Power Interface Clock
	
	//Set LSE Oscillator from Nucleo-64 Board to be RTC Clock
	
	PWR->CR |= 0x1 << 8; 					// Disable Write Protection to Backup Domain
	RCC->BDCR |= 0x1 << 16;				// Resets the entire Backup Domain as a precaution
	RCC->BDCR &= ~(0x1 << 16);		// Clear the bit from the Backup Domain Reset
	RCC->BDCR |= 0x1;							// Enable the LSE Clock (32.768 Khz)
	
	while (((RCC->BDCR >> 1) & 0x1) != 0x1); 		//While the LSE Clock is not stable yet,
	
	RCC->BDCR |= 0x1 << 8; 				// Set LSE Clock as RTC Clock
	RCC->BDCR |= 0x11 << 15; 			// Enable RTC Clock
	
	//Configure RTC Clock
	
	RTC->CRL |= 0x1 << 4; 							// Enter RTC Configuration Mode
	
	RTC->PRLH = RTC_PRESCALAR >> 16;		//Set Prescalar value to RTC_PRESCALAR
	RTC->PRLL = 0xFFFF & RTC_PRESCALAR;
	
	RTC->CRL &= ~(0x1 << 4);						// Exit RTC Configuration Mode
	
}