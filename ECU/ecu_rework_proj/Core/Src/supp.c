//-------------------------------------------
//File: supp.c
//Brief: supply voltage filtering and calculation
//Author: Yuqi Fu
//-------------------------------------------

//-------------------------------------------
// Copyright 2021 UBC Solar, all rights reserved
//-------------------------------------------

//-------------------------------------------
// Standard Lib Include Files
//-------------------------------------------
#include <stdint.h>
//-------------------------------------------
// Other Lib Include Files
//-------------------------------------------

//-------------------------------------------
// Constant Definitions & Macros
//-------------------------------------------
#define SUPP_RAW_READING_BUFFER_LENGTH       (8u)
#define SUPP_GET_NEXT_IND(x)				 (( (x) + 1u) % SUPP_RAW_READING_BUFFER_LENGTH)
#define SUPP_ADC_MAX_VAL					 ((1u << 12u) - 1u)
#define SUPP_FLOAT_TO_UINT_ROUND(x)			 ((uint16_t)( (x) + 0.5f ))
#define SUPP_BUFFER_FIRST_INDEX				 (0u)
#define SUPP_BUFFER_LAST_INDEX				 (SUPP_RAW_READING_BUFFER_LENGTH - 1u)
#define SUPP_READING_CLAMP(x)				 (((( SUPP_ADC_MAX_VAL - (x) ) >> 15u) & SUPP_ADC_MAX_VAL ) | (x))
//-------------------------------------------
// Local Variables
//-------------------------------------------

//
//FIR filter related variables
//
static volatile uint16_t supp_aAdcVal[SUPP_RAW_READING_BUFFER_LENGTH] = {0u};
static volatile uint8_t  supp_bufferIndex = 0u;
//static volatile uint8_t  supp_bufferSumIndex = 0u;
//static volatile float    supp_fFilteredOut = 0.0f;   //filtered output in float
static volatile uint16_t supp_filteredOut = 0u;		 //filtered output converted to uint

//
//FIR filter taps, obtained from http://t-filter.engineerjs.com/
//Inputs:
//Sampling frequency = 100Hz
//pass band 0-20Hz
//stop band 30-50Hz
//Desired pass band ripple = 5dB
//Desired stop band ripple = -40dB
//Obtained actual pass band ripple = 2.1dB
//Obtained actual stop band ripple = -45.78dB
//
/*static const float supp_filterTaps[SUPP_RAW_READING_BUFFER_LENGTH] =
{		0.08892327054865834,
		0.2866893107218228,
		0.36497209245498763,
		0.2866893107218228,
		0.08892327054865834,
};*/
//-------------------------------------------
// Global Variables
//-------------------------------------------

//-------------------------------------------
// Local Function Prototypes
//-------------------------------------------

//-------------------------------------------
// Local Function Implementations
//-------------------------------------------


//-------------------------------------------
// Global Function Implementations
//-------------------------------------------

//
// Shoves an ADC reading into the filter circular buffer and update filter output
//
void SUPP_UpdateAdcFilter(uint16_t adcVal)
{
	uint8_t index;
	uint16_t supp_bufferSum = 0u;

	//update circular buffer
	supp_aAdcVal[supp_bufferIndex] = adcVal;
	supp_bufferIndex = SUPP_GET_NEXT_IND(supp_bufferIndex);

	for(index = SUPP_BUFFER_FIRST_INDEX; index < SUPP_RAW_READING_BUFFER_LENGTH; index++)
	{
		supp_bufferSum += supp_aAdcVal[index];
	}

	supp_filteredOut = supp_bufferSum >> 3u;

	//supp_bufferSumIndex = supp_bufferIndex;
	//supp_fFilteredOut = 0.0f;
	//digital convolution of buffer values with filter impulse response
	//for(index = SUPP_BUFFER_FIRST_INDEX; index < SUPP_RAW_READING_BUFFER_LENGTH; index++)
	//{
	//	if(0u < supp_bufferSumIndex)
	//	{
	//		supp_bufferSumIndex--;
	//	}
	//	else
	//	{
	//		supp_bufferSumIndex = SUPP_BUFFER_LAST_INDEX;
	//	}
	//	supp_fFilteredOut += supp_filterTaps[index] * supp_aAdcVal[supp_bufferSumIndex];
	//}


}

uint16_t SUPP_GetAdcVal(void)
{
	return supp_filteredOut;
}
