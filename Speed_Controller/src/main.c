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
	TimerInit(1000);

	CAN_msg_t CAN_drive;   
	CAN_msg_t CAN_regen;   
	
	CAN_drive.len = 8;
	CAN_regen.len = 8;
	
	CAN_drive.id = DRIVE_CONTROL_ID + 1;
	CAN_regen.id = DRIVE_CONTROL_ID + 1;
	
	u.float_var = 100.0;
	
	//set velocity to 100 for drive messages
	CAN_drive.data[0] = u.chars[0];
	CAN_drive.data[1] = u.chars[1];
	CAN_drive.data[2] = u.chars[2];
	CAN_drive.data[3] = u.chars[3];

	//set velocity to 0 for regen messages
	CAN_regen.data[0] = 0;
	CAN_regen.data[1] = 0;
	CAN_regen.data[2] = 0;
	CAN_regen.data[3] = 0;
	
	//set initial 
	CAN_drive.data[4] = 0;
	CAN_drive.data[5] = 0;
	CAN_drive.data[6] = 0;
	CAN_drive.data[7] = 0;
	
	CAN_regen.data[4] = 0;
	CAN_regen.data[5] = 0;
	CAN_regen.data[6] = 0;
	CAN_regen.data[7] = 0;
		
	//RCC->APB2ENR |= 0x1 << 2;  //
	//GPIOA->CRL |= 0x8 << 8;   //pinMode(A2, INPUT PULL_UP)
	
	volatile uint16_t encoder_reading;
	volatile uint16_t ADC_reading;
	uint16_t old_ADC_reading = 0;
	uint16_t old_encoder_reading = 0;
	
	while(1) 
	{
		
		encoder_reading = EncoderRead();
		ADC_reading = ReadADC();
		
		//If regen is enabled AND ADC count changed, send new regen CAN message and restart timer
		//if( (((GPIOA->IDR) >> 2) & 0x1) && (old_ADC_reading != ADC_reading) )
		if( ((old_ADC_reading - ADC_reading) > 0xF) || ((ADC_reading - old_ADC_reading) > 0xF))
		{
			StopTimer();
			
			u.float_var = (float)((float) ADC_reading/ADC_MAX);
			CAN_regen.data[4] = u.chars[0];
			CAN_regen.data[5] = u.chars[1];
			CAN_regen.data[6] = u.chars[2];
			CAN_regen.data[7] = u.chars[3];
			
			CANSend(&CAN_regen);
			
			RestartTimer();
		}		
		//if encoder count changed, send new drive CAN message and restart timer
		else if(old_encoder_reading != encoder_reading)
		{
			
			u.float_var = (float)( (float) encoder_reading/PEDAL_MAX);
			
			//Any overflow should be treated as zero current to the motor
			if (u.float_var > 1.0 || u.float_var < 0.0)
			{
				u.float_var = 0.0;
			}
			
			CAN_drive.data[4] = u.chars[0];
			CAN_drive.data[5] = u.chars[1];
			CAN_drive.data[6] = u.chars[2];
			CAN_drive.data[7] = u.chars[3];
			
			CANSend(&CAN_drive);
		
			RestartTimer();
		
		}		
		//If a timeout occured, send the previously sent CAN drive message
		else if(timeoutFlag == TRUE)
		{	
			
			//If the driver is holding regen at a positive value, send the previous regen message
			if(ADC_reading > 0)
			{	
				CANSend(&CAN_regen);
			}
			//If the driver is not braking, send the previous CAN drive message.
			else
			{	
				CANSend(&CAN_drive);
			}
			
			timeoutFlag = FALSE;
			
			RestartTimer();
		}
		//otherwise, nothing is to be done - nothing has changed and nothing is due
		
		old_ADC_reading = ADC_reading;
		old_encoder_reading = encoder_reading;
	
	}
}
