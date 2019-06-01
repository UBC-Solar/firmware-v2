<<<<<<< HEAD
#include "stm32f10x.h"
#include "ADC.h"
#include "encoder.h"
#include "CAN.h"


int main(void){
	CANInit();
	EncoderInit();
	ADCInit();
	
	
	RCC->APB2ENR = 0x1 << 2;  //
	GPIOA->CRL = 0x8 << 8;   //pinMode(A2, INPUT PULL_UP)
	
	
	
	
	uint16_t encoder_reading;
	uint16_t ADC_reading;
	uint16_t old_ADC_reading = ReadADC();
	uint16_t old_encoder_reading = EncoderRead();
	
	while(1) 
	{
		encoder_reading = EncoderRead();
		ADC_reading = ReadADC();
		
	if( (((GPIOA->IDR) >> 2) & 0x1) && (old_ADC_reading != ADC_reading) )
	{
		CAN_msg_t.id = 
		
		for(int x = 0; x < 8; x++){
			CAN_msg_t.data[x] = 
		}
		CAN_msg_t.len = 8;
		
		reset_timer();
		
			
	}
	
	else if(old_encoder_reading != encoder_reading)
	{
		
		CAN_msg_t.id = 
		
		for(int x = 0; x < 8; x++){
			CAN_msg_t.data[x] = 
		}
		CAN_msg_t.len = 8;
		
		reset_timer();
		
	}
	
	else if(time_out())
	{
		ADC_reading = ReadADC();
		
		if(ADC_reading > 0)
		{
			CAN_msg_t.id = 
		
			for(int x = 0; x < 8; x++){
				CAN_msg_t.data[x] = 
			}
			CAN_msg_t.len = 8;
		
			reset_timer();
			
		}
		
		else
		{
			CAN_msg_t.id = 
		
			for(int x = 0; x < 8; x++){
				CAN_msg_t.data[x] = 
			}
			CAN_msg_t.len = 8
		
			reset_timer();
			
		}
		
		
	}
	
	
		
	
		old_ADC_reading = ADC_reading;
		old_encoder_reading = EncoderRead();
	}

	
}




=======
#include "stm32f10x.h"

#include "ADC/ADC.h"
#include "Encoder/encoder.h"
#include "CAN.h"
#include "virtual_com.h"
#include "Timer/Timer.h"

int main(void) {
	
}
>>>>>>> 47dd0cef6a73faa2d37f88596b873e714727ef48
