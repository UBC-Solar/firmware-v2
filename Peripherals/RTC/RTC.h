#include "stm32f10x.h"

#define RTC_PRESCALAR 2184 

/** Initializes the RTC Clock with the LSE Oscillator on the Nucleo-64 board,
		using a prescalar value defined as a constant RTC_PRESCALAR.
**/
void RTCinit(void);