#include "RTC.h"

/**
 * Initializes the RTC Clock with the LSE Oscillator on the Nucleo-64 board,
 * using a prescalar value defined as a constant RTC_PRESCALAR.
 */
void RTCinit(void)
{
	// Enable the Clocks
	RCC->APB1ENR |= RCC_APB1ENR_BKPEN; // Enable Backup Interface Clock
	RCC->APB1ENR |= RCC_APB1ENR_PWREN; // Enable Power Interface Clock

	// Set LSE Oscillator from Nucleo-64 Board to be RTC Clock

	PWR->CR |= PWR_CR_DBP;		  // Disable Write Protection to Backup Domain
	RCC->BDCR |= RCC_BDCR_BDRST;  // Resets the entire Backup Domain as a precaution
	RCC->BDCR &= ~RCC_BDCR_BDRST; // Clear the bit from the Backup Domain Reset
	RCC->BDCR |= RCC_BDCR_LSEON;  // Enable the LSE Clock (32.768 Khz)

	while (!(RCC->BDCR & RCC_BDCR_LSERDY)); // While the LSE Clock is not stable yet,

	RCC->BDCR |= RCC_BDCR_RTCSEL_LSE; // Set LSE Clock as RTC Clock
	RCC->BDCR |= RCC_BDCR_RTCEN;	  // Enable RTC Clock

	// Configure RTC

	RTC->CRL |= RTC_CRL_CNF; // Enter RTC Configuration Mode

	RTC->PRLH = RTC_PRESCALAR >> 16; // Set Prescalar value to RTC_PRESCALAR
	RTC->PRLL = 0xFFFF & RTC_PRESCALAR;

	RTC->CRL &= ~RTC_CRL_CNF; // Exit RTC Configuration Mode
}
