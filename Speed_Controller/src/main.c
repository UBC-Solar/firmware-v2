#include "stm32f10x.h"

#include "ADC/ADC.h"
#include "Encoder/encoder.h"
#include "CAN.h"
#include "virtual_com.h"
#include "Timer/Timer.h"

#define DRIVE_CONTROL_ID 0x500

union {
	float float_var;
	uint8_t chars[4];
} u;

int main(void){
	CANInit();
	EncoderInit();
	ADCInit();
	TimerInit(100);
	
	CAN_msg_t CAN_drive;   
	CAN_msg_t CAN_regen;   
	
	
	CAN_drive.len = 8;
	CAN_regen.len = 8;
	
	CAN_drive.id = DRIVE_CONTROL_ID + 2;
	CAN_regen.id = DRIVE_CONTROL_ID + 2;
	
	u.float_var = 100.0;
	
	CAN_drive.data[0] = u.chars[0];
	CAN_drive.data[1] = u.chars[1];
	CAN_drive.data[2] = u.chars[2];
	CAN_drive.data[3] = u.chars[3];

	
	CAN_regen.data[0] = 0;
	CAN_regen.data[1] = 0;
	CAN_regen.data[2] = 0;
	CAN_regen.data[3] = 0;
		
	RCC->APB2ENR = 0x1 << 2;  //
	GPIOA->CRL = 0x8 << 8;   //pinMode(A2, INPUT PULL_UP)
	
	
	uint16_t encoder_reading;
	uint16_t ADC_reading;
	uint16_t old_ADC_reading = 0;
	uint16_t old_encoder_reading = 0;
	
	while(1) 
	{
		
		encoder_reading = EncoderRead();
		ADC_reading = ReadADC();
	
		//If regen is enabled AND ADC count changed, send new regen CAN message and restart timer
		if( (((GPIOA->IDR) >> 2) & 0x1) && (old_ADC_reading != ADC_reading) )
		{
			
			StopTimer();
			
			u.float_var = (float)(ADC_reading/ADC_MAX);
			CAN_regen.data[4] = u.chars[0];
			CAN_regen.data[5] = u.chars[1];
			CAN_regen.data[6] = u.chars[2];
			CAN_regen.data[7] = u.chars[3];
			
			CANSend(&CAN_regen);  //send CAN message
			
			RestartTimer();   //restart the timer
		}
		
		//if encoder count changed, send new drive CAN message and restart timer
		else if(old_encoder_reading != encoder_reading)
		{
			
			StopTimer();
			
			u.float_var = (float)(ADC_reading/ADC_MAX);
			CAN_drive.data[4] = u.chars[0];
			CAN_drive.data[5] = u.chars[1];
			CAN_drive.data[6] = u.chars[2];
			CAN_drive.data[7] = u.chars[3];
			
			CANSend(&CAN_drive);
			
			RestartTimer();  //restart the timer
		}
		//if timeout occurred
		else if(CheckTimerStatus())
		{
			if(ADC_reading > 0)
			{
				CANSend(&CAN_regen);
				RestartTimer();	
			}
			else
			{
				CANSend(&CAN_drive);
				RestartTimer();
			}
			
			SetTimerStatus(FALSE);
		}
		old_ADC_reading = ADC_reading;
		old_encoder_reading = encoder_reading;
	}
}
